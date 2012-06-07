#include "common.h"
// ============================================================================
//
// UNREFENCED FUNCTIONS
// 
// ============================================================================

// Another text -> number from macro function. Moved out the action for simplicity.
int Extension::Unreferenced_WorkOutAddressFamily(tchar * t)
{
	if (_tcsicmp(t, _T("UNSPEC")) == 0 || _tcsicmp(t, _T("UNSPECIFIED")) == 0)
		return AF_UNSPEC; else
	if (_tcsicmp(t, _T("UNIX")) == 0)
		return AF_UNIX; else
	if (_tcsicmp(t, _T("INET")) == 0 || _tcsicmp(t, _T("INTERNET")) == 0 || _tcsicmp(t, _T("INTERNET v4")) == 0)
		return AF_INET; else
	if (_tcsicmp(t, _T("IMPLINK")) == 0)
		return AF_IMPLINK; else
	if (_tcsicmp(t, _T("PUP")) == 0)
		return AF_PUP; else
	if (_tcsicmp(t, _T("CHAOS")) == 0)
		return AF_CHAOS; else
	if (_tcsicmp(t, _T("NS")) == 0)
		return AF_NS; else
	if (_tcsicmp(t, _T("IPX")) == 0)
		return AF_IPX; else
	if (_tcsicmp(t, _T("ISO")) == 0)
		return AF_ISO; else
	if (_tcsicmp(t, _T("OSI")) == 0)
		return AF_OSI; else
	if (_tcsicmp(t, _T("ECMA")) == 0)
		return AF_ECMA; else
	if (_tcsicmp(t, _T("DATAKIT")) == 0)
		return AF_DATAKIT; else
	if (_tcsicmp(t, _T("CCITT")) == 0)
		return AF_CCITT; else
	if (_tcsicmp(t, _T("SNA")) == 0)
		return AF_SNA; else
	if (_tcsicmp(t, _T("DECnet")) == 0)
		return AF_DECnet; else
	if (_tcsicmp(t, _T("DLI")) == 0)
		return AF_DLI; else
	if (_tcsicmp(t, _T("LAT")) == 0)
		return AF_LAT; else
	if (_tcsicmp(t, _T("HYLINK")) == 0)
		return AF_HYLINK; else
	if (_tcsicmp(t, _T("APPLETALK")) == 0)
		return AF_APPLETALK; else
	if (_tcsicmp(t, _T("NETBIOS")) == 0)
		return AF_NETBIOS; else
	if (_tcsicmp(t, _T("VOICEVIEW")) == 0)
		return AF_VOICEVIEW; else
	if (_tcsicmp(t, _T("FIREFOX")) == 0)
		return AF_FIREFOX; else
	if (_tcsicmp(t, _T("BAN")) == 0 || _tcsicmp(t, _T("BANYAN")) == 0)
		return AF_BAN; else
	if (_tcsicmp(t, _T("ATM")) == 0)
		return AF_ATM; else
	if (_tcsicmp(t, _T("INET6")) == 0 || _tcsicmp(t, _T("ITERNET6")) == 0 || _tcsicmp(t, _T("INTERNET v6")) == 0)
		return AF_INET6; else
	if (_tcsicmp(t, _T("CLUSTER")) == 0)
		return AF_CLUSTER; else
	if (_tcsicmp(t, _T("12844")) == 0)
		return AF_12844; else
	if (_tcsicmp(t, _T("IRDA")) == 0)
		return AF_IRDA; else
	if (_tcsicmp(t, _T("NETDES")) == 0)
		return AF_NETDES; else
	if (_tcsicmp(t, _T("TCNPROCESS")) == 0)
		return AF_TCNPROCESS; else
	if (_tcsicmp(t, _T("TCNMESSAGE")) == 0)
		return AF_TCNMESSAGE; else
	if (_tcsicmp(t, _T("ICLFXBM")) == 0)
		return AF_ICLFXBM; else
	if (_tcsicmp(t, _T("BTH")) == 0 || _tcsicmp(t, _T("Bluetooth")) == 0)
		return AF_BTH; else
	return -1;
}

// A further text -> number from macro function. Moved out the action for simplicity.
int Extension::Unreferenced_WorkOutSocketType(tchar * t)
{
	if (_tcscmp(t, _T("STREAM")) == 0)
		return SOCK_STREAM; else
	if (_tcscmp(t, _T("DGRAM")) == 0|| _tcscmp(t, _T("DATAGRAM")) == 0)
		return SOCK_DGRAM; else
	if (_tcscmp(t, _T("RAW")) == 0)
		return SOCK_RAW; else
	if (_tcscmp(t, _T("RDM")) == 0)
		return SOCK_RDM; else
	if (_tcscmp(t, _T("SEQPACKET")) == 0)
		return SOCK_SEQPACKET; else
	return -1;
}

// A simple text -> number from macro function. Moved out the action for simplicity.
int Extension::Unreferenced_WorkOutProtocolType(tchar * t)
{
	// Using t==_T("Type") fails, as both are pointers
	if (_tcscmp(t, _T("ICMP")) == 0)
		return IPPROTO_ICMP; else
	if (_tcscmp(t, _T("IGMP")) == 0)
		return IPPROTO_IGMP; else
	if (_tcscmp(t, _T("GGP")) == 0)
		return IPPROTO_GGP; else
	if (_tcscmp(t, _T("IPv4")) == 0)
		return IPPROTO_IPV4; else
	if (_tcscmp(t, _T("ST")) == 0)
		return IPPROTO_ST; else
	if (_tcscmp(t, _T("TCP")) == 0)
		return IPPROTO_TCP; else
	if (_tcscmp(t, _T("CBT")) == 0)
		return IPPROTO_CBT; else
	if (_tcscmp(t, _T("EGP")) == 0)
		return IPPROTO_EGP; else
	if (_tcscmp(t, _T("IGP")) == 0)
		return IPPROTO_IGP; else
	if (_tcscmp(t, _T("PUP")) == 0)
		return IPPROTO_PUP; else
	if (_tcscmp(t, _T("UDP")) == 0)
		return IPPROTO_UDP; else
	if (_tcscmp(t, _T("IDP")) == 0)
		return IPPROTO_IDP; else
	if (_tcscmp(t, _T("RDP")) == 0)
		return IPPROTO_RDP; else
	if (_tcscmp(t, _T("IPv6")) == 0)
		return IPPROTO_IPV6; else
	if (_tcscmp(t, _T("IPv6 Routing")) == 0||_tcscmp(t, _T("Routing")) == 0)
		return IPPROTO_ROUTING; else
	if (_tcscmp(t, _T("IPv6 Fragment")) == 0||_tcscmp(t, _T("Fragment")) == 0)
		return IPPROTO_FRAGMENT; else
	if (_tcscmp(t, _T("IPv6 ESP")) == 0||_tcscmp(t, _T("ESP")) == 0)
		return IPPROTO_ESP; else
	if (_tcscmp(t, _T("IPv6 AH")) == 0||_tcscmp(t, _T("AH")) == 0)
		return IPPROTO_AH; else
	if (_tcscmp(t, _T("IPv6 ICMP")) == 0||_tcscmp(t, _T("ICMPv6")) == 0)
		return IPPROTO_ICMPV6; else
	if (_tcscmp(t, _T("IPv6 None")) == 0||_tcscmp(t, _T("None")) == 0)
		return IPPROTO_NONE; else
	if (_tcscmp(t, _T("IPv6 DSTOPTS")) == 0||_tcscmp(t, _T("IPv6 DstOpts")) == 0||_tcscmp(t, _T("IPv6 Destination Options")) == 0)
		return IPPROTO_DSTOPTS; else
	if (_tcscmp(t, _T("ND")) == 0)
		return IPPROTO_ND; else
	if (_tcscmp(t, _T("ICLFXBM")) == 0)
		return IPPROTO_ICLFXBM; else
	if (_tcscmp(t, _T("PIM")) == 0)
		return IPPROTO_PIM; else
	if (_tcscmp(t, _T("PGM")) == 0)
		return IPPROTO_PGM; else
	if (_tcscmp(t, _T("L2TP")) == 0)
		return IPPROTO_L2TP; else
	if (_tcscmp(t, _T("SCTP")) == 0)
		return IPPROTO_SCTP; else
	if (_tcscmp(t, _T("RAW")) == 0)
		return IPPROTO_RAW; else
	if (_tcscmp(t, _T("MAX")) == 0)
		return IPPROTO_MAX; else
	//if (_tcscmp(t, _T("DARKNET")) == 0)
	//	return IPPROTO_DARK; else
	return -1;
}

// Report - For non-errors
void Extension::Unreferenced_Report(tchar * report = _T("Unknown report..."), int SocketID = -1)
{
	// Not from a thread
	if (SocketID == -1)
	{
		ThreadSafe_Start();
		CompleteStatus += report;
		CompleteStatus += _T("\r\n");
		ThreadSafe_End();
		
		if (UsePopups)
			MessageBox(NULL, report, _T("DarkSocket - Latest Report:"), MB_OK);
	}
	else // From a thread
	{
		tchar text [512];
		sprintf_s(text, 512, "Socket ID = %i >> %s\r\n", SocketID, report);
		
		ThreadSafe_Start(); // cause of last threadsafe_start()
		CompleteStatus += text;
		ThreadSafe_End();
		
		if (UsePopups)
		{
			tchar title [512];
			sprintf_s(title, 512, "DarkSocket - Latest Report from %i:", SocketID);
			MessageBox(NULL, report, title, MB_OK);
		}
	}

	CallEvent(MF2C_DEBUG_ON_NEW_STATUS);
}

// Explode - For errors
void Extension::Unreferenced_Error(tchar * error = _T("Unknown error..."), int SocketID = -1)
{
	// Not from a thread
	if (SocketID == -1)
	{
		ThreadSafe_Start();
		LastError += error;
		LastError += _T("\r\n");
		CompleteStatus += error;
		CompleteStatus += _T("\r\n");
		ThreadSafe_End();
		if (UsePopups)
			MessageBox(NULL, error, _T("DarkSocket - Latest Error:"), MB_OK);
	}
	else
	// From a thread
	{
		tchar text [255];
		sprintf_s(text, 255, "Socket ID = %i >> %s.", SocketID, error);
		ThreadSafe_Start();
		LastError += text;
		LastError += _T("\r\n");
		CompleteStatus += text;
		CompleteStatus += _T("\r\n");
		ThreadSafe_End();
		if (UsePopups)
		{
			tchar title [255];
			sprintf_s(title, 255, "DarkSocket - Latest Error from %i:", SocketID);
			MessageBox(NULL, error, title, MB_OK);
		}
	}
	CallEvent(MF2C_DEBUG_ON_ERROR);
	CallEvent(MF2C_DEBUG_ON_NEW_STATUS);
}

// Return - For threads reporting things to MMF2
void Extension::Unreferenced_ReturnToMMF(int ReturnAsI, int SocketID, void * Msg = _T("Unknown message..."), int Length = (_tcslen(_T("Unknown message..."))+1)*ts)
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
unsigned long Extension::Unreferenced_WorkOutInAddr(tchar * t)
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