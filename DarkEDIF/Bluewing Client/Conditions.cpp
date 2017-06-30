
#include "Common.h"
#define MessageMatches() (ThreadData.ReceivedMsg.Subchannel == Subchannel || Subchannel == -1)

#define LoopNameMatches(cond) \
	if (!LoopName || LoopName[0] == '\0') \
	{ \
		CreateError("Cannot detect condition "#cond": invalid loop name supplied."); \
		return false; \
	} \
	return !strcmp(ThreadData.Loop.Name, LoopName)

bool Extension::OnSentTextMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentTextMessageFromChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentNumberMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentNumberMessageFromChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedTextMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedNumberMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedTextMessageFromChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedNumberMessageFromChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentBinaryMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentBinaryMessageFromChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedBinaryMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedBinaryMessageFromChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentTextMessageFromPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentNumberMessageFromPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentBinaryMessageFromPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedTextMessageFromPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedNumberMessageFromPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedBinaryMessageFromPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::IsConnected()
{
	return Cli.connected();
}
bool Extension::OnAnySentMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnySentMessageFromChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnySentMessageFromPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedMessageFromChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedMessageFromPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::ClientHasAName()
{
	return (Cli.name() && Cli.name()[0] == '\0');
}
bool Extension::SelectedPeerIsChannelMaster()
{
	if (!ThreadData.Peer)
	{
		CreateError("Error, Selected Peer Is Channel Master condition called without valid peer being selected.");
		return false;
	}

	return ThreadData.Channel->channelmaster() == ThreadData.Peer;
}
bool Extension::YouAreChannelMaster()
{
	if (!ThreadData.Channel)
	{
		CreateError("Error, You Are Channel Master condition called without valid channel being selected.");
		return false;
	}

	bool isitme = false;
	ThreadData.Channel->channelmaster(&isitme);
	return isitme;
}
bool Extension::OnChannelListLoopWithName(char * LoopName)
{
	LoopNameMatches("Channel List Loop With Name");
}
bool Extension::OnChannelListLoopWithNameFinished(char * LoopName)
{
	LoopNameMatches("Channel List Loop With Name Finished");
}
bool Extension::OnPeerLoopWithName(char * LoopName)
{
	LoopNameMatches("Peer Loop With Name");
}
bool Extension::OnPeerLoopWithNameFinished(char * LoopName)
{
	LoopNameMatches("Peer Loop With Name Finished");
}
bool Extension::OnClientChannelLoopWithName(char * LoopName)
{
	LoopNameMatches("Client Channel Loop With Name");
}
bool Extension::OnClientChannelLoopWithNameFinished(char * LoopName)
{
	LoopNameMatches("Client Channel Loop With Name Finished");
}
bool Extension::OnSentTextChannelMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentNumberChannelMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentBinaryChannelMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnySentChannelMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedTextChannelMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedNumberChannelMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedBinaryChannelMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedChannelMessageFromServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::IsJoinedToChannel(char * ChannelName)
{
	if (ChannelName[0] == '\0')
		CreateError("Error checking if joined to a channel, channel name supplied was blank.");
	else
	{
		auto &channels = Channels;
		auto C = std::find_if(Channels.cbegin(), Channels.cend(), [=](ChannelCopy * const &c) {
			return !_stricmp(c->name(), ChannelName); });
		return C != Channels.cend() && !(**C).isclosed;
	}
	return false;
}
bool Extension::IsPeerOnChannel_Name(char * PeerName, char * ChannelName)
{
	if (PeerName[0] == '\0')
		CreateError("Error checking if peer is joined to a channel, peer name supplied was blank.");
	else if (ChannelName[0] != '\0')
	{
		auto &channels = Channels;
		auto C = std::find_if(Channels.cbegin(), Channels.cend(), [=](ChannelCopy * const &c) {
			return !_stricmp(c->name(), ChannelName); });
		if (C == Channels.cend())
		{
			CreateError("Error checking if peer is joined to a channel; not connected to channel supplied.");
			return false;
		}
		if ((**C).isclosed)
			return false;

		auto &peers = (**C).getpeers();
		auto P = std::find_if(peers.cbegin(), peers.cend(), [=](PeerCopy * const &p) {
			return !_stricmp(p->name(), PeerName); });
		return P != peers.cend() && !(**P).isclosed;
	}
	else if (ThreadData.Channel) // Use currently selected channel
	{
		auto &peers = ThreadData.Channel->getpeers();
		auto P = std::find_if(peers.cbegin(), peers.cend(), [=](PeerCopy * const &p) {
			return !_stricmp(p->name(), PeerName); });
		return P != peers.cend() && !(**P).isclosed;
	}
	else // No currently selected channel!
	{
		CreateError("Error checking if peer is joined to a channel; no channel selected, and no channel name supplied.");
	}
	return false;
}
bool Extension::IsPeerOnChannel_ID(int PeerID, char * ChannelName)
{
	if (ChannelName[0] != '\0')
	{
		auto &channels = Channels;
		auto C = std::find_if(Channels.cbegin(), Channels.cend(), [=](ChannelCopy * const &c) {
			return !_stricmp(c->name(), ChannelName); });
		if (C == Channels.cend())
		{
			CreateError("Error checking if peer is joined to a channel; not connected to channel supplied.");
			return false;
		}
		if ((**C).isclosed)
			return false;

		auto &peers = (**C).getpeers();
		auto P = std::find_if(peers.cbegin(), peers.cend(), [=](PeerCopy * const &p) {
			return p->id() == PeerID; });
		return P != peers.cend() && !(**P).isclosed;
	}
	else if (ThreadData.Channel)// Use currently selected channel
	{
		auto &peers = ThreadData.Channel->getpeers();
		auto P = std::find_if(peers.cbegin(), peers.cend(), [=](PeerCopy * const &p) {
			return p->id() == PeerID; });
		return P != peers.cend() && !(**P).isclosed;
	}
	else // No currently selected channel!
	{
		CreateError("Error checking if peer is joined to a channel; no channel selected, and no channel name supplied.");
	}
	return false;
}
