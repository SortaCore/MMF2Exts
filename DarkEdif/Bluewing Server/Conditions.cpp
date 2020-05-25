
#include "Common.h"

#define LoopNameMatches(cond) \
	if (loopName[0] == '\0') \
	{ \
		CreateError("Cannot detect condition "#cond": blank loop name supplied."); \
		return false; \
	} \
	/* If tempted to put "is this->loopName.empty()" then you're not copying out the loop name. */ \
	return !strcmp(this->loopName.data(), loopName)

bool Extension::IsLacewingServerHosting()
{
	return Srv.hosting();
}

bool Extension::SubchannelMatches(int subchannel)
{
	return (threadData->receivedMsg.subchannel == subchannel || subchannel == -1);
}
bool Extension::Client_IsChannelMaster()
{
	if (!selChannel)
		return CreateError("Error, Client Is Channel Master condition called without valid channel being selected."), false;
	if (!selClient)
		return CreateError("Error, Client Is Channel Master condition called without valid client being selected."), false;

	return selChannel->channelmaster() == selClient;
}
bool Extension::OnAllClientsLoopWithName(char * loopName)
{
	LoopNameMatches("Peer Loop With Name");
}
bool Extension::OnClientsJoinedChannelLoopWithName(char * loopName)
{
	LoopNameMatches("Client's Joined Channel Loop With Name");
}
bool Extension::OnClientsJoinedChannelLoopWithNameFinished(char * loopName)
{
	LoopNameMatches("Client's Joined Channel Loop With Name Finished");
}
bool Extension::IsClientOnChannel_Name(char * clientNamePtr, char * channelNamePtr)
{
	if (clientNamePtr[0] == '\0' && !selClient)
		return CreateError("Error checking if client is joined to a channel, client name supplied was blank and no client pre-selected."), false;
	if (channelNamePtr[0] == '\0' && !selChannel)
		return CreateError("Error checking if client is joined to a channel, channel name supplied was blank and no channel pre-selected."), false;

	std::string_view clientName(clientNamePtr);
	std::string_view channelName(channelNamePtr);

	auto origSelClient = selClient;
	auto origSelChannel = selChannel;
	decltype(selChannel) foundCh;
	decltype(selClient) foundCli;

	// If blank client name, use currently selected
	// (check that one is selected is done above)
	if (clientName[0] == '\0')
		foundCli = selClient;
	else
	{
		auto serverReadLock = Srv.lock.createReadLock();
		auto &clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[&](auto const & copy) {
					return lw_sv_icmp(copy->name(), clientName);
				});
		if (foundCliIt == clients.cend())
			return CreateError("Error checking if client is joined to a channel, client name \"%s\" was not found on server.", clientNamePtr), false;
		foundCli = *foundCliIt;
	}

	// If blank channel name, use currently selected
	if (channelName[0] == '\0')
		foundCh = selChannel;
	else
	{
		auto serverReadLock = Srv.lock.createReadLock();
		auto &channels = Srv.getchannels();
		auto foundChIt =
			std::find_if(channels.cbegin(), channels.cend(),
				[&](auto const & copy) {
					return lw_sv_icmp(copy->name(), channelName);
				});
		if (foundChIt == channels.cend())
			return CreateError("Error checking if client is joined to a channel, channel name \"%s\" was not found on server.", channelNamePtr), false;
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
bool Extension::IsClientOnChannel_ID(int clientID, char * channelNamePtr)
{
	if (clientID >= 0xFFFF)
		return CreateError("Error checking if client is joined to a channel, client ID %i is not in valid ID range of 0-65534.", clientID), false;
	if (channelNamePtr[0] == '\0' && !selChannel)
		return CreateError("Error checking if client is joined to a channel, channel name supplied was blank and no channel pre-selected."), false;

	std::string_view channelName(channelNamePtr);

	auto origSelClient = selClient;
	auto origSelChannel = selChannel;
	decltype(selChannel) foundCh;
	decltype(selClient) foundCli;

	// Always look up client ID
	// If user wants to use currently selected client, they can use the shortcut of IsClientOnChannel_Name with a blank name
	{
		auto serverReadLock = Srv.lock.createReadLock();
		const auto &clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[=](auto const & copy) {
					return copy->id() == clientID;
				});
		if (foundCliIt == clients.cend())
			return CreateError("Error checking if client is joined to a channel, client ID %i was not found on server.", clientID), false;
		foundCli = *foundCliIt;
	}

	// If blank channel name, use currently selected
	if (channelName[0] == '\0')
		foundCh = selChannel;
	else
	{
		auto serverReadLock = Srv.lock.createReadLock();
		const auto &channels = Srv.getchannels();
		auto foundChIt =
			std::find_if(channels.cbegin(), channels.cend(),
				[&](auto const & copy) {
					return lw_sv_icmp(copy->name(), channelName);
				});
		if (foundChIt == channels.cend())
			return CreateError("Error checking if client is joined to a channel, channel name \"%s\" was not found on server.", channelNamePtr), false;
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

bool Extension::OnAllChannelsLoopWithName(char * loopName)
{
	LoopNameMatches("All-Channel Loop With Name");
}
bool Extension::OnAllChannelsLoopWithNameFinished(char * loopName)
{
	LoopNameMatches("All-Channel Loop With Name Finished");
}

bool Extension::OnChannelClientsLoopWithName(char * loopName)
{
	LoopNameMatches("Channel's Client Loop With Name");
}
bool Extension::OnChannelClientsLoopWithNameFinished(char * loopName)
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
bool Extension::OnAllClientsLoopWithNameFinished(char * loopName)
{
	LoopNameMatches("Peer Loop With Name Finished");
}

bool Extension::DoesChannelNameExist(char * channelNamePtr)
{
	if (channelNamePtr[0] == '\0')
		return CreateError("Error checking if client is joined to a channel, channel name supplied was blank."), false;

	std::string_view channelName(channelNamePtr);

	auto serverReadLock = Srv.lock.createReadLock();
	const auto &channels = Srv.getchannels();
	auto foundChIt =
		std::find_if(channels.cbegin(), channels.cend(),
			[=](const auto & ch) {
				return lw_sv_icmp(ch->name(), channelNamePtr);
			});
	return foundChIt != channels.cend();
}
bool Extension::DoesClientNameExist(char * clientNamePtr)
{
	if (clientNamePtr[0] == '\0')
		return CreateError("Error checking if client is joined to a channel, client name supplied is blank."), false;

	std::string_view clientName(clientNamePtr);

	auto serverReadLock = Srv.lock.createReadLock();
	const auto &clients = Srv.getclients();
	auto foundCliIt =
		std::find_if(clients.cbegin(), clients.cend(),
			[=](const auto & cli) {
				return lw_sv_icmp(cli->name(), clientName);
			});
	return foundCliIt != clients.cend();
}
bool Extension::DoesChannelIDExist(int channelID)
{
	if (channelID < 0 || channelID >= 0xFFFF)
		return CreateError("Error checking if channel exists, channel ID was invalid."), false;

	auto serverReadLock = Srv.lock.createReadLock();
	const auto &channels = Srv.getchannels();
	auto foundChIt =
		std::find_if(channels.cbegin(), channels.cend(),
			[=](const auto & ch) {
				return ch->id() == channelID;
			});
	return foundChIt != channels.cend();
}
bool Extension::DoesClientIDExist(int clientID)
{
	if (clientID < 0 || clientID >= 0xFFFF)
		return CreateError("Error checking if client exists, client ID was invalid."), false;

	auto serverReadLock = Srv.lock.createReadLock();
	const auto &clients = Srv.getclients();
	auto foundCliIt =
		std::find_if(clients.cbegin(), clients.cend(),
			[=](const auto & cli) {
				return cli->id() == clientID;
			});
	return foundCliIt != clients.cend();
}

bool Extension::IsHTML5Hosting()
{
	static bool HTML5Warning = false;
	if (!HTML5Warning)
	{
		HTML5Warning = true;
		CreateError("HTML5 Hosting condition not usable yet. Returning false.");
	}
	return false; // HTML5Srv->hosting();
}
