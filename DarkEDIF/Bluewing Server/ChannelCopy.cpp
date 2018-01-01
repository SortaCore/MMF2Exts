#include "ChannelCopy.h"
#include <assert.h>

unsigned short ChannelCopy::id() const
{
	return _id;
}

ClientCopy * ChannelCopy::channelmaster() const
{
	return _channelmaster;
}

const char * ChannelCopy::name() const
{
	return _name;
}
void ChannelCopy::name(const char * newName)
{
	if (isclosed)
		return;
	channel->name(newName);
	free((char *)_name);
	_name = _strdup(newName);
}

bool ChannelCopy::hidden() const
{
	return _hidden;
}
bool ChannelCopy::autocloseenabled() const
{
	return _autoclose;
}

/// <summary> Closes this channel. Unlike the relay one, data is then retained for readonly
/// 		  until dtor is called. </summary>
void ChannelCopy::close()
{
	if (isclosed)
		return;
	isclosed = true;

	channel->close();
}

void ChannelCopy::send(int subchannel, const char * data, size_t size /* = MAXSIZE_T */, int variant /* = 0 */)
{
	return channel->send(subchannel, data, size, variant);
}

void ChannelCopy::blast(int subchannel, const char * data, size_t size /* = MAXSIZE_T */, int variant /* = 0 */)
{
	return channel->blast(subchannel, data, size, variant);
}


const std::vector<ClientCopy *> & ChannelCopy::getclients()
{
	return _clients;
}
size_t ChannelCopy::clientcount() const
{
	return _clients.size();
}
ChannelCopy::ChannelCopy(lacewing::relayserver::channel * chan)
{
	isclosed = false;
	channel = chan;
	_channelmaster = nullptr; // setchannelmaster should be called after creating
	_id = chan->id();
	_name = _strdup(chan->name());
	_autoclose = chan->autocloseenabled();
	_hidden = chan->hidden();
	_newlycreated = true;
}
void ChannelCopy::setchannelmaster(ClientCopy * client)
{
	// We can only set to null or null to specific
	assert((!!_channelmaster) != (!!client));

	// Client must exist in channel list already
	assert(!client || std::find(_clients.cbegin(), _clients.cend(), client) != _clients.cend());

	_channelmaster = client;
	_newlycreated = false;
}

bool ChannelCopy::newlycreated() const
{
	return _newlycreated;
}
ChannelCopy::~ChannelCopy() noexcept(false)
{
	assert(isclosed);

	free((char *)_name);
	_name = nullptr;
}
void ChannelCopy::SetLocalData(const char * key, const char * value)
{
	// NB: Nulls checked for by caller

	size_t keyLen = strlen(key);
	auto i = std::find_if(localdata.begin(), localdata.end(),
		[&](const std::pair<std::string, std::string> & s) {
		return !_strnicmp(s.first.c_str(), key, keyLen);
	});

	// Blank value: Delete
	if (value[0] == '\0')
	{
		if (i != localdata.end())
			localdata.erase(i);
		return;
	}
	if (i != localdata.end())
		i->second = value;
	else
		localdata.push_back(std::make_pair(key, value));
}
const std::string & ChannelCopy::GetLocalData(const char * key) const
{
	static std::string blanky = "";
	// NB: Nulls checked for by caller

	size_t keyLen = strlen(key);
	auto i = std::find_if(localdata.cbegin(), localdata.cend(),
		[&](const std::pair<std::string, std::string> & s) {
		return !_strnicmp(s.first.c_str(), key, keyLen); });
	return i == localdata.cend() ? blanky : i->second;
}

lacewing::relayserver::channel & ChannelCopy::orig()
{
	return *channel;
}

void ChannelCopy::addclient(ClientCopy * copy, bool serverInited)
{
	_clients.push_back(copy);

	// If serverInited, the core relay server has not been notified.
	// If non-server inited, it's a reaction to client, so the orig already has it from joinchannel_response
	if (serverInited)
		channel->addclient(copy->orig());
}
void ChannelCopy::removeclient(ClientCopy * copy, bool serverInited)
{
	// if serverInited, the core relay server has not been notified.
	// if not serverInited, the core relay server has already informed all clients and removed from lists.
	// In either case, we need to reflect the leave here.

	if (serverInited)
		channel->removeclient(*copy->client);

	// channel has now closed server-side. Reflect it here.
	if ((_channelmaster == copy && _autoclose) ||
		(_clients.empty() && _hidden))
	{
		isclosed = true;
	}

	// Cleanup Fusion copies: drop client from channel's client list
	auto cli = std::find(_clients.begin(), _clients.end(), copy);
	if (cli != _clients.end())
		_clients.erase(cli);

	// Drop channel from client's joined channel list
	auto ch = std::find(copy->_channels.begin(), copy->_channels.end(), this);
	if (ch != copy->_channels.end())
		copy->_channels.erase(ch);
}
