// Handles all Lacewing functions.
#include "Common.h"

#define Ext (*((GlobalInfo *) Client.tag)->_ext)
#define globals ((GlobalInfo *) Client.tag)
#define GThread globals->_thread

void OnError(lacewing::relayclient &Client, lacewing::error error)
{
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	char * c = _strdup(error->tostring());
	if (c)
		globals->AddEvent1(0, nullptr, nullptr, c);
	else
		globals->AddEvent1(0, nullptr, nullptr, "Error copying Lacewing error string to local buffer.");

	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
}
void OnConnect(lacewing::relayclient &Client)
{
	lacewing::address addr = Client.serveraddress();
	char ipAddr[64];
	lw_addr_prettystring(addr->tostring(), ipAddr, sizeof(ipAddr));
	HostIP = ipAddr;
	globals->AddEvent1(1);
}
void OnConnectDenied(lacewing::relayclient &Client, const char * DenyReason)
{
	// On Connect is not called during TCP Connect but Connect Response message.
	// Ditto for Connect Denied. The serveraddress() is set during TCP Connect, so it should be valid here.
	lacewing::address addr = Client.serveraddress();
	char ipAddr[64];
	lw_addr_prettystring(addr->tostring(), ipAddr, sizeof(ipAddr));
	HostIP = ipAddr;

	// Old deny reason? Free it.
	free(DenyReasonBuffer);

	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
	globals->AddEvent1(2); // no 0xFFFF; Disconnect should be called separately
}
void OnDisconnect(lacewing::relayclient &Client)
{
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	// Close all channels/peers in our copy
	for (auto j : Channels)
		j->close();

	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);

	// 0xFFFF: Empty all channels and peers, and reset HostIP
	globals->AddEvent2(3, 0xFFFF);
}
void OnChannelListReceived(lacewing::relayclient &Client)
{
	globals->AddEvent1(26);
}
void OnJoinChannel(lacewing::relayclient &Client, lacewing::relayclient::channel &Target)
{
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	ChannelCopy * channel = new ChannelCopy(&Target);
	Channels.push_back(channel);
#if 0
	// Autoselect the first channel?
	if (Channels.size() == 1U)
		Ext.threadData.channel = channel;
#endif

	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
	
	globals->AddEvent1(4, channel);
}
void OnJoinChannelDenied(lacewing::relayclient &Client, const char * ChannelName, const char * DenyReason)
{
	// Old deny reason? Free it.
	free(DenyReasonBuffer);

	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
	globals->AddEvent1(5, nullptr, nullptr, _strdup(ChannelName));
}
void OnLeaveChannel(lacewing::relayclient &Client, lacewing::relayclient::channel &Target)
{
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	// Close client in our copy
	for (auto j : Channels)
	{
		if (j->id() == Target.id())
		{
			j->close();

			// 0xFFFF: Clear channel copy after this event is handled
			globals->AddEvent2(43, 0xFFFF, j);

			if (GThread)
				LeaveCriticalSectionDebug(&globals->lock);
			return;
		}
	}
	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
	globals->CreateError("Couldn't find channel copy.");
}
void OnLeaveChannelDenied(lacewing::relayclient &Client, lacewing::relayclient::channel &Target, const char * DenyReason)
{
	// Old deny reason? Free it.
	free(DenyReasonBuffer);

	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
	globals->AddEvent1(44, &Target);
}
void OnNameSet(lacewing::relayclient &Client)
{
	globals->AddEvent1(6);
}
void OnNameDenied(lacewing::relayclient &Client, const char * DeniedName, const char * DenyReason)
{
	// Old deny reason? Free it.
	free(DenyReasonBuffer);

	DenyReasonBuffer = _strdup(DenyReason);
	if (!DenyReasonBuffer)
		globals->CreateError("Error copying deny reason from Lacewing to local buffer.");
	globals->AddEvent1(7);
}
void OnNameChanged(lacewing::relayclient &Client, const char * OldName)
{
	// Old name? Free it.
	free(PreviousName);

	PreviousName = _strdup(OldName);
	if (!PreviousName)
		globals->CreateError("Error copying self previous name from Lacewing to local buffer.");
	globals->AddEvent1(53);
}
void OnPeerConnect(lacewing::relayclient &Client, lacewing::relayclient::channel &channel, lacewing::relayclient::channel::peer &peer)
{
	// Add peer to our copy
	for (auto j : Channels)
	{
		if (j->id() == channel.id())
		{
			PeerCopy * PeerCopy = j->addpeer(&peer);
			if (PeerCopy)
				globals->AddEvent1(10, j, PeerCopy);
			return;
		}
	}

	globals->CreateError("Couldn't find peer copy.");
}
void OnPeerDisconnect(lacewing::relayclient &Client, lacewing::relayclient::channel &channel,
	lacewing::relayclient::channel::peer &peer)
{
	// Disconnect makes write impossible. To prevent Fusion being halfway through writing on another thread,
	// lock the event loop.
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	// Close peer in our copy.
	// Original peer will be deleted after OnPeerDisconnect (this func) ends.
	// Then once 0xFFFF triggers, copy will be deleted too.
	for (auto j : Channels)
	{
		if (j->id() == channel.id())
		{
			PeerCopy * PeerCopy = j->closepeer(peer);

			if (GThread)
				LeaveCriticalSectionDebug(&globals->lock);
		
			if (PeerCopy)
				globals->AddEvent2(11, 0xFFFF, j, PeerCopy);
			return;
		}
	}

	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);

	globals->CreateError("Couldn't find copy of channel for peer disconnect.");
}
void OnPeerNameChanged(lacewing::relayclient &Client, lacewing::relayclient::channel &channel,
	lacewing::relayclient::channel::peer &peer, const char * OldName)
{
	for (auto j : Channels)
	{
		if (j->id() == channel.id())
		{
			PeerCopy * k = j->updatepeername(peer);
			if (k)
				globals->AddEvent1(45, j, k);
			else
				globals->CreateError("Couldn't find copy of peer on copy of channel for peer name change.");
			return;
		}
	}

	globals->CreateError("Couldn't find copy of channel for peer name change.");
	return;
}
void OnPeerMessage(lacewing::relayclient &Client, lacewing::relayclient::channel &channel,
	lacewing::relayclient::channel::peer &peer,
	bool Blasted, int subchannel, const char * Data, size_t size, int Variant)
{
	auto cc = std::find_if(Channels.begin(), Channels.end(), [&](ChannelCopy *&c) {
		return &c->orig() == &channel;
	});
	if (cc == Channels.end())
	{
		globals->CreateError("Couldn't find copy of channel for peer message.");
		return;
	}
	ChannelCopy * channelCopy = *cc;
	auto& peers = channelCopy->getpeers();
	auto pp = std::find_if(peers.begin(), peers.end(), [&](PeerCopy * p) {
		return &p->orig() == &peer;
	});
	if (pp == peers.end())
	{
		globals->CreateError("Couldn't find copy of peer on copy of channel for peer message.");
		return;
	}
	PeerCopy * peerCopy = *pp;
	
	char * Dup = (char *)malloc(size);
	if (!Dup)
	{
		globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
		return;
	}
	if (memcpy_s(Dup, size, Data, size))
	{
		free(Dup);
		globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
		return;
	}

	if (Blasted)
	{
		// text
		if (Variant == 0)
			globals->AddEvent2(52, 39, channelCopy, peerCopy, Dup, size, subchannel);
		// Number
		else if (Variant == 1)
			globals->AddEvent2(52, 40, channelCopy, peerCopy, Dup, size, subchannel);
		// Binary
		else if (Variant == 2)
			globals->AddEvent2(52, 41, channelCopy, peerCopy, Dup, size, subchannel);
		// ???
		else
		{
			free(Dup);
			globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	else // Sent
	{
		// text
		if (Variant == 0)
			globals->AddEvent2(49, 36, channelCopy, peerCopy, Dup, size, subchannel);
		// Number
		else if (Variant == 1)
			globals->AddEvent2(49, 37, channelCopy, peerCopy, Dup, size, subchannel);
		// Binary
		else if (Variant == 2)
			globals->AddEvent2(49, 38, channelCopy, peerCopy, Dup, size, subchannel);
		// ???
		else
		{
			free(Dup);
			globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}
void OnChannelMessage(lacewing::relayclient &Client, lacewing::relayclient::channel &channel,
	lacewing::relayclient::channel::peer &peer,
	bool Blasted, int subchannel, const char * Data, size_t size, int Variant)
{
	auto cc = std::find_if(Channels.begin(), Channels.end(), [&](ChannelCopy *&c) {
		return &c->orig() == &channel;
	});
	if (cc == Channels.end())
	{
		globals->CreateError("Couldn't find copy of channel for channel message.");
		return;
	}
	ChannelCopy * channelCopy = *cc;
	auto& peers = channelCopy->getpeers();
	auto pp = std::find_if(peers.begin(), peers.end(), [&](PeerCopy * p) {
		return &p->orig() == &peer;
	});
	if (pp == peers.end())
	{
		globals->CreateError("Couldn't find copy of peer on copy of channel for channel message.");
		return;
	}
	PeerCopy * peerCopy = *pp; 
	
	char * Dup = (char *)malloc(size);
	if (!Dup)
	{
		globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
		return;
	}
	if (memcpy_s(Dup, size, Data, size))
	{
		free(Dup);
		globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
		return;
	}

	if (Blasted)
	{
		// text
		if (Variant == 0)
			globals->AddEvent2(51, 22, channelCopy, peerCopy, Dup, size, subchannel);
		// Number
		else if (Variant == 1)
			globals->AddEvent2(51, 23, channelCopy, peerCopy, Dup, size, subchannel);
		// Binary
		else if (Variant == 2)
			globals->AddEvent2(51, 35, channelCopy, peerCopy, Dup, size, subchannel);
		// ???
		else
		{
			free(Dup);
			globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	else // Sent
	{
		// text
		if (Variant == 0)
			globals->AddEvent2(48, 9, channelCopy, peerCopy, Dup, size, subchannel);
		// Number
		else if (Variant == 1)
			globals->AddEvent2(48, 16, channelCopy, peerCopy, Dup, size, subchannel);
		// Binary
		else if (Variant == 2)
			globals->AddEvent2(48, 33, channelCopy, peerCopy, Dup, size, subchannel);
		// ???
		else
		{
			free(Dup);
			globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}
void OnServerMessage(lacewing::relayclient &Client,
	bool Blasted, int subchannel, const char * Data, size_t size, int Variant)
{
	char * Dup = (char *)malloc(size);
	if (!Dup)
	{
		globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
		return;
	}
	if (memcpy_s(Dup, size, Data, size))
	{
		free(Dup);
		globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
		return;
	}
	if (Blasted)
	{
		// text
		if (Variant == 0)
			globals->AddEvent2(50, 20, nullptr, nullptr, Dup, size, subchannel);
		// Number
		else if (Variant == 1)
			globals->AddEvent2(50, 21, nullptr, nullptr, Dup, size, subchannel);
		// Binary
		else if (Variant == 2)
			globals->AddEvent2(50, 34, nullptr, nullptr, Dup, size, subchannel);
		// ???
		else
		{
			free(Dup);
			globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	else // Sent
	{
		// text
		if (Variant == 0)
			globals->AddEvent2(47, 8, nullptr, nullptr, Dup, size, subchannel);
		// Number
		else if (Variant == 1)
			globals->AddEvent2(47, 15, nullptr, nullptr, Dup, size, subchannel);
		// Binary
		else if (Variant == 2)
			globals->AddEvent2(47, 32, nullptr, nullptr, Dup, size, subchannel);
		// ???
		else
		{
			free(Dup);
			globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}
void OnServerChannelMessage(lacewing::relayclient &Client, lacewing::relayclient::channel &channel,
	bool Blasted, int subchannel, const char * Data, size_t size, int Variant)
{
	auto cc = std::find_if(Channels.begin(), Channels.end(), [&](ChannelCopy *&c) {
		return &c->orig() == &channel;
	});
	if (cc == Channels.end())
	{
		globals->CreateError("Couldn't find copy of channel for server-to-channel message.");
		return;
	}
	ChannelCopy * channelCopy = *cc;

	char * Dup = (char *)malloc(size);
	if (!Dup)
	{
		globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
		return;
	}
	if (memcpy_s(Dup, size, Data, size))
	{
		free(Dup);
		globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
		return;
	}

	if (Blasted)
	{
		// text
		if (Variant == 0)
			globals->AddEvent2(72, 69, channelCopy, nullptr, Dup, size, subchannel);
		// Number
		else if (Variant == 1)
			globals->AddEvent2(72, 70, channelCopy, nullptr, Dup, size, subchannel);
		// Binary
		else if (Variant == 2)
			globals->AddEvent2(72, 71, channelCopy, nullptr, Dup, size, subchannel);
		// ???
		else
		{
			free(Dup);
			globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
	else // Sent
	{
		// text
		if (Variant == 0)
			globals->AddEvent2(68, 65, channelCopy, nullptr, Dup, size, subchannel);
		// Number
		else if (Variant == 1)
			globals->AddEvent2(68, 66, channelCopy, nullptr, Dup, size, subchannel);
		// Binary
		else if (Variant == 2)
			globals->AddEvent2(68, 67, channelCopy, nullptr, Dup, size, subchannel);
		// ???
		else
		{
			free(Dup);
			globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}


#undef Ext
#undef globals