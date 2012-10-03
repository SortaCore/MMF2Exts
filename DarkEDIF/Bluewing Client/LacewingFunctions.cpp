// Handles all Lacewing functions.
#include "Common.h"

#define Ext (*((Extension *) Client.Tag))
void OnChannelListReceived(Lacewing::RelayClient &Client)
{
	Ext.AddEvent(26);
}

void OnChannelMessage(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, Lacewing::RelayClient::Channel::Peer &Peer,
					  bool Blasted, int Subchannel, char * Data, int Size, int Variant)
{
	SaveExtInfo &S = Ext.AddEvent(Blasted ? 51 : 48);
	S.Channel = (Lacewing::RelayClient::Channel *)Channel.Tag;
	S.Peer = &Peer;
	S.ReceivedMsg.Subchannel = (unsigned char) Subchannel;
	S.ReceivedMsg.Size = Size;
	
	S.ReceivedMsg.Content = (char *)malloc(Size);
	if (!S.ReceivedMsg.Content)
	{
		Ext.CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	}
	else if (memcpy_s(S.ReceivedMsg.Content, Size, Data, Size))
	{
		Ext.Saved.erase(Ext.Saved.end());
		free(S.ReceivedMsg.Content);
		Ext.CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else 
	{
		if (Blasted)
		{
			// Text
			if (Variant == 0)
				Ext.AddEvent(22, true);
			// Number
			else if (Variant == 1)
				Ext.AddEvent(23, true);
			// Binary
			else if (Variant == 2)
				Ext.AddEvent(35, true);
			// ???
			else
				Ext.CreateError("Warning: message type is neither binary, number nor text.");
		}
		else // Sent
		{
			// Text
			if (Variant == 0)
				Ext.AddEvent(9, true);
			// Number
			else if (Variant == 1)
				Ext.AddEvent(16, true);
			// Binary
			else if (Variant == 2)
				Ext.AddEvent(33, true);
			// ???
			else
				Ext.CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}

void OnConnect(Lacewing::RelayClient &Client)
{
	Ext.AddEvent(1);
}

void OnConnectDenied(Lacewing::RelayClient &Client, const char * DenyReason)
{
	Ext.AddEvent(2);
}

void OnDisconnect(Lacewing::RelayClient &Client)
{
	SaveExtInfo &S = Ext.AddEvent(3);
	S.Channel = NULL;
	S.Peer = NULL;

	// Empty all channels and peers
	Ext.AddEvent(0xFFFF);
}

void OnError(Lacewing::RelayClient &Client, Lacewing::Error &Error)
{
	SaveExtInfo &S = Ext.AddEvent(0);
	S.Error.Text = _strdup(Error.ToString());
	
	if (!S.Error.Text)
	{
		Ext.CreateError("Error copying Lacewing error string to local buffer.");
		Ext.Saved.erase(Ext.Saved.end()); // Remove S from vector
	}
}

void OnJoinChannel(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Target)
{
	SaveExtInfo &S = Ext.AddEvent(4);
	S.Channel = Ext.DuplicateChannel(Target);
	Target.Tag = S.Channel;
}

void OnJoinChannelDenied(Lacewing::RelayClient &Client, const char * ChannelName, const char * DenyReason)
{
	SaveExtInfo &S = Ext.AddEvent(5);
	S.Loop.Name = _strdup(ChannelName);
	
	// Old deny reason? Free it.
	if (Ext.DenyReasonBuffer)
		free(Ext.DenyReasonBuffer);
	
	Ext.DenyReasonBuffer = _strdup(DenyReason);
	if (!Ext.DenyReasonBuffer)
		Ext.CreateError("Error copying deny reason from Lacewing to local buffer.");
}

void OnLeaveChannel(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Target)
{
	SaveExtInfo &S = Ext.AddEvent(43);
	S.Channel = (Lacewing::RelayClient::Channel *)Target.Tag;
	
	// Clear channel copy after this event is handled
	SaveExtInfo &C = Ext.AddEvent(0xFFFF);
	C.Channel = S.Channel;
}

void OnLeaveChannelDenied(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Target, const char * DenyReason)
{
	SaveExtInfo &S = Ext.AddEvent(44);
	S.Channel = (Lacewing::RelayClient::Channel *)Target.Tag;
	
	// Old deny reason? Free it.
	if (Ext.DenyReasonBuffer)
		free(Ext.DenyReasonBuffer);
	
	Ext.DenyReasonBuffer = _strdup(DenyReason);
	if (!Ext.DenyReasonBuffer)
		Ext.CreateError("Error copying deny reason from Lacewing to local buffer.");
}

void OnNameChanged(Lacewing::RelayClient &Client, const char * OldName)
{
	Ext.AddEvent(53);

	// Old previous name? Free it.
	if (Ext.PreviousName)
		free(Ext.PreviousName);

	Ext.PreviousName = _strdup(OldName);
	if (!Ext.PreviousName)
		Ext.CreateError("Error copying self previous name from Lacewing to local buffer.");
}

void OnNameDenied(Lacewing::RelayClient &Client, const char * DeniedName, const char * DenyReason)
{
	Ext.AddEvent(7);
	
	// Old deny reason? Free it.
	if (Ext.DenyReasonBuffer)
		free(Ext.DenyReasonBuffer);
	
	Ext.DenyReasonBuffer = _strdup(DenyReason);
	if (!Ext.DenyReasonBuffer)
		Ext.CreateError("Error copying deny reason from Lacewing to local buffer.");
}

void OnNameSet(Lacewing::RelayClient &Client)
{
	SaveExtInfo &S = Ext.AddEvent(6);
}

void OnPeerConnect(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, Lacewing::RelayClient::Channel::Peer &Peer)
{
	SaveExtInfo &S = Ext.AddEvent(10);
	S.Channel = (Lacewing::RelayClient::Channel *)Channel.Tag;

	// The Tag is used as char * to a name for the subpeer, and as a pointer to the real peer in the main peer.
	Peer.Tag = Ext.DuplicatePeer(Peer);
	S.Peer = (Lacewing::RelayClient::Channel::Peer *)Peer.Tag;
}

void OnPeerDisconnect(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, Lacewing::RelayClient::Channel::Peer &Peer)
{
	SaveExtInfo &S = Ext.AddEvent(11);
	S.Channel = (Lacewing::RelayClient::Channel *)Channel.Tag;
	S.Peer = (Lacewing::RelayClient::Channel::Peer *)Peer.Tag;
	
	// Clear peer copy after this event is handled
	SaveExtInfo &C = Ext.AddEvent(0xFFFF);
	C.Channel = S.Channel;
	C.Peer = S.Peer;
}

void OnPeerMessage(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, Lacewing::RelayClient::Channel::Peer &Peer,
				   bool Blasted, int Subchannel, char * Data, int Size, int Variant)
{
	SaveExtInfo &S = Ext.AddEvent(Blasted ? 52 : 49);
	S.Channel = (Lacewing::RelayClient::Channel *)Channel.Tag;
	S.Peer = (Lacewing::RelayClient::Channel::Peer *)Peer.Tag;
	S.ReceivedMsg.Subchannel = (unsigned char) Subchannel;
	S.ReceivedMsg.Size = Size;

	S.ReceivedMsg.Content = (char *)malloc(Size);
	if (!S.ReceivedMsg.Content)
	{
		Ext.CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	}
	else if (memcpy_s(S.ReceivedMsg.Content, Size, Data, Size))
	{
		Ext.Saved.erase(Ext.Saved.end());
		free(S.ReceivedMsg.Content);
		Ext.CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else 
	{
		if (Blasted)
		{
			// Text
			if (Variant == 0)
				Ext.AddEvent(39, true);
			// Number
			else if (Variant == 1)
				Ext.AddEvent(40, true);
			// Binary
			else if (Variant == 2)
				Ext.AddEvent(41, true);
			// ???
			else
				Ext.CreateError("Warning: message type is neither binary, number nor text.");
		}
		else // Sent
		{
			// Text
			if (Variant == 0)
				Ext.AddEvent(36, true);
			// Number
			else if (Variant == 1)
				Ext.AddEvent(37, true);
			// Binary
			else if (Variant == 2)
				Ext.AddEvent(38, true);
			// ???
			else
				Ext.CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}

void OnPeerNameChanged(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, Lacewing::RelayClient::Channel::Peer &Peer, const char * OldName)
{
	SaveExtInfo &S = Ext.AddEvent(45);
	S.Channel = (Lacewing::RelayClient::Channel *)Channel.Tag;
	S.Peer = (Lacewing::RelayClient::Channel::Peer *)Peer.Tag;

	// Old previous name? Free it
	if (Peer.Tag)
		free(Peer.Tag);

	// Store new previous name in Tag.
	Peer.Tag = _strdup(OldName);
	if (!Peer.Tag)
		Ext.CreateError("Error copying old peer name.");
}

void OnServerChannelMessage(Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel,
							bool Blasted, int Subchannel, char * Data, int Size, int Variant)
{
	SaveExtInfo &S = Ext.AddEvent(Blasted ? 72 : 68);
	S.Channel = (Lacewing::RelayClient::Channel *)Channel.Tag;
	S.ReceivedMsg.Subchannel = (unsigned char) Subchannel;
	S.ReceivedMsg.Size = Size;

	S.ReceivedMsg.Content = (char *)malloc(Size);
	if (!S.ReceivedMsg.Content)
	{
		Ext.CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	}
	else if (memcpy_s(S.ReceivedMsg.Content, Size, Data, Size))
	{
		Ext.Saved.erase(Ext.Saved.end());
		free(S.ReceivedMsg.Content);
		Ext.CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else 
	{
		if (Blasted)
		{
			// Text
			if (Variant == 0)
				Ext.AddEvent(69, true);
			// Number
			else if (Variant == 1)
				Ext.AddEvent(70, true);
			// Binary
			else if (Variant == 2)
				Ext.AddEvent(71, true);
			// ???
			else
				Ext.CreateError("Warning: message type is neither binary, number nor text.");
		}
		else // Sent
		{
			// Text
			if (Variant == 0)
				Ext.AddEvent(65, true);
			// Number
			else if (Variant == 1)
				Ext.AddEvent(66, true);
			// Binary
			else if (Variant == 2)
				Ext.AddEvent(67, true);
			// ???
			else
				Ext.CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}

void OnServerMessage(Lacewing::RelayClient &Client,
					 bool Blasted, int Subchannel, char * Data, int Size, int Variant)
{
	SaveExtInfo &S = Ext.AddEvent(Blasted ? 50 : 47);
	S.ReceivedMsg.Subchannel = (unsigned char) Subchannel;
	S.ReceivedMsg.Size = Size; // Do NOT add null. There's error checking for that.

	S.ReceivedMsg.Content = (char *)malloc(Size);
	
	if (!S.ReceivedMsg.Content)
	{
		Ext.CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	}
	else if (memcpy_s(S.ReceivedMsg.Content, Size, Data, Size))
	{
		Ext.Saved.erase(Ext.Saved.end());
		free(S.ReceivedMsg.Content);
		Ext.CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else 
	{
		if (Blasted)
		{
			// Text
			if (Variant == 0)
				Ext.AddEvent(20, true);
			// Number
			else if (Variant == 1)
				Ext.AddEvent(21, true);
			// Binary
			else if (Variant == 2)
				Ext.AddEvent(34, true);
			// ???
			else
				Ext.CreateError("Warning: message type is neither binary, number nor text.");
		}
		else // Sent
		{
			// Text
			if (Variant == 0)
				Ext.AddEvent(8, true);
			// Number
			else if (Variant == 1)
				Ext.AddEvent(15, true);
			// Binary
			else if (Variant == 2)
				Ext.AddEvent(32, true);
			// ???
			else
				Ext.CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}

#undef Ext