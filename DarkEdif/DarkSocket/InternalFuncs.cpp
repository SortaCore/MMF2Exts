#include "Common.h"
// ============================================================================
//
// UNREFENCED FUNCTIONS
//
// ============================================================================
#include <map>

// Vista+ protocols only, not defined if XP target
#if (_WIN32_WINNT < 0x0600)
	#define AF_BTH          32
	#if (_WIN32_WINNT < 0x0601)
		#define AF_LINK     33
	#endif
	#define IPPROTO_ST 5
	#define IPPROTO_CBT 7
	#define IPPROTO_EGP 8
	#define IPPROTO_IGP 9
	#define IPPROTO_RDP 27
	#define IPPROTO_PIM 103
	#define IPPROTO_PGM 113
	#define IPPROTO_L2TP 115
	#define IPPROTO_SCTP 132
#endif

const std::map<const std::string_view, const int> addressFamily = {
	{ "UNSPEC"sv, AF_UNSPEC }, { "UNSPECIFIED"sv, AF_UNSPEC },
	{ "UNIX"sv, AF_UNIX },
	{ "INET"sv, AF_INET }, { "INTERNET"sv, AF_INET }, { "IPV4"sv, AF_INET },
	{ "IMPLINK"sv, AF_IMPLINK },
	{ "PUP"sv, AF_PUP },
	{ "CHAOS"sv, AF_CHAOS },
	{ "NS"sv, AF_NS },
	{ "IPX"sv, AF_IPX },
	{ "ISO"sv, AF_ISO },
	{ "OSI"sv, AF_OSI },
	{ "ECMA"sv, AF_ECMA },
	{ "DATAKIT"sv, AF_DATAKIT },
	{ "CCITT"sv, AF_CCITT },
	{ "DECNET"sv, AF_DECnet },
	{ "DLI"sv, AF_DLI },
	{ "LAT"sv, AF_LAT },
	{ "HYLINK"sv, AF_HYLINK },
	{ "APPLETALK"sv, AF_APPLETALK },
	{ "NETBIOS"sv, AF_NETBIOS },
	{ "VOICEVIEW"sv, AF_VOICEVIEW },
	{ "FIREFOX"sv, AF_FIREFOX },
	{ "BAN"sv, AF_BAN }, { "BANYAN"sv, AF_BAN },
	{ "ATM"sv, AF_ATM },
	{ "INET6"sv, AF_INET6 }, { "INTERNET6"sv, AF_INET6 }, { "IPV6"sv, AF_INET6 },
	{ "CLUSTER"sv, AF_CLUSTER },
	{ "12844"sv, AF_12844 },
	{ "IRDA"sv, AF_IRDA },
	{ "NETDES"sv, AF_NETDES },
	{ "TCNPROCESS"sv, AF_TCNPROCESS },
	{ "TCNMESSAGE"sv, AF_TCNMESSAGE },
	{ "ICLFXBM"sv, AF_ICLFXBM },
// Vista+ only
	{ "BTH"sv, AF_BTH }, { "BLUETOOTH"sv, AF_BTH },
	{ "LINK"sv, AF_LINK },
};
const std::map<const std::string_view, const int> socketType = {
	{ "STREAM"sv, SOCK_STREAM },
	{ "DGRAM"sv, SOCK_DGRAM },
	{ "DATAGRAM"sv, SOCK_DGRAM },
	{ "RAW"sv, SOCK_RAW },
	{ "RDM"sv, SOCK_RDM },
	{ "SEQPACKET"sv, SOCK_SEQPACKET },
};
const std::map<const std::string_view, const int> protocolTypes = {
	{ "HOPOPTS"sv, IPPROTO_HOPOPTS },
	{ "ICMP"sv, IPPROTO_ICMP },
	{ "IGMP"sv, IPPROTO_IGMP },
	{ "GGP"sv, IPPROTO_GGP },
	{ "IPV4"sv, IPPROTO_IPV4 },
	{ "ST"sv, IPPROTO_ST },
	{ "IPPROTO_TCP"sv, IPPROTO_TCP },
	{ "TCP"sv, IPPROTO_TCP },
	{ "CBT"sv, IPPROTO_CBT },
	{ "EGP"sv, IPPROTO_EGP },
	{ "IGP"sv, IPPROTO_IGP },
	{ "PUP"sv, IPPROTO_PUP },
	{ "UDP"sv, IPPROTO_UDP },
	{ "IDP"sv, IPPROTO_IDP },
	{ "RDP"sv, IPPROTO_RDP },
	{ "ICMP"sv, IPPROTO_IPV6 },
	{ "IPV6 ROUTING"sv, IPPROTO_ROUTING },
	{ "IPV6 ESP"sv, IPPROTO_ESP }, { "IPV6 ENCAPSULATING SECURITY PAYLOAD"sv, IPPROTO_ESP },
	{ "IPV6 AH"sv, IPPROTO_AH }, { "IPV6 AUTHENTICATION HEADER"sv, IPPROTO_AH },
	{ "IPV6 FRAGMENT"sv, IPPROTO_FRAGMENT }, { "FRAGMENT"sv, IPPROTO_FRAGMENT },
	{ "IPV6 ICMP"sv, IPPROTO_ICMPV6 }, { "ICMPV6"sv, IPPROTO_ICMPV6 },
	{ "IPV6 NONE"sv, IPPROTO_NONE }, { "IPV6 NO NEXT HEADER"sv, IPPROTO_NONE },
	{ "IPV6 DSTOPTS"sv, IPPROTO_DSTOPTS }, { "IPV6 DESTINATION OPTIONS"sv, IPPROTO_DSTOPTS },
	{ "ND"sv, IPPROTO_ND },
	{ "ICLFXBM"sv, IPPROTO_ICLFXBM },
	{ "PIM"sv, IPPROTO_PIM },
	{ "PGM"sv, IPPROTO_PGM },
	{ "L2TP"sv, IPPROTO_L2TP },
	{ "SCTP"sv, IPPROTO_SCTP },
	{ "PGM"sv, IPPROTO_PGM },
	{ "RAW"sv, IPPROTO_RAW },
	// { "DARKNET"sv, IPPROTO_DARK },
};

// Another text->number from macro function. Moved out the action for simplicity.
int Extension::Internal_WorkOutAddressFamily(const TCHAR * addrFamAsText)
{
	// Get string, convert to ANSI and uppercase, then search in the map
	std::string param = TStringToANSI(addrFamAsText);
	std::transform(param.begin(), param.end(), param.begin(),
		[](unsigned char c) { return std::toupper(c); });

	const auto ci = addressFamily.find(param);
	if (ci != addressFamily.cend())
		return ci->second;
	return -1;
}

// A further text->number from macro function. Moved out the action for simplicity.
int Extension::Internal_WorkOutSocketType(const TCHAR * socketTypeAsText)
{
	// Get string, convert to ANSI and uppercase, then search in the map
	std::string param = TStringToANSI(socketTypeAsText);
	std::transform(param.begin(), param.end(), param.begin(),
		[](unsigned char c) { return std::toupper(c); });

	const auto ci = socketType.find(param);
	if (ci != socketType.cend())
		return ci->second;
	return -1;
}

// A simple text->number from macro function. Moved out the action for simplicity.
int Extension::Internal_WorkOutProtocolType(const TCHAR * socketTypeAsText)
{
	// Get string, convert to ANSI and uppercase, then search in the map
	std::string param = TStringToANSI(socketTypeAsText);
	std::transform(param.begin(), param.end(), param.begin(),
		[](unsigned char c) { return std::toupper(c); });

	const auto ci = protocolTypes.find(param);
	if (ci != protocolTypes.cend())
		return ci->second;
	return -1;
}

// Report - For non-errors
void Extension::Internal_Report(int socketID, const TCHAR * report, ...)
{
	// Not from a socket
	if (socketID == -1)
	{
		threadsafe.edif_lock();
		CompleteStatus += report;
		CompleteStatus += _T("\r\n");
		threadsafe.edif_unlock();
	}
	else // From a thread
	{
		TCHAR text [512];
		_stprintf_s(text, 512, "Socket ID = %i >> %s\r\n", socketID, report);

		threadsafe.edif_lock();
		CompleteStatus += text;
		threadsafe.edif_unlock();
	}

	CallEvent(MF2C_DEBUG_ON_NEW_STATUS);
}

// Explode - For errors
void Extension::Internal_Error(int socketID, const TCHAR * error, ...)
{
	TCHAR text[1024];
	va_list v;
	va_start(v, error);
	if (_vstprintf_s(text, error, v) <= 0)
		DarkEdif::MsgBox::Error(_T("Error"), _T("Couldn't print an error with format:\n%s"), error);
	va_end(v);

	// Not from a thread
	if (socketID == -1)
	{
		threadsafe.edif_lock();
		LastError += error;
		LastError += _T("\r\n");
		CompleteStatus += error;
		CompleteStatus += _T("\r\n");
		threadsafe.edif_unlock();
	}
	else
	// From a thread
	{
		_tsprintf_s(text, _T("Socket ID = %i >> %s."), SocketID, error);
		ThreadSafe_Start();
		LastError += text;
		LastError += _T("\r\n");
		CompleteStatus += text;
		CompleteStatus += _T("\r\n");
		ThreadSafe_End();
		if (UsePopups)
			DarkEdif::MsgBox::Error(_T("Error"), _T("Error from socket %i:\n%s"), SocketID, error);
	}
	CallEvent(MF2C_DEBUG_ON_ERROR);
	CallEvent(MF2C_DEBUG_ON_NEW_STATUS);
}

// Return - For threads reporting things to MMF2
void Extension::Unreferenced_ReturnToMMF(int ReturnAsI, int SocketID, void * Msg, int Length)
{
	bool ReturnAs = false;

	if (ReturnAsI != 0)
	{
		ReturnAs = true;
		ReturnAsI = MF2C_SERVER_RECEIVED_MESSAGE;
	}
	else
	{
		ReturnAsI = MF2C_CLIENT_RECEIVED_MESSAGE;
	}

	ThreadSafe_Start();
	// New struct in the vector queue
	CarryMsg c;
	// Set variables in the struct
	c.ClientOrServer = ReturnAs;
	c.Message = Msg;
	c.Socket = SocketID;
	Returns.push_back(c);
	ThreadSafe_End();
	CallEvent(ReturnAsI);
}

// Gets some server thing.
unsigned long Extension::Unreferenced_WorkOutInAddr(TCHAR * t)
{
	if (_tcsicmp(t, _T("Any")) == 0)
		return 0; else
	if (_tcsicmp(t, _T("LoopBack")) == 0)
		return 0; else
	if (_tcsicmp(t, _T("Broadcast")) == 0)
		return 0; else
	if (_tcsicmp(t, _T("None")) == 0)
		return 0; else
	return 12345;
}
