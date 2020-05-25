// Handles all Lacewing functions.
#include "Lacewing.h"

void OnChannelListReceived(lacewing::relayclient &client);
void OnChannelMessage(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel, std::shared_ptr<lacewing::relayclient::channel::peer> peer,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant);
void OnConnect(lacewing::relayclient &client);
void OnConnectDenied(lacewing::relayclient &client, std::string_view denyReason);
void OnDisconnect(lacewing::relayclient &client);
void OnError(lacewing::relayclient &client, lacewing::error error);
void OnJoinChannel(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target);
void OnJoinChannelDenied(lacewing::relayclient &client, std::string_view channelName, std::string_view denyReason);
void OnLeaveChannel(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target);
void OnLeaveChannelDenied(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target, std::string_view denyReason);
void OnNameChanged(lacewing::relayclient &client, std::string_view oldName);
void OnNameDenied(lacewing::relayclient &client, std::string_view deniedName, std::string_view denyReason);
void OnNameSet(lacewing::relayclient &client);
void OnPeerConnect(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,std::shared_ptr<lacewing::relayclient::channel::peer> peer);
void OnPeerDisconnect(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,std::shared_ptr<lacewing::relayclient::channel::peer> peer);
void OnPeerMessage(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel, std::shared_ptr<lacewing::relayclient::channel::peer> peer,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant);
void OnPeerNameChanged(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel, std::shared_ptr<lacewing::relayclient::channel::peer> peer, std::string oldname);
void OnServerChannelMessage(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant);
void OnServerMessage(lacewing::relayclient &client,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant);
