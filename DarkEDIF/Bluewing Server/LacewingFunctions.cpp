// Handles all Lacewing functions.
#include "Common.h"

#define Ext (*((Extension *) Server.tag))

void OnError(lacewing::relayserver &Server, lacewing::error Error)
{
	SaveExtInfo &S = Ext.AddEvent(0);
	S.Error.Text = _strdup(Error->tostring());

	if (!S.Error.Text)
	{
		Ext.CreateError("Error copying Lacewing error string to local buffer.");
		Ext.Saved.erase(Ext.Saved.end()); // Remove S from vector
	}
}
void OnClientConnectRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client)
{
	Client.isclosed = false;
	SaveExtInfo &S = Ext.AddEvent(1);
	S.Client = &Client;
}
void OnClientDisconnect(lacewing::relayserver &Server, lacewing::relayserver::client &Client)
{
	Client.isclosed = true;

	SaveExtInfo &S = Ext.AddEvent(3);
	S.Client = &Client;
	S.Channel = NULL;
}
void OnJoinChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::channel &Channel, lacewing::relayserver::client &Client)
{
	Channel.isclosed = false;
	// TODO: this is pointless.
	for (lacewing::relayserver::client * i = Channel.firstclient(); i != nullptr; i = i->next())
		i->isclosed = false;

	SaveExtInfo &S = Ext.AddEvent(4);
	S.Channel = &Channel;
}
void OnLeaveChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client, lacewing::relayserver::channel &Channel)
{
	// why doubled
	// Clear channel copy after this event is handled
	SaveExtInfo &S = Ext.AddEvent(0x0);
	S.Channel = &Channel;
	S.Client = &Client;
	
	// Clear channel copy after this event is handled
	SaveExtInfo &C = Ext.AddEvent(0xFFFF);
	C.Channel = S.Channel;
}
void OnNameSetRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client, const char * nameRequested)
{
	SaveExtInfo &S = Ext.AddEvent(6);
	S.Client = &Client;
	S.Requested.Name = nameRequested;
}
void OnPeerConnect(lacewing::relayserver &Server, lacewing::relayserver::channel &Channel, lacewing::relayserver::client &Client)
{
	Client.isclosed = false;

	SaveExtInfo &S = Ext.AddEvent(10);
	S.Channel = &Channel;
	S.Client = &Client;
}
void OnPeerDisconnect(lacewing::relayserver &Server, lacewing::relayserver::channel &Channel, lacewing::relayserver::client &Client)
{
	Client.isclosed = true;

	SaveExtInfo &S = Ext.AddEvent(11);
	S.Channel = &Channel;
	S.Client = &Client;
	
	// Remove closed peer entirely after above event is handled
	SaveExtInfo &C = Ext.AddEvent(0xFFFF);
	C.Channel = S.Channel;
	C.Client = S.Client;
}
void OnPeerNameChanged(lacewing::relayserver &Server, lacewing::relayserver::channel &Channel, lacewing::relayserver::client &Client, const char * OldName)
{
	SaveExtInfo &S = Ext.AddEvent(45);
	S.Channel = &Channel;
	S.Client = &Client;

	// Old previous name? Free it
	if (Client.tag)
		free(Client.tag);

	// Store new previous name in Tag.
	Client.tag = _strdup(OldName);
	if (!Client.tag)
		Ext.CreateError("Error copying old peer name.");
}
void OnPeerMessage(lacewing::relayserver &Server, lacewing::relayserver::client &SenderClient, lacewing::relayserver::channel &Channel, 
	lacewing::relayserver::client &ReceivingClient, bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	SaveExtInfo &S = Ext.AddEvent(Blasted ? 52 : 49);
	S.Channel = &Channel;
	S.SenderClient = &SenderClient;
	S.ReceivingClient = &ReceivingClient;
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
void OnChannelMessage(
	lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	lacewing::relayserver::channel &Channel,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	SaveExtInfo &S = Ext.AddEvent(Blasted ? 51 : 48);
	S.Channel = &Channel;
	S.Client = &Client;
	S.ReceivedMsg.Subchannel = (unsigned char)Subchannel;
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
void OnServerMessage(lacewing::relayserver &Server, lacewing::relayserver::client &Client,
					 bool Blasted, int Subchannel, const  char * Data, size_t Size, int Variant)
{
	SaveExtInfo &S = Ext.AddEvent(Blasted ? 50 : 47);
	S.Client = &Client;
	S.ReceivedMsg.Subchannel = (unsigned char)Subchannel;
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