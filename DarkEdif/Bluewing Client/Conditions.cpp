
#include "Common.h"
#include <string_view>

#define MessageMatches() (threadData->receivedMsg.subchannel == subchannel || subchannel == -1)

#define LoopNameMatches(cond) \
	if (loopName[0] == '\0') \
	{ \
		CreateError("Cannot detect condition "#cond": loop name is blank."); \
		return false; \
	} \
	return !strcmp(loopName.data(), passedLoopName)

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
	return !Cli.name().empty();
}
bool Extension::SelectedPeerIsChannelMaster()
{
	if (!selPeer)
	{
		CreateError("Error, Selected Peer Is Channel Master condition called without valid peer being selected.");
		return false;
	}

	return selPeer->ischannelmaster();
}
bool Extension::YouAreChannelMaster()
{
	if (!selChannel)
	{
		CreateError("Error, You Are Channel Master condition called without valid channel being selected.");
		return false;
	}

	return selChannel->ischannelmaster();
}
bool Extension::OnChannelListLoopWithName(char * passedLoopName)
{
	LoopNameMatches("Channel List Loop With Name");
}
bool Extension::OnChannelListLoopWithNameFinished(char * passedLoopName)
{
	LoopNameMatches("Channel List Loop With Name Finished");
}
bool Extension::OnPeerLoopWithName(char * passedLoopName)
{
	LoopNameMatches("Peer Loop With Name");
}
bool Extension::OnPeerLoopWithNameFinished(char * passedLoopName)
{
	LoopNameMatches("Peer Loop With Name Finished");
}
bool Extension::OnClientChannelLoopWithName(char * passedLoopName)
{
	LoopNameMatches("Client Channel Loop With Name");
}
bool Extension::OnClientChannelLoopWithNameFinished(char * passedLoopName)
{
	LoopNameMatches("Client Channel Loop With Name Finished");
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
bool Extension::IsJoinedToChannel(char * channelNamePtr)
{
	if (channelNamePtr[0] == '\0')
		return CreateError("Error checking if joined to a channel, channel name supplied was blank."), false;

	std::string_view channelName(channelNamePtr);

	auto cliReadLock = Cli.lock.createReadLock();
	const auto &channels = Cli.getchannels();
	auto chIt = std::find_if(channels.cbegin(), channels.cend(),
		[=](const auto &c) { return lw_sv_icmp(c->name(), channelName); });
	return chIt != channels.cend() && !(*chIt)->readonly();
}
bool Extension::IsPeerOnChannel_Name(char * peerNamePtr, char * channelNamePtr)
{
	if (peerNamePtr[0] == '\0' && !selPeer)
		return CreateError("Error checking if peer is joined to a channel, peer name supplied was blank and no peer pre-selected."), false;
	if (channelNamePtr[0] == '\0' && !selChannel)
		return CreateError("Error checking if peer is joined to a channel, channel name supplied was blank and no channel pre-selected."), false;
	if (channelNamePtr[0] == '\0' && peerNamePtr[0] == '\0')
		return selPeer->readonly();

	std::string_view channelName(channelNamePtr);
	std::string_view peerName(peerNamePtr);

	decltype(selChannel) foundCh;
	decltype(selPeer) foundPeer;

	// If blank channel name, use currently selected
	if (channelNamePtr[0] == '\0')
		foundCh = selChannel;
	else
	{
		auto serverReadLock = Cli.lock.createReadLock();
		const auto & channels = Cli.getchannels();
		auto foundChIt =
			std::find_if(channels.cbegin(), channels.cend(),
				[&](const auto & ch) {
					return lw_sv_icmp(ch->name(), channelName);
				});
		if (foundChIt == channels.cend())
		{
			std::stringstream error;
			error << "Error checking if peer is joined to a channel, channel name \"" << channelName << "\" was not found on server.";
			return CreateError(error.str().c_str()), false;
		}
		foundCh = *foundChIt;
	}

	auto channelReadLock = foundCh->lock.createReadLock();
	const auto & peers = foundCh->getpeers();

	// If blank peer name, use currently selected peer; it might be in found channel
	if (peerName[0] == '\0')
	{
		// selChannel + selPeer = obviously the currently selected peer is on current channel
		if (foundCh == selChannel)
			return selPeer->readonly();

		lw_ui16 peerID = selPeer->id();
		auto foundPeerIt =
			std::find_if(peers.cbegin(), peers.cend(),
				[=](const auto & peer) {
					return peer->id() == peerID;
				});
		return foundPeerIt != peers.cend() && (*foundPeerIt)->readonly();
	}

	auto foundPeerIt =
		std::find_if(peers.cbegin(), peers.cend(),
			[=](const auto & peer) {
				return lw_sv_icmp(peer->name(), peerName);
			});
	return foundPeerIt != peers.cend() && (*foundPeerIt)->readonly();
}
bool Extension::IsPeerOnChannel_ID(int peerID, char * channelNamePtr)
{
	if (peerID <= 0 || peerID >= 0xFFFF)
		return CreateError("Error checking if peer is joined to a channel, peer ID was invalid."), false;
	if (channelNamePtr[0] == '\0' && !selChannel)
		return CreateError("Error checking if peer is joined to a channel, channel name supplied was blank and no channel pre-selected."), false;

	decltype(selChannel) foundCh;
	std::string_view channelName(channelNamePtr);

	// If blank channel name, use currently selected
	if (channelNamePtr[0] == '\0')
		foundCh = selChannel;
	else
	{
		auto serverReadLock = Cli.lock.createReadLock();
		const auto & channels = Cli.getchannels();
		auto foundChIt =
			std::find_if(channels.cbegin(), channels.cend(),
				[&](const auto & ch) {
					return lw_sv_icmp(ch->name(), channelName);
				});
		if (foundChIt == channels.cend())
		{
			std::stringstream error;
			error << "Error checking if peer is joined to a channel, channel name \"" << channelName << "\" was not found on server.";
			return CreateError(error.str().c_str()), false;
		}
		foundCh = *foundChIt;
	}

	auto channelReadLock = foundCh->lock.createReadLock();
	const auto & peers = foundCh->getpeers();

	auto foundPeerIt =
		std::find_if(peers.cbegin(), peers.cend(),
			[=](const auto & peer) {
				return peer->id() == peerID;
			});
	return foundPeerIt != peers.cend() && (*foundPeerIt)->readonly();
}
