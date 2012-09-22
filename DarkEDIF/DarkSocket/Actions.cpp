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
	ThreadSafe_Start();
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
void Extension::ClientInitialise_Basic(tchar * HostnameP, int Port, tchar * ProtocolP, tchar * InitialTextP)
{
	// Copy parameters out
	tchar * Hostname = _tcsdup(HostnameP);
	tchar * Protocol = _tcsdup(ProtocolP);
	tchar * InitialText = _tcsdup(InitialTextP);
#define Exit()	free(Hostname); free(Protocol); free(InitialText)

	if (Port < 1 || Port > 36535)
		Explode("Port must be greater than 0 and less than 36536.");
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
			Exit();
		}
		else 
		{
			StructPassThru* Parameters = new StructPassThru;
			Parameters -> para_AddressFamily = AF_INET;		// INET = INTERNET = IPv4
			Parameters -> para_Ext = this;					// Don't use = Extension; !
			Parameters -> para_client_hostname = Hostname;
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
					Parameters -> para_client_InitialSend = PacketFormLocation;
					ThreadSafe_End();
				}
				else
				{
					Explode("Cannot use packet as the initial text; packet is empty!");
					ThreadSafe_End();
					Exit();
				}
			}
			else
			{
				Parameters -> para_client_InitialSend = InitialText;
			}
			Parameters -> para_Port = Port;
			Parameters -> para_ProtocolType = Protocol2;
			if (_tcscmp(Protocol, _T("TCP")) == 0)
				Parameters -> para_SocketType = SOCK_STREAM; // Default for TCP
			if (_tcscmp(Protocol, _T("UDP")) == 0)
				Parameters -> para_SocketType = SOCK_DGRAM;	 // Default for UDP
			if (_tcscmp(Protocol, _T("ICMP")) == 0)
				Parameters -> para_SocketType = SOCK_RAW;	 // Default for ICMP
			Report("Client regular thread booting with basic parameters...");
			TermPush(CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ClientThread, Parameters, NULL, NULL));
		}
	}
	else 
	{
		// Multiline exit() function, so {} needed.
		Exit();
	}
#undef Exit
}
// ID = 3
void Extension::ClientInitialise_Advanced(tchar * HostnameP, int Port, tchar * ProtocolP, tchar * AddressFamilyP, tchar * SocketTypeP, tchar * InitialTextP)
{
	// Copy parameters out
	tchar * Hostname = _tcsdup(HostnameP);
	tchar * Protocol = _tcsdup(ProtocolP);
	tchar * AddressFamily = _tcsdup(AddressFamilyP);
	tchar * SocketType = _tcsdup(SocketTypeP);
	tchar * InitialText = _tcsdup(InitialTextP);
#define Exit()	free(Hostname); free(Protocol); free(AddressFamily); free(SocketType); free(InitialText)

	if (Port < 1 || Port > 36535)
		Explode("Port must be greater than 0 and less than 36536.");
	if (_tcscmp(Hostname, _T("")) == 0)
		Explode("Hostname must not be blank.");
	// Check both and continue
	if ( (Port > -1 && Port < 36536) &&
		(_tcscmp(Hostname, _T("")) != 0))
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
					Parameters -> para_client_InitialSend = PacketFormLocation;
				}
				else // PacketFormLocation == NULL: Invalid memory address
					Explode("Cannot use packet as the initial text; packet is empty!");
			}
			else
				Parameters -> para_client_InitialSend = InitialText;
			
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
	else 
	{
		// Multiline exit() function, so {} needed.
		Exit();
	}
#undef Exit
}
// ID = 4
void Extension::ClientShutdownSocket(int SocketID)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	r.Cmd = SHUTDOWNTHREAD;
	r.Socket = SocketID;
	r.Message = "";
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 5
void Extension::ClientSend(int SocketID, tchar * MessageP)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	tchar * Message = _tcsdup(MessageP);
	r.Cmd = SENDMSG;
	r.Socket = SocketID;
	// Send formed packet
	if (_tcscmp(Message, _T("PACKET")) == 0)
	{
		r.Message = PacketFormLocation;
		r.MessageSize = PacketFormSize;
		free(Message);
	}
	// Otherwise send the text
	else
	{
		r.Message = Message;
		r.MessageSize = (_tcslen(Message)+1)*ts;
	}
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 6
void Extension::ClientGoIndependent(int SocketID)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	r.Cmd = GOINDEPENDENT;
	r.Socket = SocketID;
	r.Message = "";
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 7
void Extension::ClientReceiveOnly(int SocketID)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	r.Cmd = RECEIVEONLY;
	r.Socket = SocketID;
	r.Message = "";
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 8
void Extension::ClientLinkFileOutput(int SocketID, tchar * File)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	r.Cmd = LINKOUTPUTTOFILE;
	r.Socket = SocketID;
	r.Message = File;
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 9
void Extension::ClientUnlinkFileOutput(int SocketID)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	r.Cmd = UNLINKFILEOUTPUT;
	r.Socket = SocketID;
	r.Message = "";
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 10
void Extension::ClientMMF2Report(int SocketID, int OnOrOff)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	if (OnOrOff)
		r.Cmd = MMFREPORTON;
	else
		r.Cmd = MMFREPORTOFF;
	r.Socket = SocketID;
	r.Message = "";
	Senders.push_back(r);
	ThreadSafe_End();
}


// ============================================================================
//
/// SERVER
//
// ============================================================================

// ID = 11
void Extension::ServerInitialise_Basic(tchar * ProtocolP, int Port)
{
	// Copy parameters out
	tchar * Protocol = _tcsdup(ProtocolP);
#define Exit()	free(Protocol);

	if (Port < 1 || Port > 65536)
		Explode("Port must be greater than 0 and less than 65537.");
	// This is the basic action, so limit it to 3 protocols
	if (_tcscmp(Protocol, _T("TCP")) != 0 && _tcscmp(Protocol, _T("UDP")) != 0 && _tcscmp(Protocol, _T("ICMP")) != 0)
		Explode("Protocol unrecognised. Use \"TCP\", \"UDP\", or \"ICMP\".\
				Or you may want to used the advanced functionality.");
	// Check all and continue
	if ((_tcscmp(Protocol, _T("TCP")) == 0 || _tcscmp(Protocol, _T("UDP")) == 0 || _tcscmp(Protocol, _T("ICMP")) == 0) &&
		(Port > -1 && Port < 65537))
	{
		// Move text to a number from a macro
		int Protocol2 = Unreferenced_WorkOutProtocolType(Protocol);
		// If error'd out report it.
		if (Protocol2 == -1)
		{
			Explode("Protocol unrecognised. Use the help file to see the available protocols.");
			Exit();
		}
		else 
		{
			StructPassThru* Parameters = new StructPassThru;
			Parameters -> para_AddressFamily = AF_INET;		// INET = INTERNET = IPv4
			Parameters -> para_Ext = this;					// Don't use = Extension; !
			Parameters -> para_Port = Port;
			Parameters -> para_ProtocolType = Protocol2;
			if (_tcscmp(Protocol, _T("TCP")) == 0)
				Parameters -> para_SocketType = SOCK_STREAM; // Default for TCP
			if (_tcscmp(Protocol, _T("UDP")) == 0)
				Parameters -> para_SocketType = SOCK_DGRAM;	 // Default for UDP
			if (_tcscmp(Protocol, _T("ICMP")) == 0)
				Parameters -> para_SocketType = SOCK_RAW;	 // Default for ICMP
			Parameters -> para_server_InAddr = INADDR_ANY;
			Report("Server regular thread booting with basic parameters...");
			TermPush(CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ServerThread, Parameters, NULL, NULL));
		}
	}
	else 
	{
		// Multiline exit() function, so {} needed.
		Exit();
	}
#undef Exit
}
// ID = 12
void Extension::ServerInitialise_Advanced(tchar * ProtocolP, tchar * AddressFamilyP, tchar * SocketTypeP, int Port, tchar * InAddrP)
{
	// Copy parameters out
	tchar * Protocol = _tcsdup(ProtocolP);
	tchar * AddressFamily = _tcsdup(AddressFamilyP);
	tchar * SocketType = _tcsdup(SocketTypeP);
	tchar * InAddr = _tcsdup(InAddrP);
#define Exit()	free(Protocol); free(AddressFamily); free(SocketType); free(InAddr)

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
	else 
	{
		// Multiline exit() function, so {} needed.
		Exit();
	}
#undef Exit
}
// ID = 13
void Extension::ServerShutdownSocket(int SocketID)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	r.Cmd = SHUTDOWNTHREAD;
	r.Socket = SocketID;
	r.Message = "";
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 14
void Extension::ServerSend(int SocketID, tchar * MessageP)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	tchar * Message = _tcsdup(MessageP);
	r.Cmd = SENDMSG;
	r.Socket = SocketID;
	// Send formed packet
	if (_tcscmp(Message, _T("PACKET")) == 0)
	{
		r.Message = PacketFormLocation;
		r.MessageSize = PacketFormSize;
		free(Message);
	}
	// Otherwise send the text
	else
	{
		r.Message = Message;
		r.MessageSize = (_tcslen(Message)+1)*ts;
	}
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 15
void Extension::ServerGoIndependent(int SocketID)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	r.Cmd = GOINDEPENDENT;
	r.Socket = SocketID;
	r.Message = "";
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 16
void Extension::ServerAutoAccept(int SocketID, int OnOrOff)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	if (OnOrOff)
		r.Cmd = AUTOACCEPTSETON;
	else
		r.Cmd = AUTOACCEPTSETOFF;
	r.Socket = SocketID;
	r.Message = "";
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 17
void Extension::ServerLinkFileOutput(int SocketID, tchar * File)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	r.Cmd = LINKOUTPUTTOFILE;
	r.Socket = SocketID;
	r.Message = "";
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 18
void Extension::ServerUnlinkFileOutput(int SocketID)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	r.Cmd = UNLINKFILEOUTPUT;
	r.Socket = SocketID;
	r.Message = "";
	Senders.push_back(r);
	ThreadSafe_End();
}
// ID = 19
void Extension::ServerMMF2Report(int SocketID, int OnOrOff)
{
	ThreadSafe_Start();
	RevCarryMsg r;
	if (OnOrOff)
		r.Cmd = MMFREPORTON;
	else
		r.Cmd = MMFREPORTOFF;
	r.Socket = SocketID;
	r.Message = "";	
	Senders.push_back(r);
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
		// Alloticate new memory
		PacketFormLocation = malloc(Size);
		PacketFormSize = Size;
		
		// Initialise new memory to \0
		if (PacketFormLocation)	
		{
			tchar temp [255];
			sprintf_s(temp, 255, "PacketFormLocation is now at %p, size %u.", PacketFormLocation, PacketFormSize);
			ThreadSafe_End();
			Report(temp);
			ThreadSafe_Start();
			ZeroMemory(PacketFormLocation, Size);
			ThreadSafe_End();
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
		void * v = realloc(PacketFormLocation, Size);
		if (PacketFormLocation)
		{
			PacketFormLocation = v;
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