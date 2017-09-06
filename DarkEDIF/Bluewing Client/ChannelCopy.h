#pragma once

#include "Common.h"
#include <vector>
#include "Lacewing.h"

struct PeerCopy;
struct ChannelCopy
{
	friend PeerCopy;
	unsigned short id() const;
	const char * name() const;
	PeerCopy * channelmaster(bool * isme = nullptr) const;

	void close();

	void send(int subchannel, const char * data, size_t size = MAXSIZE_T, int variant = 0);
	void blast(int subchannel, const char * data, size_t size = MAXSIZE_T, int variant = 0);

	const std::vector<PeerCopy *> & getpeers();
	size_t peercount() const;

	PeerCopy * addpeer(lacewing::relayclient::channel::peer * peer);

	PeerCopy * updatepeername(lacewing::relayclient::channel::peer &Peer);

	/// <summary> Sets peer to read-only. </summary>
	PeerCopy * closepeer(lacewing::relayclient::channel::peer& peer);

	/// <summary> Deletes a peer that was set to read-only with closepeer(). </summary>
	void deletepeer(PeerCopy * peer);



	ChannelCopy(lacewing::relayclient::channel *);
	~ChannelCopy() noexcept(false);
	bool isclosed;
	void setlocaldata(const char * Key, const char * Value);
	const std::string & getlocaldata(const char * Key) const;

	lacewing::relayclient::channel & orig();
protected:
	lacewing::relayclient::channel * channel;
	std::vector<PeerCopy *> _peers;
	std::vector<std::pair<std::string, std::string>> localdata;
	PeerCopy * _channelmaster;
	bool youaremaster;
	unsigned short _id;
	const char * _name;
};