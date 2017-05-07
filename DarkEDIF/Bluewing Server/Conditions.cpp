
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
bool Extension::OnAllClientsLoopWithNameFinished(char * LoopName)
{
	LoopNameMatches("Peer Loop With Name Finished");
}
bool Extension::OnClientsJoinedChannelLoopWithName(char * LoopName)
{
	LoopNameMatches("Client's Joined Channel Loop With Name");
}
bool Extension::OnClientsJoinedChannelLoopWithNameFinished(char * LoopName)
{
	LoopNameMatches("Client's Joined Channel Loop With Name Finished");
}
bool Extension::IsClientOnChannel_Name(char * PeerName, char * ChannelName)
{
	if (PeerName[0] == '\0')
		CreateError("Error checking if peer is joined to a channel, peer name supplied was blank.");
	else if (ChannelName[0] != '\0')
	{
		lacewing::relayserver::channel * C = Srv.firstchannel();
		while (C)
		{
			if (!C->isclosed && !_stricmp(ChannelName, C->name()))
			{
				lacewing::relayserver::client * P = C->firstclient();
				while (P)
				{
					if (!_stricmp(PeerName, P->name()))
						return !P->isclosed;
					P = P->next();
				}
				return false;
			}
			C = C->next();
		}
		
		CreateError("Error checking if peer is joined to a channel; not connected to channel supplied.");
	}
	else if (ThreadData.Channel) // Use currently selected channel
	{
		lacewing::relayserver::client * P = ThreadData.Channel->firstclient();
		while (P)
		{
			if (!_stricmp(P->name(), PeerName))
				return !P->isclosed;
			P = P->next();
		}
		return false;
	}
	else // No currently selected channel!
	{
		CreateError("Error checking if peer is joined to a channel; no channel selected, and no channel name supplied.");
	}
	return false;
}
bool Extension::IsClientOnChannel_ID(int ClientID, char * ChannelName)
{
	if (ChannelName[0] != '\0')
	{
		lacewing::relayserver::channel * C = Srv.firstchannel();
		while (C)
		{
			if (!C->isclosed && !_stricmp(ChannelName, C->name()))
			{
				lacewing::relayserver::client * P = C->firstclient();
				while (P)
				{
					if (P->id() == ClientID)
						return !P->isclosed;
					P = P->next();
				}
				return false;
			}
			C = C->next();
		}
		
		CreateError("Error checking if peer is joined to a channel; not connected to channel supplied.");
	}
	else if (ThreadData.Channel)// Use currently selected channel
	{
		lacewing::relayserver::client * P = ThreadData.Channel->firstclient();
		while (P)
		{
			if (P->id() == ClientID)
				return !P->isclosed;
			P = P->next();
		}
		return false;
	}
	else // No currently selected channel!
	{
		CreateError("Error checking if peer is joined to a channel; no channel selected, and no channel name supplied.");
	}
	return false;
}
