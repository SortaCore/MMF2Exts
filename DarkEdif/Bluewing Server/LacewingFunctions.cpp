// Handles all Lacewing functions.
#include "Common.h"

#define globals ((GlobalInfo *) Server.tag)
#define Ext (*(((GlobalInfo *) Server.tag)->_ext))

ClientCopy * FindClientCopy(GlobalInfo * global, lacewing::relayserver::client &client)
{
	for (auto c : global->_Clients)
		if (c->id() == client.id())
			return c;
	return nullptr;
}

ChannelCopy * FindChannelCopy(GlobalInfo * global, lacewing::relayserver::channel &channel)
{
	for (auto c : global->_channels)
		if (c->id() == channel.id())
			return c;
	return nullptr;
}

void OnError(lacewing::relayserver &Server, lacewing::error error)
{
	globals->CreateError(error->tostring());
}
void OnClientConnectRequest(lacewing::relayserver &Server, lacewing::relayserver::client &client)
{
	// Auto deny quietly can be handled without any lookups or fuss
	// If we're denying and need to tell Fusion, we need a client copy to reference
	if (globals->AutoResponse_Connect == GlobalInfo::Deny_Quiet)
	{
		Server.connect_response(client, globals->AutoResponse_Connect_DenyReason);
		return;
	}

	// Found ID, wasn't expecting to
	if (FindClientCopy(globals, client) != nullptr)
	{
		globals->CreateError("Copy existed where none was permitted. Denying copy request.");
		Server.connect_response(client, "Server error, copy existed where none was expected.");
		return;
	}

	auto c = new ClientCopy(&client);
	Clients.push_back(c);

	// Auto approve, auto deny
	if (globals->AutoResponse_Connect != GlobalInfo::WaitForFusion)
	{
		Server.connect_response(client, globals->AutoResponse_Connect_DenyReason);

		// Update copy if we auto-DENIED it. No change needed in copy if we auto-approved.
		if (globals->AutoResponse_Connect == GlobalInfo::Deny_Quiet ||
			globals->AutoResponse_Connect == GlobalInfo::Deny_TellFusion)
		{
			// Simulate being disconnected in copy.
			// We won't need to actually disconnect, as connect_response does it.
			c->disconnect(false);
		}

		// Not set to tell Fusion
		if (globals->AutoResponse_Connect == GlobalInfo::Approve_Quiet ||
			globals->AutoResponse_Connect == GlobalInfo::Deny_Quiet)
		{
			return;
		}
	}

	// We're denying it, but telling Fusion about the connect attempt.
	// Run event 0xFFFF to delete the client copy immediately after Fusion gets connect request notification.
	bool twoEvents = globals->AutoResponse_Connect == GlobalInfo::Deny_TellFusion;
	globals->AddEventF(twoEvents, 1, twoEvents ? 0xFFFF : 35353,
		nullptr, c, nullptr, 0U, 255, nullptr, InteractiveType::ConnectRequest);
}
void OnClientDisconnect(lacewing::relayserver &Server, lacewing::relayserver::client &client)
{
	auto c = FindClientCopy(globals, client);
	if (c)
	{
		// Disconnects can mess with writing messages
		if (GThread)
			EnterCriticalSectionDerpy(&globals->lock);
		c->disconnect(false);

		// 0xFFFF: Clear client copy after this event is handled
		globals->AddEvent2(2, 0xFFFF, nullptr, c);
		if (GThread)
			LeaveCriticalSectionDebug(&globals->lock);
		return;
	}
	// If copy is missing, chances are connect request was denied, before a Fusion copy was made.
	// Don't make an error.
	// globals->CreateError("Couldn't find client copy for disconnect. Presumed connect request denied.");
}
void OnJoinChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &client, lacewing::relayserver::channel &channel,
	// Provided in case Fusion edits channel name, we need the create settings to persist
	bool hidden, bool autoclose)
{
	// Auto deny quietly can be handled without any lookups or fuss
	// If we're denying and need to tell Fusion, we need a channel copy to reference
	if (globals->AutoResponse_ChannelJoin == GlobalInfo::Deny_Quiet)
	{
		Server.joinchannel_response(channel, client, globals->AutoResponse_ChannelJoin_DenyReason);
		return;
	}

	auto cli = FindClientCopy(globals, client);
	
	// Should never be possible, as liblacewing won't allow a request without Connect being sent, and if Connect is sent a copy is made.
	if (cli == nullptr)
		return Server.joinchannel_response(channel, client, "Your client has been lost to the abyss and can no longer make write actions.");
	
	// Unlikely, but if client is closed by server a couple milliseconds before a join channel request is sent...
	if (cli->isclosed)
		return Server.joinchannel_response(channel, client, "Your client is marked as closed and can no longer make write actions.");
	
	auto ch = FindChannelCopy(globals, channel);
	if (ch == nullptr)
	{
		ch = new ChannelCopy(&channel);
		Channels.push_back(ch);
	}
	// Unlikely, but if channel is closed by server a couple milliseconds before a join channel request is sent...
	else if (ch->isclosed)
		return Server.joinchannel_response(channel, client, "Busy closing that channel. Try again in a couple of seconds.");
	
	// Auto approve, auto deny
	if (globals->AutoResponse_ChannelJoin != GlobalInfo::WaitForFusion)
	{
		Server.joinchannel_response(channel, client, globals->AutoResponse_ChannelJoin_DenyReason);
		
		// Update copy if we approved it
		if (globals->AutoResponse_ChannelJoin == GlobalInfo::Approve_Quiet ||
			globals->AutoResponse_ChannelJoin == GlobalInfo::Approve_TellFusion)
		{
			bool channelmasterset = ch->newlycreated();
			ch->addclient(cli, false);
			if (channelmasterset)
				ch->setchannelmaster(cli);
		}
		
		// Not set to tell Fusion
		if (globals->AutoResponse_ChannelJoin == GlobalInfo::Approve_Quiet ||
			globals->AutoResponse_ChannelJoin == GlobalInfo::Deny_Quiet)
		{
			return;
		}
	}

	globals->AddEvent1(3, ch, cli, _strdup(ch->name()), 0U, 255, nullptr, InteractiveType::ChannelJoin, '\0', false, hidden, autoclose);
}
void OnLeaveChannelRequest(lacewing::relayserver &Server, lacewing::relayserver::client &client, lacewing::relayserver::channel &channel)
{
	// Leave channel can mess with writing messages
	if (GThread)
		EnterCriticalSectionDerpy(&globals->lock);

	// Auto deny quietly can be handled without any lookups or fuss
	if (globals->autoResponse_ChannelLeave == GlobalInfo::Deny_Quiet)
	{
		Server.leavechannel_response(channel, client, globals->AutoResponse_ChannelLeave_DenyReason);
		if (GThread)
			LeaveCriticalSectionDebug(&globals->lock);
		return;
	}

	auto ch = FindChannelCopy(globals, channel);
	auto cli = FindClientCopy(globals, client);

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
		Server.leavechannel_response(channel, client, nullptr);
		if (GThread)
			LeaveCriticalSectionDebug(&globals->lock);
		return;
	}

	// Auto approve, auto deny
	if (globals->autoResponse_ChannelLeave != GlobalInfo::WaitForFusion)
	{
		Server.leavechannel_response(channel, client, globals->AutoResponse_ChannelLeave_DenyReason);
		
		// Update copy if we approved it
		if (globals->autoResponse_ChannelLeave == GlobalInfo::Approve_Quiet ||
			globals->autoResponse_ChannelLeave == GlobalInfo::Approve_TellFusion)
		{
			ch->removeclient(cli, false);
		}

		// Not set to tell Fusion
		if (globals->autoResponse_ChannelLeave == GlobalInfo::Approve_Quiet ||
			globals->autoResponse_ChannelLeave == GlobalInfo::Deny_Quiet)
		{
			if (GThread)
				LeaveCriticalSectionDebug(&globals->lock);
			return;
		}
	}
		
	// 0xFFFF = Clear channel copy after this event is handled.
	// the two Quiets are handled already (see last return)
	// Approve_TellFusion has already made change in copy (ch->removeclient() )
	// Deny_TellFusion does nothing.
	// WaitForFusion will add 0xFFFF if needed.
	globals->AddEvent1(4, ch, cli, nullptr, 0U, 255, nullptr, InteractiveType::ChannelLeave);
	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
}
void OnNameSetRequest(lacewing::relayserver &Server, lacewing::relayserver::client &client, const char * nameRequested)
{
	// Auto deny quietly can be handled without any lookups or fuss
	if (globals->AutoResponse_NameSet == GlobalInfo::Deny_Quiet)
	{
		Server.nameset_response(client, nameRequested, globals->AutoResponse_NameSet_DenyReason);
		return;
	}

	// Client copy is generated on connect, so this should always be true
	auto cli = FindClientCopy(globals, client);

	if (!cli)
	{
		globals->CreateError("Error: Client attempted to set name unexpectedly early. Ignoring request.");
		return;
	}

	// Run response for consistency
	if (cli->isclosed)
		return Server.nameset_response(client, nameRequested, "Your client is marked as closed and can no longer change their name.");

	// Update implementation
	cli->getimplementation();

	// Auto approve, auto deny
	if (globals->AutoResponse_NameSet != GlobalInfo::WaitForFusion)
	{
		Server.nameset_response(client, nameRequested, globals->AutoResponse_NameSet_DenyReason);

		// Update copy if we approved it
		if (globals->AutoResponse_NameSet == GlobalInfo::Approve_Quiet ||
			globals->AutoResponse_NameSet == GlobalInfo::Approve_TellFusion)
		{
			cli->name(nameRequested);
		}

		// Not set to tell Fusion
		if (globals->AutoResponse_NameSet == GlobalInfo::Approve_Quiet ||
			globals->AutoResponse_NameSet == GlobalInfo::Deny_Quiet)
		{
			return;
		}
	}

	// Note: we're not using name change request condition in Fusion (just "name set" for first name and name change),
	// but you can do Client.name() for old name, if you want to see if the client even has one.
	globals->AddEvent1(10, nullptr, cli, _strdup(nameRequested), 0U, 255, nullptr, InteractiveType::ClientNameSet);
}
void OnPeerMessage(lacewing::relayserver &Server, lacewing::relayserver::client &senderClient, lacewing::relayserver::channel &channel, 
	lacewing::relayserver::client &ReceivingClient, bool blasted, int subchannel, const char * Data, size_t size, int variant)
{
	// Auto approve, auto deny (handle before message cloning)
	if (globals->AutoResponse_MessageClient != GlobalInfo::WaitForFusion)
	{
		Server.clientmessage_permit(senderClient, channel, ReceivingClient, blasted, subchannel, Data, size, variant,
			// approve parameter is true or false; true matches these
			globals->AutoResponse_MessageClient == GlobalInfo::Approve_Quiet || 
			globals->AutoResponse_MessageClient == GlobalInfo::Approve_TellFusion);

		// Not set to tell Fusion
		if (globals->AutoResponse_MessageClient == GlobalInfo::Approve_Quiet ||
			globals->AutoResponse_MessageClient == GlobalInfo::Deny_Quiet)
		{
			return;
		}
	}

	char * content = (char *)malloc(size);
	if (!content)
		globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	else if (memcpy_s(content, size, Data, size))
	{
		free(content);
		globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else 
	{
		auto sendcli = FindClientCopy(globals, senderClient);
		auto recvcli = sendcli ? FindClientCopy(globals, ReceivingClient) : nullptr;
		auto ch = sendcli && recvcli ? FindChannelCopy(globals, channel) : nullptr;
		if (!sendcli || !recvcli || !ch)
		{
			free(content);
			return globals->CreateError("Missing copies during on peer message. Message dropped.");
		}

		if (blasted)
		{
			// Text
			if (variant == 0)
				globals->AddEvent2(35, 32, ch, sendcli, content, size, subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, variant, blasted);
			// Number
			else if (variant == 1)
				globals->AddEvent2(35, 33, ch, sendcli, content, size, subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, variant, blasted);
			// Binary
			else if (variant == 2)
				globals->AddEvent2(35, 34, ch, sendcli, content, size, subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, variant, blasted);
			// ???
			else
				globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
		else // Sent
		{
			// Text
			if (variant == 0)
				globals->AddEvent2(23, 20, ch, sendcli, content, size, subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, variant, blasted);
			// Number
			else if (variant == 1)
				globals->AddEvent2(23, 21, ch, sendcli, content, size, subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, variant, blasted);
			// Binary
			else if (variant == 2)
				globals->AddEvent2(23, 22, ch, sendcli, content, size, subchannel, recvcli,
					InteractiveType::ClientMessageIntercept, variant, blasted);
			// ???
			else
				globals->CreateError("Warning: message type is neither binary, number nor text.");
		}
	}
}
void OnChannelMessage(
	lacewing::relayserver &Server, lacewing::relayserver::client &client,
	lacewing::relayserver::channel &channel,
	bool blasted, int subchannel, const char * Data, size_t size, int variant)
{
	// Auto approve, auto deny (handle before message cloning)
	if (globals->AutoResponse_MessageChannel != GlobalInfo::WaitForFusion)
	{
		Server.channelmessage_permit(client, channel, blasted, subchannel, Data, size, variant,
			// approve parameter is true or false; true matches these
			globals->AutoResponse_MessageChannel == GlobalInfo::Approve_Quiet ||
			globals->AutoResponse_MessageChannel == GlobalInfo::Approve_TellFusion);

		// Not set to tell Fusion
		if (globals->AutoResponse_MessageChannel == GlobalInfo::Approve_Quiet ||
			globals->AutoResponse_MessageChannel == GlobalInfo::Deny_Quiet)
		{
			return;
		}
	}

	char * content = (char *)malloc(size);
	if (!content)
		globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	else if (memcpy_s(content, size, Data, size))
	{
		free(content);
		globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else
	{
		auto cli = FindClientCopy(globals, client);
		auto ch = FindChannelCopy(globals, channel);

		if (blasted)
		{
			// TGlobals
			if (variant == 0)
				globals->AddEvent2(31, 28, ch, cli, content, size, subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, variant, blasted);
			// Number
			else if (variant == 1)
				globals->AddEvent2(31, 29, ch, cli, content, size, subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, variant, blasted);
				// Binary
			else if (variant == 2)
				globals->AddEvent2(31, 30, ch, cli, content, size, subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, variant, blasted);
			// ???
			else
				globals->CreateError("Warning: message type is neither binary, number nor tGlobals->");
		}
		else // Sent
		{
			// TGlobals
			if (variant == 0)
				globals->AddEvent2(19, 16, ch, cli, content, size, subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, variant, blasted);
				// Number
			else if (variant == 1)
				globals->AddEvent2(19, 17, ch, cli, content, size, subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, variant, blasted);
				// Binary
			else if (variant == 2)
				globals->AddEvent2(19, 18, ch, cli, content, size, subchannel, nullptr,
					InteractiveType::ChannelMessageIntercept, variant, blasted);
			// ???
			else
				globals->CreateError("Warning: message type is neither binary, number nor tGlobals->");
		}
	}
}
void OnServerMessage(lacewing::relayserver &Server, lacewing::relayserver::client &client,
					 bool blasted, int subchannel, const  char * Data, size_t size, int variant)
{
	// We either have deny quiet, or wait for Fusion. For server messages, nothing else makes sense.
	// Due to this, deny quiet is handled by simply turning off OnServerMessage from being called entirely.
	if (globals->AutoResponse_MessageServer == GlobalInfo::Deny_Quiet)
		return;

	char * content = (char *)malloc(size);

	if (!content)
		globals->CreateError("Failed to create local buffer for copying received message from Lacewing. Message discarded.");
	else if (memcpy_s(content, size, Data, size))
	{
		free(content);
		globals->CreateError("Failed to copy message from Lacewing to local buffer. Message discarded.");
	}
	else
	{
		auto cli = FindClientCopy(globals, client);

		if (blasted)
		{
			// TGlobals
			if (variant == 0)
				globals->AddEvent2(27, 24, nullptr, cli, content, size, subchannel);
			// Number
			else if (variant == 1)
				globals->AddEvent2(27, 25, nullptr, cli, content, size, subchannel);
			// Binary
			else if (variant == 2)
				globals->AddEvent2(27, 26, nullptr, cli, content, size, subchannel);
			// ???
			else
			{
				free(content);
				globals->CreateError("Warning: message type is neither binary, number nor tGlobals->");
			}
		}
		else // Sent
		{
			// TGlobals
			if (variant == 0)
				globals->AddEvent2(15, 12, nullptr, cli, content, size, subchannel);
			// Number
			else if (variant == 1)
				globals->AddEvent2(15, 13, nullptr, cli, content, size, subchannel);
			// Binary
			else if (variant == 2)
				globals->AddEvent2(15, 14, nullptr, cli, content, size, subchannel);
			// ???
			else
			{
				free(content);
				globals->CreateError("Warning: message type is neither binary, number nor tGlobals->");
			}
		}
	}
}

#undef globals