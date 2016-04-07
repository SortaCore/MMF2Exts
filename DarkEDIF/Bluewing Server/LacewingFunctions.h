// Handles all Lacewing functions.
#include "Lacewing.h"

void OnClientConnectRequest(lacewing::relayserver &Server, lacewing::relayserver &Client, const char * DenyReason);
void OnClientDisconnect(lacewing::relayserver &Server, lacewing::relayserver &Client);
void OnError(lacewing::relayserver &Server, lacewing::error &Error);
void OnServerMessage(lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	bool Blasted, int Subchannel, char * Data, int Size, int Variant);
void OnChannelMessage(lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	lacewing::relayserver::channel &Channel,
	bool Blasted, int Subchannel, char * Data, int Size, int Variant);
void OnPeerMessage(lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	lacewing::relayserver::channel &Channel, lacewing::relayserver::client &TargetClient,
	bool Blasted, int Subchannel, char * Packet, int Size, int Variant);
void OnJoinChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	lacewing::relayserver::channel &Channel);
void OnLeaveChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	lacewing::relayserver::channel &Channel);
void OnNameSet(lacewing::relayserver &Server, lacewing::relayserver::client &Client, const char * Name);

void OnFlashError(lacewing::flashpolicy &FlashPolicy, lacewing::error &Error);