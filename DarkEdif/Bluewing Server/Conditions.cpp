
#include "Common.h"

#define LoopNameMatches(cond) \
	if (loopName[0] == _T('\0')) \
	{ \
		CreateError("Cannot detect condition "#cond": blank loop name supplied."); \
		return false; \
	} \
	/* If tempted to put "is this->loopName.empty()" then you're not copying out the loop name. */ \
	return this->loopName == loopName

bool Extension::IsLacewingServerHosting()
{
	return Srv.hosting();
}

bool Extension::SubchannelMatches(int subchannel)
{
	return (subchannel == -1 || threadData->receivedMsg.subchannel == subchannel);
}
bool Extension::Client_IsChannelMaster()
{
	if (!selChannel)
		return CreateError("Error, Client Is Channel Master condition called without valid channel being selected."), false;
	if (!selClient)
		return CreateError("Error, Client Is Channel Master condition called without valid client being selected."), false;

	return selChannel->channelmaster() == selClient;
}
bool Extension::OnAllClientsLoopWithName(const TCHAR * loopName)
{
	LoopNameMatches("Peer Loop With Name");
}
bool Extension::OnClientsJoinedChannelLoopWithName(const TCHAR * loopName)
{
	LoopNameMatches("Client's Joined Channel Loop With Name");
}
bool Extension::OnClientsJoinedChannelLoopWithNameFinished(const TCHAR * loopName)
{
	LoopNameMatches("Client's Joined Channel Loop With Name Finished");
}
bool Extension::IsClientOnChannel_ByClientName(const TCHAR * clientNamePtr, const TCHAR * channelNamePtr)
{
	if (clientNamePtr[0] == _T('\0') && !selClient)
		return CreateError("Error checking if client is joined to a channel, client name supplied was blank and no client pre-selected."), false;
	if (channelNamePtr[0] == _T('\0') && !selChannel)
		return CreateError("Error checking if client is joined to a channel, channel name supplied was blank and no channel pre-selected."), false;

	auto origSelClient = selClient;
	auto origSelChannel = selChannel;
	decltype(selChannel) foundCh;
	decltype(selClient) foundCli;

	// If blank client name, use currently selected
	// (check that one is selected is done above)
	if (clientNamePtr[0] == _T('\0'))
		foundCli = selClient;
	else
	{
		const std::string clientNameU8Simplified = TStringToUTF8Simplified(clientNamePtr);
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		auto &clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[&](auto const & cli) { return lw_sv_cmp(cli->nameSimplified(), clientNameU8Simplified); });
		if (foundCliIt == clients.cend())
			return CreateError("Error checking if client is joined to a channel, client name \"%s\" was not found on server.", DarkEdif::TStringToUTF8(clientNamePtr).c_str()), false;
		foundCli = *foundCliIt;
	}

	// If blank channel name, use currently selected
	if (channelNamePtr[0] == _T('\0'))
		foundCh = selChannel;
	else
	{
		const std::string channelNameU8Simplified = TStringToUTF8Simplified(channelNamePtr);
		auto serverChannelListReadLock = Srv.lock_channellist.createReadLock();
		auto &channels = Srv.getchannels();
		auto foundChIt =
			std::find_if(channels.cbegin(), channels.cend(),
				[&](auto const & ch) { return lw_sv_cmp(ch->nameSimplified(), channelNameU8Simplified); });
		if (foundChIt == channels.cend())
			return CreateError("Error checking if client is joined to a channel, channel name \"%s\" was not found on server.", DarkEdif::TStringToUTF8(channelNamePtr).c_str()), false;
		foundCh = *foundChIt;
	}

	// Check on client's joined channel list
	bool isInCliJoinedList = false;
	{
		auto cliReadLock = foundCli->lock.createReadLock();
		const auto& clientJoinedChannels = foundCli->getchannels();
		isInCliJoinedList = std::find(clientJoinedChannels.cbegin(), clientJoinedChannels.cend(), foundCh) != clientJoinedChannels.cend();
	}
	if (isInCliJoinedList)
		return true;

	// If it is not, check that channel list does not contains it either.
	// In a peer leave/channel leave message, the behaviour of lists is currently defined as:
	// When a request is processed, the queues are updated AFTER response.
	// In the event the server kicks someone, it's equivalent to an immediate channel leave.

	auto chReadLock = foundCh->lock.createReadLock();
	auto& channelClients = foundCh->getclients();
	return std::find(channelClients.cbegin(), channelClients.cend(), foundCli) != channelClients.cend();
}
bool Extension::IsClientOnChannel_ByClientID(int clientID, const TCHAR * channelNamePtr)
{
	if (clientID >= 0xFFFF)
		return CreateError("Error checking if client is joined to a channel, client ID %i is not in valid ID range of 0-65534.", clientID), false;
	if (channelNamePtr[0] == '\0' && !selChannel)
		return CreateError("Error checking if client is joined to a channel, channel name supplied was blank and no channel pre-selected."), false;

	auto origSelClient = selClient;
	auto origSelChannel = selChannel;
	decltype(selChannel) foundCh;
	decltype(selClient) foundCli;

	// Always look up client ID
	// If user wants to use currently selected client, they can use the shortcut of IsClientOnChannel_Name with a blank name
	{
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		const auto &clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[=](auto const & copy) { return copy->id() == clientID; });
		if (foundCliIt == clients.cend())
			return CreateError("Error checking if client is joined to a channel, client ID %i was not found on server.", clientID), false;
		foundCli = *foundCliIt;
	}

	// If blank channel name, use currently selected
	if (channelNamePtr[0] == _T('\0'))
		foundCh = selChannel;
	else
	{
		const std::string channelNameU8Simplified = TStringToUTF8Simplified(channelNamePtr);
		auto serverChannelListReadLock = Srv.lock_channellist.createReadLock();
		const auto &channels = Srv.getchannels();
		auto foundChIt =
			std::find_if(channels.cbegin(), channels.cend(),
				[&](auto const & ch) { return lw_sv_cmp(ch->name(), channelNameU8Simplified); });
		if (foundChIt == channels.cend())
			return CreateError("Error checking if client is joined to a channel, channel name \"%s\" was not found on server.", DarkEdif::TStringToUTF8(channelNamePtr).c_str()), false;
		foundCh = *foundChIt;
	}

	// Check on client's joined channel list
	bool isInCliJoinedList = false;
	{
		auto cliReadLock = foundCli->lock.createReadLock();
		const auto & clientJoinedChannels = foundCli->getchannels();
		isInCliJoinedList = std::find(clientJoinedChannels.cbegin(), clientJoinedChannels.cend(), foundCh) != clientJoinedChannels.cend();
	}
	if (isInCliJoinedList)
		return true;

	// If it is not, check that channel list does not contains it either.
	// In a peer leave/channel leave message, the behaviour of lists is currently defined as:
	// When a request is processed, the queues are updated AFTER response.
	// In the event the server kicks someone, it's equivalent to an immediate channel leave.

	auto chReadLock = foundCh->lock.createReadLock();
	const auto& channelClients = foundCh->getclients();
	return std::find(channelClients.cbegin(), channelClients.cend(), foundCli) != channelClients.cend();
}

bool Extension::OnAllChannelsLoopWithName(const TCHAR * loopName)
{
	LoopNameMatches("All-Channel Loop With Name");
}
bool Extension::OnAllChannelsLoopWithNameFinished(const TCHAR * loopName)
{
	LoopNameMatches("All-Channel Loop With Name Finished");
}

bool Extension::OnChannelClientsLoopWithName(const TCHAR * loopName)
{
	LoopNameMatches("Channel's Client Loop With Name");
}
bool Extension::OnChannelClientsLoopWithNameFinished(const TCHAR * loopName)
{
	LoopNameMatches("Channel's Client Loop With Name Finished");
}

bool Extension::IsFlashPolicyServerHosting()
{
	return FlashSrv->hosting();
}

bool Extension::ChannelIsHiddenFromChannelList()
{
	return selChannel ? selChannel->hidden() : false;
}

bool Extension::ChannelIsSetToCloseAutomatically()
{
	return selChannel ? selChannel->autocloseenabled() : false;
}
bool Extension::OnAllClientsLoopWithNameFinished(const TCHAR * loopName)
{
	LoopNameMatches("Peer Loop With Name Finished");
}

bool Extension::DoesChannelNameExist(const TCHAR * channelNamePtr)
{
	if (channelNamePtr[0] == '\0')
		return CreateError("Error checking if client is joined to a channel, channel name supplied was blank."), false;

	const std::string channelNameU8Simplified = TStringToUTF8Simplified(channelNamePtr);

	auto serverChannelListReadLock = Srv.lock_channellist.createReadLock();
	const auto &channels = Srv.getchannels();
	auto foundChIt =
		std::find_if(channels.cbegin(), channels.cend(),
			[&](const auto & ch) {
				return lw_sv_cmp(ch->nameSimplified(), channelNameU8Simplified);
			});
	return foundChIt != channels.cend();
}
bool Extension::DoesClientNameExist(const TCHAR * clientNamePtr)
{
	if (clientNamePtr[0] == _T('\0'))
		return CreateError("Error checking if client is joined to a channel, client name supplied is blank."), false;

	const std::string clientNameU8Simplified = TStringToUTF8Simplified(clientNamePtr);

	auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
	const auto &clients = Srv.getclients();
	auto foundCliIt =
		std::find_if(clients.cbegin(), clients.cend(),
			[&](const auto & cli) {
				return lw_sv_cmp(cli->nameSimplified(), clientNameU8Simplified);
			});
	return foundCliIt != clients.cend();
}
bool Extension::DoesChannelIDExist(int channelID)
{
	if (channelID < 0 || channelID >= 0xFFFF)
		return CreateError("Error checking if channel exists, channel ID was invalid."), false;

	auto serverChannelListReadLock = Srv.lock_channellist.createReadLock();
	const auto &channels = Srv.getchannels();
	auto foundChIt =
		std::find_if(channels.cbegin(), channels.cend(),
			[=](const auto & ch) { return ch->id() == channelID; });
	return foundChIt != channels.cend();
}
bool Extension::DoesClientIDExist(int clientID)
{
	if (clientID < 0 || clientID >= 0xFFFF)
		return CreateError("Error checking if client exists, client ID was invalid."), false;

	auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
	const auto &clients = Srv.getclients();
	auto foundCliIt =
		std::find_if(clients.cbegin(), clients.cend(),
			[=](const auto & cli) {
				return cli->id() == clientID;
			});
	return foundCliIt != clients.cend();
}

bool Extension::IsWebSocketHosting(const TCHAR * serverTypeParam)
{
	const std::string serverType = TStringToUTF8Simplified(serverTypeParam);
	const bool hostingSecure = Srv.websocket->hosting_secure(),
		hostingInsecure = Srv.websocket->hosting();

	if (serverType == "both"sv)
		return hostingSecure && hostingInsecure;
	if (serverType == "secure"sv)
		return hostingSecure;
	// i is converted to l as part of text simplifying, so it's actually lnsecure
	if (serverType == "lnsecure"sv)
		return hostingInsecure;
	if (serverType == "either"sv || serverType == "any"sv)
		return hostingSecure || hostingInsecure;

	CreateError("Is WebSocket Hosting condition passed an invalid parameter \"%s\". Expecting \"both\", \"either\", \"secure\" or \"insecure\". Returning false.",
		DarkEdif::TStringToUTF8(serverTypeParam).c_str());
	return false;
}
