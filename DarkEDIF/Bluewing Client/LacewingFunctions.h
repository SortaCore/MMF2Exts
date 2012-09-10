// Handles all Lacewing functions.
#include "Lacewing.h"

void OnChannelListReceived(Lacewing::RelayClient &Client);
void OnChannelMessage(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, Lacewing::RelayClient::Channel::Peer &Peer,
					  bool Blasted, int Subchannel, char * Data, int Size, int Variant);
void OnConnect(Lacewing::RelayClient &Client);
void OnConnectDenied(Lacewing::RelayClient &Client, const char * DenyReason);
void OnDisconnect(Lacewing::RelayClient &Client);
void OnError(Lacewing::RelayClient &Client, Lacewing::Error &Error);
void OnJoinChannel(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Target);
void OnJoinChannelDenied(Lacewing::RelayClient &Client, const char * ChannelName, const char * DenyReason);
void OnLeaveChannel(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Target);
void OnLeaveChannelDenied(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Target,const char * DenyReason);
void OnNameChanged(Lacewing::RelayClient &Client, const char * OldName);
void OnNameDenied(Lacewing::RelayClient &Client, const char * DeniedName, const char * DenyReason);
void OnNameSet(Lacewing::RelayClient &Client);
void OnPeerConnect(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel,Lacewing::RelayClient::Channel::Peer &Peer);
void OnPeerDisconnect(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel,Lacewing::RelayClient::Channel::Peer &Peer);
void OnPeerMessage(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, Lacewing::RelayClient::Channel::Peer &Peer,
				   bool Blasted, int Subchannel, char * Data, int Size, int Variant);
void OnPeerNameChanged(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, Lacewing::RelayClient::Channel::Peer &Peer, const char * OldName);
void OnServerChannelMessage(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel,
							bool Blasted, int Subchannel, char * Data, int Size, int Variant);
void OnServerMessage(Lacewing::RelayClient &Client,
					 bool Blasted, int Subchannel, char * Data, int Size, int Variant);
