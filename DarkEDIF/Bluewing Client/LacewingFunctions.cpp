// Handles all Lacewing functions.
#include "Common.h"

#define Ext (*((GlobalInfo *) Client.tag)->_Ext)
#define Saved (((GlobalInfo *) Client.tag)->_Saved)
#define Globals ((GlobalInfo *) Client.tag)

void OnError(lacewing::relayclient &Client, lacewing::error Error)
{
	EnterCriticalSectionDerpy(&Globals->Lock);

	SaveExtInfo &S = Globals->AddEvent(0);
	S.Error.Text = _strdup(Error->tostring());

	if (!S.Error.Text)
	{
		Globals->CreateError("Error copying Lacewing error string to local buffer.");
		Saved.erase(Saved.end()); // Remove S from vector
	}
	LeaveCriticalSectionDerpy(&Globals->Lock);
}
void OnConnect(lacewing::relayclient &Client)
{
	Globals->AddEvent(1);
}
void OnConnectDenied(lacewing::relayclient &Client, const char * DenyReason)
{
	// Old deny reason? Free it.
	if (DenyReasonBuffer)
		free(DenyReasonBuffer);

	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		Globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
	Globals->AddEvent(2);
}
void OnDisconnect(lacewing::relayclient &Client)
{
	// Pass disconnect event
	Globals->AddEvent(3);

	// Empty all channels and peers
	Globals->AddEvent(0xFFFF);
}
void OnChannelListReceived(lacewing::relayclient &Client)
{
	Globals->AddEvent(26);
}
void OnJoinChannel(lacewing::relayclient &Client, lacewing::relayclient::channel &Target)
{
	Target.isclosed = false;
	
	for (auto i = Target.firstpeer(); i != nullptr; i = i->next())
		i->isclosed = false;

	EnterCriticalSectionDerpy(&Globals->Lock);

	SaveExtInfo &S = Globals->AddEvent(4);
	S.Channel = &Target;
	LeaveCriticalSectionDerpy(&Globals->Lock);
}
void OnJoinChannelDenied(lacewing::relayclient &Client, const char * ChannelName, const char * DenyReason)
{
	EnterCriticalSectionDerpy(&Globals->Lock);

	SaveExtInfo &S = Globals->AddEvent(5);
	S.Loop.Name = _strdup(ChannelName);
	LeaveCriticalSectionDerpy(&Globals->Lock);
	
	// Old deny reason? Free it.
	if (DenyReasonBuffer)
		free(DenyReasonBuffer);
	
	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		Globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
}
void OnLeaveChannel(lacewing::relayclient &Client, lacewing::relayclient::channel &Target)
{
	Target.isclosed = true;

	EnterCriticalSectionDerpy(&Globals->Lock);
	SaveExtInfo &S = Globals->AddEvent(43);
	S.Channel = &Target;
	
	// Clear channel copy after this event is handled
	SaveExtInfo &C = Globals->AddEvent(0xFFFF);
	C.Channel = S.Channel;
	LeaveCriticalSectionDerpy(&Globals->Lock);
}
void OnLeaveChannelDenied(lacewing::relayclient &Client, lacewing::relayclient::channel &Target, const char * DenyReason)
{
	EnterCriticalSectionDerpy(&Globals->Lock);
	SaveExtInfo &S = Globals->AddEvent(44);
	S.Channel = &Target;
	LeaveCriticalSectionDerpy(&Globals->Lock);
	
	// Old deny reason? Free it.
	if (DenyReasonBuffer)
		free(DenyReasonBuffer);
	
	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		Globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
}
void OnNameSet(lacewing::relayclient &Client)
{
	Globals->AddEvent(6);
}
void OnNameDenied(lacewing::relayclient &Client, const char * DeniedName, const char * DenyReason)
{
	Globals->AddEvent(7);
	
	// Old deny reason? Free it.
	if (DenyReasonBuffer)
		free(DenyReasonBuffer);
	
	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		Globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
}
void OnNameChanged(lacewing::relayclient &Client, const char * OldName)
{

	Globals->AddEvent(53);

	// Old previous name? Free it.
	if (PreviousName)
		free(PreviousName);

	PreviousName = _strdup(OldName);
	if (!PreviousName)
		Globals->CreateError("Error copying self previous name from Lacewing to local buffer.");
}
void OnPeerConnect(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel, lacewing::relayclient::channel::peer &Peer)
{
	Peer.isclosed = false;

	EnterCriticalSectionDerpy(&Globals->Lock);

	SaveExtInfo &S = Globals->AddEvent(10);
	S.Channel = &Channel;
	S.Peer = &Peer;
	LeaveCriticalSectionDerpy(&Globals->Lock);
}
void OnPeerDisconnect(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel,
	lacewing::relayclient::channel::peer &Peer)
{
	Peer.isclosed = true;

	EnterCriticalSectionDerpy(&Globals->Lock);

	SaveExtInfo &S = Globals->AddEvent(11);
	S.Channel = &Channel;
	S.Peer = &Peer;
	
	// Remove closed peer entirely after above event is handled
	SaveExtInfo &C = Globals->AddEvent(0xFFFF);
	C.Channel = S.Channel;
	C.Peer = S.Peer;
	LeaveCriticalSectionDerpy(&Globals->Lock);

}
void OnPeerNameChanged(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel,
	lacewing::relayclient::channel::peer &Peer, const char * OldName)
{
	EnterCriticalSectionDerpy(&Globals->Lock);
	SaveExtInfo &S = Globals->AddEvent(45);
	S.Channel = &Channel;
	S.Peer = &Peer;

	// Old previous name? Free it
	if (Peer.tag)
		free(Peer.tag);

	// Store new previous name in Tag.
	Peer.tag =_strdup(OldName);
	if (!Peer.tag)
		Globals->CreateError("Error copying old peer name.");
	LeaveCriticalSectionDerpy(&Globals->Lock);
}
void OnPeerMessage(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel,
	lacewing::relayclient::channel::peer &Peer,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	EnterCriticalSectionDerpy(&Globals->Lock);
	SaveExtInfo &S = Globals->AddEvent(Blasted ? 52 : 49);
	S.Channel = &Channel;
	S.Peer = &Peer;
	S.ReceivedMsg.Subchannel = (unsigned char) Subchannel;
	S.ReceivedMsg.Size = Size;

	S.ReceivedMsg.Content = (char *)malloc(Size);
	if (!S.ReceivedMsg.Content)
	{
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	}
	else if (memcpy_s(S.ReceivedMsg.Content, Size, Data, Size))
	{
		Saved.erase(Saved.end());
		free(S.ReceivedMsg.Content);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else 
	{
		if (Blasted)
		{
			// Text
			if (Variant == 0)
				Globals->AddEvent(39, true);
			// Number
			else if (Variant == 1)
				Globals->AddEvent(40, true);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent(41, true);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
		else // Sent
		{
			// Text
			if (Variant == 0)
				Globals->AddEvent(36, true);
			// Number
			else if (Variant == 1)
				Globals->AddEvent(37, true);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent(38, true);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	LeaveCriticalSectionDerpy(&Globals->Lock);
}
void OnChannelMessage(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel,
	lacewing::relayclient::channel::peer &Peer,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	EnterCriticalSectionDerpy(&Globals->Lock);
	SaveExtInfo &S = Globals->AddEvent(Blasted ? 51 : 48);
	S.Channel = &Channel;
	S.Peer = &Peer;
	S.ReceivedMsg.Subchannel = (unsigned char)Subchannel;
	S.ReceivedMsg.Size = Size;

	S.ReceivedMsg.Content = (char *)malloc(Size);
	if (!S.ReceivedMsg.Content)
	{
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	}
	else if (memcpy_s(S.ReceivedMsg.Content, Size, Data, Size))
	{
		Saved.erase(Saved.end());
		free(S.ReceivedMsg.Content);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else
	{
		if (Blasted)
		{
			// Text
			if (Variant == 0)
				Globals->AddEvent(22, true);
			// Number
			else if (Variant == 1)
				Globals->AddEvent(23, true);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent(35, true);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
		else // Sent
		{
			// Text
			if (Variant == 0)
				Globals->AddEvent(9, true);
			// Number
			else if (Variant == 1)
				Globals->AddEvent(16, true);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent(33, true);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	LeaveCriticalSectionDerpy(&Globals->Lock);
}
void OnServerMessage(lacewing::relayclient &Client,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	EnterCriticalSectionDerpy(&Globals->Lock);
	SaveExtInfo &S = Globals->AddEvent(Blasted ? 50 : 47);
	S.ReceivedMsg.Subchannel = (unsigned char)Subchannel;
	S.ReceivedMsg.Size = Size; // Do NOT add null. There's error checking for that.

	S.ReceivedMsg.Content = (char *)malloc(Size);

	if (!S.ReceivedMsg.Content)
	{
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	}
	else if (memcpy_s(S.ReceivedMsg.Content, Size, Data, Size))
	{
		Saved.erase(Saved.end());
		free(S.ReceivedMsg.Content);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else
	{
		if (Blasted)
		{
			// Text
			if (Variant == 0)
				Globals->AddEvent(20, true);
			// Number
			else if (Variant == 1)
				Globals->AddEvent(21, true);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent(34, true);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
		else // Sent
		{
			// Text
			if (Variant == 0)
				Globals->AddEvent(8, true);
			// Number
			else if (Variant == 1)
				Globals->AddEvent(15, true);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent(32, true);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	LeaveCriticalSectionDerpy(&Globals->Lock);
}
void OnServerChannelMessage(lacewing::relayclient &Client, lacewing::relayclient::channel &Channel,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	EnterCriticalSectionDerpy(&Globals->Lock);
	SaveExtInfo &S = Globals->AddEvent(Blasted ? 72 : 68);
	S.Channel = &Channel;
	S.ReceivedMsg.Subchannel = (unsigned char) Subchannel;
	S.ReceivedMsg.Size = Size;

	S.ReceivedMsg.Content = (char *)malloc(Size);
	if (!S.ReceivedMsg.Content)
	{
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	}
	else if (memcpy_s(S.ReceivedMsg.Content, Size, Data, Size))
	{
		Saved.erase(Saved.end());
		free(S.ReceivedMsg.Content);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else 
	{
		if (Blasted)
		{
			// Text
			if (Variant == 0)
				Globals->AddEvent(69, true);
			// Number
			else if (Variant == 1)
				Globals->AddEvent(70, true);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent(71, true);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
		else // Sent
		{
			// Text
			if (Variant == 0)
				Globals->AddEvent(65, true);
			// Number
			else if (Variant == 1)
				Globals->AddEvent(66, true);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent(67, true);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	LeaveCriticalSectionDerpy(&Globals->Lock);
}


#undef Ext
#undef Saved
#undef Globals