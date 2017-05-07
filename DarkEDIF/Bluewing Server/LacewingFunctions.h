// Handles all Lacewing functions.
#include "Lacewing.h"

void OnClientConnectRequest(
	lacewing::relayserver &server, lacewing::relayserver::client &client);
void OnClientDisconnect(lacewing::relayserver &Server, lacewing::relayserver::client &Client);
void OnError(lacewing::relayserver &Server, lacewing::error Error);
void OnServerMessage(lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant);
void OnChannelMessage(
	lacewing::relayserver &server, lacewing::relayserver::client &Client,
	lacewing::relayserver::channel &Channel,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant);
void OnPeerMessage(	
	lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	lacewing::relayserver::channel &Channel, lacewing::relayserver::client &TargetClient,
	bool Blasted, int Subchannel, const char * Packet, size_t Size, int Variant);
void OnJoinChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	lacewing::relayserver::channel &Channel);
void OnLeaveChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	lacewing::relayserver::channel &Channel);
void OnNameSet(lacewing::relayserver &Server, lacewing::relayserver::client &Client, const char * NewName);