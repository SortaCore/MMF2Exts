
#include "Common.h"

#define LoopNameMatches(cond) \
	if (!LoopName || LoopName[0] == '\0') \
	{ \
		CreateError("Cannot detect condition "#cond": invalid loop name supplied."); \
		return false; \
	} \
	return !strcmp(ThreadData.Loop.Name, LoopName)

bool Extension::IsLacewingServerHosting()
{
	return Srv.hosting();
}

bool Extension::SubchannelMatches(int Subchannel)
{
	return (ThreadData.ReceivedMsg.Subchannel == Subchannel || Subchannel == -1);
}
bool Extension::Client_IsChannelMaster()
{
	if (!ThreadData.Channel || !ThreadData.Client)
	{
		CreateError("Error, You Are Channel Master condition called without valid channel being selected.");
		return false;
	}

	return ThreadData.Channel->channelmaster() == ThreadData.Client;
}
bool Extension::OnAllClientsLoopWithName(char * LoopName)
{
	LoopNameMatches("Peer Loop With Name");
}
bool Extension::OnClientsJoinedChannelLoopWithName(char * LoopName)
{
	LoopNameMatches("Client's Joined Channel Loop With Name");
}
bool Extension::OnClientsJoinedChannelLoopWithNameFinished(char * LoopName)
{
	LoopNameMatches("Client's Joined Channel Loop With Name Finished");
}
bool Extension::IsClientOnChannel_Name(char * ClientName, char * ChannelName)
{
	if (ClientName[0] == '\0' && !ThreadData.Client)
		CreateError("Error checking if client is joined to a channel, client name supplied was blank and no client pre-selected.");
	else if (ChannelName[0] == '\0' && !ThreadData.Channel)
		CreateError("Error checking if client is joined to a channel, channel name supplied was blank and no channel pre-selected.");
	else
	{
		auto SelectedClient = ThreadData.Client;
		if (ClientName[0])
		{
			auto SelectedClient2 =
				std::find_if(Clients.cbegin(), Clients.cend(), [&](ClientCopy * const & copy) {
				return !_stricmp(copy->name(), ClientName); });
			if (SelectedClient2 == Clients.cend())
			{
				std::stringstream Error;
				Error << "Error checking if client is joined to a channel, client name \"" << ClientName << "\" was not found on server.";
				CreateError(Error.str().c_str());
				return false;
			}
			SelectedClient = *SelectedClient2;
		}

		auto SelectedChannel = ThreadData.Channel;
		if (ChannelName[0])
		{
			auto SelectedChannel2 =
				std::find_if(Channels.cbegin(), Channels.cend(), [&](ChannelCopy * const & copy) {
				return !_stricmp(copy->name(), ChannelName); });
			if (SelectedChannel2 == Channels.cend())
			{
				std::stringstream Error;
				Error << "Error checking if client is joined to a channel, channel name \"" << ChannelName << "\" was not found on server.";
				CreateError(Error.str().c_str());
				return false;
			}
			SelectedChannel = *SelectedChannel2;
		}

		auto& channelClients = SelectedChannel->getclients();
		auto& clientChannels = SelectedClient->getchannels();

		return std::find(clientChannels.cbegin(), clientChannels.cend(), SelectedChannel) != clientChannels.cend() ||
			std::find(channelClients.cbegin(), channelClients.cend(), SelectedClient) != channelClients.cend();
	}
	return false;
}
bool Extension::IsClientOnChannel_ID(int ClientID, char * ChannelName)
{
	if (ClientID < 0 || ClientID > 0xFFFF)
	{
		std::stringstream Error;
		Error << "Error checking if client is joined to a channel, client ID " << ClientID << " is not between total ID range of 0-65535.";
		CreateError(Error.str().c_str());
	}
	else if (ChannelName[0] == '\0' && !ThreadData.Channel)
		CreateError("Error checking if client is joined to a channel, channel name supplied was blank and no channel pre-selected.");
	else
	{
		ClientCopy * SelectedClient = nullptr;
		{		
			auto SelectedClient2 =
				std::find_if(Clients.cbegin(), Clients.cend(), [&](ClientCopy * const & copy) {
				return copy->id() == ClientID; });
			if (SelectedClient2 == Clients.cend())
			{
				std::stringstream Error;
				Error << "Error checking if client is joined to a channel, client ID " << ClientID << " was not found on server.";
				CreateError(Error.str().c_str());
				return false;
			}
			SelectedClient = *SelectedClient2;
		}

		auto SelectedChannel = ThreadData.Channel;
		if (ChannelName[0])
		{
			auto SelectedChannel2 =
				std::find_if(Channels.cbegin(), Channels.cend(), [&](ChannelCopy * const & copy) {
				return !_stricmp(copy->name(), ChannelName); });
			if (SelectedChannel2 == Channels.cend())
			{
				std::stringstream Error;
				Error << "Error checking if client is joined to a channel, channel name \"" << ChannelName << "\" was not found on server.";
				CreateError(Error.str().c_str());
				return false;
			}
			SelectedChannel = *SelectedChannel2;
		}

		auto& channelClients = SelectedChannel->getclients();
		auto& clientChannels = SelectedClient->getchannels();

		return std::find(clientChannels.cbegin(), clientChannels.cend(), SelectedChannel) != clientChannels.cend() ||
			std::find(channelClients.cbegin(), channelClients.cend(), SelectedClient) != channelClients.cend();
	}
	return false;
}

bool Extension::OnAllChannelsLoopWithName(char * LoopName)
{
	LoopNameMatches("All-Channel Loop With Name");
}
bool Extension::OnAllChannelsLoopWithNameFinished(char * LoopName)
{
	LoopNameMatches("All-Channel Loop With Name Finished");
}

bool Extension::OnChannelClientsLoopWithName(char * LoopName)
{
	LoopNameMatches("Channel's Client Loop With Name");
}
bool Extension::OnChannelClientsLoopWithNameFinished(char * LoopName)
{
	LoopNameMatches("Channel's Client Loop With Name Finished");
}

bool Extension::IsFlashPolicyServerHosting()
{
	return FlashSrv->hosting();
}

bool Extension::ChannelIsHiddenFromChannelList()
{
	return ThreadData.Channel ? ThreadData.Channel->hidden() : false;
}

bool Extension::ChannelIsSetToCloseAutomatically()
{
	return ThreadData.Channel ? ThreadData.Channel->autocloseenabled() : false;
}
bool Extension::OnAllClientsLoopWithNameFinished(char * LoopName)
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

