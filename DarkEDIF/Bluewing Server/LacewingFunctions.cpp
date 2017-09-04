// Handles all Lacewing functions.
#include "Common.h"

#define Globals ((GlobalInfo *) Server.tag)
#define Ext (*(((GlobalInfo *) Server.tag)->_Ext))

ClientCopy * FindClientCopy(GlobalInfo * global, lacewing::relayserver::client &Client)
{
	for (auto c : global->_Clients)
		if (c->id() == Client.id())
			return c;
	return nullptr;
}

ChannelCopy * FindChannelCopy(GlobalInfo * global, lacewing::relayserver::channel &Channel)
{
	for (auto c : global->_Channels)
		if (c->id() == Channel.id())
			return c;
	return nullptr;
}

void OnError(lacewing::relayserver &Server, lacewing::error Error)
{
	Globals->CreateError(Error->tostring());
}
void OnClientConnectRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client)
{
	// Auto deny quietly can be handled without any lookups or fuss
	// If we're denying and need to tell Fusion, we need a client copy to reference
	if (Globals->AutoResponse_Connect == GlobalInfo::Deny_Quiet)
	{
		Server.connect_response(Client, Globals->AutoResponse_Connect_DenyReason);
		return;
	}

	// Found ID, wasn't expecting to
	if (FindClientCopy(Globals, Client) != nullptr)
	{
		Globals->CreateError("Copy existed where none was permitted. Denying copy request.");
		Server.connect_response(Client, "Server error, copy existed where none was expected.");
		return;
	}

	auto c = new ClientCopy(&Client);
	Clients.push_back(c);

	// Auto approve, auto deny
	if (Globals->AutoResponse_Connect != GlobalInfo::WaitForFusion)
	{
		Server.connect_response(Client, Globals->AutoResponse_Connect_DenyReason);

		// Update copy if we auto-DENIED it. No change needed in copy if we auto-approved.
		if (Globals->AutoResponse_Connect == GlobalInfo::Deny_Quiet ||
			Globals->AutoResponse_Connect == GlobalInfo::Deny_TellFusion)
		{
			// Simulate being disconnected in copy.
			// We won't need to actually disconnect, as connect_response does it.
			c->disconnect(false);
		}

		// Not set to tell Fusion
		if (Globals->AutoResponse_Connect == GlobalInfo::Approve_Quiet ||
			Globals->AutoResponse_Connect == GlobalInfo::Deny_Quiet)
		{
			return;
		}
	}

	// We're denying it, but telling Fusion about the connect attempt.
	// Run event 0xFFFF to delete the client copy immediately after Fusion gets connect request notification.
	bool twoEvents = Globals->AutoResponse_Connect == GlobalInfo::Deny_TellFusion;
	Globals->AddEventF(twoEvents, 1, twoEvents ? 0xFFFF : 35353,
		nullptr, c, nullptr, 0U, 255, nullptr, InteractiveType::ConnectRequest);
}
void OnClientDisconnect(lacewing::relayserver &Server, lacewing::relayserver::client &Client)
{
	auto c = FindClientCopy(Globals, Client);
	if (c)
	{
		// Disconnects can mess with writing messages
		if (GThread)
			EnterCriticalSectionDerpy(&Globals->Lock);
		c->disconnect(false);

		// 0xFFFF: Clear client copy after this event is handled
		Globals->AddEvent2(2, 0xFFFF, nullptr, c);
		if (GThread)
			LeaveCriticalSectionDerpy(&Globals->Lock);
		return;
	}
	// If copy is missing, chances are connect request was denied, before a Fusion copy was made.
	// Don't make an error.
	// Globals->CreateError("Couldn't find client copy for disconnect. Presumed connect request denied.");
}
void OnJoinChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client, lacewing::relayserver::channel &Channel,
	// Provided in case Fusion edits channel name, we need the create settings to persist
	bool hidden, bool autoclose)
{
	// Auto deny quietly can be handled without any lookups or fuss
	// If we're denying and need to tell Fusion, we need a channel copy to reference
	if (Globals->AutoResponse_ChannelJoin == GlobalInfo::Deny_Quiet)
	{
		Server.joinchannel_response(Channel, Client, Globals->AutoResponse_ChannelJoin_DenyReason);
		return;
	}

	auto cli = FindClientCopy(Globals, Client);
	
	// Should never be possible, as liblacewing won't allow a request without Connect being sent, and if Connect is sent a copy is made.
	if (cli == nullptr)
		return Server.joinchannel_response(Channel, Client, "Your client has been lost to the abyss and can no longer make write actions.");
	
	// Unlikely, but if client is closed by server a couple milliseconds before a join channel request is sent...
	if (cli->isclosed)
		return Server.joinchannel_response(Channel, Client, "Your client is marked as closed and can no longer make write actions.");
	
	auto ch = FindChannelCopy(Globals, Channel);
	if (ch == nullptr)
	{
		ch = new ChannelCopy(&Channel);
		Channels.push_back(ch);
	}
	// Unlikely, but if channel is closed by server a couple milliseconds before a join channel request is sent...
	else if (ch->isclosed)
		return Server.joinchannel_response(Channel, Client, "Busy closing that channel. Try again in a couple of seconds.");
	
	// Auto approve, auto deny
	if (Globals->AutoResponse_ChannelJoin != GlobalInfo::WaitForFusion)
	{
		Server.joinchannel_response(Channel, Client, Globals->AutoResponse_ChannelJoin_DenyReason);
		
		// Update copy if we approved it
		if (Globals->AutoResponse_ChannelJoin == GlobalInfo::Approve_Quiet ||
			Globals->AutoResponse_ChannelJoin == GlobalInfo::Approve_TellFusion)
		{
			bool channelmasterset = ch->newlycreated();
			ch->addclient(cli, false);
			if (channelmasterset)
				ch->setchannelmaster(cli);
		}
		
		// Not set to tell Fusion
		if (Globals->AutoResponse_ChannelJoin == GlobalInfo::Approve_Quiet ||
			Globals->AutoResponse_ChannelJoin == GlobalInfo::Deny_Quiet)
		{
			return;
		}
	}

	Globals->AddEvent1(3, ch, cli, _strdup(ch->name()), 0U, 255, nullptr, InteractiveType::ChannelJoin, '\0', false, hidden, autoclose);
}
void OnLeaveChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client, lacewing::relayserver::channel &Channel)
{
	// Leave channel can mess with writing messages
	if (GThread)
		EnterCriticalSectionDerpy(&Globals->Lock);

	// Auto deny quietly can be handled without any lookups or fuss
	if (Globals->AutoResponse_ChannelLeave == GlobalInfo::Deny_Quiet)
	{
		Server.leavechannel_response(Channel, Client, Globals->AutoResponse_ChannelLeave_DenyReason);
		if (GThread)
			LeaveCriticalSectionDerpy(&Globals->Lock);
		return;
	}

	auto ch = FindChannelCopy(Globals, Channel);
	auto cli = FindClientCopy(Globals, Client);

	// User may do leave channel message then disconnect immediately, we'll have a state here of
	// socket closed when sending "leave channel OK" response.
	if (ch->isclosed || cli->isclosed)
	{
		// Reflect leave in copy even when closed.
		// This way, when disconnect triggers later, we'll have the expected channel list for reading,
		// in case user wants a list of channels the client was on at the point they disconnect
		// (e.g. after this leave channel request message was sent).
		ch->removeclient(cli, false);
		
		// Since we need consistency we'll run the response anyway.
		Server.leavechannel_response(Channel, Client, nullptr);
		if (GThread)
			LeaveCriticalSectionDerpy(&Globals->Lock);
		return;
	}

	// Auto approve, auto deny
	if (Globals->AutoResponse_ChannelLeave != GlobalInfo::WaitForFusion)
	{
		Server.leavechannel_response(Channel, Client, Globals->AutoResponse_ChannelLeave_DenyReason);
		
		// Update copy if we approved it
		if (Globals->AutoResponse_ChannelLeave == GlobalInfo::Approve_Quiet ||
			Globals->AutoResponse_ChannelLeave == GlobalInfo::Approve_TellFusion)
		{
			ch->removeclient(cli, false);
		}

		// Not set to tell Fusion
		if (Globals->AutoResponse_ChannelLeave == GlobalInfo::Approve_Quiet ||
			Globals->AutoResponse_ChannelLeave == GlobalInfo::Deny_Quiet)
		{
			if (GThread)
				LeaveCriticalSectionDerpy(&Globals->Lock);
			return;
		}
	}
		
	// 0xFFFF = Clear channel copy after this event is handled.
	// the two Quiets are handled already (see last return)
	// Approve_TellFusion has already made change in copy (ch->removeclient() )
	// Deny_TellFusion does nothing.
	// WaitForFusion will add 0xFFFF if needed.
	Globals->AddEvent1(4, ch, cli, nullptr, 0U, 255, nullptr, InteractiveType::ChannelLeave);
	if (GThread)
		LeaveCriticalSectionDerpy(&Globals->Lock);
}
void OnNameSetRequest(lacewing::relayserver &Server, lacewing::relayserver::client &Client, const char * nameRequested)
{
	// Auto deny quietly can be handled without any lookups or fuss
	if (Globals->AutoResponse_NameSet == GlobalInfo::Deny_Quiet)
	{
		Server.nameset_response(Client, nameRequested, Globals->AutoResponse_NameSet_DenyReason);
		return;
	}

	// Client copy is generated on connect, so this should always be true
	auto cli = FindClientCopy(Globals, Client);

	if (!cli)
	{
		Globals->CreateError("Error: Client attempted to set name unexpectedly early. Ignoring request.");
		return;
	}

	// Run response for consistency
	if (cli->isclosed)
		return Server.nameset_response(Client, nameRequested, "Your client is marked as closed and can no longer change their name.");

	// Update implementation
	cli->getimplementation();

	// Auto approve, auto deny
	if (Globals->AutoResponse_NameSet != GlobalInfo::WaitForFusion)
	{
		Server.nameset_response(Client, nameRequested, Globals->AutoResponse_NameSet_DenyReason);

		// Update copy if we approved it
		if (Globals->AutoResponse_NameSet == GlobalInfo::Approve_Quiet ||
			Globals->AutoResponse_NameSet == GlobalInfo::Approve_TellFusion)
		{
			cli->name(nameRequested);
		}

		// Not set to tell Fusion
		if (Globals->AutoResponse_NameSet == GlobalInfo::Approve_Quiet ||
			Globals->AutoResponse_NameSet == GlobalInfo::Deny_Quiet)
		{
			return;
		}
	}

	// Note: we're not using name change request condition in Fusion (just "name set" for first name and name change),
	// but you can do Client.name() for old name, if you want to see if the client even has one.
	Globals->AddEvent1(10, nullptr, cli, _strdup(nameRequested), 0U, 255, nullptr, InteractiveType::ClientNameSet);
}
void OnPeerMessage(lacewing::relayserver &Server, lacewing::relayserver::client &SenderClient, lacewing::relayserver::channel &Channel, 
	lacewing::relayserver::client &ReceivingClient, bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	// Auto approve, auto deny (handle before message cloning)
	if (Globals->AutoResponse_MessageClient != GlobalInfo::WaitForFusion)
	{
		Server.clientmessage_permit(SenderClient, Channel, ReceivingClient, Blasted, Subchannel, Data, Size, Variant,
			// approve parameter is true or false; true matches these
			Globals->AutoResponse_MessageClient == GlobalInfo::Approve_Quiet || 
			Globals->AutoResponse_MessageClient == GlobalInfo::Approve_TellFusion);

		// Not set to tell Fusion
		if (Globals->AutoResponse_MessageClient == GlobalInfo::Approve_Quiet ||
			Globals->AutoResponse_MessageClient == GlobalInfo::Deny_Quiet)
		{
			return;
		}
	}

	char * Content = (char *)malloc(Size);
	if (!Content)
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	else if (memcpy_s(Content, Size, Data, Size))
	{
		free(Content);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else 
	{
		auto sendcli = FindClientCopy(Globals, SenderClient);
		auto recvcli = sendcli ? FindClientCopy(Globals, ReceivingClient) : nullptr;
		auto ch = sendcli && recvcli ? FindChannelCopy(Globals, Channel) : nullptr;
		if (!sendcli || !recvcli || !ch)
		{
			free(Content);
			return Globals->CreateError("Missing copies during on peer message. Message dropped.");
		}

		if (Blasted)
		{
			// Text
			if (Variant == 0)
				Globals->AddEvent2(35, 32, ch, sendcli, Content, Size, Subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, Variant, Blasted);
			// Number
			else if (Variant == 1)
				Globals->AddEvent2(35, 33, ch, sendcli, Content, Size, Subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, Variant, Blasted);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent2(35, 34, ch, sendcli, Content, Size, Subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, Variant, Blasted);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
		else // Sent
		{
			// Text
			if (Variant == 0)
				Globals->AddEvent2(23, 20, ch, sendcli, Content, Size, Subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, Variant, Blasted);
			// Number
			else if (Variant == 1)
				Globals->AddEvent2(23, 21, ch, sendcli, Content, Size, Subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, Variant, Blasted);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent2(23, 22, ch, sendcli, Content, Size, Subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, Variant, Blasted);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}
void OnChannelMessage(
	lacewing::relayserver &Server, lacewing::relayserver::client &Client,
	lacewing::relayserver::channel &Channel,
	bool Blasted, int Subchannel, const char * Data, size_t Size, int Variant)
{
	// Auto approve, auto deny (handle before message cloning)
	if (Globals->AutoResponse_MessageChannel != GlobalInfo::WaitForFusion)
	{
		Server.channelmessage_permit(Client, Channel, Blasted, Subchannel, Data, Size, Variant,
			// approve parameter is true or false; true matches these
			Globals->AutoResponse_MessageChannel == GlobalInfo::Approve_Quiet ||
			Globals->AutoResponse_MessageChannel == GlobalInfo::Approve_TellFusion);

		// Not set to tell Fusion
		if (Globals->AutoResponse_MessageChannel == GlobalInfo::Approve_Quiet ||
			Globals->AutoResponse_MessageChannel == GlobalInfo::Deny_Quiet)
		{
			return;
		}
	}

	char * Content = (char *)malloc(Size);
	if (!Content)
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	else if (memcpy_s(Content, Size, Data, Size))
	{
		free(Content);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else
	{
		auto cli = FindClientCopy(Globals, Client);
		auto ch = FindChannelCopy(Globals, Channel);

		if (Blasted)
		{
			// TGlobals
			if (Variant == 0)
				Globals->AddEvent2(31, 28, ch, cli, Content, Size, Subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, Variant, Blasted);
			// Number
			else if (Variant == 1)
				Globals->AddEvent2(31, 29, ch, cli, Content, Size, Subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, Variant, Blasted);
				// Binary
			else if (Variant == 2)
				Globals->AddEvent2(31, 30, ch, cli, Content, Size, Subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, Variant, Blasted);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor tGlobals->");
		}
		else // Sent
		{
			// TGlobals
			if (Variant == 0)
				Globals->AddEvent2(19, 16, ch, cli, Content, Size, Subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, Variant, Blasted);
				// Number
			else if (Variant == 1)
				Globals->AddEvent2(19, 17, ch, cli, Content, Size, Subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, Variant, Blasted);
				// Binary
			else if (Variant == 2)
				Globals->AddEvent2(19, 18, ch, cli, Content, Size, Subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, Variant, Blasted);
			// ???
			else
				Globals->CreateError("Warning: message type is neither binary, number nor tGlobals->");
		}
	}
}
void OnServerMessage(lacewing::relayserver &Server, lacewing::relayserver::client &Client,
					 bool Blasted, int Subchannel, const  char * Data, size_t Size, int Variant)
{
	// We either have deny quiet, or wait for Fusion. For server messages, nothing else makes sense.
	// Due to this, deny quiet is handled by simply turning off OnServerMessage from being called entirely.
	if (Globals->AutoResponse_MessageServer == GlobalInfo::Deny_Quiet)
		return;

	char * Content = (char *)malloc(Size);

	if (!Content)
		Globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	else if (memcpy_s(Content, Size, Data, Size))
	{
		free(Content);
		Globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else
	{
		auto cli = FindClientCopy(Globals, Client);

		if (Blasted)
		{
			// TGlobals
			if (Variant == 0)
				Globals->AddEvent2(27, 24, nullptr, cli, Content, Size, Subchannel);
			// Number
			else if (Variant == 1)
				Globals->AddEvent2(27, 25, nullptr, cli, Content, Size, Subchannel);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent2(27, 26, nullptr, cli, Content, Size, Subchannel);
			// ???
			else
			{
				free(Content);
				Globals->CreateError("Warning: message type is neither binary, number nor tGlobals->");
			}
		}
		else // Sent
		{
			// TGlobals
			if (Variant == 0)
				Globals->AddEvent2(15, 12, nullptr, cli, Content, Size, Subchannel);
			// Number
			else if (Variant == 1)
				Globals->AddEvent2(15, 13, nullptr, cli, Content, Size, Subchannel);
			// Binary
			else if (Variant == 2)
				Globals->AddEvent2(15, 14, nullptr, cli, Content, Size, Subchannel);
			// ???
			else
			{
				free(Content);
				Globals->CreateError("Warning: message type is neither binary, number nor tGlobals->");
			}
		}
	}
}

#undef Globals