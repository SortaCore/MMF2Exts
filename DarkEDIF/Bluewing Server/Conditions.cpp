
#include "Common.h"

#define LoopNameMatches(cond) \
	if (loopName[0] == '\0') \
	{ \
		CreateError("Cannot detect condition "#cond": blank loop name supplied."); \
		return false; \
	} \
	return !strcmp(threadData.loop.name, loopName)

bool Extension::IsLacewingServerHosting()
{
	return Srv.hosting();
}

bool Extension::SubchannelMatches(int subchannel)
{
	return (threadData.receivedMsg.subchannel == subchannel || subchannel == -1);
}
bool Extension::Client_IsChannelMaster()
{
	if (!threadData.channel || !threadData.client)
	{
		CreateError("Error, You Are Channel Master condition called without valid channel being selected.");
		return false;
	}

	return threadData.channel->channelmaster() == threadData.client;
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
bool Extension::IsClientOnChannel_Name(char * clientName, char * channelName)
{
	if (clientName[0] == '\0' && !threadData.client)
		CreateError("Error checking if client is joined to a channel, client name supplied was blank and no client pre-selected.");
	else if (channelName[0] == '\0' && !threadData.channel)
		CreateError("Error checking if client is joined to a channel, channel name supplied was blank and no channel pre-selected.");
	else
	{
		auto selectedClient = threadData.client;
		if (clientName[0])
		{
			auto selectedClient2 =
				std::find_if(Clients.cbegin(), Clients.cend(),
					[&](ClientCopy * const & copy) {
						return !_stricmp(copy->name(), clientName);
			});
			if (selectedClient2 == Clients.cend())
			{
				std::stringstream error;
				error << "Error checking if client is joined to a channel, client name \"" << clientName << "\" was not found on server.";
				CreateError(error.str().c_str());
				return false;
			}
			selectedClient = *selectedClient2;
		}

		auto selectedChannel = threadData.channel;
		if (channelName[0])
		{
			auto SelectedChannel2 =
				std::find_if(Channels.cbegin(), Channels.cend(), [&](ChannelCopy * const & copy) {
					return !_stricmp(copy->name(), channelName);
			});
			if (SelectedChannel2 == Channels.cend())
			{
				std::stringstream error;
				error << "Error checking if client is joined to a channel, channel name \"" << channelName << "\" was not found on server.";
				CreateError(error.str().c_str());
				return false;
			}
			selectedChannel = *SelectedChannel2;
		}

		auto& channelClients = selectedChannel->getclients();
		auto& clientChannels = selectedClient->getchannels();

		return std::find(clientChannels.cbegin(), clientChannels.cend(), selectedChannel) != clientChannels.cend() ||
			std::find(channelClients.cbegin(), channelClients.cend(), selectedClient) != channelClients.cend();
	}
	return false;
}
bool Extension::IsClientOnChannel_ID(int clientID, char * channelName)
{
	if (clientID > 0xFFFF)
	{
		std::stringstream error;
		error << "Error checking if client is joined to a channel, client ID " << clientID << " is not between total ID range of 0-65535.";
		CreateError(error.str().c_str());
	}
	else if (channelName[0] == '\0' && !threadData.channel)
		CreateError("Error checking if client is joined to a channel, channel name supplied was blank and no channel pre-selected.");
	else
	{
		ClientCopy * selectedClient = nullptr;
		{		
			auto SelectedClient2 =
				std::find_if(Clients.cbegin(), Clients.cend(), [&](ClientCopy * const & copy) {
					return copy->id() == clientID;
			});
			if (SelectedClient2 == Clients.cend())
			{
				std::stringstream error;
				error << "Error checking if client is joined to a channel, client ID " << clientID << " was not found on server.";
				CreateError(error.str().c_str());
				return false;
			}
			selectedClient = *SelectedClient2;
		}

		auto selectedChannel = threadData.channel;
		if (channelName[0])
		{
			auto selectedChannel2 =
				std::find_if(Channels.cbegin(), Channels.cend(), [&](ChannelCopy * const & copy) {
					return !_stricmp(copy->name(), channelName);
			});
			if (selectedChannel2 == Channels.cend())
			{
				std::stringstream error;
				error << "Error checking if client is joined to a channel, channel name \"" << channelName << "\" was not found on server.";
				CreateError(error.str().c_str());
				return false;
			}
			selectedChannel = *selectedChannel2;
		}

		auto& channelClients = selectedChannel->getclients();
		auto& clientChannels = selectedClient->getchannels();

		return std::find(clientChannels.cbegin(), clientChannels.cend(), selectedChannel) != clientChannels.cend() ||
			std::find(channelClients.cbegin(), channelClients.cend(), selectedClient) != channelClients.cend();
	}
	return false;
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
	return threadData.channel ? threadData.channel->hidden() : false;
}

bool Extension::ChannelIsSetToCloseAutomatically()
{
	return threadData.channel ? threadData.channel->autocloseenabled() : false;
}
bool Extension::OnAllClientsLoopWithNameFinished(char * loopName)
{
	LoopNameMatches("Peer Loop With Name Finished");
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

