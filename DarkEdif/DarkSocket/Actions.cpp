#include "Common.h"

// ============================================================================
//
// ACTIONS
//
// ============================================================================

/// DEBUG
// ID = 0
void Extension::TestReportAndExplode()
{
	Report("Test of action 2's Report() okay.");
	Explode("Test of action 2's Explode() okay.");
}
// ID = 1
void Extension::UsePopupMessages(int OnOrOff)
{
	lock.
	if (OnOrOff)
		UsePopups = true;
	else
		UsePopups = false;
	ThreadSafe_End();
}

// ============================================================================
//
/// CLIENT
//
// ============================================================================

// ID = 2
void Extension::ClientInitialise_Basic(const TCHAR * Hostname, int Port, const TCHAR * Protocol, const TCHAR * InitialText)
{
	if (Port < 1 || Port > 36535)
		Indirect_Error(-1, _T("%s: Port must be greater than 0 and less than 36536; you passed %d."), _T(__FUNCTION__), Port);
	if (_tcscmp(Hostname, _T("")) == 0)
		Explode("Hostname must not be blank.");
	// This is the basic action, so limit it to 3 protocols
	if (_tcscmp(Protocol, _T("TCP")) != 0 && _tcscmp(Protocol, _T("UDP")) != 0 && _tcscmp(Protocol, _T("ICMP")) != 0)
		Explode("Protocol unrecognised. Use \"TCP\", \"UDP\", or \"ICMP\". "
				"Or you may want to used the advanced functionality.");
	// Check all and continue
	if ((_tcscmp(Protocol, _T("TCP")) == 0 || _tcscmp(Protocol, _T("UDP")) == 0 || _tcscmp(Protocol, _T("ICMP")) == 0) &&
		(Port > -1 && Port < 36536) &&
		(_tcscmp(Hostname, _T("")) != 0))
	{
		// Move text to a number from a macro
		int Protocol2 = Unreferenced_WorkOutProtocolType(Protocol);
		// If error'd out report it.
		if (Protocol2 == -1)
		{
			Explode("Protocol unrecognised. Use the help file to see the available protocols.");
		}
		else
		{
			StructPassThru* Parameters = new StructPassThru;
			Parameters->para_AddressFamily = AF_INET;		// INET = INTERNET = IPv4
			Parameters->para_Ext = this;					// Don't use = Extension; !
			Parameters->para_client_hostname = Hostname;
			// No need to work around if someone wants to send "PACKET"...
			// They can use the form packet actions with a string :)
			if (_tcscmp(InitialText, _T("PACKET")) == 0)
			{
				ThreadSafe_Start();
				// Is memory address not NULL?
				if (PacketFormLocation)
				{
					// Remove this parameter, and set
					free(InitialText);
					Parameters->para_client_InitialSend = PacketFormLocation;
					ThreadSafe_End();
				}
				else
				{
					Explode("Cannot use packet as the initial text; packet is empty!");
					ThreadSafe_End();
				}
			}
			else
			{
				Parameters->para_client_InitialSend = InitialText;
			}
			Parameters->para_Port = Port;
			Parameters->para_ProtocolType = Protocol2;
			if (_tcscmp(Protocol, _T("TCP")) == 0)
				Parameters->para_SocketType = SOCK_STREAM; // Default for TCP
			if (_tcscmp(Protocol, _T("UDP")) == 0)
				Parameters->para_SocketType = SOCK_DGRAM;	 // Default for UDP
			if (_tcscmp(Protocol, _T("ICMP")) == 0)
				Parameters->para_SocketType = SOCK_RAW;	 // Default for ICMP
			Report("Client regular thread booting with basic parameters...");
			TermPush(CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ClientThread, Parameters, NULL, NULL));
		}
	}
}
// ID = 3
void Extension::ClientInitialise_Advanced(TCHAR * Hostname, int Port, TCHAR * Protocol, TCHAR * AddressFamily, TCHAR * SocketType, TCHAR * InitialText)
{
	if (Port < 1 || Port > 36535)
		Explode("Port must be greater than 0 and less than 36536.");
	if (_tcscmp(Hostname, _T("")) == 0)
		Explode("Hostname must not be blank.");
	// Check both and continue
	if ( (Port > -1 && Port < 36536) &&
		Hostname[0] != '\0')
	{
		// Move text to a number from a macro
		int Protocol2 = Unreferenced_WorkOutProtocolType(Protocol);
		int AddressFamily2 = Unreferenced_WorkOutAddressFamily(AddressFamily);
		int SocketType2 = Unreferenced_WorkOutSocketType(SocketType);
		// If error'd out report it
		if (Protocol2 == -1)
			Explode("Protocol unrecognised. Use the help file to see the available protocols.");
		if (AddressFamily2 == -1)
			Explode("Address Family unrecognised. Use the help file to see the available families.");
		if (SocketType2 == -1)
			Explode("Socket type unrecognised. Use the help file to see the available socket types.");

		// If no errors, go for it. This is a seperate if, rather than
		// if-else-if, so several errors with the parameters can be reported.
		if (Protocol2 != -1 && AddressFamily2 != -1 && SocketType2 != -1)
		{
			StructPassThru* Parameters = new StructPassThru;
			Parameters->para_AddressFamily = AddressFamily2;
			Parameters->para_Ext = this;
			Parameters->para_client_hostname = Hostname;
			Parameters->para_Port = Port;
			Parameters->para_ProtocolType = Protocol2;
			Parameters->para_SocketType = SocketType2;
			// No need to work around if someone wants to send "PACKET"...
			// They can use the form packet actions with a string :)
			if (_tcsicmp(InitialText, _T("PACKET")) == 0)
			{
				// Is memory address not NULL?
				if (PacketFormLocation)
				{
					// Remove this parameter, and set
					free(InitialText);
					Parameters->para_client_InitialSend = PacketFormLocation;
				}
				else // PacketFormLocation == NULL: Invalid memory address
					Explode("Cannot use packet as the initial text; packet is empty!");
			}
			else
				Parameters->para_client_InitialSend = InitialText;

			// IRDA uses different struct to other protocols
			if (Protocol2 != AF_IRDA)
			{
				Report("Client regular thread booting with advanced parameters...");
				TermPush(CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ClientThread, Parameters, NULL, NULL)); // Add to vector for closing later
			}
			else
			{
				Report("Client IRDA thread booting with advanced parameters...");
				TermPush(CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ClientThreadIRDA, Parameters, NULL, NULL)); // Add to vector for closing later
			}
		}
	}
}
// ID = 4
void Extension::ClientShutdownSocket(int SocketID)
{
	ThreadSafe_Start();
	Senders.push_back(RevCarryMsg(Commands::SHUTDOWNTHREAD, SocketID));
	ThreadSafe_End();
}
// ID = 5
void Extension::ClientSend(int SocketID, TCHAR * Message)
{
	RevCarryMsg r(Commands::SENDMSG, SocketID);
	// Send formed packet
	if (!_tcscmp(Message, _T("PACKET")))
		r.Message = std::string((char *)PacketFormLocation, PacketFormSize);
	// Otherwise send the text
	else if (!_tcsncmp(Message, _T("UTF8"), sizeof("UTF8")-1))
		r.Message = TStringToUTF8(Message);
	else if (!_tcsncmp(Message, _T("UTF8"), sizeof("UTF8") - 1))
	{
		std::wstring msgWide = TStringToWide(Message);
		r.Message.assign((char *)msgWide.c_str(), msgWide.size() * sizeof(wchar_t));
	}
	Indirect_QueueCommandForSocket(r);
}
// ID = 6
void Extension::ClientGoIndependent(int SocketID)
{
	Indirect_Error(SocketID, _T("Going independent was removed from the functionality."));
}
// ID = 7
void Extension::ClientReceiveOnly(int SocketID)
{
	Indirect_QueueCommandForSocket(RevCarryMsg(Commands::RECEIVEONLY, SocketID));
}
// ID = 8
void Extension::ClientLinkFileOutput(int SocketID, TCHAR * File)
{
	RevCarryMsg r(Commands::LINKOUTPUTTOFILE, SocketID);
	r.Message = File;
	Indirect_QueueCommandForSocket(r);
}
// ID = 9
void Extension::ClientUnlinkFileOutput(int SocketID)
{
	Indirect_QueueCommandForSocket(RevCarryMsg(Commands::UNLINKFILEOUTPUT, SocketID));
}
// ID = 10
void Extension::ClientMMF2Report(int SocketID, int OnOrOff)
{
	Indirect_Error(SocketID, _T("Disabling reporting was removed from the extension's functionality."));
}


// ============================================================================
//
/// SERVER
//
// ============================================================================

// ID = 11
void Extension::ServerInitialise_Basic(const TCHAR * Protocol, int Port)
{
	if (Port < 1 || Port > 0xFFFF)
		return Indirect_Error(-1, _T("Port must be greater than 0 and less than 65536, you passed %d."), Port);
	// This is the basic action, so limit it to 3 protocols
	if (!_tcscmp(Protocol, _T("TCP")) && !_tcscmp(Protocol, _T("UDP")) && !_tcscmp(Protocol, _T("ICMP")))
		return Indirect_Error(-1, _T("Protocol unrecognised. Use \"TCP\", \"UDP\", or \"ICMP\"."
				"Or you may want to use the advanced functionality."));
	// Check all and continue
	if ((_tcscmp(Protocol, _T("TCP")) == 0 || _tcscmp(Protocol, _T("UDP")) == 0 || _tcscmp(Protocol, _T("ICMP")) == 0) &&
		(Port > -1 && Port <= 0xFFFF))
	{
		// Move text to a number from a macro
		int Protocol2 = Unreferenced_WorkOutProtocolType(Protocol);
		// If error'd out report it.
		if (Protocol2 == -1)
			return Explode("Protocol unrecognised. Use the help file to see the available protocols.");

		StructPassThru* Parameters = new StructPassThru;
		Parameters->para_AddressFamily = AF_INET;		// INET = INTERNET = IPv4
		Parameters->para_Ext = this;					// Don't use = Extension; !
		Parameters->para_Port = Port;
		Parameters->para_ProtocolType = Protocol2;
		if (_tcscmp(Protocol, _T("TCP")) == 0)
			Parameters->para_SocketType = SOCK_STREAM; // Default for TCP
		if (_tcscmp(Protocol, _T("UDP")) == 0)
			Parameters->para_SocketType = SOCK_DGRAM;	 // Default for UDP
		if (_tcscmp(Protocol, _T("ICMP")) == 0)
			Parameters->para_SocketType = SOCK_RAW;	 // Default for ICMP
		Parameters->para_server_InAddr = INADDR_ANY;
		Report("Server regular thread booting with basic parameters...");
		TermPush(CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ServerThread, Parameters, NULL, NULL));
	}
#undef Exit
}
// ID = 12
void Extension::ServerInitialise_Advanced(TCHAR * Protocol, TCHAR * AddressFamily, TCHAR * SocketType, int Port, TCHAR * InAddr)
{
	if (Port < 1 || Port > 65536)
		Explode("Port must be greater than 0 and less than 36536.");
	// Check both and continue
	if (Port > -1 && Port < 65537)
	{
		// Move text to a number from a macro
		int Protocol2 = Unreferenced_WorkOutProtocolType(Protocol);
		int AddressFamily2 = Unreferenced_WorkOutAddressFamily(AddressFamily);
		int SocketType2 = Unreferenced_WorkOutSocketType(SocketType);
		unsigned long InAddr2 = Unreferenced_WorkOutInAddr(InAddr);
		// If error'd out report it.
		if (Protocol2 == -1)
			Explode("Protocol unrecognised. Use the help file to see the available protocols.");
		if (AddressFamily2 == -1)
			Explode("Address Family unrecognised. Use the help file to see the available families.");
		if (SocketType2 == -1)
			Explode("Socket type unrecognised. Use the help file to see the available socket types."); else
		if (InAddr2 == 12345)
			Explode("InAddr type unrecognised. Use the help file to see the available InAddr types."); else
		if (Protocol2 != -1 &&
			AddressFamily2 != -1 &&
			SocketType2 != -1 &&
			InAddr2 != 12345)
		{
			StructPassThru* Parameters = new StructPassThru;
			Parameters->para_AddressFamily = AddressFamily2;
			Parameters->para_Ext = this;
			Parameters->para_Port = Port;
			Parameters->para_ProtocolType = Protocol2;
			Parameters->para_SocketType = SocketType2;
			Parameters->para_server_InAddr = InAddr2;

			// IRDA uses different struct to other protocols
			if (Protocol2 != AF_IRDA)
			{
				Report("Socket regular thread booting with advanced parameters...");
				TermPush(CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ServerThread, Parameters, NULL, NULL)); // Add to vector for closing later
			}
			else
			{
				Report("Socket IRDA thread booting with advanced parameters...");
				TermPush(CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ServerThreadIRDA, Parameters, NULL, NULL)); // Add to vector for closing later
			}
		}
	}
#undef Exit
}
// ID = 13
void Extension::ServerShutdownSocket(int SocketID)
{
	ThreadSafe_Start();
	Senders.push_back(RevCarryMsg(Commands::SHUTDOWNTHREAD, SocketID));
	ThreadSafe_End();
}
// ID = 14
void Extension::ServerSend(int SocketID, TCHAR * Message)
{
	ThreadSafe_Start();
	RevCarryMsg r(Commands::SENDMSG, SocketID);
	// Send formed packet
	if (_tcscmp(Message, _T("PACKET")) == 0)
	{
		r.Message = PacketFormLocation;
		r.MessageSize = PacketFormSize;
	}
	// Otherwise send the text
	else
	{
		r.Message = _tcsdup(Message);
		r.MessageSize = (_tcslen(Message) + 1) * sizeof(TCHAR);
	}
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 15
void Extension::ServerGoIndependent(int SocketID)
{
	ThreadSafe_Start();
	Senders.push_back(RevCarryMsg(Commands::GOINDEPENDENT, SocketID));
	ThreadSafe_End();
}
// ID = 16
void Extension::ServerAutoAccept(int SocketID, int OnOrOff)
{
	ThreadSafe_Start();
	Senders.push_back(RevCarryMsg(OnOrOff ? Commands::AUTOACCEPTSETON : Commands::AUTOACCEPTSETOFF, SocketID));
	ThreadSafe_End();
}
// ID = 17
void Extension::ServerLinkFileOutput(int SocketID, TCHAR * File)
{
	ThreadSafe_Start();
	Senders.push_back(RevCarryMsg(Commands::LINKOUTPUTTOFILE, SocketID));
	ThreadSafe_End();
}
// ID = 18
void Extension::ServerUnlinkFileOutput(int SocketID)
{
	ThreadSafe_Start();
	Senders.push_back(RevCarryMsg(Commands::UNLINKFILEOUTPUT, SocketID));
	ThreadSafe_End();
}
// ID = 19
void Extension::ServerMMF2Report(int SocketID, int OnOrOff)
{
	ThreadSafe_Start();
	Senders.push_back(RevCarryMsg(OnOrOff ? Commands::MMFREPORTON : Commands::MMFREPORTOFF, SocketID));
	ThreadSafe_End();
}


// ============================================================================
//
/// PACKET FORM
//
// ============================================================================
// Quick function to copy the size to a local variable.
#define DPFS()	ThreadSafe_Start(); \
				size_t PacketFormSize = Extension::PacketFormSize; \
				ThreadSafe_End()
// ID = 20
void Extension::PacketForm_NewPacket(size_t Size)
{
	if (Size == 0)
		Explode("Could not begin new packet; size under 1.");
	else
	{
		ThreadSafe_Start();
		// Free old memory
		if (PacketFormLocation)
		{
			free(PacketFormLocation);
			ThreadSafe_End();
			Report("free'd old PacketFormLocation.");
			ThreadSafe_Start();
		}
		// Allocate new memory
		PacketFormLocation = calloc(Size, 1);
		PacketFormSize = Size;

		// Initialise new memory to \0
		if (PacketFormLocation)
		{
			TCHAR temp [255];
			sprintf_s(temp, 255, "PacketFormLocation is now at %p, size %u.", PacketFormLocation, PacketFormSize);
			ThreadSafe_End();
			Report(temp);
			Report("Zero'd memory.");
		}
		else
		{
			ThreadSafe_End();
			Explode("Memory could not be allocated! Packet is blank.");
		}
	}
}
// ID = 21
void Extension::PacketForm_ResizePacket(size_t Size)
{
	ThreadSafe_Start();
	if (Size == 0)
	{
		if (!PacketFormLocation)
		{
			free(PacketFormLocation);
			PacketFormLocation = NULL;
			PacketFormSize = 0;
		}
		ThreadSafe_End();
		Report("Packet resized to 0.");
	}
	else
	{
		void * newPtr = realloc(PacketFormLocation, Size);
		if (newPtr)
		{
			PacketFormLocation = newPtr;
			PacketFormSize = Size;
			ThreadSafe_End();
		}
		else
		{
			ThreadSafe_End();
			Explode("Memory could not be reallocated! Size unchanged.");
		}
	}
}
// ID = 22
void Extension::PacketForm_SetByte(unsigned char Byte, size_t WhereTo)
{
	DPFS();
	if ((PacketFormSize - WhereTo) < sizeof(char))
		Explode("Add byte failed: memory alloticated is not enough to "
				"set the memory at the position given.");
	else
	{
		ThreadSafe_Start();
		if (!ReadProcessMemory(GetCurrentProcess(), &Byte, (void *)(((size_t)PacketFormLocation)+WhereTo), sizeof(char), NULL))
		{
			ThreadSafe_End();
			Explode("Add byte action failed, reading from the given address failed.");
		}
		else
			ThreadSafe_End();
	}
}
// ID = 23
void Extension::PacketForm_SetShort(unsigned short ShortP, size_t WhereTo, int RunHTON)
{
	unsigned short Short = 0;
	if (RunHTON)
		Short = htons(ShortP);
	else
		Short = ShortP;
	DPFS();
	if ((PacketFormSize - WhereTo) < sizeof(short))
		Explode("Add short failed: memory alloticated is not enough to "
				"set the memory at the position given.");
	else
	{
		ThreadSafe_Start();
		if (!ReadProcessMemory(GetCurrentProcess(), &Short, (void *)(((size_t)PacketFormLocation)+WhereTo), sizeof(short), NULL))
		{
			ThreadSafe_End();
			Explode("Add short failed, reading from the given address failed.");
		}
		else
			ThreadSafe_End();
	}
}
// ID = 24
void Extension::PacketForm_SetInteger(unsigned int Integer, size_t WhereTo)
{
	DPFS();
	if ((PacketFormSize - WhereTo) < sizeof(int))
		Explode("Add short failed: memory alloticated is not enough to "
				"set the memory at the position given.");
	else
	{
		ThreadSafe_Start();
		if (!ReadProcessMemory(GetCurrentProcess(), &Integer, (void *)(((size_t)PacketFormLocation)+WhereTo), sizeof(Integer), NULL))
		{
			ThreadSafe_End();
			Explode("Add integer failed, reading from the given variable failed.");
		}
		else
			ThreadSafe_End();
	}
}
// ID = 25
void Extension::PacketForm_SetLong(unsigned long LongP, size_t WhereTo, int RunHTON)
{
	unsigned long Long = 0;
	if (RunHTON)
		Long = htonl(LongP);
	else
		Long = LongP;
	DPFS();
	if ((PacketFormSize - WhereTo) < sizeof(long))
		Explode("Add long failed: memory alloticated is not enough to "
				"set the memory at the position given.");
	else
	{
		ThreadSafe_Start();
		if (!ReadProcessMemory(GetCurrentProcess(), &Long, (void *)(((size_t)PacketFormLocation)+WhereTo), 8, NULL))
		{
			ThreadSafe_End();
			Explode("Add long failed, reading from the given variable failed.");
		}
		else
			ThreadSafe_End();
	}
}
// ID = 26
void Extension::PacketForm_SetFloat(float Float, size_t WhereTo)
{
	DPFS();
	if ((PacketFormSize - WhereTo) < sizeof(float))
		Explode("Add float failed: memory alloticated is not enough to "
				"set the memory at the position given.");
	else
	{
		ThreadSafe_Start();
		if (!ReadProcessMemory(GetCurrentProcess(), &Float, (void *)(((size_t)PacketFormLocation)+WhereTo), sizeof(float), NULL))
		{
			ThreadSafe_End();
			Explode("Add float failed, reading from the given address failed.");
		}
		else
			ThreadSafe_End();
	}
}
// ID = 27
void Extension::PacketForm_SetDouble(double Double, size_t WhereTo)
{
	DPFS();
	if ((PacketFormSize - WhereTo) < sizeof(double))
		Explode("Set double failed: memory alloticated is not enough to "
				"set the memory at the position given.");
	else
	{
		ThreadSafe_Start();
		if (!ReadProcessMemory(GetCurrentProcess(), &Double, (void *)(((size_t)PacketFormLocation)+WhereTo), sizeof(double), NULL))
		{
			ThreadSafe_End();
			Explode("Add double failed, reading from the given address failed.");
		}
		else
			ThreadSafe_End();
	}

}
// ID = 28
void Extension::PacketForm_SetString(size_t WhereFrom, size_t WhereTo, size_t SizeOfString)
{
	DPFS();
	if (SizeOfString == 0)
		Explode("Set char string failed; size under 1.");
	if ((PacketFormSize - WhereTo) < SizeOfString)
		Explode("Set char string failed: memory alloticated is not enough to "
				"set the memory at the position given.");
	if ((SizeOfString > 0) &&
		((PacketFormSize - WhereTo) >= SizeOfString))
	{
		ThreadSafe_Start();
		if (!ReadProcessMemory(GetCurrentProcess(), (void *) WhereFrom, (void *)(((size_t)PacketFormLocation)+WhereTo), SizeOfString, NULL))
		{
			ThreadSafe_End();
			Explode("Add char string failed, reading from the given address failed.");
		}
		else
			ThreadSafe_End();
	}
}
// ID = 29
void Extension::PacketForm_SetWString(size_t WhereFrom, size_t WhereTo, size_t SizeOfString)
{
	DPFS();
	if (SizeOfString == 0)
		Explode("Set wchar_t string failed; size under 1.");
	if ((PacketFormSize - WhereTo) < SizeOfString*2)
		Explode("Set wchar_t string failed: memory alloticated is not enough to "
				"set the memory at the position given.");
	if ((SizeOfString > 0) &&
		((PacketFormSize - WhereTo) >= SizeOfString*2))
	{
		ThreadSafe_Start();
		if (!ReadProcessMemory(GetCurrentProcess(), (void *) WhereFrom, (void *)(((size_t)PacketFormLocation)+WhereTo), SizeOfString*sizeof(wchar_t), NULL))
		{
			ThreadSafe_End();
			Explode("Add wchar_t string failed, reading from the given address failed.");
		}
		else
			ThreadSafe_End();
	}
}
// ID = 30
void Extension::PacketForm_SetBankFromBank(size_t WhereFrom, size_t WhereTo, size_t SizeOfBank)
{
	DPFS();
	if (SizeOfBank == 0)
		Explode("Set bank from bank failed; size under 1.");
	if ((PacketFormSize - WhereTo) < SizeOfBank)
		Explode("Set bank from bank failed: memory alloticated is not enough to "
				"set the memory at the position given.");
	if ((SizeOfBank > 0) &&
		((PacketFormSize - WhereTo) >= SizeOfBank))
	{
		ThreadSafe_Start();
		if (!ReadProcessMemory(GetCurrentProcess(), (void *) WhereFrom, (void *)(((size_t)PacketFormLocation)+WhereTo), SizeOfBank, NULL))
		{
			ThreadSafe_End();
			Explode("Set bank to bank failed, reading from the given address failed.");
		}
		else
			ThreadSafe_End();
	}
}

#undef DPFS
