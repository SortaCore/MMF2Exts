// Handles all Lacewing functions.
#include "Common.hpp"

#define Ext (*((Extension::GlobalInfo *) client.tag)->_ext)
#define globals ((Extension::GlobalInfo *) client.tag)
#define GThread globals->_thread

void OnError(lacewing::relayclient &client, lacewing::error error)
{
	globals->AddEvent1(0, nullptr, nullptr, nullptr, error->tostring());
}
void OnConnect(lacewing::relayclient &client)
{
	lacewing::address addr = client.serveraddress();
	char ipAddr[64];
	lw_addr_prettystring(addr->tostring(), ipAddr, sizeof(ipAddr));
	HostIP = ipAddr;
	HostPort = addr->port();
	globals->AddEvent1(1);
}
void OnConnectDenied(lacewing::relayclient &client, std::string_view denyReason)
{
	// On Connect is not called during TCP Connect but when a Lacewing Connect Response
	// message is received. Ditto for On Connect Denied.
	// The serveraddress() is set during TCP Connect, so it should be valid here.
	lacewing::address addr = client.serveraddress();
	char ipAddr[64];
	lw_addr_prettystring(addr->tostring(), ipAddr, sizeof(ipAddr));

	if (GThread.joinable())
		globals->lock.edif_lock();

	HostIP = ipAddr;
	HostPort = addr->port();
	DenyReasonBuffer = denyReason;

	if (GThread.joinable())
		globals->lock.edif_unlock();
	globals->AddEvent1(2); // no CLEAR_EVTNUM, as OnDisconnect() will be called after
}
void OnDisconnect(lacewing::relayclient &client)
{
	// CLEAR_EVTNUM: Empty all channels and peers, and reset HostIP
	globals->AddEvent2(3, CLEAR_EVTNUM);
}
void OnChannelListReceived(lacewing::relayclient &client)
{
	globals->AddEvent1(26);
}
void OnJoinChannel(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target)
{
#if 0
	if (GThread)
		globals->lock.edif_lock();

	// Autoselect the first channel?
	if (client.getchannelcount() == 1U)
	{
		auto cliReadLock = client.lock.createReadLock();
		selChannel = client.getchannels()[0];
	}

	if (GThread)
		globals->lock.edif_unlock();
#endif

	globals->AddEvent1(4, target);
}
void OnJoinChannelDenied(lacewing::relayclient &client, std::string_view channelName, std::string_view denyReason)
{
	if (GThread.joinable())
		globals->lock.edif_lock();

	DenyReasonBuffer = denyReason;

	if (GThread.joinable())
		globals->lock.edif_unlock();
	globals->AddEvent1(5, nullptr, nullptr, nullptr, channelName);
}
void OnLeaveChannel(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target)
{
	// CLEAR_EVTNUM: Clear channel copy after this event is handled
	globals->AddEvent2(43, CLEAR_EVTNUM, target);
}
void OnLeaveChannelDenied(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target, std::string_view denyReason)
{
	if (GThread.joinable())
		globals->lock.edif_lock();

	DenyReasonBuffer = denyReason;

	if (GThread.joinable())
		globals->lock.edif_unlock();
	globals->AddEvent1(44, target);
}
void OnNameSet(lacewing::relayclient &client)
{
	globals->AddEvent1(6);
}
void OnNameDenied(lacewing::relayclient &client, std::string_view deniedName, std::string_view denyReason)
{
	if (GThread.joinable())
		globals->lock.edif_lock();

	DenyReasonBuffer = denyReason;

	if (GThread.joinable())
		globals->lock.edif_unlock();
	globals->AddEvent1(7);
}
void OnNameChanged(lacewing::relayclient &client, std::string_view oldName)
{
	if (GThread.joinable())
		globals->lock.edif_lock();

	PreviousName = oldName;

	if (GThread.joinable())
		globals->lock.edif_unlock();
	globals->AddEvent1(53);
}
void OnPeerConnect(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel, std::shared_ptr<lacewing::relayclient::channel::peer> peer)
{
	globals->AddEvent1(10, channel, nullptr, peer);
}
void OnPeerDisconnect(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
	std::shared_ptr<lacewing::relayclient::channel::peer> peer)
{
	globals->AddEvent2(11, CLEAR_EVTNUM, channel, nullptr, peer);
}
void OnPeerNameChanged(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
	std::shared_ptr<lacewing::relayclient::channel::peer> peer, std::string oldName)
{
	globals->AddEvent1(45, channel, nullptr, peer, oldName);
}
void OnPeerMessage(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
	std::shared_ptr<lacewing::relayclient::channel::peer> peer,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	if (variant > 2)
		return globals->CreateError("Peer message type is neither binary, number nor text.");

	// First number in pair: "On any message from", second, the specific variant.
	// First pair is text, then number, then binary.
	static const std::pair<std::uint16_t, std::uint16_t> eventNumsBlasted[] = { { 52, 39 }, { 52, 40 }, { 52, 41 } };
	static const std::pair<std::uint16_t, std::uint16_t> eventNumsSent[] = { { 49, 36 }, { 49, 37 }, { 49, 38 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, channel, nullptr, peer, message, subchannel, variant);
}
void OnChannelMessage(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
	std::shared_ptr<lacewing::relayclient::channel::peer> peer,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	if (variant > 2)
		return globals->CreateError("Channel message type is neither binary, number nor text.");

	// First number in pair: "On any message from", second, the specific variant.
	// First pair is text, then number, then binary.
	static const std::pair<std::uint16_t, std::uint16_t> eventNumsBlasted[] = { { 51, 22 }, { 51, 23 }, { 51, 35 } };
	static const std::pair<std::uint16_t, std::uint16_t> eventNumsSent[] = { { 48, 9 }, { 48, 16 }, { 48, 33 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, channel, nullptr, peer, message, subchannel, variant);
}
void OnServerMessage(lacewing::relayclient &client,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	if (variant > 2)
		return globals->CreateError("Server message type is neither binary, number nor text.");

	// First number in pair: "On any message from", second, the specific variant.
	// First pair is text, then number, then binary.
	static const std::pair<std::uint16_t, std::uint16_t> eventNumsBlasted[] = { { 50, 20 }, { 50, 21 }, { 50, 34 } };
	static const std::pair<std::uint16_t, std::uint16_t> eventNumsSent[] = { { 47, 8 }, { 47, 15 }, { 47, 32 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, nullptr, nullptr, nullptr, message, subchannel, variant);
}
void OnServerChannelMessage(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	if (variant > 2)
		return globals->CreateError("Server channel message type is neither binary, number nor text.");

	// First number in pair: "On any message from", second, the specific variant.
	// First pair is text, then number, then binary.
	static const std::pair<std::uint16_t, std::uint16_t> eventNumsBlasted[] = { { 72, 69 }, { 72, 70 }, { 72, 71 } };
	static const std::pair<std::uint16_t, std::uint16_t> eventNumsSent[] = { { 68, 65 }, { 68, 66 }, { 68, 67 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, channel, nullptr, nullptr, message, subchannel, variant);
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


#undef Ext
#undef globals
