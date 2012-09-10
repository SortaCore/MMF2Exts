
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
	return Cli.Connected();
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
	return (Cli.Name() && Cli.Name()[0] == '\0');
}

bool Extension::SelectedPeerIsChannelMaster()
{
	return ThreadData.Peer->IsChannelMaster();
}

bool Extension::YouAreChannelMaster()
{
	return ThreadData.Channel->IsChannelMaster();
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
		Lacewing::RelayClient::Channel * C = Cli.FirstChannel();
		while (C)
		{
			if (!_stricmp(ChannelName, C->Name()))
				return true;
			C = C->Next();
		}
	}
	return false;
}

bool Extension::IsPeerOnChannel_Name(char * PeerName, char * ChannelName)
{
	if (PeerName[0] == '\0')
		CreateError("Error checking if peer is joined to a channel, peer name supplied was blank.");
	else if (ChannelName[0] != '\0')
	{
		Lacewing::RelayClient::Channel * C = Cli.FirstChannel();
		while (C)
		{
			if (!_stricmp(ChannelName, C->Name()))
			{
				Lacewing::RelayClient::Channel::Peer * P = C->FirstPeer();
				while (P)
				{
					if (!_stricmp(PeerName, P->Name()))
						return true;
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
		Lacewing::RelayClient::Channel::Peer * P = ThreadData.Channel->FirstPeer();
		while (P)
		{
			if (!_stricmp(P->Name(), PeerName))
				return true;
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

bool Extension::IsPeerOnChannel_ID(int PeerID, char * ChannelName)
{
	if (ChannelName[0] != '\0')
	{
		Lacewing::RelayClient::Channel * C = Cli.FirstChannel();
		while (C)
		{
			if (!_stricmp(ChannelName, C->Name()))
			{
				Lacewing::RelayClient::Channel::Peer * P = C->FirstPeer();
				while (P)
				{
					if (P->ID() == PeerID)
						return true;
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
		Lacewing::RelayClient::Channel::Peer * P = ThreadData.Channel->FirstPeer();
		while (P)
		{
			if (P->ID() == PeerID)
				return true;
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
