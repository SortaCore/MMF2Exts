#include "ClientCopy.h"
#include <assert.h>
#include <chrono>

unsigned short ClientCopy::id() const
{
	return _id;
}

const char * ClientCopy::getaddress() const
{
	return _address;
}
const char * ClientCopy::getimplementation()
{
	// There's no event for updating implemtation.
	if (!isclosed)
	{
		free((char *)_implementation);
		_implementation = _strdup(client->getimplementation());
	}
	return _implementation;
}

void ClientCopy::close()
{
	if (isclosed)
		return;
	isclosed = true;
	client->disconnect();
}

/// <summary> Disconnects the client, or reflects it was disconnected. </summary>
/// <param name="fusionServerInited"> If true, disconnect will be run. If false, it'll
/// 								  just be reflected and the copy will be closed.
/// 								  Pass true when Fusion is closing the connect, not liblacewing. </param>
void ClientCopy::disconnect(bool fusionServerInited)
{
	if (isclosed)
		return;

	isclosed = true;
	
	// If nameset events are approved quietly, name in copy won't be set.
	// Disconnect is the last time to read the name.
	name(client->name());

	if (fusionServerInited)
		client->disconnect();
}

const char * ClientCopy::name()
{
	// If nameset events are approved quietly, name in copy won't be set.
	// So update name whenever it's referred to (and client is not closed)
	if (!isclosed)
		this->name(client->name());
	return _name;
}
void ClientCopy::name(const char * newName)
{
	if (isclosed)
		return;

	// Does not send name change message. Use nameset_response for that.
	client->name(newName);
	free((char *)_name);
	_name = _strdup(newName);
}

size_t ClientCopy::channelcount() const
{
	return _channels.size();
}

using namespace std::chrono;

__int64 ClientCopy::connecttime() const
{
	high_resolution_clock::time_point end = high_resolution_clock::now();
	nanoseconds time = end - _connecttime;
	return duration_cast<seconds>(time).count();
}

const std::vector<ChannelCopy *> & ClientCopy::getchannels()
{
	return _channels;
}


void ClientCopy::SetLocalData(const char * Key, const char * Value)
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

const std::string & ClientCopy::GetLocalData(const char * Key) const
{
	static std::string blanky = "";
	// NB: Nulls checked for by caller

	auto i = std::find_if(localdata.cbegin(), localdata.cend(),
		[&](const pair<string, string> & s) {
		return !_stricmp(s.first.c_str(), Key); });
	return i == localdata.cend() ? blanky : i->second;
}

lacewing::relayserver::client & ClientCopy::orig()
{
	return *client;
}

void ClientCopy::send(int subchannel, const char * data, size_t size /* = MAXSIZE_T */, int variant /* = 0 */)
{
	return client->send(subchannel, data, size, variant);
}

void ClientCopy::blast(int subchannel, const char * data, size_t size /* = MAXSIZE_T */, int variant /* = 0 */)
{
	return client->blast(subchannel, data, size, variant);
}



ClientCopy::ClientCopy(lacewing::relayserver::client * cli)
{
	isclosed = false;
	client = cli;

	char * addr = (char *)malloc(64);
	lacewing::lw_addr_prettystring(cli->getaddress(), addr, 64U);

	this->_address = addr;
	this->_id = cli->id();
	this->_implementation = _strdup(cli->getimplementation());
	this->_connecttime = std::chrono::high_resolution_clock::now();
	
	// Name unset at this point, pre-connect request approved, so obv. pre-name set
	this->_name = nullptr;
}
ClientCopy::~ClientCopy() noexcept(false)
{
	assert(isclosed);

	free((char *)_name);
	_name = nullptr;

	free((char *)_address);
	_address = nullptr;

	free((char *)_implementation);
	_implementation = nullptr;

	_id = 0xFFFF;
}