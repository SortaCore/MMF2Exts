#include "Common.hpp"
// ============================================================================
//
// UNREFENCED FUNCTIONS
//
// ============================================================================
#include <map>

// Vista+ protocols only, not defined if XP target
#if (_WIN32_WINNT < 0x0600)
	#define AF_BTH		  32
	#if (_WIN32_WINNT < 0x0601)
		#define AF_LINK	 33
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
#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8				   0x0602 // Windows 8
#define _WIN32_WINNT_WINBLUE				0x0603 // Windows 8.1
#define _WIN32_WINNT_WIN10				  0x0A00 // Windows 10
//#define _WIN32_WINNT_WIN11				  0x0B00 // Windows 11? Currently v10.0.buildnum, so not defined in SDK.
#endif

static int winVer;
int GlobalInfo::Internal_GetWinVer()
{
	if (winVer != 0)
		return winVer;
#ifdef _WIN32
	// Using GetVersion/GetVersionEx is both deprecated, and on later OSes, will change their results
	// depending on the manifest of the app - pretending to be a different OS version for compatibility.
	// RtlGetVersion doesn't, but isn't in Windows SDK, so we'll have to look up the address manually.

	LONG(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW) = NULL;

	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleW(L"ntdll"), "RtlGetVersion");
	if (NULL != RtlGetVersion)
	{

		OSVERSIONINFOEXW osInfo = {};
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);
		winVer = osInfo.dwMajorVersion;
		// We can assume it's XP+. XP is 5.1, 5.2.
		if (osInfo.dwMajorVersion == 5)
			winVer = _WIN32_WINNT_WINXP;
		else if (osInfo.dwMajorVersion == 6)
		{
			if (osInfo.dwMinorVersion == 0)
				winVer = _WIN32_WINNT_VISTA;
			else if (osInfo.dwMinorVersion == 1)
				winVer = _WIN32_WINNT_WIN7;
			else if (osInfo.dwMinorVersion == 2)
				winVer = _WIN32_WINNT_WIN8;
		}
		else if (osInfo.dwMajorVersion == 10)
			winVer = _WIN32_WINNT_WIN10;
		// note Win 11 is dwMajorVersion 10, weirdly, with dwMinorVersion > 22000; this may change
	}
#else
	// Not Windows, so using this doesn't make much sense; we'll just pretend all modern features are usable,
	// as the OS will error out later if it's not anyway
	winVer = _WIN32_WINNT_WIN10;
#endif

	return winVer;
}

const std::map<const std::string_view, const int> addressFamily = {
	{ "UNSPEC"sv, AF_UNSPEC }, { "UNSPECIFIED"sv, AF_UNSPEC }, { "BOTHIP"sv, AF_UNSPEC },
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
	{ "TCP"sv, IPPROTO_TCP },
	{ "CBT"sv, IPPROTO_CBT },
	{ "EGP"sv, IPPROTO_EGP },
	{ "IGP"sv, IPPROTO_IGP },
	{ "PUP"sv, IPPROTO_PUP },
	{ "UDP"sv, IPPROTO_UDP },
	{ "IDP"sv, IPPROTO_IDP },
	{ "RDP"sv, IPPROTO_RDP },
	{ "IPV6"sv, IPPROTO_IPV6 },
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
int GlobalInfo::AddressFamilyTextToNum(const std::string_view addrFamAsASCIIUpper)
{
	const auto ci = addressFamily.find(addrFamAsASCIIUpper);
	if (ci != addressFamily.cend())
		return ci->second;
	return -1;
}

// A further text->number from macro function. Moved out the action for simplicity.
int GlobalInfo::SocketTypeTextToNum(const std::string_view sockTypeAsASCIIUpper)
{
	const auto ci = socketType.find(sockTypeAsASCIIUpper);
	if (ci != socketType.cend())
		return ci->second;
	return -1;
}

// A simple text->number from macro function. Moved out the action for simplicity.
int GlobalInfo::ProtocolTypeTextToNum(const std::string_view protocolTypeAsText)
{
	const auto ci = protocolTypes.find(protocolTypeAsText);
	if (ci != protocolTypes.cend())
		return ci->second;
	return -1;
}

// Report - For non-errors
void GlobalInfo::ReportInfo(int socketID, PrintFHintInside const TCHAR * report, ...)
{
	TCHAR text[1024];
	va_list v;
	va_start(v, report);
	if (_vstprintf_s(text, report, v) <= 0)
	{
		DarkEdif::MsgBox::Error(_T("DarkSocket Error"), _T("Couldn't print info with format:\n%s"), report);
		DarkEdif::BreakIfDebuggerAttached();
	}
	va_end(v);

	std::tstringstream text2;

	// We can get current event, woo!
	int curEvent = -1;
	if (std::this_thread::get_id() == DarkEdif::MainThreadID &&
		(curEvent = DarkEdif::GetCurrentFusionEventNum(this->extsHoldingGlobals[0])) != -1)
	{
		text2 << _T("[Fusion event ") << curEvent;
		if (socketID != -1)
			text2 << _T(", socket ID "sv) << socketID;
		text2 << _T("] "sv);
	}
	else if (socketID != -1)
		text2 << _T("[socket ID "sv) << socketID << _T("] "sv);

	text2 << text;

	EventToRun etr(socketID, nullptr, Conditions::OnInfo);
	etr.info = text2.str();
	AddEvent(std::move(etr));
}

// Returns pointer or null, reports error if not found
std::shared_ptr<Thread> GlobalInfo::GetSocket(const char * func, SocketType socketType, int socketID)
{
	// Socket IDs aren't reused, so being less than size() doesn't make it valid, but being out of it is definitely invalid
	if (socketID < 0 || (size_t)socketID >= socketThreadList.size())
	{
		return CreateError(socketID, _T("%s: Couldn't get socket ID %d; socket ID is outside valid range (0 to %zu)."),
			DarkEdif::UTF8ToTString(func).c_str(), socketID, socketThreadList.size() - 1U), nullptr;
	}

	// It's safe to read from Extension without locking. Only main thread will write to Ext directly.
	auto thdIt = std::find_if(socketThreadList.cbegin(), socketThreadList.cend(), [socketID](const std::shared_ptr<Thread> t) {
		return t->fusionSocketID == socketID;
	});
	if (thdIt == socketThreadList.cend())
	{
		return CreateError(socketID, _T("%s: Couldn't get socket ID %d; socket ID is not valid."),
			DarkEdif::UTF8ToTString(func).c_str(), socketID), nullptr;
	}
	if (socketType == SocketType::Client && (*thdIt)->isServer)
	{
		return CreateError(socketID, _T("%s: Socket ID %d is a server, not a client."),
			DarkEdif::UTF8ToTString(func).c_str(), socketID), nullptr;
	}
	else if (socketType == SocketType::Server && !(*thdIt)->isServer)
	{
		return CreateError(socketID, _T("%s: Socket ID %d is a client, not a server."),
			DarkEdif::UTF8ToTString(func).c_str(), socketID), nullptr;
	}

	return *thdIt;
}
std::shared_ptr<SocketSource> GlobalInfo::GetSocketSource(const char* func, int socketID, int peerSocketID)
{
	auto sock = GetSocket(func, SocketType::Server, socketID);
	if (!sock)
		return nullptr;

	if (peerSocketID < 0)
	{
		CreateError(socketID, _T("Peer socket ID %i is invalid."), peerSocketID);
		return nullptr;
	}

	sock->lock.edif_lock();
	auto ps = std::find_if(sock->sources.cbegin(), sock->sources.cend(),
		[=](const std::shared_ptr<SocketSource>& s) { return s->peerSocketID == peerSocketID; });
	if (ps == sock->sources.cend())
	{
		CreateError(socketID, _T("Can't find peer socket with ID %i."), peerSocketID);
		sock->lock.edif_unlock();
		return nullptr;
	}
	std::shared_ptr<SocketSource> ps2 = *ps;
	sock->lock.edif_unlock();
	return std::move(ps2);
}

// Smart error reporting
void GlobalInfo::CreateError(int socketID, PrintFHintInside const TCHAR * error, ...)
{
	TCHAR text[1024];
	va_list v;
	va_start(v, error);
	if (_vstprintf_s(text, error, v) <= 0)
	{
		DarkEdif::MsgBox::Error(_T("DarkSocket Error"), _T("Couldn't print an error with format:\n%s"), error);
		DarkEdif::BreakIfDebuggerAttached();
	}
	va_end(v);

	std::tstringstream text2;

	// We can get current event if we're calling from main thread. This also guarantees extHoldingGlobals has at least one entry.
	int curEvent = -1;
	if (std::this_thread::get_id() == DarkEdif::MainThreadID &&
		(curEvent = DarkEdif::GetCurrentFusionEventNum(this->extsHoldingGlobals[0])) != -1)
	{
		text2 << _T("[Fusion event "sv) << curEvent;
		if (socketID != -1)
			text2 << _T(", socket ID "sv) << socketID;
		text2 << _T("] "sv);
	}
	else if (socketID != -1)
		text2 << _T("[socket ID "sv) << socketID << _T("] "sv);

	text2 << text;

	EventToRun etr(socketID, nullptr, Conditions::OnError);
	etr.info = text2.str();
	AddEvent(std::move(etr));
}

// Gets the internet mask for incoming connections
bool GlobalInfo::InAddrTextToStruct(const std::string_view inaddrAsASCIIUpper, in6_addr * writeTo)
{
	if (inaddrAsASCIIUpper == "ANY"sv)
	{
		*writeTo = in6addr_any;
		return true;
	}
	if (inaddrAsASCIIUpper == "LOOPBACK"sv)
	{
		*writeTo = in6addr_loopback;
		return true;
	}
	if (inaddrAsASCIIUpper == "BROADCAST"sv)
	{
		*(PIN_ADDR)writeTo = in4addr_broadcast;
		return true;
	}
	if (inaddrAsASCIIUpper == "NONE"sv)
	{
		memset(writeTo, 0xFF, sizeof(*writeTo));
		return true;
	}
	return false;
}
ULONG encodingToCodePage(const std::string_view encoding)
{
	// from https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
	std::map<std::string_view, int> encodings = {
		{ "ibm037"sv, 37 }, // IBM EBCDIC US-Canada
		{ "ibm437"sv, 437 }, // OEM United States
		{ "ibm500"sv, 500 }, // IBM EBCDIC International
		{ "asmo708"sv, 708 }, // Arabic (ASMO 708)
		{ "dos720"sv, 720 }, // Arabic (Transparent ASMO); Arabic (DOS)
		{ "ibm737"sv, 737 }, // OEM Greek (formerly 437G); Greek (DOS)
		{ "ibm775"sv, 775 }, // OEM Baltic; Baltic (DOS)
		{ "ibm850"sv, 850 }, // OEM Multilingual Latin 1; Western European (DOS)
		{ "ibm852"sv, 852 }, // OEM Latin 2; Central European (DOS)
		{ "ibm855"sv, 855 }, // OEM Cyrillic (primarily Russian)
		{ "ibm857"sv, 857 }, // OEM Turkish; Turkish (DOS)
		{ "ibm00858"sv, 858 }, // OEM Multilingual Latin 1 + Euro symbol
		{ "ibm860"sv, 860 }, // OEM Portuguese; Portuguese (DOS)
		{ "ibm861"sv, 861 }, // OEM Icelandic; Icelandic (DOS)
		{ "dos862"sv, 862 }, // OEM Hebrew; Hebrew (DOS)
		{ "ibm863"sv, 863 }, // OEM French Canadian; French Canadian (DOS)
		{ "ibm864"sv, 864 }, // OEM Arabic; Arabic (864)
		{ "ibm865"sv, 865 }, // OEM Nordic; Nordic (DOS)
		{ "cp866"sv, 866 }, // OEM Russian; Cyrillic (DOS)
		{ "ibm869"sv, 869 }, // OEM Modern Greek; Greek, Modern (DOS)
		{ "ibm870"sv, 870 }, // IBM EBCDIC Multilingual/ROECE (Latin 2); IBM EBCDIC Multilingual Latin 2
		{ "windows874"sv, 874 }, // Thai (Windows)
		{ "cp875"sv, 875 }, // IBM EBCDIC Greek Modern
		{ "shiftjis"sv, 932 }, // ANSI/OEM Japanese; Japanese (Shift-JIS)
		{ "gb2312"sv, 936 }, // ANSI/OEM Simplified Chinese (PRC, Singapore); Chinese Simplified (GB2312)
		{ "ksc56011987"sv, 949 }, // ANSI/OEM Korean (Unified Hangul Code)
		{ "big5"sv, 950 }, // ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC); Chinese Traditional (Big5)
		{ "ibm1026"sv, 1026 }, // IBM EBCDIC Turkish (Latin 5)
		{ "ibm01047"sv, 1047 }, // IBM EBCDIC Latin 1/Open System
		{ "ibm01140"sv, 1140 }, // IBM EBCDIC US-Canada (037 + Euro symbol); IBM EBCDIC (US-Canada-Euro)
		{ "ibm01141"sv, 1141 }, // IBM EBCDIC Germany (20273 + Euro symbol); IBM EBCDIC (Germany-Euro)
		{ "ibm01142"sv, 1142 }, // IBM EBCDIC Denmark-Norway (20277 + Euro symbol); IBM EBCDIC (Denmark-Norway-Euro)
		{ "ibm01143"sv, 1143 }, // IBM EBCDIC Finland-Sweden (20278 + Euro symbol); IBM EBCDIC (Finland-Sweden-Euro)
		{ "ibm01144"sv, 1144 }, // IBM EBCDIC Italy (20280 + Euro symbol); IBM EBCDIC (Italy-Euro)
		{ "ibm01145"sv, 1145 }, // IBM EBCDIC Latin America-Spain (20284 + Euro symbol); IBM EBCDIC (Spain-Euro)
		{ "ibm01146"sv, 1146 }, // IBM EBCDIC United Kingdom (20285 + Euro symbol); IBM EBCDIC (UK-Euro)
		{ "ibm01147"sv, 1147 }, // IBM EBCDIC France (20297 + Euro symbol); IBM EBCDIC (France-Euro)
		{ "ibm01148"sv, 1148 }, // IBM EBCDIC International (500 + Euro symbol); IBM EBCDIC (International-Euro)
		{ "ibm01149"sv, 1149 }, // IBM EBCDIC Icelandic (20871 + Euro symbol); IBM EBCDIC (Icelandic-Euro)
		{ "windows1250"sv, 1250 }, // ANSI Central European; Central European (Windows)
		{ "windows1251"sv, 1251 }, // ANSI Cyrillic; Cyrillic (Windows)
		{ "windows1252"sv, 1252 }, // ANSI Latin 1; Western European (Windows)
		{ "windows1253"sv, 1253 }, // ANSI Greek; Greek (Windows)
		{ "windows1254"sv, 1254 }, // ANSI Turkish; Turkish (Windows)
		{ "windows1255"sv, 1255 }, // ANSI Hebrew; Hebrew (Windows)
		{ "windows1256"sv, 1256 }, // ANSI Arabic; Arabic (Windows)
		{ "windows1257"sv, 1257 }, // ANSI Baltic; Baltic (Windows)
		{ "windows1258"sv, 1258 }, // ANSI/OEM Vietnamese; Vietnamese (Windows)
		{ "johab"sv, 1361 }, // Korean (Johab)
		{ "macintosh"sv, 10000 }, // MAC Roman; Western European (Mac)
		{ "macjapanese"sv, 10001 }, // Japanese (Mac)
		{ "macchinesetrad"sv, 10002 }, // MAC Traditional Chinese (Big5); Chinese Traditional (Mac)
		{ "mackorean"sv, 10003 }, // Korean (Mac)
		{ "macarabic"sv, 10004 }, // Arabic (Mac)
		{ "machebrew"sv, 10005 }, // Hebrew (Mac)
		{ "macgreek"sv, 10006 }, // Greek (Mac)
		{ "maccyrillic"sv, 10007 }, // Cyrillic (Mac)
		{ "macchinesesimp"sv, 10008 }, // MAC Simplified Chinese (GB 2312); Chinese Simplified (Mac)
		{ "macromanian"sv, 10010 }, // Romanian (Mac)
		{ "macukrainian"sv, 10017 }, // Ukrainian (Mac)
		{ "macthai"sv, 10021 }, // Thai (Mac)
		{ "macce"sv, 10029 }, // MAC Latin 2; Central European (Mac)
		{ "macicelandic"sv, 10079 }, // Icelandic (Mac)
		{ "macturkish"sv, 10081 }, // Turkish (Mac)
		{ "maccroatian"sv, 10082 }, // Croatian (Mac)
		{ "chinesecns"sv, 20000 }, // CNS Taiwan; Chinese Traditional (CNS)
		{ "cp20001"sv, 20001 }, // TCA Taiwan
		{ "chineseeten"sv, 20002 }, // Eten Taiwan; Chinese Traditional (Eten)
		{ "cp20003"sv, 20003 }, // IBM5550 Taiwan
		{ "cp20004"sv, 20004 }, // TeleText Taiwan
		{ "cp20005"sv, 20005 }, // Wang Taiwan
		{ "ia5"sv, 20105 }, // IA5 (IRV International Alphabet No. 5, 7-bit); Western European (IA5)
		{ "ia5german"sv, 20106 }, // IA5 German (7-bit)
		{ "ia5swedish"sv, 20107 }, // IA5 Swedish (7-bit)
		{ "ia5norwegian"sv, 20108 }, // IA5 Norwegian (7-bit)
		{ "usascii"sv, 20127 }, // US-ASCII (7-bit)
		{ "cp20261"sv, 20261 }, // T.61
		{ "cp20269"sv, 20269 }, // ISO 6937 Non-Spacing Accent
		{ "ibm273"sv, 20273 }, // IBM EBCDIC Germany
		{ "ibm277"sv, 20277 }, // IBM EBCDIC Denmark-Norway
		{ "ibm278"sv, 20278 }, // IBM EBCDIC Finland-Sweden
		{ "ibm280"sv, 20280 }, // IBM EBCDIC Italy
		{ "ibm284"sv, 20284 }, // IBM EBCDIC Latin America-Spain
		{ "ibm285"sv, 20285 }, // IBM EBCDIC United Kingdom
		{ "ibm290"sv, 20290 }, // IBM EBCDIC Japanese Katakana Extended
		{ "ibm297"sv, 20297 }, // IBM EBCDIC France
		{ "ibm420"sv, 20420 }, // IBM EBCDIC Arabic
		{ "ibm423"sv, 20423 }, // IBM EBCDIC Greek
		{ "ibm424"sv, 20424 }, // IBM EBCDIC Hebrew
		{ "ebcdickoreanextended"sv, 20833 }, // IBM EBCDIC Korean Extended
		{ "ibmthai"sv, 20838 }, // IBM EBCDIC Thai
		{ "koi8r"sv, 20866 }, // Russian (KOI8-R); Cyrillic (KOI8-R)
		{ "ibm871"sv, 20871 }, // IBM EBCDIC Icelandic
		{ "ibm880"sv, 20880 }, // IBM EBCDIC Cyrillic Russian
		{ "ibm905"sv, 20905 }, // IBM EBCDIC Turkish
		{ "ibm00924"sv, 20924 }, // IBM EBCDIC Latin 1/Open System (1047 + Euro symbol)
		{ "eucjp"sv, 20932 }, // Japanese (JIS 0208-1990 and 0212-1990)
		{ "cp20936"sv, 20936 }, // Simplified Chinese (GB2312); Chinese Simplified (GB2312-80)
		{ "cp20949"sv, 20949 }, // Korean Wansung
		{ "cp1025"sv, 21025 }, // IBM EBCDIC Cyrillic Serbian-Bulgarian
		{ "koi8u"sv, 21866 }, // Ukrainian (KOI8-U); Cyrillic (KOI8-U)
		{ "iso88591"sv, 28591 }, // ISO 8859-1 Latin 1; Western European (ISO)
		{ "iso88592"sv, 28592 }, // ISO 8859-2 Central European; Central European (ISO)
		{ "iso88593"sv, 28593 }, // ISO 8859-3 Latin 3
		{ "iso88594"sv, 28594 }, // ISO 8859-4 Baltic
		{ "iso88595"sv, 28595 }, // ISO 8859-5 Cyrillic
		{ "iso88596"sv, 28596 }, // ISO 8859-6 Arabic
		{ "iso88597"sv, 28597 }, // ISO 8859-7 Greek
		{ "iso88598"sv, 28598 }, // ISO 8859-8 Hebrew; Hebrew (ISO-Visual)
		{ "iso88599"sv, 28599 }, // ISO 8859-9 Turkish
		{ "iso885913"sv, 28603 }, // ISO 8859-13 Estonian
		{ "iso885915"sv, 28605 }, // ISO 8859-15 Latin 9
		{ "europa"sv, 29001 }, // Europa 3
		{ "iso88598i"sv, 38598 }, // ISO 8859-8 Hebrew; Hebrew (ISO-Logical)
		{ "iso2022jp"sv, 50220 }, // ISO 2022 Japanese with no halfwidth Katakana; Japanese (JIS)
		{ "csiso2022jp"sv, 50221 }, // ISO 2022 Japanese with halfwidth Katakana; Japanese (JIS-Allow 1 byte Kana)
		{ "iso2022jp"sv, 50222 }, // ISO 2022 Japanese JIS X 0201-1989; Japanese (JIS-Allow 1 byte Kana - SO/SI)
		{ "iso2022kr"sv, 50225 }, // ISO 2022 Korean
		{ "cp50227"sv, 50227 }, // ISO 2022 Simplified Chinese; Chinese Simplified (ISO 2022)
		{ "eucjp"sv, 51932 }, // EUC Japanese
		{ "euccn"sv, 51936 }, // EUC Simplified Chinese; Chinese Simplified (EUC)
		{ "euckr"sv, 51949 }, // EUC Korean
		{ "hzgb2312"sv, 52936 }, // HZ-GB2312 Simplified Chinese; Chinese Simplified (HZ)
		{ "gb18030"sv, 54936 }, // WinXP+: GB18030 Simplified Chinese (4 byte); Chinese Simplified (GB18030)
		{ "isciide"sv, 57002 }, // ISCII Devanagari
		{ "isciibe"sv, 57003 }, // ISCII Bangla
		{ "isciita"sv, 57004 }, // ISCII Tamil
		{ "isciite"sv, 57005 }, // ISCII Telugu
		{ "isciias"sv, 57006 }, // ISCII Assamese
		{ "isciior"sv, 57007 }, // ISCII Odia
		{ "isciika"sv, 57008 }, // ISCII Kannada
		{ "isciima"sv, 57009 }, // ISCII Malayalam
		{ "isciigu"sv, 57010 }, // ISCII Gujarati
		{ "isciipa"sv, 57011 }, // ISCII Punjabi
		{ "utf7"sv, 65000 }, // Unicode (UTF-7)
		{ "utf8"sv, 65001 }, // Unicode (UTF-8)
	};
	std::string encodingStr;
	for (auto e : encoding)
	{
		if (e == '-' || e == '_' || e == ' ')
			continue;
		e = std::tolower(e);
		encodingStr.push_back(e);
	};
	if (encodingStr[0] == 'x')
		encodingStr.erase(0);
	auto it = encodings.find(encodingStr);
	if (it == encodings.cend())
	{
		// Parse it as a numeric codepage.
		ULONG encULong = strtoul(encodingStr.c_str(), NULL, 0);
		// If returns 0, could be encoding 0 (Windows ANSI codepage)
		if (encULong == 0 && encodingStr != "0"sv)
			return -1;
		return encULong;
	}

	return it->second;
}

bool Extension::Internal_GetTextWithEncoding(const std::string_view encoding, const std::string_view inputBytes, std::tstring &outputString, int socketID)
{
#ifndef _WIN32
#define OUTPUTCHAR "UTF-8"
#elif defined(_UNICODE)
#define OUTPUTCHAR "UTF-16"
#else
#define OUTPUTCHAR "ANSI"
#endif
	bool allValidChars;

	// Can't use "packet" for *incoming* text
	if (encoding == "PACKET"sv)
		return globals->CreateError(socketID, _T("\"Packet\" encoding can't be used for incoming data, only outgoing data.")), false;
	if (encoding == "ANSI"sv) {
		return globals->CreateError(socketID, _T("Can't read incoming data with \"ANSI\" encoding, the system codepage can vary per machine, "
			"and will produce inconsistent results. Use a Unicode format like UTF-8, or explicitly specify a Windows codepage.")), false;
	}

	if (encoding == "UTF-16"sv)
	{
		// Not a multiple of 2 bytes; can't be UTF-16!
		if (inputBytes.size() % 2)
		{
			return globals->CreateError(socketID, _T("Couldn't convert incoming data of size %zu bytes as UTF-16; each UTF-16 code unit is a set of 2 bytes, so an odd number isn't valid UTF-16."),
				inputBytes.size()), false;
		}
#if _WIN32
		bool allValidChars;
		outputString = DarkEdif::WideToTString(std::wstring_view((const wchar_t *)inputBytes.data(), inputBytes.size() / sizeof(wchar_t)), &allValidChars);
		if (!allValidChars)
		{
			globals->CreateError(socketID, _T("Couldn't fully convert incoming data \"%.*s...\" to local codepage. Are you using a non-Unicode runtime, or is the text not encoded properly?"),
				std::min(outputString.size(), 15U), outputString.c_str());
		}
		return true;
#else
		return globals->CreateError(socketID, _T("Reading from \"UTF-16\" encoding to " OUTPUTCHAR " isn't supported on non-Windows.")), false;
#endif
	}
	if (encoding == "UTF-8"sv)
	{
		outputString = DarkEdif::UTF8ToTString(inputBytes, &allValidChars);
		if (!allValidChars)
		{
			globals->CreateError(socketID, _T("Couldn't convert incoming data \"%.*s...\" to local codepage. Are you using a non-Unicode runtime, or is the text not encoded properly?"),
				std::min(outputString.size(), 15U), outputString.c_str());
		}
		return true;
	}

	// Parse it as a numeric codepage.
	ULONG encULong = encodingToCodePage(encoding);

	// Parsing failed, or returned "0" which is ACP - ANSI
	if (encULong == -1 && encoding != "0"sv)
	{
		return globals->CreateError(socketID, _T("Couldn't understand encoding \"%s\". Maybe use \"UTF-8\"?"),
			DarkEdif::ANSIToTString(encoding).c_str()), false;
	}

	// Delegate!
	if (encULong == 65001 /* CP_UTF8 */)
	{
		globals->ReportInfo(socketID, _T("Use of \"%s\" as an encoding for UTF-8 is unnecessary; just use \"UTF-8\"."), DarkEdif::ANSIToTString(encoding).c_str());
		return Internal_GetTextWithEncoding("UTF-8"sv, inputBytes, outputString, socketID);
	}

#if _WIN32
	std::wstring temp;
	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = MultiByteToWideChar(encULong, 0, inputBytes.data(), inputBytes.size(), NULL, 0);
	if (length <= 0)
	{
		return globals->CreateError(socketID, _T("Failed to convert between codepage %u to " OUTPUTCHAR ", input string is not encoded correctly."),
			encULong), false;
	}

	temp.resize(length, L'\0');

	// Actually convert
	length = MultiByteToWideChar(encULong, 0, inputBytes.data(), inputBytes.size(), temp.data(), temp.size());
	if (length <= 0)
	{
		outputString.clear();
		return globals->CreateError(socketID, _T("Failed to convert from codepage %u to " OUTPUTCHAR ", input string is not encoded correctly."), encULong), false;
	}
	outputString = DarkEdif::WideToTString(temp, &allValidChars);
	if (!allValidChars)
	{
		std::tstring cpName;
#if _WIN32
		cpName = _T(' ');
		CPINFOEX cpInfoEx;
		if (GetCPInfoEx(encULong, 0, &cpInfoEx) == 0)
			cpName = _T("(error reading codepage)"sv);
		else
			cpName = cpInfoEx.CodePageName;
#endif
		globals->CreateError(socketID, _T("Failed to convert string \"%.*s...\" fully. It was read as codepage %u okay, but couldn't convert it to your system codepage%s."),
			std::min(outputString.size(), 15U), outputString.c_str(),
			encULong, cpName.c_str());
	}

	return true;
#else // !_WIN32
	// While most of the DarkEdif text conversion functions are defined on Android/iOS, they're wrappers and no-ops.

	return globals->CreateError(socketID, _T("Converting text from encoding %u to " OUTPUTCHAR " isn't possible on non-Windows."), encULong), false;
#endif
#undef OUTPUTCHAR
}
bool Extension::Internal_SetTextWithEncoding(const std::string_view encoding, const std::tstring_view inputText, std::string &outputBytes, int socketID)
{
	// Send built packet
	if (encoding == "PACKET"sv)
	{
		if (packetBeingBuilt.empty())
			return globals->CreateError(socketID, _T("\"Packet\" encoding chosen, but is empty.")), false;
		if (!inputText.empty())
		{
			return globals->CreateError(socketID, _T("\"Packet\" encoding chosen, but text \"%.*s...\" provided too."),
				std::min(inputText.size(), 15U), inputText.data()), false;
		}
		outputBytes = packetBeingBuilt;
		return true;
	}
	else if (encoding == "ANSI"sv)
	{
		bool allValidChars;
		outputBytes = DarkEdif::TStringToANSI(inputText, &allValidChars);

		std::tstring cpName;
#if _WIN32
		cpName = _T(' ');
		CPINFOEX cpInfoEx;
		if (GetCPInfoEx(CP_ACP, 0, &cpInfoEx) == 0)
			cpName = _T("(error reading codepage)"sv);
		else
			cpName = cpInfoEx.CodePageName;
#endif
		if (!allValidChars)
			return globals->CreateError(socketID, _T("Couldn't convert text \"%.*s...\" to system's ANSI codepage%s. Maybe use UTF-8?"),
				DarkEdif::GetCurrentFusionEventNum(this), std::min(inputText.size(), 15U), inputText.data(), cpName.c_str()), false;
		return true;
	}
	if (encoding == "UTF-16"sv)
	{
		const std::wstring msgWide = DarkEdif::TStringToWide(inputText);
		outputBytes.assign((const char *)msgWide.c_str(), msgWide.size() * sizeof(wchar_t));
		return true;
	}
	if (encoding == "UTF-8"sv)
	{
		outputBytes = DarkEdif::TStringToUTF8(inputText);
		return true;
	}

	// Parse it as a numeric codepage.
	ULONG encULong = strtoul(std::string(encoding).c_str(), NULL, 0);

	// Parsing failed, or returned "0" which is ACP - ANSI
	if (encULong == 0 && encoding != "0"sv)
	{
		return globals->CreateError(socketID, _T("Couldn't understand encoding \"%s\". Maybe use \"UTF-8\"?"),
			DarkEdif::ANSIToTString(encoding).c_str()), false;
	}
	// Delegate!
	if (encULong == CP_UTF8)
	{
		globals->ReportInfo(socketID, _T("Use of \"%s\" as an encoding for UTF-8 is unnecessary; just use \"UTF-8\"."), DarkEdif::ANSIToTString(encoding).c_str());
		return Internal_SetTextWithEncoding("UTF-8"sv, inputText, outputBytes, socketID);
	}

#if _WIN32
	std::wstring input = DarkEdif::TStringToWide(inputText);
	BOOL someFailed = FALSE;

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = WideCharToMultiByte(encULong, 0, input.data(), input.size(), NULL, 0, 0, &someFailed);
	if (length <= 0)
	{
		return globals->CreateError(socketID, _T("Failed to convert between UTF-16 and codepage %u, input string \"%.*s...\" is broken."), encULong,
			encULong, std::min(inputText.size(), 15U), inputText.data()), false;
	}

	if (someFailed != FALSE)
	{
		return globals->CreateError(socketID, _T("Converting text \"%.*s...\" from UTF-16 to encoding %u resulted in replaced characters."),
			std::min(inputText.size(), 15U), inputText.data(), encULong), false;
	}

	outputBytes.resize(length, '\0');

	// Actually convert
	length = WideCharToMultiByte(encULong, 0, input.data(), input.size(), outputBytes.data(), outputBytes.size(), 0, NULL);
	if (length <= 0)
	{
		outputBytes.clear();
		return globals->CreateError(socketID, _T("Failed to convert between UTF-16 and codepage %u, input string \"%.*s...\" is broken."),
			encULong, std::min(inputText.size(), 15U), inputText.data()), false;
	}
	return true;
#else // !_WIN32
	// While most of the DarkEdif text conversion functions are defined on Android/iOS, they're wrappers and no-ops.

	return globals->CreateError(socketID, _T("Converting text \"%.*s...\" from UTF-16 to encoding %u isn't possible on non-Windows."),
		std::min(inputText.size(), 15U), inputText.data(), encULong), false;
#endif
}


std::tstring Extension::Internal_GetIPFromSockaddr(sockaddr_storage* sockadd, size_t sockaddSize)
{
	char buffer[256];
	if (sockadd->ss_family == AF_INET)
	{
		sprintf_s(buffer, sizeof(buffer), "%s:%d",
			inet_ntoa(((struct sockaddr_in*)sockadd)->sin_addr),
			ntohs(((struct sockaddr_in*)sockadd)->sin_port));
	}
	else if (sockadd->ss_family == AF_INET6)
	{
		int length = sizeof(buffer) - 1;

#ifdef _WIN32
		WSAAddressToStringA((sockaddr*)sockadd,
			sockaddSize,
			0,
			buffer,
			(LPDWORD)&length);
#else
		inet_ntop(AF_INET6,
			&((struct sockaddr_in6*)sockadd)->sin6_addr,
			buffer,
			length);
		// Add local port to end
		sprintf_s(buffer + strlen(buffer),
			sizeof(buffer) - strlen(buffer) - 1,
			":%d",
			ntohs(((struct sockaddr_in6*)sockadd)->sin6_port));
#endif

		// IPv4 wrapped inside IPv6; unwrap it
		if (globals->unwrapIPv6)
		{
			char output[256];
			if (strncmp(buffer, "[::ffff:", 8) == 0)
			{
				// Start search for "]" at offset of 15
				// 8 due to "[::ffff:" -> 8 chars
				// 7 due to "1.2.3.4" -> 7 chars
				for (std::size_t i = 15, len = strnlen(&buffer[15], 64 - 15) + 15; i < len; ++i)
				{
					if (buffer[i] == ']')
					{
						// Skip the first 8 chars of "[::ffff:"
						assert(std::size(output) >= i - 8 && "IP output buffer too small");
						memmove(output, &buffer[8], i - 8);
						strcpy(&output[i - 8], &buffer[i + 1]); // append port
						return DarkEdif::UTF8ToTString(output);
					}
				}
			}
			// localhost is ::1 in IPv6, but 127.0.0.1 for IPv4 local
			else if (strncmp(buffer, "[::1]", 5) == 0)
			{
				sprintf_s(output, std::size(output), "127.0.0.1:%s", &buffer[6]);
				return DarkEdif::UTF8ToTString(output);
			}
		}
	}
	else
		return _T("Incompatible address family"s);

	return DarkEdif::UTF8ToTString(buffer);
}
