// Handles all Lacewing functions.
#include "Common.h"

#define Ext (*((GlobalInfo *) client.tag)->_ext)
#define globals ((GlobalInfo *) client.tag)
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
	
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	HostIP = ipAddr;
	DenyReasonBuffer = denyReason;

	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
	globals->AddEvent1(2); // no 0xFFFF; Disconnect will be called separately
}
void OnDisconnect(lacewing::relayclient &client)
{
	// 0xFFFF: Empty all channels and peers, and reset HostIP
	globals->AddEvent2(3, 0xFFFF);
}
void OnChannelListReceived(lacewing::relayclient &client)
{
	globals->AddEvent1(26);
}
void OnJoinChannel(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target)
{
#if 0
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	// Autoselect the first channel?
	if (client.getchannelcount() == 1U)
	{
		auto cliReadLock = client.lock.createReadLock();
		selChannel = client.getchannels()[0];
	}

	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
#endif
	
	globals->AddEvent1(4, target);
}
void OnJoinChannelDenied(lacewing::relayclient &client, std::string_view channelName, std::string_view denyReason)
{
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	DenyReasonBuffer = denyReason;

	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
	globals->AddEvent1(5, nullptr, nullptr, nullptr, channelName);
}
void OnLeaveChannel(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target)
{
	// 0xFFFF: Clear channel copy after this event is handled
	globals->AddEvent2(43, 0xFFFF, target);
}
void OnLeaveChannelDenied(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target, std::string_view denyReason)
{
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	DenyReasonBuffer = denyReason;

	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
	globals->AddEvent1(44, target);
}
void OnNameSet(lacewing::relayclient &client)
{
	globals->AddEvent1(6);
}
void OnNameDenied(lacewing::relayclient &client, std::string_view deniedName, std::string_view denyReason)
{
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	DenyReasonBuffer = denyReason;

	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
	globals->AddEvent1(7);
}
void OnNameChanged(lacewing::relayclient &client, std::string_view oldName)
{
	if (GThread)
		EnterCriticalSectionDebug(&globals->lock);

	PreviousName = oldName;

	if (GThread)
		LeaveCriticalSectionDebug(&globals->lock);
	globals->AddEvent1(53);
}
void OnPeerConnect(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel, std::shared_ptr<lacewing::relayclient::channel::peer> peer)
{
	globals->AddEvent1(10, channel, nullptr, peer);
}
void OnPeerDisconnect(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
	std::shared_ptr<lacewing::relayclient::channel::peer> peer)
{
	globals->AddEvent2(11, 0xFFFF, channel, nullptr, peer);
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
	static const std::pair<int, int> eventNumsBlasted[] = { { 52, 39 }, { 52, 40 }, { 52, 41 } };
	static const std::pair<int, int> eventNumsSent[] = { { 49, 36 }, { 49, 37 }, { 49, 38 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, channel, nullptr, peer, message, subchannel);
}
void OnChannelMessage(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
	std::shared_ptr<lacewing::relayclient::channel::peer> peer,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	if (variant > 2)
		return globals->CreateError("Channel message type is neither binary, number nor text.");

	// First number in pair: "On any message from", second, the specific variant.
	// First pair is text, then number, then binary.
	static const std::pair<int, int> eventNumsBlasted[] = { { 51, 22 }, { 51, 23 }, { 51, 35 } };
	static const std::pair<int, int> eventNumsSent[] = { { 48, 9 }, { 48, 16 }, { 48, 33 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, channel, nullptr, peer, message, subchannel);
}
void OnServerMessage(lacewing::relayclient &client,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	if (variant > 2)
		return globals->CreateError("Server message type is neither binary, number nor text.");

	// First number in pair: "On any message from", second, the specific variant.
	// First pair is text, then number, then binary.
	static const std::pair<int, int> eventNumsBlasted[] = { { 50, 20 }, { 50, 21 }, { 50, 34 } };
	static const std::pair<int, int> eventNumsSent[] = { { 47, 8 }, { 47, 15 }, { 47, 32 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, nullptr, nullptr, nullptr, message, subchannel);
}
void OnServerChannelMessage(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
	bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	if (variant > 2)
		return globals->CreateError("Server channel message type is neither binary, number nor text.");

	// First number in pair: "On any message from", second, the specific variant.
	// First pair is text, then number, then binary.
	static const std::pair<int, int> eventNumsBlasted[] = { { 72, 69 }, { 72, 70 }, { 72, 71 } };
	static const std::pair<int, int> eventNumsSent[] = { { 68, 65 }, { 68, 66 }, { 68, 67 } };

	const auto & eventNums = blasted ? eventNumsBlasted[variant] : eventNumsSent[variant];
	globals->AddEvent2(eventNums.first, eventNums.second, channel, nullptr, nullptr, message, subchannel);
}


#undef Ext
#undef globals
