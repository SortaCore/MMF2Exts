// Handles all Lacewing functions.
#include "Lacewing.h"

void OnClientConnectRequest(
	lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client);
void OnClientDisconnect(lacewing::relayserver &Server, std::shared_ptr<lacewing::relayserver::client> client);
void OnError(lacewing::relayserver &Server, lacewing::error error);
void OnServerMessage(lacewing::relayserver &Server, std::shared_ptr<lacewing::relayserver::client> client,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant);
void OnChannelMessage(
	lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client,
	std::shared_ptr<lacewing::relayserver::channel> channel,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant);
void OnPeerMessage(	
	lacewing::relayserver &Server, std::shared_ptr<lacewing::relayserver::client> SendingClient,
	std::shared_ptr<lacewing::relayserver::channel> channel, std::shared_ptr<lacewing::relayserver::client> TargetClient,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant);
void OnJoinChannelRequest(lacewing::relayserver &Server, std::shared_ptr<lacewing::relayserver::client> client, std::shared_ptr<lacewing::relayserver::channel> channel,
	// Provided in case Fusion edits channel name, we need the create settings to persist
	bool hidden, bool autoclose);
void OnLeaveChannelRequest(lacewing::relayserver &Server, std::shared_ptr<lacewing::relayserver::client> client,
	std::shared_ptr<lacewing::relayserver::channel> channel);
void OnNameSetRequest(lacewing::relayserver &Server, std::shared_ptr<lacewing::relayserver::client> client, std::string_view newName);
