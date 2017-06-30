#include "ChannelCopy.h"
#include "PeerCopy.h"
#include <vector>
#include <assert.h>

unsigned short ChannelCopy::id() const
{
	return _id;
}

PeerCopy * ChannelCopy::channelmaster(bool * isme) const
{
	if (isme)
		*isme = youaremaster;
	return _channelmaster;
}

const char * ChannelCopy::name() const
{
	return _name;
}

/// <summary> Run this when you get a leave channel message (NOT when you WANT to leave). Closes all peers too. </summary>
void ChannelCopy::close()
{
	if (isclosed)
		return;

	isclosed = true;
	for (PeerCopy * p : _peers)
		p->close();
}

void ChannelCopy::send(int subchannel, const char * data, size_t size /* = MAXSIZE_T */, int variant /* = 0 */)
{
	return channel->send(subchannel, data, size, variant);
}

void ChannelCopy::blast(int subchannel, const char * data, size_t size /* = MAXSIZE_T */, int variant /* = 0 */)
{
	return channel->blast(subchannel, data, size, variant);
}

const std::vector<PeerCopy *> & ChannelCopy::getpeers()
{
	return _peers;
}
size_t ChannelCopy::peercount() const
{
	return _peers.size();
}

/// <summary> Sets peer to read-only. </summary>
void ChannelCopy::closepeer(lacewing::relayclient::channel::peer & peer)
{
	auto pr = std::find_if(_peers.begin(), _peers.end(), [&](PeerCopy *& p) { return p->peer == &peer; });
	assert(pr != _peers.end());
	(**pr).isclosed = true;
}

void ChannelCopy::deletepeer(PeerCopy * peer)
{
	auto pr = std::find(_peers.begin(), _peers.end(), peer);
	assert(pr != _peers.end());
	PeerCopy * pr2 = *pr;
	_peers.erase(pr);
	delete pr2;
}
PeerCopy * ChannelCopy::addpeer(lacewing::relayclient::channel::peer * peer)
{
	PeerCopy * pc = new PeerCopy(channel, peer);
	_peers.push_back(pc);

	if (peer->ischannelmaster())
		_channelmaster = pc;
	return pc;
}
PeerCopy * ChannelCopy::updatepeername(lacewing::relayclient::channel::peer &Peer)
{
	auto pr = std::find_if(_peers.begin(), _peers.end(), [=](PeerCopy *&p) {
		return p->peer == &Peer;
	});
	assert(pr != _peers.end());
	PeerCopy * pr2 = *pr;
	pr2->name(Peer.name());
	return pr2;
}

ChannelCopy::ChannelCopy(lacewing::relayclient::channel * chan)
{
	isclosed = false;
	channel = chan;

	_id = chan->id();
	_name = _strdup(chan->name());

	youaremaster = chan->ischannelmaster();
	_channelmaster = nullptr;

	for (lacewing::relayclient::channel::peer * p = chan->firstpeer(); p; p = p->next())
	{
		PeerCopy * pc = new PeerCopy(chan, p);
		_peers.push_back(pc);
		if (p->ischannelmaster())
			_channelmaster = pc;
	}
}

ChannelCopy::~ChannelCopy() noexcept(false)
{
	assert(isclosed);

	free((char *)_name);
	_name = nullptr;

	for (auto &p : _peers)
		delete p;
	_peers.clear();
}
void ChannelCopy::setlocaldata(const char * Key, const char * Value)
{
	// NB: Nulls checked for by caller

	auto i = std::find_if(localdata.begin(), localdata.end(),
		[&](const pair<string, string> & s) {
		return !_stricmp(s.first.c_str(), Key); });

	// Blank value: Delete
	if (Value[0] == '\0')
	{
		if (i != localdata.end())
			localdata.erase(i);
		return;
	}
	if (i != localdata.end())
		i->second = Value;
	else
		localdata.push_back(std::make_pair(Key, Value));
}
const std::string & ChannelCopy::getlocaldata(const char * Key) const
{
	static std::string blanky = "";
	// NB: Nulls checked for by caller

	auto i = std::find_if(localdata.cbegin(), localdata.cend(),
		[&](const pair<string, string> & s) {
		return !_stricmp(s.first.c_str(), Key); });
	return i == localdata.cend() ? blanky : i->second;
}

lacewing::relayclient::channel & ChannelCopy::orig()
{
	return *channel;
}