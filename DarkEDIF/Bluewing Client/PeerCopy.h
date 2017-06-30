#pragma once

#include "Common.h"
#include "Lacewing.h"
#include <vector>

struct PeerCopy
{
	friend struct ChannelCopy;
	unsigned short id() const;

	void close();

	void send(int subchannel, const char * data, size_t size = MAXSIZE_T, int variant = 0);
	void blast(int subchannel, const char * data, size_t size = MAXSIZE_T, int variant = 0);

	const char * name() const;
	void name(const char * newName);
	const char * prevname() const;

	PeerCopy(lacewing::relayclient::channel *, lacewing::relayclient::channel::peer *);
	~PeerCopy() noexcept(false);
	bool isclosed;
	
	void setlocaldata(const char * Key, const char * Value);
	const std::string & getlocaldata(const char * Key) const;

	lacewing::relayclient::channel::peer & orig();

protected:
	lacewing::relayclient::channel:: peer * peer;
	lacewing::relayclient::channel * channel;
	std::vector<std::pair<std::string, std::string>> localdata;
	unsigned short _id;
	const char * _name;
	const char * _prevname;
};