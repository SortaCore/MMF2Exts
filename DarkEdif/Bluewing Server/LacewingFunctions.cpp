// Handles all Lacewing functions.
#include "Common.hpp"

#define globals ((Extension::GlobalInfo *) server.tag)
#define Ext (*(((Extension::GlobalInfo *) server.tag)->_ext))

void OnError(lacewing::relayserver &server, lacewing::error error)
{
	globals->CreateError("%s", error->tostring());
}
void OnClientConnectRequest(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client)
{
	// Auto deny quietly can be handled without any lookups or fuss
	// If we're denying and need to tell Fusion, we need a client shared_ptr to reference
	if (globals->autoResponse_Connect == AutoResponse::Deny_Quiet)
	{
		server.connect_response(client, globals->autoResponse_Connect_DenyReason);
		return;
	}

	// Found ID, wasn't expecting to
	if (client->readonly())
	{
		globals->CreateError("Client is already disconnected? Denying connect request.");
		server.connect_response(client, "Server error, your client is already read-only."sv);
		return;
	}

	// Auto approve, auto deny
	if (globals->autoResponse_Connect != AutoResponse::WaitForFusion)
	{
		server.connect_response(client, globals->autoResponse_Connect_DenyReason);

		// Not set to tell Fusion
		if (globals->autoResponse_Connect == AutoResponse::Approve_Quiet ||
			globals->autoResponse_Connect == AutoResponse::Deny_Quiet)
		{
			return;
		}
	}

	// If we're denying it, but telling Fusion about the connect attempt,
	// then after disconnect event (ID 1) we run event CLEAR_EVTNUM to clean up the
	// client shared_ptr immediately after ID 1 is done.
	bool twoEvents = globals->autoResponse_Connect == AutoResponse::Deny_TellFusion;
	globals->AddEventF(twoEvents, 1, twoEvents ? CLEAR_EVTNUM : DUMMY_EVTNUM,
		nullptr, client, std::string_view(), 255, nullptr, InteractiveType::ConnectRequest);
}
void OnClientDisconnect(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client)
{
	// Worth noting this is the only handler that will be run by all two/three threads.
	// Ping timer thread will invoke this if it force-disconnects someone, Fusion ext will likewise cause it via Disconnect,
	// and of course the normal Lacewing event loop thread from clients disconnecting.

	// CLEAR_EVTNUM: Clear selection of client after event 2 (disconnect) is handled
	globals->AddEvent2(2, CLEAR_EVTNUM, nullptr, client);
}
void OnJoinChannelRequest(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client, std::shared_ptr<lacewing::relayserver::channel> channel,
	// Provided in case Fusion edits channel name, we need the create settings to persist
	bool hidden, bool autoclose)
{
	// Auto deny quietly can be handled without any lookups or fuss
	if (globals->autoResponse_ChannelJoin == AutoResponse::Deny_Quiet)
		return server.joinchannel_response(channel, client, globals->autoResponse_ChannelJoin_DenyReason);

	// Unlikely, but if client is closed by server a couple milliseconds before a join channel request is sent...
	if (client->readonly())
		return server.joinchannel_response(channel, client, "Your client is marked as closed and can no longer make write actions."sv);

	// Unlikely, but if channel is closed by server a couple milliseconds before a join channel request is sent...
	if (channel->readonly())
		return server.joinchannel_response(channel, client, "Busy closing that channel. Try again in a couple of seconds."sv);

	// Auto approve, auto deny
	if (globals->autoResponse_ChannelJoin != AutoResponse::WaitForFusion)
	{
		server.joinchannel_response(channel, client, globals->autoResponse_ChannelJoin_DenyReason);

		// Not set to tell Fusion
		if (globals->autoResponse_ChannelJoin == AutoResponse::Approve_Quiet ||
			globals->autoResponse_ChannelJoin == AutoResponse::Deny_Quiet)
		{
			return;
		}
	}

	globals->AddEvent1(3, channel, client, channel->name(), 255, nullptr, InteractiveType::ChannelJoin, '\0', false, hidden, autoclose);
}
bool OnChannelClose(lacewing::relayserver & server, std::shared_ptr<lacewing::relayserver::channel> channel)
{
	if (globals->unhostingInProgress)
		globals->AddEvent1(CLEAR_EVTNUM, channel);
	else
		globals->AddEvent2(59, CLEAR_EVTNUM, channel);

	// We can't prevent a channel closure, but returning false indicate the clsosure should be suspended,
	// with the messages and peer lists not updated until server.closechannel_finish() is run, by the queued CLEAR_EVTNUM.
	return false;
}
void OnLeaveChannelRequest(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client, std::shared_ptr<lacewing::relayserver::channel> channel)
{
	// Leave channel can mess with writing messages
	if (GThread.joinable())
		globals->lock.edif_lock();

	// Auto deny quietly can be handled without any lookups or fuss
	if (globals->autoResponse_ChannelLeave == AutoResponse::Deny_Quiet)
	{
		server.leavechannel_response(channel, client, globals->autoResponse_ChannelLeave_DenyReason);

		// CLEAR_EVTNUM will be run by OnChannelClose handler
		if (GThread.joinable())
			globals->lock.edif_unlock();
		return;
	}

	// User may do leave channel message then disconnect immediately, we'll have a state here of
	// socket closed when sending "leave channel OK" response.
	if (channel->readonly() || client->readonly())
	{
		// Since we need consistency we'll run the response anyway.
		server.leavechannel_response(channel, client, std::string_view());

		// CLEAR_EVTNUM will be run by OnChannelClose handler
		if (GThread.joinable())
			globals->lock.edif_unlock();
		return;
	}

	// Auto approve, auto deny
	if (globals->autoResponse_ChannelLeave != AutoResponse::WaitForFusion)
	{
		server.leavechannel_response(channel, client, globals->autoResponse_ChannelLeave_DenyReason);

		// Not set to tell Fusion
		if (globals->autoResponse_ChannelLeave == AutoResponse::Approve_Quiet ||
			globals->autoResponse_ChannelLeave == AutoResponse::Deny_Quiet)
		{
			// CLEAR_EVTNUM will be run by OnChannelClose handler
			if (GThread.joinable())
				globals->lock.edif_unlock();
			return;
		}
	}

	// CLEAR_EVTNUM will be run by OnChannelClose handler
	globals->AddEvent1(4, channel, client, std::string_view(), 255, nullptr, InteractiveType::ChannelLeave);

	if (GThread.joinable())
		globals->lock.edif_unlock();
}

void OnNameSetRequest(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client, std::string_view nameRequested)
{
	// Auto deny quietly can be handled without any lookups or fuss
	if (globals->autoResponse_NameSet == AutoResponse::Deny_Quiet)
	{
		server.nameset_response(client, nameRequested, globals->autoResponse_NameSet_DenyReason);
		return;
	}

	// Run response for consistency
	if (client->readonly())
		return server.nameset_response(client, nameRequested, "Your client is marked as closed and can no longer change their name."sv);

	// Auto approve, auto deny
	if (globals->autoResponse_NameSet != AutoResponse::WaitForFusion)
	{
		server.nameset_response(client, nameRequested, globals->autoResponse_NameSet_DenyReason);

		// Not set to tell Fusion (Deny_Quiet handled above)
		if (globals->autoResponse_NameSet == AutoResponse::Approve_Quiet)
			return;
	}

	// Note: we're not using name change request condition in Fusion (just "name set" for first name and name change),
	// but you can do Client.name() for old name, if you want to see if the client even has one.
	globals->AddEvent1(10, nullptr, client, nameRequested, 255, nullptr, InteractiveType::ClientNameSet);
}
void OnPeerMessage(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> senderClient, std::shared_ptr<lacewing::relayserver::channel> channel,
	std::shared_ptr<lacewing::relayserver::client> receivingClient, bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	if (variant > 2)
		globals->CreateError("Peer message type is neither binary, number nor text.");

	if (variant > 2 || receivingClient->readonly() || channel->readonly())
		return server.clientmessage_permit(senderClient, channel, receivingClient, blasted, subchannel, message, variant, false);

	// Auto approve, auto deny (handle before message cloning)
	if (globals->autoResponse_MessageClient != AutoResponse::WaitForFusion)
	{
		server.clientmessage_permit(senderClient, channel, receivingClient, blasted, subchannel, message, variant,
			// approve parameter is true or false; true matches these
			globals->autoResponse_MessageClient == AutoResponse::Approve_Quiet ||
			globals->autoResponse_MessageClient == AutoResponse::Approve_TellFusion);

		// Not set to tell Fusion
		if (globals->autoResponse_MessageClient == AutoResponse::Approve_Quiet ||
			globals->autoResponse_MessageClient == AutoResponse::Deny_Quiet)
		{
			return;
		}
	}

	// First number in pair: "On any message from", second, the specific variant.
	// First pair is text, then number, then binary.
	static const std::pair<int, int> eventNumsBlasted[] = { { 35, 32 }, { 35, 33 }, { 35, 34 } };
	static const std::pair<int, int> eventNumsSent[] = { { 23, 20 }, { 23, 21 }, { 23, 22 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, channel, senderClient, message, subchannel, receivingClient, InteractiveType::ClientMessageIntercept, variant, blasted);
}
void OnChannelMessage(
	lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> senderClient,
	std::shared_ptr<lacewing::relayserver::channel> channel,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	if (variant > 2)
		globals->CreateError("Channel message type is neither binary, number nor text.");

	if (variant > 2 || channel->readonly())
		return server.channelmessage_permit(senderClient, channel, blasted, subchannel, message, variant, false);

	// Auto approve, auto deny (handle before message cloning)
	if (globals->autoResponse_MessageChannel != AutoResponse::WaitForFusion)
	{
		server.channelmessage_permit(senderClient, channel, blasted, subchannel, message, variant,
			// approve parameter is true or false; true matches these
			globals->autoResponse_MessageChannel == AutoResponse::Approve_Quiet ||
			globals->autoResponse_MessageChannel == AutoResponse::Approve_TellFusion);

		// Not set to tell Fusion
		if (globals->autoResponse_MessageChannel == AutoResponse::Approve_Quiet ||
			globals->autoResponse_MessageChannel == AutoResponse::Deny_Quiet)
		{
			return;
		}
	}

	// First number in pair: "On any message from", second, the specific variant.
	// First pair is text, then number, then binary.
	static const std::pair<int, int> eventNumsBlasted[] = { { 31, 28 }, { 31, 29 }, { 31, 30 } };
	static const std::pair<int, int> eventNumsSent[] = { { 19, 16 }, { 19, 17 }, { 19, 18 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, channel, senderClient, message, subchannel, nullptr, InteractiveType::ChannelMessageIntercept, variant, blasted);
}
void OnServerMessage(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> senderClient,
					 bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	// We either have deny quiet, or wait for Fusion. For server messages, nothing else makes sense.
	// Due to this, deny quiet is handled by simply turning off OnServerMessage from being called entirely.
	if (globals->autoResponse_MessageServer == AutoResponse::Deny_Quiet)
		return;

	if (variant > 2)
		return globals->CreateError("Warning: message type is neither binary, number nor text.");

	// First number in pair: "On any message from", second, the specific variant.
	// First pair is text, then number, then binary.
	static const std::pair<int, int> eventNumsBlasted[] = { { 27, 24 }, { 27, 25 }, { 27, 26 } };
	static const std::pair<int, int> eventNumsSent[] = { { 15, 12 }, { 15, 13 }, { 15, 14 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, nullptr, senderClient, message, subchannel, nullptr, InteractiveType::None, variant, blasted);
}

extern "C" void always_log(const char* str, ...)
{
	// if LOGW() will do something, then log; otherwise, we'll discard
	// always_log is used for info and for errors
#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_WARN)

	// Unicode %s is UTF-16, not a UTF-8 %s; if always_log is passing %s, it means UTF-8
	std::tstring tcharStr = DarkEdif::UTF8ToTString(str);
	if (tcharStr.back() != _T('\n'))
		tcharStr += _T('\n');
#if defined (_WIN32) && defined(_UNICODE)
	// always_log reports UTF-8, DarkEdif::Log uses TCHAR, so we'll convert
	if (tcharStr.find(_T("%s")) != std::tstring::npos)
	{
		va_list v;
		va_start(v, str);
		char utf8Output[1024];
		if (vsprintf_s(utf8Output, std::size(utf8Output), str, v) <= 0)
			DarkEdif::MsgBox::Error(_T("always_log error"), _T("Couldn't print format \"%s\" in always_log."), tcharStr.c_str());

		LOGW(_T("%s"), DarkEdif::UTF8ToTString(utf8Output).c_str());
		va_end(v);
	}
	else
#endif
	{
		va_list v;
		va_start(v, str);
		DarkEdif::LogV(DARKEDIF_LOG_WARN, tcharStr.c_str(), v);
		va_end(v);
	}
#endif
}


#undef globals
