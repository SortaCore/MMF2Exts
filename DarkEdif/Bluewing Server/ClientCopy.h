#pragma once
#include "Common.h"
#include <chrono>
#include <vector>

struct ChannelCopy;
struct ClientCopy
{
	friend struct ChannelCopy;
	unsigned short id() const;

	const char * getaddress() const;
	const char * getimplementation();

	void close();
	void disconnect(bool serverInited);

	void send(int subchannel, const char * data, size_t size = MAXSIZE_T, int variant = 0);
	void blast(int subchannel, const char * data, size_t size = MAXSIZE_T, int variant = 0);

	const char * name();
	void name(const char * newName);

	size_t channelcount() const;
	__int64 connecttime() const;

	const std::vector<ChannelCopy *> & getchannels();

	ClientCopy(lacewing::relayserver::client *);
	~ClientCopy() noexcept(false);
	bool isclosed;
	void SetLocalData(const char * Key, const char * Value);
	const std::string & GetLocalData(const char * Key) const;

	lacewing::relayserver::client & orig();

protected:
	lacewing::relayserver::client * client;
	std::vector<ChannelCopy *> _channels;
	std::vector<std::pair<std::string, std::string>> localdata;
	unsigned short _id;
	const char * _address;
	const char * _name;
	const char * _implementation;
	in6_addr _addressint;

	std::chrono::time_point<std::chrono::high_resolution_clock> _connecttime;
};