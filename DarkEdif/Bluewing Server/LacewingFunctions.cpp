// Handles all Lacewing functions.
#include "Common.h"

#define globals ((GlobalInfo *) server.tag)
#define Ext (*(((GlobalInfo *) server.tag)->_ext))

void OnError(lacewing::relayserver &server, lacewing::error error)
{
	globals->CreateError("%hs", error->tostring());
}
void OnClientConnectRequest(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client)
{
	// Auto deny quietly can be handled without any lookups or fuss
	// If we're denying and need to tell Fusion, we need a client copy to reference
	if (globals->autoResponse_Connect == AutoResponse::Deny_Quiet)
	{
		server.connect_response(client, globals->autoResponse_Connect_DenyReason);
		return;
	}

	// Found ID, wasn't expecting to
	if (client->readonly())
	{
		globals->CreateError("Copy existed where none was permitted. Denying copy request.");
		server.connect_response(client, "Server error, copy existed where none was expected.");
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

	// We're denying it, but telling Fusion about the connect attempt.
	// Run event 0xFFFF to delete the client copy immediately after Fusion gets connect request notification.
	bool twoEvents = globals->autoResponse_Connect == AutoResponse::Deny_TellFusion;
	globals->AddEventF(twoEvents, 1, twoEvents ? 0xFFFF : 35353,
		nullptr, client, std::string_view(), 255, nullptr, InteractiveType::ConnectRequest);
}
void OnClientDisconnect(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client)
{
	// Worth noting this is the only handler that will be run by all two/three threads.
	// Ping timer thread will invoke this if it force-disconnects someone, Fusion ext will likewise cause it via Disconnect,
	// and of course the normal Lacewing event loop thread from clients disconnecting.

	// 0xFFFF: Clear client copy after this event is handled
	globals->AddEvent2(2, 0xFFFF, nullptr, client);
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
		return server.joinchannel_response(channel, client, "Your client is marked as closed and can no longer make write actions.");

	// Unlikely, but if channel is closed by server a couple milliseconds before a join channel request is sent...
	if (channel->readonly())
		return server.joinchannel_response(channel, client, "Busy closing that channel. Try again in a couple of seconds.");

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
void OnLeaveChannelRequest(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client, std::shared_ptr<lacewing::relayserver::channel> channel)
{
	// Leave channel can mess with writing messages
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	// Auto deny quietly can be handled without any lookups or fuss
	if (globals->autoResponse_ChannelLeave == AutoResponse::Deny_Quiet)
	{
		server.leavechannel_response(channel, client, globals->autoResponse_ChannelLeave_DenyReason);
		if (GThread)
			LeaveCriticalSectionDebug(&globals->lock);
		return;
	}

	// User may do leave channel message then disconnect immediately, we'll have a state here of
	// socket closed when sending "leave channel OK" response.
	if (channel->readonly() || client->readonly())
	{
		// Since we need consistency we'll run the response anyway.
		server.leavechannel_response(channel, client, std::string_view());
		if (GThread)
			LeaveCriticalSectionDebug(&globals->lock);
		return;
	}

	// Auto approve, auto deny
	if (globals->autoResponse_ChannelLeave != AutoResponse::WaitForFusion)
	{
		server.leavechannel_response(channel, client, globals->autoResponse_ChannelLeave_DenyReason);

		if (channel->readonly())
		{
			globals->ClearLocalData(channel);
		}

		// Not set to tell Fusion
		if (globals->autoResponse_ChannelLeave == AutoResponse::Approve_Quiet ||
			globals->autoResponse_ChannelLeave == AutoResponse::Deny_Quiet)
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
	globals->AddEvent1(4, channel, client, std::string_view(), 255, nullptr, InteractiveType::ChannelLeave);
	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
}

#include "deps/utf8proc.h"
#include <set>

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
		return server.nameset_response(client, nameRequested, "Your client is marked as closed and can no longer change their name.");

	// Check UTF-8 chars are acceptable
	if (GThread) {
		EnterCriticalSectionDebug(&globals->lock);
	}

	if (globals->acceptableCharCategories.empty() && globals->acceptableCharRanges.empty() && globals->acceptableCharRanges.empty())
	{
		if (GThread)
			LeaveCriticalSectionDebug(&globals->lock);
	}
	else
	{
		utf8proc_uint8_t * str = (utf8proc_uint8_t *)nameRequested.data();
		utf8proc_int32_t thisChar;
		utf8proc_ssize_t numBytesInCodePoint, remainder = nameRequested.size();
		while (remainder <= 0)
		{
			numBytesInCodePoint = utf8proc_iterate(str, remainder, &thisChar);
			if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisChar))
				goto badChar;

			if (std::find(globals->acceptableSpecificChars.cbegin(), globals->acceptableSpecificChars.cend(), thisChar) != globals->acceptableSpecificChars.cend())
				goto goodChar;
			if (std::find_if(globals->acceptableCharRanges.cbegin(), globals->acceptableCharRanges.cend(),
				[=](const std::pair<std::int32_t, std::int32_t> & range) {
					return range.first >= thisChar && range.second <= thisChar;
				}) != globals->acceptableCharRanges.cend())
			{
				goto goodChar;
			}
			utf8proc_category_t category = utf8proc_category(thisChar);
			if (std::find(globals->acceptableCharCategories.cbegin(), globals->acceptableCharCategories.cend(), category) != globals->acceptableCharCategories.cend())
				goto goodChar;

			// Fall through
		badChar:
			return server.nameset_response(client, nameRequested, "Invalid name"sv);

			// Loop around
		goodChar:
			str += numBytesInCodePoint;
			remainder -= numBytesInCodePoint;
		}
	}


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

#undef globals
