// Handles all Lacewing functions.
#include "Lacewing.h"

void OnClientConnectRequest(Lacewing::RelayServer &Server, Lacewing::RelayServer &Client);
void OnClientDisconnect(Lacewing::RelayServer &Server, Lacewing::RelayServer &Client);
void OnError(Lacewing::RelayServer &Server, Lacewing::Error &Error);
void OnServerMessage(Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client,
	bool Blasted, int Subchannel, char * Data, int Size, int Variant);
void OnChannelMessage(Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client,
	Lacewing::RelayServer::Channel &Channel,
	bool Blasted, int Subchannel, char * Data, int Size, int Variant);
void OnPeerMessage(Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client,
	Lacewing::RelayServer::Channel &Channel, Lacewing::RelayServer::Client &TargetClient,
	bool Blasted, int Subchannel, char * Packet, int Size, int Variant);
void OnJoinChannelRequest(Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client,
	Lacewing::RelayServer::Channel &Channel);
void OnLeaveChannelRequest(Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client,
	Lacewing::RelayServer::Channel &Channel);
void OnNameSet(Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client, const char * Name);

void OnFlashError(Lacewing::FlashPolicy &FlashPolicy, Lacewing::Error &Error);