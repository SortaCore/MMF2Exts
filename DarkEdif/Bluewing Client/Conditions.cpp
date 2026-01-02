#include "Common.hpp"
#define MessageMatches(T) ( subchannel == -1 || threadData->As<T>()->msg.subchannel == subchannel)

#define LoopNameMatches(cond) \
	if (loopName[0] == _T('\0')) \
	{ \
		CreateError("Cannot detect condition "#cond": loop name is blank."); \
		return false; \
	} \
	return !_tcscmp(loopName.data(), passedLoopName)

bool Extension::ServerMessageCondition(int subchannel)
{
	return MessageMatches(ServerMsgEvent);
}
bool Extension::ChannelMessageCondition(int subchannel)
{
	return MessageMatches(ChannelMsgEvent);
}
bool Extension::PeerMessageCondition(int subchannel)
{
	return MessageMatches(PeerMsgEvent);
}
bool Extension::ServerChannelMessageCondition(int subchannel)
{
	return MessageMatches(ServerChannelMsgEvent);
}
bool Extension::IsConnected()
{
	return Cli.connected();
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
bool Extension::OnChannelListLoopWithName(const TCHAR * passedLoopName)
{
	LoopNameMatches("Channel List Loop With Name");
}
bool Extension::OnChannelListLoopWithNameFinished(const TCHAR * passedLoopName)
{
	LoopNameMatches("Channel List Loop With Name Finished");
}
bool Extension::OnPeerLoopWithName(const TCHAR * passedLoopName)
{
	LoopNameMatches("Peer Loop With Name");
}
bool Extension::OnPeerLoopWithNameFinished(const TCHAR * passedLoopName)
{
	LoopNameMatches("Peer Loop With Name Finished");
}
bool Extension::OnClientChannelLoopWithName(const TCHAR * passedLoopName)
{
	LoopNameMatches("Client Channel Loop With Name");
}
bool Extension::OnClientChannelLoopWithNameFinished(const TCHAR * passedLoopName)
{
	LoopNameMatches("Client Channel Loop With Name Finished");
}
bool Extension::IsJoinedToChannel(const TCHAR * channelNamePtr)
{
	if (channelNamePtr[0] == _T('\0'))
		return CreateError("Error checking if joined to a channel, channel name supplied was blank."), false;

	const std::string channelNameU8Simplified = TStringToUTF8Simplified(channelNamePtr);
	auto cliReadLock = Cli.lock.createReadLock();
	const auto &channels = Cli.getchannels();
	auto chIt = std::find_if(channels.cbegin(), channels.cend(),
		[&](const auto &c) { return lw_sv_cmp(c->namesimplified(), channelNameU8Simplified); });
	return chIt != channels.cend() && !(*chIt)->readonly();
}
bool Extension::IsPeerOnChannel_Name(const TCHAR * peerNameTStr, const TCHAR * channelNameTStr)
{
	if (peerNameTStr[0] == _T('\0') && !selPeer)
		return CreateError("Error checking if peer is joined to a channel, peer name supplied was blank and no peer pre-selected."), false;
	if (channelNameTStr[0] == _T('\0') && !selChannel)
		return CreateError("Error checking if peer is joined to a channel, channel name supplied was blank and no channel pre-selected."), false;
	if (channelNameTStr[0] == _T('\0') && peerNameTStr[0] == _T('\0'))
		return selPeer->readonly();

	decltype(selChannel) foundCh;
	decltype(selPeer) foundPeer;

	// If blank channel name, use currently selected
	if (channelNameTStr[0] == _T('\0'))
		foundCh = selChannel;
	else
	{
		const std::string channelNameU8Simplified = TStringToUTF8Simplified(channelNameTStr);

		auto serverReadLock = Cli.lock.createReadLock();
		const auto & channels = Cli.getchannels();
		auto foundChIt =
			std::find_if(channels.cbegin(), channels.cend(),
				[&](const auto & ch) { return lw_sv_cmp(ch->namesimplified(), channelNameU8Simplified); });
		if (foundChIt == channels.cend())
			return CreateError("Error checking if peer is joined to a channel, channel name \"%s\" was not found on server.", DarkEdif::TStringToUTF8(channelNameTStr).c_str()), false;
		foundCh = *foundChIt;
	}

	auto channelReadLock = foundCh->lock.createReadLock();
	const auto & peers = foundCh->getpeers();

	// If blank peer name, use currently selected peer; it might be in found channel
	if (peerNameTStr[0] == _T('\0'))
	{
		// selChannel + selPeer = obviously the currently selected peer is on current channel
		if (foundCh == selChannel)
			return selPeer->readonly();

		lw_ui16 peerID = selPeer->id();
		auto foundPeerIt =
			std::find_if(peers.cbegin(), peers.cend(),
				[=](const auto & peer) { return peer->id() == peerID; });
		return foundPeerIt != peers.cend() && (*foundPeerIt)->readonly();
	}

	const std::string peerNameStripped = TStringToUTF8Simplified(peerNameTStr);
	auto foundPeerIt =
		std::find_if(peers.cbegin(), peers.cend(),
			[&](const auto & peer) { return lw_sv_cmp(peer->namesimplified(), peerNameStripped); });
	return foundPeerIt != peers.cend() && !(*foundPeerIt)->readonly();
}
bool Extension::IsPeerOnChannel_ID(int peerID, const TCHAR * channelNamePtr)
{
	if (peerID <= 0 || peerID >= 0xFFFF)
		return CreateError("Error checking if peer is joined to a channel, peer ID was invalid."), false;
	if (channelNamePtr[0] == _T('\0') && !selChannel)
		return CreateError("Error checking if peer is joined to a channel, channel name supplied was blank and no channel pre-selected."), false;

	decltype(selChannel) foundCh;

	// If blank channel name, use currently selected
	if (channelNamePtr[0] == _T('\0'))
		foundCh = selChannel;
	else
	{
		const std::string channelNameStripped = TStringToUTF8Simplified(channelNamePtr);
		auto serverReadLock = Cli.lock.createReadLock();
		const auto & channels = Cli.getchannels();

		auto foundChIt =
			std::find_if(channels.cbegin(), channels.cend(),
				[&](const auto & ch) { return lw_sv_cmp(ch->namesimplified(), channelNameStripped); });
		if (foundChIt == channels.cend())
			return CreateError("Error checking if peer is joined to a channel, channel name \"%s\" was not found on server.", DarkEdif::TStringToUTF8(channelNamePtr).c_str()), false;
		foundCh = *foundChIt;
	}

	auto channelReadLock = foundCh->lock.createReadLock();
	const auto & peers = foundCh->getpeers();

	auto foundPeerIt =
		std::find_if(peers.cbegin(), peers.cend(),
			[=](const auto & peer) { return peer->id() == peerID; });
	return foundPeerIt != peers.cend() && !(*foundPeerIt)->readonly();
}

bool Extension::MandatoryTriggeredEvent()
{
	globals->lastMandatoryEventWasChecked = true;
	return true;
}
