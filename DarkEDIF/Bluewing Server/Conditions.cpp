
#include "Common.h"
#define MessageMatches() (ThreadData.ReceivedMsg.Subchannel == Subchannel || Subchannel == -1)

#define LoopNameMatches(cond) \
	if (!LoopName || LoopName[0] == '\0') \
	{ \
		CreateError("Cannot detect condition "#cond": invalid loop name supplied."); \
		return false; \
	} \
	return !strcmp(ThreadData.Loop.Name, LoopName)

bool Extension::IsLacewingServerHosting()
{
	return Srv.Hosting();
}
bool Extension::OnSentTextMessageToChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentNumberMessageToChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentBinaryMessageToChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedTextMessageToChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedNumberMessageToChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedBinaryMessageToChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentTextMessageToPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentNumberMessageToPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnSentBinaryMessageToPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedTextMessageToPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedNumberMessageToPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnBlastedBinaryMessageToPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnySentMessageToChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnySentMessageToPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedMessageToServer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedMessageToChannel(int Subchannel)
{
	return MessageMatches();
}
bool Extension::OnAnyBlastedMessageToPeer(int Subchannel)
{
	return MessageMatches();
}
bool Extension::Client_IsChannelMaster()
{
	if (!ThreadData.Channel || !ThreadData.Client)
	{
		CreateError("Error, You Are Channel Master condition called without valid channel being selected.");
		return false;
	}

	return ThreadData.Channel->ChannelMaster() == ThreadData.Client ;
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
		Lacewing::RelayServer::Channel * C = Srv.FirstChannel();
		while (C)
		{
			if (!C->IsClosed && !_stricmp(ChannelName, C->Name()))
			{
				Lacewing::RelayServer::Client * P = C->FirstClient();
				while (P)
				{
					if (!_stricmp(PeerName, P->Name()))
						return !P->IsClosed;
					P = P->Next();
				}
				return false;
			}
			C = C->Next();
		}
		
		CreateError("Error checking if peer is joined to a channel; not connected to channel supplied.");
	}
	else if (ThreadData.Channel) // Use currently selected channel
	{
		Lacewing::RelayServer::Client * P = ThreadData.Channel->FirstClient();
		while (P)
		{
			if (!_stricmp(P->Name(), PeerName))
				return !P->IsClosed;
			P = P->Next();
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
		Lacewing::RelayServer::Channel * C = Srv.FirstChannel();
		while (C)
		{
			if (!C->IsClosed && !_stricmp(ChannelName, C->Name()))
			{
				Lacewing::RelayServer::Client * P = C->FirstClient();
				while (P)
				{
					if (P->ID() == ClientID)
						return !P->IsClosed;
					P = P->Next();
				}
				return false;
			}
			C = C->Next();
		}
		
		CreateError("Error checking if peer is joined to a channel; not connected to channel supplied.");
	}
	else if (ThreadData.Channel)// Use currently selected channel
	{
		Lacewing::RelayServer::Client * P = ThreadData.Channel->FirstClient();
		while (P)
		{
			if (P->ID() == ClientID)
				return !P->IsClosed;
			P = P->Next();
		}
		return false;
	}
	else // No currently selected channel!
	{
		CreateError("Error checking if peer is joined to a channel; no channel selected, and no channel name supplied.");
	}
	return false;
}
