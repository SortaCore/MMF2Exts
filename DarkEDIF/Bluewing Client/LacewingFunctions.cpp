// Handles all Lacewing functions.
#include "Common.h"

#define Ext (*((GlobalInfo *) Client.tag)->_Ext)
#define Saved (((GlobalInfo *) Client.tag)->_Saved)
#define Globals ((GlobalInfo *) Client.tag)

void OnError(lacewing::relayclient &Client, lacewing::error Error)
{
	EnterCriticalSectionDerpy(&Globals->Lock);

	char * c = _strdup(Error->tostring());
	if (c)
		Globals->AddEvent1(0, nullptr, nullptr, c);
	else
		Globals->AddEvent1(0, nullptr, nullptr, "Error copying Lacewing error string to local buffer.");
	
}
void OnConnect(lacewing::relayclient &Client)
{
	Globals->AddEvent1(1);
}
void OnConnectDenied(lacewing::relayclient &Client, const char * DenyReason)
{
	// Old deny reason? Free it.
	free(DenyReasonBuffer);

	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		Globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
	Globals->AddEvent1(2);
}
void OnDisconnect(lacewing::relayclient &Client)
{
	// Pass disconnect event,
	// 0xFFFF: Empty all channels and peers
	Globals->AddEvent2(3, 0xFFFF);
}
void OnChannelListReceived(lacewing::relayclient &Client)
{
	Globals->AddEvent1(26);
}
void OnJoinChannel(lacewing::relayclient &Client, lacewing::relayclient::channel &Target)
{
	Target.isclosed = false;
	
	for (auto i = Target.firstpeer(); i != nullptr; i = i->next())
		i->isclosed = false;

	Globals->AddEvent1(4, &Target);
}
void OnJoinChannelDenied(lacewing::relayclient &Client, const char * ChannelName, const char * DenyReason)
{
	// Old deny reason? Free it.
	free(DenyReasonBuffer);

	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		Globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
	Globals->AddEvent1(5, nullptr, nullptr, _strdup(ChannelName));
}
void OnLeaveChannel(lacewing::relayclient &Client, lacewing::relayclient::channel &Target)
{
	Target.isclosed = true;

	// 0xFFFF: Clear channel copy after this event is handled
	Globals->AddEvent2(43, 0xFFFF, &Target);
}
void OnLeaveChannelDenied(lacewing::relayclient &Client, lacewing::relayclient::channel &Target, const char * DenyReason)
{
	// Old deny reason? Free it.
	free(DenyReasonBuffer);

	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		Globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
	Globals->AddEvent1(44, &Target);
}
void OnNameSet(lacewing::relayclient &Client)
{
	Globals->AddEvent1(6);
}
void OnNameDenied(lacewing::relayclient &Client, const char * DeniedName, const char * DenyReason)
{
	// Old deny reason? Free it.
	free(DenyReasonBuffer);

	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		Globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
	Globals->AddEvent1(7);
}
void OnNameChanged(lacewing::relayclient &Client, const char * OldName)
{
	// Old name? Free it.
	free(PreviousName);

	PreviousName = _strdup(OldName);
	if (!PreviousName)
		Globals->CreateError("Error copying self previous name from Lacewing to local buffer.");
	Globals->AddEvent1(53);
}
void OnPeerConnect(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel, lacewing::relayclient::channel::peer &Peer)
{
	Peer.isclosed = false;

	Globals->AddEvent1(10, &Channel, &Peer);
}
void OnPeerDisconnect(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel,
	lacewing::relayclient::channel::peer &Peer)
{
	Peer.isclosed = true;

	// 0xFFFF: Remove closed peer entirely after regular event handled
	Globals->AddEvent2(11, 0xFFFF, &Channel, &Peer);
}
void OnPeerNameChanged(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel,
	lacewing::relayclient::channel::peer &Peer, const char * OldName)
{
	Globals->AddEvent1(45, &Channel, &Peer);

	// Old previous name? Free it
	free(Peer.tag);

	// Store new previous name in Tag.
	Peer.tag =_strdup(OldName);
	if (!Peer.tag)
		Globals->CreateError("Error copying old peer name.");
}
void OnPeerMessage(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel,
	lacewing::relayclient::channel::peer &Peer,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	char * Dup = (char *)malloc(Size);
	if (!Dup)
	{
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
		return;
	}
	if (memcpy_s(Dup, Size, Data, Size))
	{
		free(Dup);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
		return;
	}
	if (Blasted)
	{
		// Text
		if (Variant == 0)
			Globals->AddEvent2(52, 39, &Channel, &Peer, Dup, Size, Subchannel);
		// Number
		else if (Variant == 1)
			Globals->AddEvent2(52, 40, &Channel, &Peer, Dup, Size, Subchannel);
		// Binary
		else if (Variant == 2)
			Globals->AddEvent2(52, 41, &Channel, &Peer, Dup, Size, Subchannel);
		// ???
		else
		{
			free(Dup);
			Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	else // Sent
	{
		// Text
		if (Variant == 0)
			Globals->AddEvent2(49, 36, &Channel, &Peer, Dup, Size, Subchannel);
		// Number
		else if (Variant == 1)
			Globals->AddEvent2(49, 37, &Channel, &Peer, Dup, Size, Subchannel);
		// Binary
		else if (Variant == 2)
			Globals->AddEvent2(49, 38, &Channel, &Peer, Dup, Size, Subchannel);
		// ???
		else
		{
			free(Dup);
			Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}
void OnChannelMessage(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel,
	lacewing::relayclient::channel::peer &Peer,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	char * Dup = (char *)malloc(Size);
	if (!Dup)
	{
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
		return;
	}
	if (memcpy_s(Dup, Size, Data, Size))
	{
		free(Dup);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
		return;
	}
	if (Blasted)
	{
		// Text
		if (Variant == 0)
			Globals->AddEvent2(51, 22, &Channel, &Peer, Dup, Size, Subchannel);
		// Number
		else if (Variant == 1)
			Globals->AddEvent2(51, 23, &Channel, &Peer, Dup, Size, Subchannel);
		// Binary
		else if (Variant == 2)
			Globals->AddEvent2(51, 35, &Channel, &Peer, Dup, Size, Subchannel);
		// ???
		else
		{
			free(Dup);
			Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	else // Sent
	{
		// Text
		if (Variant == 0)
			Globals->AddEvent2(48, 9, &Channel, &Peer, Dup, Size, Subchannel);
		// Number
		else if (Variant == 1)
			Globals->AddEvent2(48, 16, &Channel, &Peer, Dup, Size, Subchannel);
		// Binary
		else if (Variant == 2)
			Globals->AddEvent2(48, 33, &Channel, &Peer, Dup, Size, Subchannel);
		// ???
		else
		{
			free(Dup);
			Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}
void OnServerMessage(lacewing::relayclient &Client,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	char * Dup = (char *)malloc(Size);
	if (!Dup)
	{
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
		return;
	}
	if (memcpy_s(Dup, Size, Data, Size))
	{
		free(Dup);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
		return;
	}
	if (Blasted)
	{
		// Text
		if (Variant == 0)
			Globals->AddEvent2(50, 20, nullptr, nullptr, Dup, Size, Subchannel);
		// Number
		else if (Variant == 1)
			Globals->AddEvent2(50, 21, nullptr, nullptr, Dup, Size, Subchannel);
		// Binary
		else if (Variant == 2)
			Globals->AddEvent2(50, 34, nullptr, nullptr, Dup, Size, Subchannel);
		// ???
		else
		{
			free(Dup);
			Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	else // Sent
	{
		// Text
		if (Variant == 0)
			Globals->AddEvent2(47, 8, nullptr, nullptr, Dup, Size, Subchannel);
		// Number
		else if (Variant == 1)
			Globals->AddEvent2(47, 15, nullptr, nullptr, Dup, Size, Subchannel);
		// Binary
		else if (Variant == 2)
			Globals->AddEvent2(47, 32, nullptr, nullptr, Dup, Size, Subchannel);
		// ???
		else
		{
			free(Dup);
			Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}
void OnServerChannelMessage(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	char * Dup = (char *)malloc(Size);
	if (!Dup)
	{
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
		return;
	}
	if (memcpy_s(Dup, Size, Data, Size))
	{
		free(Dup);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
		return;
	}
	if (Blasted)
	{
		// Text
		if (Variant == 0)
			Globals->AddEvent2(72, 69, &Channel, nullptr, Dup, Size, Subchannel);
		// Number
		else if (Variant == 1)
			Globals->AddEvent2(72, 70, &Channel, nullptr, Dup, Size, Subchannel);
		// Binary
		else if (Variant == 2)
			Globals->AddEvent2(72, 71, &Channel, nullptr, Dup, Size, Subchannel);
		// ???
		else
		{
			free(Dup);
			Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	else // Sent
	{
		// Text
		if (Variant == 0)
			Globals->AddEvent2(68, 65, &Channel, nullptr, Dup, Size, Subchannel);
		// Number
		else if (Variant == 1)
			Globals->AddEvent2(68, 66, &Channel, nullptr, Dup, Size, Subchannel);
		// Binary
		else if (Variant == 2)
			Globals->AddEvent2(68, 67, &Channel, nullptr, Dup, Size, Subchannel);
		// ???
		else
		{
			free(Dup);
			Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}


#undef Ext
#undef Saved
#undef Globals