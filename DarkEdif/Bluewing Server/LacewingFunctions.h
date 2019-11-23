// Handles all Lacewing functions.
#include "Lacewing.h"

void OnClientConnectRequest(
	lacewing::relayserver &server, lacewing::relayserver::client &client);
void OnClientDisconnect(lacewing::relayserver &Server, lacewing::relayserver::client &client);
void OnError(lacewing::relayserver &Server, lacewing::error error);
void OnServerMessage(lacewing::relayserver &Server, lacewing::relayserver::client &client,
	bool blasted, int subchannel, const char * Data, size_t size, int variant);
void OnChannelMessage(
	lacewing::relayserver &server, lacewing::relayserver::client &client,
	lacewing::relayserver::channel &channel,
	bool blasted, int subchannel, const char * Data, size_t size, int variant);
void OnPeerMessage(	
	lacewing::relayserver &Server, lacewing::relayserver::client &SendingClient,
	lacewing::relayserver::channel &channel, lacewing::relayserver::client &TargetClient,
	bool blasted, int subchannel, const char * Data, size_t size, int variant);
void OnJoinChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &client, lacewing::relayserver::channel &channel,
	// Provided in case Fusion edits channel name, we need the create settings to persist
	bool hidden, bool autoclose);
void OnLeaveChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &client,
	lacewing::relayserver::channel &channel);
void OnNameSetRequest(lacewing::relayserver &Server, lacewing::relayserver::client &client, const char * NewName);
