
#include "Common.h"
#define MessageMatches() (threadData.receivedMsg.subchannel == subchannel || subchannel == -1)

#define loopNameMatches(cond) \
	if (loopName[0] == '\0') \
	{ \
		CreateError("Cannot detect condition "#cond": loop name is blank."); \
		return false; \
	} \
	return !strcmp(threadData.loop.name, loopName)

bool Extension::OnSentTextMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentTextMessageFromChannel(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentNumberMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentNumberMessageFromChannel(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedTextMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedNumberMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedTextMessageFromChannel(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedNumberMessageFromChannel(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentBinaryMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentBinaryMessageFromChannel(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedBinaryMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedBinaryMessageFromChannel(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentTextMessageFromPeer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentNumberMessageFromPeer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentBinaryMessageFromPeer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedTextMessageFromPeer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedNumberMessageFromPeer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedBinaryMessageFromPeer(int subchannel)
{
	return MessageMatches();
}
bool Extension::IsConnected()
{
	return Cli.connected();
}
bool Extension::OnAnySentMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnySentMessageFromChannel(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnySentMessageFromPeer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedMessageFromChannel(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedMessageFromPeer(int subchannel)
{
	return MessageMatches();
}
bool Extension::ClientHasAName()
{
	return (Cli.name() && Cli.name()[0] == '\0');
}
bool Extension::SelectedPeerIsChannelMaster()
{
	if (!threadData.peer)
	{
		CreateError("Error, Selected Peer Is Channel Master condition called without valid peer being selected.");
		return false;
	}

	return threadData.channel->channelmaster() == threadData.peer;
}
bool Extension::YouAreChannelMaster()
{
	if (!threadData.channel)
	{
		CreateError("Error, You Are Channel Master condition called without valid channel being selected.");
		return false;
	}

	bool isitme = false;
	threadData.channel->channelmaster(&isitme);
	return isitme;
}
bool Extension::OnChannelListLoopWithName(char * loopName)
{
	loopNameMatches("Channel List Loop With Name");
}
bool Extension::OnChannelListLoopWithNameFinished(char * loopName)
{
	loopNameMatches("Channel List Loop With Name Finished");
}
bool Extension::OnPeerLoopWithName(char * loopName)
{
	loopNameMatches("Peer Loop With Name");
}
bool Extension::OnPeerLoopWithNameFinished(char * loopName)
{
	loopNameMatches("Peer Loop With Name Finished");
}
bool Extension::OnClientChannelLoopWithName(char * loopName)
{
	loopNameMatches("Client Channel Loop With Name");
}
bool Extension::OnClientChannelLoopWithNameFinished(char * loopName)
{
	loopNameMatches("Client Channel Loop With Name Finished");
}
bool Extension::OnSentTextChannelMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentNumberChannelMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentBinaryChannelMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnySentChannelMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedTextChannelMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedNumberChannelMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedBinaryChannelMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedChannelMessageFromServer(int subchannel)
{
	return MessageMatches();
}
bool Extension::IsJoinedToChannel(char * channelName)
{
	if (channelName[0] == '\0')
		CreateError("Error checking if joined to a channel, channel name supplied was blank.");
	else
	{
		auto &channels = Channels;
		auto C = std::find_if(Channels.cbegin(), Channels.cend(), [=](ChannelCopy * const &c) {
			return !_stricmp(c->name(), channelName); });
		return C != Channels.cend() && !(**C).isclosed;
	}
	return false;
}
bool Extension::IsPeerOnChannel_Name(char * peerName, char * channelName)
{
	if (peerName[0] == '\0')
		CreateError("Error checking if peer is joined to a channel, peer name supplied was blank.");
	else if (channelName[0] != '\0')
	{
		auto &channels = Channels;
		auto C = std::find_if(Channels.cbegin(), Channels.cend(), [=](ChannelCopy * const &c) {
			return !_stricmp(c->name(), channelName); });
		if (C == Channels.cend())
		{
			CreateError("Error checking if peer is joined to a channel; not connected to channel supplied.");
			return false;
		}
		if ((**C).isclosed)
			return false;

		auto &peers = (**C).getpeers();
		auto P = std::find_if(peers.cbegin(), peers.cend(), [=](PeerCopy * const &p) {
			return !_stricmp(p->name(), peerName); });
		return P != peers.cend() && !(**P).isclosed;
	}
	else if (threadData.channel) // Use currently selected channel
	{
		auto &peers = threadData.channel->getpeers();
		auto P = std::find_if(peers.cbegin(), peers.cend(), [=](PeerCopy * const &p) {
			return !_stricmp(p->name(), peerName); });
		return P != peers.cend() && !(**P).isclosed;
	}
	else // No currently selected channel!
	{
		CreateError("Error checking if peer is joined to a channel; no channel selected, and no channel name supplied.");
	}
	return false;
}
bool Extension::IsPeerOnChannel_ID(int peerID, char * channelName)
{
	if (channelName[0] != '\0')
	{
		auto &channels = Channels;
		auto C = std::find_if(Channels.cbegin(), Channels.cend(), [=](ChannelCopy * const &c) {
			return !_stricmp(c->name(), channelName);
		});
		if (C == Channels.cend())
		{
			CreateError("Error checking if peer is joined to a channel; not connected to channel supplied.");
			return false;
		}
		if ((**C).isclosed)
			return false;

		auto &peers = (**C).getpeers();
		auto P = std::find_if(peers.cbegin(), peers.cend(), [=](PeerCopy * const &p) {
			return p->id() == peerID;
		});
		return P != peers.cend() && !(**P).isclosed;
	}
	else if (threadData.channel)// Use currently selected channel
	{
		auto &peers = threadData.channel->getpeers();
		auto P = std::find_if(peers.cbegin(), peers.cend(), [=](PeerCopy * const &p) {
			return p->id() == peerID;
		});
		return P != peers.cend() && !(**P).isclosed;
	}
	else // No currently selected channel!
	{
		CreateError("Error checking if peer is joined to a channel; no channel selected, and no channel name supplied.");
	}
	return false;
}
