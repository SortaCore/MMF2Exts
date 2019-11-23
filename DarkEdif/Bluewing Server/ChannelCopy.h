#pragma once
#include "Common.h"
#include <vector>

struct ClientCopy;
struct ChannelCopy
{
	unsigned short id() const;

	ClientCopy * channelmaster() const;

	const char * name() const;
	void name(const char *);

	bool hidden() const;
	bool autocloseenabled() const;

	void close();

	void send(int subchannel, const char * data, size_t size = MAXSIZE_T, int variant = 0);
	void blast(int subchannel, const char * data, size_t size = MAXSIZE_T, int variant = 0);

	const std::vector<ClientCopy *> & getclients();
	size_t clientcount() const;

	void removeclient(ClientCopy * copy, bool serverInited);
	void addclient(ClientCopy * copy, bool serverInited);

	ChannelCopy(lacewing::relayserver::channel *);
	~ChannelCopy() noexcept(false);
	bool isclosed;
	void SetLocalData(const char * Key, const char * Value);
	const std::string & GetLocalData(const char * Key) const;

	/// <summary> Sets channel master. Expects client to already be in internal client list.
	/// 		  Use joinchannel() if it is not. </summary>
	void setchannelmaster(ClientCopy * client);

	bool newlycreated() const;

	lacewing::relayserver::channel & orig();
protected:
	lacewing::relayserver::channel * channel;
	std::vector<ClientCopy *> _clients;
	std::vector<std::pair<std::string, std::string>> localdata;
	ClientCopy * _channelmaster;
	unsigned short _id;
	const char * _name;
	bool _autoclose;
	bool _hidden;
	bool _newlycreated;
};