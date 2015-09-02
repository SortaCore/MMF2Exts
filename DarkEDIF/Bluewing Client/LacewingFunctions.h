// Handles all Lacewing functions.
#include "Lacewing.h"

void OnChannelListReceived(lacewing::relayclient &client);
void OnChannelMessage(lacewing::relayclient &client, lacewing::relayclient::channel &channel, lacewing::relayclient::channel::peer &peer,
	bool blasted, int subchannel, const char * data, size_t size, int variant);
void OnConnect(lacewing::relayclient &client);
void OnConnectDenied(lacewing::relayclient &client, const char * denyreason);
void OnDisconnect(lacewing::relayclient &client);
void OnError(lacewing::relayclient &client, lacewing::error error);
void OnJoinChannel(lacewing::relayclient &client, lacewing::relayclient::channel &target);
void OnJoinChannelDenied(lacewing::relayclient &client, const char * channelname, const char * denyreason);
void OnLeaveChannel(lacewing::relayclient &client, lacewing::relayclient::channel &target);
void OnLeaveChannelDenied(lacewing::relayclient &client, lacewing::relayclient::channel &target,const char * denyreason);
void OnNameChanged(lacewing::relayclient &client, const char * oldname);
void OnNameDenied(lacewing::relayclient &client, const char * deniedname, const char * denyreason);
void OnNameSet(lacewing::relayclient &client);
void OnPeerConnect(lacewing::relayclient &client, lacewing::relayclient::channel &channel,lacewing::relayclient::channel::peer &peer);
void OnPeerDisconnect(lacewing::relayclient &client, lacewing::relayclient::channel &channel,lacewing::relayclient::channel::peer &peer);
void OnPeerMessage(lacewing::relayclient &client, lacewing::relayclient::channel &channel, lacewing::relayclient::channel::peer &peer,
	bool blasted, int subchannel, const char * data, size_t size, int variant);
void OnPeerNameChanged(lacewing::relayclient &client, lacewing::relayclient::channel &channel, lacewing::relayclient::channel::peer &peer, const char * oldname);
void OnServerChannelMessage(lacewing::relayclient &client, lacewing::relayclient::channel &channel,
	bool blasted, int subchannel, const char * data, size_t size, int variant);
void OnServerMessage(lacewing::relayclient &client,
	bool blasted, int subchannel, const char * data, size_t size, int variant);
