#include "PeerCopy.h"
#include <assert.h>

unsigned short PeerCopy::id() const
{
	return _id;
}

void PeerCopy::close()
{
	isclosed = true;
}

const char * PeerCopy::name() const
{
	return _name;
}
void PeerCopy::name(const char * newName)
{
	free((char *)_prevname);
	_prevname = _name;
	_name = _strdup(newName);
}
const char * PeerCopy::prevname() const
{
	return _prevname;
}

void PeerCopy::setlocaldata(const char * Key, const char * Value)
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

const std::string & PeerCopy::getlocaldata(const char * Key) const
{
	static std::string blanky = "";
	// NB: Nulls checked for by caller

	auto i = std::find_if(localdata.cbegin(), localdata.cend(),
		[&](const pair<string, string> & s) {
		return !_stricmp(s.first.c_str(), Key); });
	return i == localdata.cend() ? blanky : i->second;
}

lacewing::relayclient::channel::peer & PeerCopy::orig()
{
	return *peer;
}

void PeerCopy::send(int subchannel, const char * data, size_t size /* = MAXSIZE_T */, int variant /* = 0 */)
{
	return peer->send(subchannel, data, size, variant);
}

void PeerCopy::blast(int subchannel, const char * data, size_t size /* = MAXSIZE_T */, int variant /* = 0 */)
{
	return peer->blast(subchannel, data, size, variant);
}

PeerCopy::PeerCopy(lacewing::relayclient::channel * chan, lacewing::relayclient::channel::peer * per)
{
	isclosed = false;
	peer = per;
	channel = chan;

	this->_id = per->id();
	this->_name = _strdup(per->name());
	this->_prevname = nullptr;
}
PeerCopy::~PeerCopy() noexcept(false)
{
	assert(isclosed);
	peer = nullptr;
	channel = nullptr;

	free((char *)_name);
	_name = nullptr;

	free((char *)_prevname);
	_prevname = nullptr;

	_id = 0xFFFF;
}