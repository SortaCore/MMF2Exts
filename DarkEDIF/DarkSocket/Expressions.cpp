#include "Common.h"

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================

// ID = 0
tchar * Extension::GetErrors(int clear)
{
	ThreadSafe_Start();
	tstring temp = LastError;
	if (clear)
		LastError = _T("");	//Clear error if user chose to
	ThreadSafe_End();
	return Runtime.CopyString(temp.c_str()); // ReturnString will fail
}

// ID = 1
tchar * Extension::GetReports(int clear)
{
	ThreadSafe_Start();
	tstring temp = CompleteStatus;
	if (clear)
		CompleteStatus = _T("");	//Clear report if user chose to
	ThreadSafe_End();
	return Runtime.CopyString(temp.c_str()); // ReturnString will fail
}

// ID = 2
int Extension::GetLastMessageSocketID()
{
	ThreadSafe_Start();
	int temp = Returns[0].Socket;
	ThreadSafe_End();
	return temp;
}

// ID = 3
tchar * Extension::GetLastMessageText()
{
	ThreadSafe_Start();
	tstring temp = (tchar *)Returns[0].Message;
	ThreadSafe_End();
	return Runtime.CopyString(temp.c_str());
}

// ID = 4
size_t Extension::GetLastMessageAddress()
{
	ThreadSafe_Start();
	size_t temp = (size_t)(&Returns[0].Message);
	ThreadSafe_End();
	return temp;
}

// ID = 5
size_t Extension::GetLastMessageSize()
{
	ThreadSafe_Start();
	size_t temp = Returns[0].MessageSize;
	ThreadSafe_End();
	return temp;
}

// ID = 6
int Extension::GetNewSocketID()
{
	ThreadSafe_Start();
	int temp = NewSocketID-1;
	ThreadSafe_End();
	return temp;
}

// ID = 7
int Extension::GetSocketIDForLastEvent()
{
	ThreadSafe_Start();
	int temp = LastReturnSocketID;
	ThreadSafe_End();
	return temp;
}
// ID = 8
int Extension::GetPortFromType(tchar * t)
{
	if (_tcsicmp(t, "TCPMUX") == 0)
		return IPPORT_TCPMUX; else
	if (_tcsicmp(t, "HTTP") == 0)
		return 80; else
	if (_tcsicmp(t, "ECHO") == 0)
		return IPPORT_ECHO; else
	if (_tcsicmp(t, "DISCARD") == 0)
		return IPPORT_DISCARD; else
	if (_tcsicmp(t, "SYSTAT") == 0)
		return IPPORT_SYSTAT; else
	if (_tcsicmp(t, "DAYTIME") == 0)
		return IPPORT_DAYTIME; else
	if (_tcsicmp(t, "NETSTAT") == 0)
		return IPPORT_NETSTAT; else
	if (_tcsicmp(t, "QOTD") == 0)
		return IPPORT_QOTD; else
	if (_tcsicmp(t, "MSP") == 0)
		return IPPORT_MSP; else
	if (_tcsicmp(t, "CHARGEN") == 0)
		return IPPORT_CHARGEN; else
	if (_tcsicmp(t, "FTP_DATA") == 0 || _tcsicmp(t, "FTP DATA") == 0)
		return IPPORT_FTP_DATA; else
	if (_tcsicmp(t, "FTP") == 0)
		return IPPORT_FTP; else
	if (_tcsicmp(t, "TELNET") == 0)
		return IPPORT_TELNET; else
	if (_tcsicmp(t, "SMTP") == 0)
		return IPPORT_SMTP; else
	if (_tcsicmp(t, "TIMESERVER") == 0)
		return IPPORT_TIMESERVER; else
	if (_tcsicmp(t, "NAMESERVER") == 0)
		return IPPORT_NAMESERVER; else
	if (_tcsicmp(t, "WHOIS") == 0)
		return IPPORT_WHOIS; else
	if (_tcsicmp(t, "MTP") == 0)
		return IPPORT_MTP; else
	if (_tcsicmp(t, "TFTP") == 0)
		return IPPORT_TFTP; else
	if (_tcsicmp(t, "RJE") == 0)
		return IPPORT_RJE; else
	if (_tcsicmp(t, "FINGER") == 0)
		return IPPORT_FINGER; else
	if (_tcsicmp(t, "TTYLINK") == 0)
		return IPPORT_TTYLINK; else
	if (_tcsicmp(t, "SUPDUP") == 0)
		return IPPORT_SUPDUP; else
	if (_tcsicmp(t, "POP3") == 0) /* TCP only begins */
		return IPPORT_POP3; else
	if (_tcsicmp(t, "NTP") == 0)
		return IPPORT_NTP; else
	if (_tcsicmp(t, "EPMAP") == 0)
		return IPPORT_EPMAP; else
	if (_tcsicmp(t, "NETBIOS NS") == 0 || _tcsicmp(t, "NETBIOS_NS") == 0)
		return IPPORT_NETBIOS_NS; else
	if (_tcsicmp(t, "NETBIOS_DGM") == 0 || _tcsicmp(t, "NETBIOS DGM") == 0)
		return IPPORT_NETBIOS_DGM; else
	if (_tcsicmp(t, "NETBIOS_SSN") == 0 || _tcsicmp(t, "NETBIOS SSN") == 0)
		return IPPORT_NETBIOS_SSN; else
	if (_tcsicmp(t, "IMAP") == 0)
		return IPPORT_IMAP; else
	if (_tcsicmp(t, "SNMP") == 0)
		return IPPORT_SNMP; else
	if (_tcsicmp(t, "SNMP_TRAP") == 0 || _tcsicmp(t, "SNMP TRAP") == 0)
		return IPPORT_SNMP_TRAP; else
	if (_tcsicmp(t, "IMAP3") == 0)
		return IPPORT_IMAP3; else
	if (_tcsicmp(t, "LDAP") == 0)
		return IPPORT_LDAP; else
	if (_tcsicmp(t, "HTTPS") == 0)
		return IPPORT_HTTPS; else
	if (_tcsicmp(t, "MICROSOFT_DS") == 0 || _tcsicmp(t, "MICROSOFT DS") == 0 || _tcsicmp(t, "DS") == 0)
		return IPPORT_MICROSOFT_DS; else
	if (_tcsicmp(t, "SNMP_TRAP") == 0 || _tcsicmp(t, "SNMP TRAP") == 0)
		return IPPORT_SNMP_TRAP; else
	if (_tcsicmp(t, "EXECSERVER") == 0)
		return IPPORT_EXECSERVER; else
	if (_tcsicmp(t, "LOGINSERVER") == 0)
		return IPPORT_LOGINSERVER; else
	if (_tcsicmp(t, "CMDSERVER") == 0)
		return IPPORT_CMDSERVER; else
	if (_tcsicmp(t, "EFSSERVER") == 0)
		return IPPORT_EFSSERVER; else /* TCP only ends, UDP only begins */
	if (_tcsicmp(t, "BIFFUDP") == 0)
		return IPPORT_BIFFUDP; else
	if (_tcsicmp(t, "WHOSERVER") == 0)
		return IPPORT_WHOSERVER; else
	if (_tcsicmp(t, "ROUTESERVER") == 0)
		return IPPORT_ROUTESERVER; else
	return -1;
}

// ID = 9
size_t Extension::PacketForm_GetAddress()
{
	ThreadSafe_Start();
	size_t t = (size_t)(PacketFormLocation);
	ThreadSafe_End();
	return t;
}

// ID = 10
size_t Extension::PacketForm_GetSize()
{
	ThreadSafe_Start();
	size_t t = PacketFormSize;
	ThreadSafe_End();
	return t;
}

// ID = 11
char Extension::LastMessage_GetByte(size_t WhereFrom)
{
	ThreadSafe_Start();
	if (Returns.size() > 0)
	{
		if ((Returns[0].MessageSize - WhereFrom) >= sizeof(char))
		{
			char c = *((char *)((size_t)Returns[0].Message+WhereFrom));
			ThreadSafe_End();
			return c;
		}
		else
		{
			ThreadSafe_End();
			Explode("Could not retrieve byte: reading beyond the packet.");
		}
	}
	else
	{
		ThreadSafe_End();
		Explode("Could not retrieve byte: no messages in queue.");
	}
	return 0;
}
// ID = 12
short Extension::LastMessage_GetShort(size_t WhereFrom)
{
	ThreadSafe_Start();
	if (Returns.size() > 0)
	{
		if ((Returns[0].MessageSize - WhereFrom) >= sizeof(short))
		{
			short s = *((short *)((size_t)Returns[0].Message+WhereFrom));
			ThreadSafe_End();
			return s;
		}
		else
		{
			ThreadSafe_End();
			Explode("Could not retrieve short: reading beyond the packet.");
		}
	}
	else
	{
		ThreadSafe_End();
		Explode("Could not retrieve short: no messages in queue.");
	}
	return 0;
}
// ID = 13
int Extension::LastMessage_GetInteger(size_t WhereFrom)
{
	ThreadSafe_Start();
	if (Returns.size() > 0)
	{
		if ((Returns[0].MessageSize - WhereFrom) >= sizeof(int))
		{
			int i = *((int *)((size_t)Returns[0].Message+WhereFrom));
			ThreadSafe_End();
			return i;
		}
		else
		{
			ThreadSafe_End();
			Explode("Could not retrieve integer: reading beyond the packet.");
		}
	}
	else
	{
		ThreadSafe_End();
		Explode("Could not retrieve integer: no messages in queue.");
	}
	return 0;
}
// ID = 14
long Extension::LastMessage_GetLong(size_t WhereFrom)
{
	ThreadSafe_Start();
	if (Returns.size() > 0)
	{
		if ((Returns[0].MessageSize - WhereFrom) >= sizeof(long))
		{
			long l = *((long *)((size_t)Returns[0].Message+WhereFrom));
			ThreadSafe_End();
			return l;
		}
		else
		{
			ThreadSafe_End();
			Explode("Could not retrieve long: reading beyond the packet.");
		}
	}
	else
	{
		ThreadSafe_End();
		Explode("Could not retrieve long: no messages in queue.");
	}
	return 0;
}
// ID = 15
float Extension::LastMessage_GetFloat(size_t WhereFrom)
{
	ThreadSafe_Start();
	if (Returns.size() > 0)
	{
		if ((Returns[0].MessageSize - WhereFrom) >= sizeof(float))
		{
			float f = *((float *)((size_t)Returns[0].Message+WhereFrom));
			ThreadSafe_End();
			return f;
		}
		else
		{
			ThreadSafe_End();
			Explode("Could not retrieve float: reading beyond the packet.");
		}
	}
	else
	{
		ThreadSafe_End();
		Explode("Could not retrieve float: no messages in queue.");
	}
	return 0;
}
// ID = 16
double Extension::LastMessage_GetDouble(size_t WhereFrom)
{
	ThreadSafe_Start();
	if (Returns.size() > 0)
	{
		if ((Returns[0].MessageSize - WhereFrom) >= sizeof(double))
		{
			double d = *((double *)((size_t)Returns[0].Message+WhereFrom));
			ThreadSafe_End();
			return d;
		}
		else
		{
			ThreadSafe_End();
			Explode("Could not retrieve double: reading beyond the packet.");
		}
	}
	else
	{
		ThreadSafe_End();
		Explode("Could not retrieve double: no messages in queue.");
	}
	return 0;
}
// ID = 17
char * Extension::LastMessage_GetString(size_t WhereFrom, size_t SizeOfString)
{
	if (SizeOfString > 0)
	{
		ThreadSafe_Start();
		if (Returns.size() > 0)
		{
			if ((Returns[0].MessageSize - WhereFrom) >= (SizeOfString*sizeof(char)))
			{
				string s((char *)Returns[0].Message, WhereFrom, SizeOfString);
				ThreadSafe_End();
				return Runtime.CopyString(s.c_str());
			}
			else
			{
				ThreadSafe_End();
				Explode("Could not retrieve ascii string: reading beyond the packet.");
			}
		}
		else
		{
			ThreadSafe_End();
			Explode("Could not retrieve ascii string: no messages in queue.");
		}
	}
	else
		Explode("Size of string is under 1.");
	return "";
}
// ID = 18
wchar_t * Extension::LastMessage_GetWString(size_t WhereFrom, size_t SizeOfString)
{
	if (SizeOfString > 0)
	{
		ThreadSafe_Start();
		if (Returns.size() > 0)
		{
			if ((Returns[0].MessageSize - WhereFrom) >= (SizeOfString*sizeof(wchar_t)))
			{
				wstring s((wchar_t *)Returns[0].Message, WhereFrom, SizeOfString);
				ThreadSafe_End();
				return _wcsdup(s.c_str());
			}
			else
			{
				ThreadSafe_End();
				Explode("Could not retrieve ascii string: reading beyond the packet.");
			}
		}
		else
		{
			ThreadSafe_End();
			Explode("Could not retrieve unicode string: no messages in queue.");
		}
	}
	else
		Explode("Size of unicode string is under 1.");
	return L"";
}

// ID = 19
unsigned short Extension::PacketForm_RunOnesComplement(size_t WhereFrom, size_t SizeOfBank)
{
	// 1's complement of VAR = ~VAR.

	// Simple huh. Why couldn't they just put it that way, rather than making some long complex name.
	/*	Excempt from RFC about ICMP checksum generation:

	(1)	Adjacent octets to be checksummed are paired to form 16-bit
        integers, and the 1's complement sum of these 16-bit integers is
        formed.
	(2)	To generate a checksum, the checksum field itself is cleared,
		the 16-bit 1's complement sum is computed over the octets
		concerned, and the 1's complement of this sum is placed in the
        checksum field.
*/
#define _16BITS 2	// 16 bits == 2 bytes. Used for readability.

	unsigned long sum = 0;	// This is used to sum up all the data
	size_t addr = ((size_t)PacketFormLocation)+WhereFrom; // Read from here originally

	//  This is the inner loop
    while ( SizeOfBank > 1 )
	{
		sum += *((unsigned short *)addr);	// Read 16-bit integer from the FormPacket and add to sum
		SizeOfBank -= _16BITS;				// I've read 16 bits bytes, so remove from remainder
		addr += _16BITS;					// Read from 16 bits further in the current address
    }

    //  Add left-over byte, if any 
    if ( SizeOfBank > 0 )
        sum += *((unsigned char *)addr);		// If there's an odd number of bytes in the data, add the remainding char to the sum

    //  Fold the 32-bit ulong sum to 16 bits short (neat part)
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	// Get the one's complement of the sum
    unsigned long cksum = ~sum;

	// Rejoice evermore, for this annoying piece of code finally works.
	unsigned short checksum = (unsigned short)cksum;
	return checksum;
}
