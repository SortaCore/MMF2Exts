#define NoExt // Extension is not native, so declare this so "common.h" knows
#include "Common.h"
#undef NoExt
// ============================================================================
//
// THREADS
// 
// ============================================================================

DWORD WINAPI ClientThread(StructPassThru *Parameters)
{
	// Debugging
	char temp[1024]; //For chucking any variable into and using in Report()/Explode()
	
	// Open struct and set variables
    Extension * Extension = Parameters -> para_Ext;					// Access for Extension::
	tchar * Hostname = (tchar *)malloc(255*ts);						// Hostname
	unsigned short Port = htons(Parameters -> para_Port);			// Port to connect to (htons() rearranges bits)
	int ProtocolType = Parameters -> para_ProtocolType;				// ProtocolType eg IPPROTO_TCP
	int SocketType = Parameters -> para_SocketType;					// SocketType eg SOCK_STREAM
	int AddressFamily = Parameters -> para_AddressFamily;			// AddressFamily eg AF_INTERNET
	
	// Get the current socket ID
	ThreadSafe_Start();
	int SocketID = Extension -> NewSocketID;	// Get the current available Socket ID
	Extension -> NewSocketID++;					// Increment for further threads
	ThreadSafe_End();
	
	if (!Hostname)
	{
		Explode("Could not reserve space for hostname!");
		NullStoredHandle(SocketID);
		ThreadSafe_Start();
		if (Parameters -> para_client_InitialSend != Extension->PacketFormLocation)
		{
			ThreadSafe_End();
			free(Parameters -> para_client_InitialSend);
		}
		else
			ThreadSafe_End();
		delete Parameters;
		return 1;
	}
	else
	{
		if (_tcscpy_s(Hostname, 255, Parameters->para_client_hostname))
		{
			Explode("Could not copy hostname data! Thread exiting.");
			NullStoredHandle(SocketID);
			ThreadSafe_Start();
			if (Parameters -> para_client_InitialSend != Extension->PacketFormLocation)
			{
				ThreadSafe_End();
				free(Parameters -> para_client_InitialSend);
			}
			else
				ThreadSafe_End();
			free(Hostname);
			delete Parameters;
			return 1;
		}
		else // We're good: remove old string
			free(Parameters -> para_client_hostname);
	}
	
	// Get the text to send on initial connection
	void * InitialSend = NULL;
	size_t InitialSendSize = 0;
	
	Report("* Initial send copying BEGIN *");
	ThreadSafe_Start();
	// If FormPacket
	if (Parameters -> para_client_InitialSend == Extension->PacketFormLocation)
	{
		InitialSendSize = Extension->PacketFormSize;
		InitialSend = malloc(InitialSendSize);
		ZeroMemory(InitialSend, InitialSendSize);
		ThreadSafe_End();
		if (memcpy_s(InitialSend, InitialSendSize, Parameters ->para_client_InitialSend, InitialSendSize))
		{
			Explode("Error occured with copying the Form Packet to the thread's local buffer. Thread exiting.");
			NullStoredHandle(SocketID);
			delete Parameters;
			return 1;
		}

		// We don't want to run free() on the FormPacket, further references
		// from the user may be made to it, eg two usages.
	}
	else
	{
		InitialSendSize = (_tcslen((tchar *)Parameters -> para_client_InitialSend)+1)*ts;
		InitialSend = malloc(InitialSendSize);
		ThreadSafe_End();
		if (memcpy_s(InitialSend, InitialSendSize, Parameters -> para_client_InitialSend, InitialSendSize))
		{
			Explode("Error occured with copying the initial send text to the thread's local buffer!");
			free(Parameters -> para_client_InitialSend);
			NullStoredHandle(SocketID);
			delete Parameters;
			return 1;
		}
		else
			free(Parameters -> para_client_InitialSend);
	}
	Report("* Initial send copying END *");

	delete Parameters;	// Scat!
	Report("* Parameters retrieval END *");
	Report("* Struct creation BEGIN *");
	
	Report("Now declaring addrinfo and sockaddr.");
#ifndef UNICODE
	struct addrinfo *result = NULL, *ptr = NULL, hints;
#else
	struct addrinfoW *result = NULL, *ptr = NULL, hints;
#endif
	struct sockaddr_storage SockAddr;
	ZeroMemory(&SockAddr, sizeof(SockAddr));

	// Copy data into sockaddr_storage and retrieve address
	if (AddressFamily != AF_INET6)
	{
		DWORD d = 255, e = d, f = 255;

		sockaddr_in fun;
		fun.sin_family = AddressFamily;
		fun.sin_port = Port;

		if (WSAAddressToString((sockaddr *)&fun, sizeof(fun), NULL, Hostname, &d))
		{
			sprintf_s(temp, sizeof(temp), "Error with WSAAddressToString(), number %i.", WSAGetLastError());
			Explode(temp);
		}
		else
		{
			sprintf_s(temp, 1024, "WSAAdressToString() returned [%.14s].", (char *)((&fun)+2));
			Report(temp);
		}

		if (memcpy_s((void *)(&SockAddr), sizeof(SockAddr), (void *)(&fun), sizeof(fun)))
		{
			Explode("Struct memory copying v4 failed. Thread exiting.");
			free(InitialSend);
			NullStoredHandle(SocketID);
			return 1;
		}
	}
	else // AddressFamily == AF_INET6
	{
		DWORD d = 255;

		sockaddr_in6 fun;
		fun.sin6_family = AddressFamily;
		fun.sin6_port = Port;
		
		if (WSAAddressToString((sockaddr *)&fun, sizeof(fun), NULL, Hostname, &d))
		{
			sprintf_s(temp, sizeof(temp), "Error with WSAAddressToString(), number %i.", WSAGetLastError());
			Explode(temp);
		}

		if (memcpy_s((void *)(&SockAddr), sizeof(SockAddr), (void *)(&fun), sizeof(fun)))
		{
			Explode("Struct memory copying v6 failed. Thread exiting.");
			free(InitialSend);
			NullStoredHandle(SocketID);
			return 1;
		}
		
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AddressFamily;
	hints.ai_socktype = SocketType;
	hints.ai_protocol = ProtocolType;
	hints.ai_flags = AI_PASSIVE;

	Report("Now running GetAddrInfo");
	// Cast port to tchar *, defaults to 80
	tchar StrPort[6] = _T("80\0\0\0");
	_itot_s(Port, StrPort, 6, 10); // 6 is the size of StrPort; 10 here signifies decimal - base 10 numbers
	
	// Resolve the server address and port
	int error = GetAddrInfo(Hostname, StrPort, &hints, &result);
	
	if (error)
	{
		sprintf_s(temp, 1024, "GetAddrInfo() failed with error %i! Parameters %s, %s, N/A*2. Thread exiting.", WSAGetLastError(), Hostname, StrPort);
		Explode(temp);
		if (result) // result exists
		{
			if (!result->ai_addr) // Does not exist
				Explode("result->ai_addr == NULL");
			else // result->ai_addr exists
			{
				sprintf_s(temp, 1024, "ai_addr = %p.", result->ai_addr);
				Explode(temp);
			}
		}
		free(InitialSend);
		NullStoredHandle(SocketID);
		return 1;
	}
	
	SOCKET ConnectSocket = INVALID_SOCKET;
	
	Report("Past GetAddrInfo(), next using socket() function...");
	// Attempt to connect to the first address returned by the call to getaddrinfo
	ptr = result;
	
	Report("* Main connect BEGIN *");
	// Create a SOCKET for connecting to server
	ConnectSocket = socket(AddressFamily, SocketType, ProtocolType);
	if (ConnectSocket == INVALID_SOCKET)
	{
		sprintf_s(temp, sizeof(temp), "Error with socket(): %i. Thread exiting.", WSAGetLastError());
		Explode(temp);
		NullStoredHandle(SocketID);
		FreeAddrInfo(result);
		free(Hostname);
		return 1;
	}
	Report("ConnectSocket not invalid. Now moving on to bind()");

	Report("* bind() attempt BEGIN *");
	// Loop until successful... or not
    while (true)
	{
        if (bind(ConnectSocket, (sockaddr*)(&SockAddr), sizeof(SockAddr)) == 0)
		{
			Report("Successful bind!");
            break;
        }
		else 
		{
			if (WSAGetLastError() == WSAENOTSOCK)
			{
				Report("\"Not Socket\" error. bind() operation skipped.");
				break;
			}
			else
			{
				sprintf_s(temp, 1024, "Error with bind(): %i. Thread continues.", WSAGetLastError());
				Report(temp);
				//Report("bind address, address already in use. Moving on...");
			}
		}
        Port--;
		if (Port == IPPORT_RESERVED/2 )
		{
			Explode("Failed to bind address, all addresses already in use. Thread exiting.");
			free(InitialSend);
			NullStoredHandle(SocketID);
			FreeAddrInfo(result);
			return 1;
            /* fail--all unassigned reserved ports are */
            /* in use. */
        }
    }
	Report("* bind() attempt END *");
	
	Report("Moving on to connect()");
	// Connect to server.
	error = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (error == SOCKET_ERROR)
	{
		sprintf_s(temp, sizeof(temp), "Error with connect(): %i. Thread exiting.", WSAGetLastError());
		Explode(temp);
		closesocket(ConnectSocket);
		free(InitialSend);
		// Should really try the next address returned by getaddrinfo
		// if the connect call failed
		// But for this simple example we just free the resources
		// returned by getaddrinfo and print an error message
		FreeAddrInfo(result);
		NullStoredHandle(SocketID);
		return 1;
	}
	FreeAddrInfo(result);
	
	if (ConnectSocket == INVALID_SOCKET)
	{
		Explode("Unable to connect to server!");
		closesocket(ConnectSocket);
		free(InitialSend);
		NullStoredHandle(SocketID);
		return 1;
	}
	
	Report("* Main connect END *");

	Report("* Send initial buffer BEGIN *");
	Report("Next using send() function.");
	// Send an initial buffer
	int iResult = send(ConnectSocket, (char *)InitialSend, InitialSendSize, 0);
	if (iResult == SOCKET_ERROR)
	{
		Explode("Initial send() function failed.");
		free(InitialSend);
		NullStoredHandle(SocketID);
		closesocket(ConnectSocket);
		CallEvent(MF2C_CLIENT_ON_DISCONNECT);
		return 1;
	}
	sprintf_s(temp, sizeof(temp), "Bytes Sent: %i", iResult);
	Report(temp);		// Report send
	free(InitialSend);	// Don't need this anymore!
	CallEvent(MF2C_CLIENT_ON_CONNECT);
	Report("* Send initial buffer END *");

	Report("* Blocking -> non-blocking change BEGIN *");
	unsigned long UL = 1;
	// Set to non-blocking
	error = ioctlsocket(ConnectSocket, FIONBIO, &UL);
	if (error != 0)
		Explode("Warning: non-blocking mode could not be set.");
	Report("* Blocking -> non-blocking change END *");

	Report("The socket has initialised completely.");
	Report("The socket will now spend its time looking for commands from MMF2 and receiving messages.");
	
	Report("* Main loop BEGIN *");
	// For going independent from MMF2
	bool Independent = false, MMF2Report = true, run = true, loop = false;
	// Default stuff
	char recvbuf[512];
	int recvbuflen = sizeof(recvbuf), num_bytes_present = 0;
	string OutputTo = "", totalrec = "";
	FILE * OutputFile = NULL;
	
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MAIN PART
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	while (run)
	{
		if (!Independent)
		{
			ThreadSafe_Start();
			// Check if there's any messages from MMF2 to handle
			if (!Extension->Senders.empty())
			{
				// If so, loop through them
				for (unsigned int i = 0; i < Extension->Senders.size(); i++)
				{
					// Message is for us
					if (Extension->Senders[i].Socket == SocketID)
					{
						// Copy outside of threadsafe so Report() functions don't lock up
						char c = Extension->Senders[i].Cmd;
						ThreadSafe_End();
						// What does MMF2 want us to do?
						switch(c)
						{
							// Command to close the thread
							case SHUTDOWNTHREAD:
								Report("Thread terminated by SHUTDOWNTHREAD");
								run = false;
								break;
							
							// Command to send a message.
							case SENDMSG:
								Report("Now using send()...");
								iResult = send(ConnectSocket, (char *)Extension->Senders[i].Message, Extension->Senders[i].MessageSize, 0);
								if (iResult == SOCKET_ERROR)
									Explode("send() failed!");
								else
									Report("Send operation completed successfully.");
								ThreadSafe_Start();
								// We don't want to run free() on the FormPacket
								if (Extension->Senders[i].Message != Extension->PacketFormLocation)
									free(Extension->Senders[i].Message);
								ThreadSafe_End();
								break;
							
							// Command to switch to receiving only.
							case RECEIVEONLY:
								Report("Now using shutdown() so we don't send data. But we can receive.");
								iResult = shutdown(ConnectSocket, SD_SEND);
								if (iResult == SOCKET_ERROR)
								{
									Explode("shutdown() failed!"); // %d\n", WSAGetLastError());
									run = false;
								}
								Report("Shutdown operation completed.");
								break;
							
							// Ignore any further commands from MMF2. This will speed up the thread but make the
							// thread as unreachable as that aunt you heard of but don't know their phone number.
							case GOINDEPENDENT:
								Report("Socket is now independent and cannot be contacted.");
								Independent = true;
								break;

							// Command to copy the received data to a file
							case LINKOUTPUTTOFILE:
								Report("Linking received messages to file.");
								ThreadSafe_Start();
								OutputTo = (tchar *)Extension->Senders[i].Message;
								ThreadSafe_End();
								if (OutputTo != "")
									fclose(OutputFile);
								OutputFile = fopen(OutputTo.c_str(), "ab");
								if (!OutputFile)

								break;
							
							// Command to stop copying the received data to a file
							case UNLINKFILEOUTPUT:
								Report("Received messages unlinked from file.");
								OutputTo = "";
								fclose(OutputFile);
								break;
							
							// Command to stop reporting messages to MMF2 (disconnection will still be reported)
							case MMFREPORTOFF:
								if (OutputTo != "")
									Report("No MMF2 report enabled, outputting to file only.");
								else
									Report("No MMF2 report enabled, no file output either!"
										   "You will not receive any received messages from this socket.");
								MMF2Report = false;
								break;
							
							// Command to re-start the reporting of messages to MMF2
							case MMFREPORTON:
								MMF2Report = true;
								Report("MMF2 report re-enabled.");
								break;
							
							// Unrecognised command.
							default:
								Explode("Unrecognised command!");
						}
					ThreadSafe_Start();
					// We handled this one, so remove it.
					Extension->Senders.erase(Extension->Senders.begin() + i);
					i--;
					// Only handle one of the queue per while loop.
					break;
					}
				}
			}
			ThreadSafe_End();
		}
		
		// Receive data until the server closes the connection
		do
		{
			loop = false;
			// The call is nonblocking.
			// If nothing to receive, it would go to the WSAEWOULDBLOCK error code.
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0)
			{
				// Left-over characters may be appended: use iResult to 
				// get the correct number of bytes from the buffer
				totalrec.append(recvbuf, iResult);
			}
			else // (iResult <= 0)
			{
				// On disconnect
				if (iResult == 0)
				{
					// If not already sent
					if (totalrec != "")
					{
						// Send packet to MMF2
						if (MMF2Report)
							ReturnToMMF(CLIENT_RETURN, SocketID, (void *)totalrec.c_str(), totalrec.size()); //remove dodgy ending
						
						// Send packet to the file output
						if (OutputTo != "")
							fputs(totalrec.c_str(), OutputFile);
						totalrec = "";
					}
					ThreadSafe_Start();
					Extension -> LastReturnType = CLIENT_RETURN;
					Extension -> LastReturnSocketID = SocketID;
					ThreadSafe_End();
					CallEvent(MF2C_CLIENT_ON_DISCONNECT);
				}
				// On error (or other stuff)
				
				else // (iResult < 0)
				{
					// If a message is pending
					if (totalrec != "") 
					{
						// Send packet to MMF2
						if (MMF2Report)
							ReturnToMMF(CLIENT_RETURN, SocketID, (void *)totalrec.c_str(), totalrec.size()); //remove dodgy ending
						
						// Send packet to the file output
						if (OutputTo != "")
							fputs(totalrec.c_str(), OutputFile);

						totalrec = "";
					}
					if (iResult == SOCKET_ERROR)
					{
						iResult = WSAGetLastError();
						switch(iResult)
						{
							case ERROR_SUCCESS:
							case WSAEWOULDBLOCK:
								loop = true;
								break;
							case WSAENOTCONN:
							case WSAESHUTDOWN:
							case WSAECONNABORTED:
							case WSAECONNRESET:
							case WSAENETRESET:
								Report("Client connection was closed. Thread exiting.");
								run = false;
								break;
							case WSANOTINITIALISED:
								Explode("Very unexpected error: WSA not initialised (WSANOTINITIALISED). "
										"This is odd because WSAStartup() was error-checked earlier. "
										"For developers: WSAStartup() was run in CreateRunObject(). Thread exiting.");
								run = false;
								break;
							case WSAENETDOWN:
							case WSAETIMEDOUT:
								Explode("Network error and/or timeout. (WSAENETDOWN or WSAETIMEDOUT)\n"
										"According to MSDN:\n"
										"\"The connection has been dropped because of a network failure or because the peer system failed to respond.\"\n"
										"So uh, have fun debugging. Thread exiting.");
								run = false;
								break;
							case WSAEFAULT:
								Explode("Socket variables invalid! Check you supplied the correct variables to the initialise action. Thread exiting.");
								run = false;
								break;
							case WSAEMSGSIZE:
								Explode("Buffer too small for datagram. "
										"This is not your fault! "
										"Please inform SortaCore of this error. Thread remains open.");
								break;
							default:
								// Error that is rare and unhandled.
								sprintf_s(temp, sizeof(temp), "recv() function failed, with unusual error %i. Thread remains open.", WSAGetLastError());
								Explode(temp);
								break;
						}
						
						iResult = SOCKET_ERROR;
					}
					else // (iResult < -1)
						 // This shouldn't occur: iResult should be -1 for normal errors,
						 // or 0 for disconnection, or >0 for number of bytes received.
						Explode("Unexpected error (tell SortaCore): iResult < -1.");
				}
			}
		}
		// Loop this part if there was a message to receive, in case there's more to the message.
		while (iResult > 0); // || loop

		// Nothing to do: give the CPU a rest before the next MMF2 message check & recv() check.
		Sleep(10);
	}
	Report("* Main loop END *");
	ThreadSafe_Start();
	Extension -> LastReturnType = CLIENT_RETURN;
	Extension -> LastReturnSocketID = SocketID;
	ThreadSafe_End();
	CallEvent(MF2C_CLIENT_ON_DISCONNECT);

	Report("* Shutdown BEGIN *");
	// Cleanup - the reason TerminateThread() isn't recommended.
	if (OutputTo != "")
		fclose(OutputFile);
	free(Hostname);
	NullStoredHandle(SocketID);
	closesocket(ConnectSocket);
	Report("* Shutdown END *");

	Report("Client thread exit.");
	return 0;

}

DWORD WINAPI ClientThreadIRDA(StructPassThru *Parameters)
{
	MsgBox("IRDA has not yet been programmed due to a storage corruption.");
#if 0
#error You need to edit Common.h to include the header.
#endif
	// Can't report, no Extension-> declared.
	//Report("Client thread exit.");

	return 0;
}

DWORD WINAPI ServerThread(StructPassThru *Parameters)
{
	//Open struct and set variables
    if (!Parameters)
	{
		MsgBox("Error reading Parameters.");
		return 1;
	}
	if (!Parameters->para_Ext)
	{
		MsgBox("Error reading Parameters->para_Ext.");
		return 2;
	}


	Extension * Extension = Parameters -> para_Ext;					// Access for Extension::
	
	unsigned short Port = htons(Parameters -> para_Port);			// Port to connect to (htons() rearranges bits)
	int ProtocolType = Parameters -> para_ProtocolType;				// ProtocolType eg IPPROTO_TCP
	int SocketType = Parameters -> para_SocketType;					// SocketType eg SOCK_STREAM
	int AddressFamily = Parameters -> para_AddressFamily;			// AddressFamily eg AF_INTERNET
	int WhatEver = Parameters -> para_server_InAddr;							// Needed for something
	
	ThreadSafe_Start();
	int SocketID = Extension -> NewSocketID;	// Get the current available Socket ID
	Extension -> NewSocketID++;					// Increment for further threads
	ThreadSafe_End();
	
	Report("Socket ID acquired");
	delete Parameters;	// Scat!
	Report("After delete");


	//Create own variables
	char temp[1024]; //For chucking any variable into and using in Report()/Explode()

	Report("Now declaring addrinfo and sockaddr.");
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	//void * SockADDR;
	//if (AddressFamily == AFstruct sockaddr_in;
	struct sockaddr_storage SockAddr;
	ZeroMemory(&SockAddr, sizeof(SockAddr));

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AddressFamily;
	hints.ai_socktype = SocketType;
	hints.ai_protocol = ProtocolType;

	Report("Now running getaddrinfo");
	SOCKET MainSocket = INVALID_SOCKET;
	Report("Socket declared, next using bind() function.");
	// Attempt to connect to the first address returned by the call to getaddrinfo
	ptr = result;
	
	SockAddr.ss_family = AddressFamily;
	
	// Loop until successful... or not
    while (true)
	{
        if (bind(MainSocket, (struct sockaddr*)&SockAddr, sizeof(SockAddr)) == 0)
		{
			Report("Successful bind!");
            break;
        }
		else 
		{
			if (WSAGetLastError() == WSAENOTSOCK)
			{
				Report("\"Not Socket\" error. bind() operation skipped.");
				break;
			}
			else
			{
				sprintf_s(temp, 1024, "Error with bind(): %i.", WSAGetLastError());
				Report(temp);
				//Report("bind address, address already in use. Moving on...");
			}
		}
        Port--;
		if (Port == IPPORT_RESERVED/2 )
		{
			Explode("Failed to bind address, all addresses already in use. Thread exiting.");
			NullStoredHandle(SocketID);
			freeaddrinfo(result);
			return 1;
            /* fail--all unassigned reserved ports are */
            /* in use. */
        }
    }

	// Create a SOCKET for connecting to server
	MainSocket = socket(ptr -> ai_family, ptr -> ai_socktype, ptr -> ai_protocol);
	if (MainSocket == INVALID_SOCKET)
	{
		sprintf_s(temp, sizeof(temp), "Error with socket(): %i. Thread exiting.", WSAGetLastError());
		Explode(temp); //Error 10043
		NullStoredHandle(SocketID);
		freeaddrinfo(result);
		return 1;
	}

	Report("MainSocket not invalid. Now moving on to main loop()");
	
	unsigned long UL = 1;
	// Set to non-blocking
	int error = ioctlsocket(MainSocket, FIONBIO, &UL);
	if (error != 0)
		Report("Warning: non-blocking mode could not be set.");

	// For going independent from MMF2
	bool Independent = false, MMF2Report = true;
	// Default stuff
	char recvbuf[512];
	int recvbuflen = sizeof(recvbuf), iResult = 0;
	int num_bytes_present = 0;
	string OutputTo = "", totalrec = "";
	FILE * OutputFile = NULL;
	bool run = true, loop = false;
	vector<ClientAccessNode> ClientSockets;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MAIN PART
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	while (run)
	{
		if (!Independent)
		{
			ThreadSafe_Start();
			// Check if there's any messages from MMF2 to handle
			if (!Extension->Senders.empty())
			{
				// If so, loop through them
				for (unsigned int i = 0; i < Extension->Senders.size(); i++)
				{
					// Message is for us
					if (Extension->Senders[i].Socket == SocketID)
					{
						// Copy outside of threadsafe so Report() functions don't lock up
						char c = Extension->Senders[i].Cmd;
						ThreadSafe_End();
						// What does MMF2 want us to do?
						switch(c)
						{
							// Command to close the thread
							case SHUTDOWNTHREAD:
								Report("Thread terminated by SHUTDOWNTHREAD");
								run = false;
								break;
							
							// Command to send a message.
							case SENDMSG:
								Report("Locating client...");
								for (unsigned int j = 0; j < ClientSockets.size(); j++)
								{
									if (Extension->Senders[i].Client == _T("") || ClientSockets[j].FriendlyName == Extension->Senders[i].Client.c_str())
									{
										iResult = send(ClientSockets[j].socket, (char *)Extension->Senders[i].Message, Extension->Senders[i].MessageSize, 0);
										if (iResult == SOCKET_ERROR)
											Explode("Selected the socket, but the send() operation failed!");
										else
											Report("Send operation completed successfully.");
									}
								}
								Explode("Send operation couldn't locate the applicable client(s)!");
							break;
							
							// Command to switch to receiving only.
							case RECEIVEONLY:
								Report("Now using shutdown() so we don't send data. But we can receive.");
								iResult = shutdown(MainSocket, SD_SEND);
								if (iResult == SOCKET_ERROR)
								{
									Explode("shutdown() failed!"); // %d\n", WSAGetLastError());
									run = false;
								}
								Report("Shutdown operation completed.");
								break;
							
							// Ignore any further commands from MMF2. This will speed up the thread but make the
							// thread as unreachable as that aunt you heard of but don't know their phone number.
							case GOINDEPENDENT:
								Report("Socket is now independent and cannot be contacted.");
								Independent = true;
								break;

							// Command to copy the received data to a file
							case LINKOUTPUTTOFILE:
								Report("Linking output to file.");
								ThreadSafe_Start();
								OutputTo = (tchar *)Extension->Senders[i].Message;
								ThreadSafe_End();
								if (OutputTo != "")
									fclose(OutputFile);
								OutputFile = fopen(OutputTo.c_str(), "ab");
								break;
							
							// Command to stop copying the received data to a file
							case UNLINKFILEOUTPUT:
								Report("Output unlinked from file.");
								OutputTo = "";
								fclose(OutputFile);
								break;
							
							// Command to stop reporting messages to MMF2
							case MMFREPORTOFF:
								if (OutputTo != "")
									Report("No MMF2 report enabled, outputting to file only.");
								else
									Report("No MMF2 report enabled, no file output either!"
											"You will not receive anything from this socket.");
								MMF2Report = false;
								break;
							
							// Command to re-start the reporting of messages to MMF2
							case MMFREPORTON:
								Report("MMF2 report re-enabled.");
								MMF2Report = true;
								break;
							
							// Unrecognised command.
							default:
								Explode("Unrecognised command!");
						}
					ThreadSafe_Start();
					// We handled this one, so remove it.
					Extension->Senders.erase(Extension->Senders.begin() + i);
					i--;
					// Only handle one of the queue per while loop.
					break;
					}
				}
			}
			ThreadSafe_End();
		}
		
		// Loop the main part
		do
		{
			// Always revert to false, so MMF2 messages are checked for
			loop = false;

			// Does listen() find a socket
			if (((iResult = listen(MainSocket, SOMAXCONN)) == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK))
			{
				sprintf_s(temp, sizeof(temp), "Error with listen(): %i. Thread exiting.", WSAGetLastError());
				Explode(temp);
				run = false;
			}
			else
			{
				SOCKADDR_STORAGE AcceptStruct;
				SOCKET Temp = accept(MainSocket, (struct sockaddr *)&AcceptStruct, (int *) sizeof(AcceptStruct));
				if (Temp != SOCKET_ERROR)
				{
					ClientAccessNode c;
					c.sockaddr = AcceptStruct;
					c.socket = Temp;
					c.FriendlyName = _T("Not set.");
					ClientSockets.push_back(c);
					CallEvent(MF2C_SERVER_CLIENT_CONNECTED);
				}
				else
				{
					sprintf_s(temp, sizeof(temp), "Error with accept(): %i. Server continues to run.", WSAGetLastError());
					Explode(temp);
				}
			}
			// The call is nonblocking.
			// If nothing to receive, it would go to the WSAEWOULDBLOCK error code.
			
			if ((iResult = recv(MainSocket, recvbuf, recvbuflen, 0)) > 0)
			{
				// For some odd reason random characters are appended. So cast to std::string and use its' substr command.			
				totalrec.append(recvbuf, iResult);
				//RtlZeroMemory(&recvbuf, recvbuflen);
				//memcp
			}
			else
			{
				// On disconnect
				if (iResult == 0)
				{
					// If some message remains to be sent
					if (totalrec != "")
					{
						if (MMF2Report)
							ReturnToMMF(CLIENT_RETURN, SocketID, (void *)totalrec.c_str(), totalrec.size()); //remove dodgy ending
						if (OutputTo != "")
							fputs(totalrec.c_str(), OutputFile);
						totalrec = "";
					}
					// Tell MMF2 the socket is idle
					ThreadSafe_Start();
					Extension -> LastReturnSocketID = SocketID;
					ThreadSafe_End();
					CallEvent(MF2C_SERVER_SOCKET_DONE);
				}
				// On error (or other stuff)
				else
				{
					if (totalrec != "")
					{
						if (MMF2Report)
							ReturnToMMF(CLIENT_RETURN, SocketID, (void *)totalrec.c_str(), totalrec.size()); //remove dodgy ending
						if (OutputTo != "")
							fputs(totalrec.c_str(), OutputFile);
						totalrec = "";
					}
					if (iResult == SOCKET_ERROR)
					{
						iResult = WSAGetLastError();
						switch(iResult)
						{
							case ERROR_SUCCESS:
							case WSAEWOULDBLOCK:
								loop = true;
								break;
							case WSAENOTCONN:
							case WSAESHUTDOWN:
							case WSAECONNABORTED:
							case WSAECONNRESET:
							case WSAENETRESET:
								Report("Server socket closed.");
								ThreadSafe_Start();
								Extension -> LastReturnSocketID = SocketID;
								ThreadSafe_End();
								CallEvent(MF2C_SERVER_SOCKET_DONE);
								break;
							case WSANOTINITIALISED:
								Explode("Very unexpected error: WSA not initialised (WSANOTINITIALISED).\n\
										This is odd because WSAStartup() was error-checked earlier.\n\
										FYI, WSAStartup() was run in CreateRunObject.");
								break;
							case WSAENETDOWN:
							case WSAETIMEDOUT:
								Explode("Network error and/or timeout. (WSAENETDOWN or WSAETIMEDOUT)\n\
										According to MSDN:\n\
										\"The connection has been dropped because of a network failure or because the peer system failed to respond.\"\n\
										So uh, have fun debugging.");
								break;
							case WSAEFAULT:
								Explode("Socket variables invalid!");
								break;
							case WSAEMSGSIZE:
								Explode("Buffer too small for datagram. Message not received.");
								break;
							default:
								// Error.... ?
								sprintf_s(temp, sizeof(temp), "recv() function failed, with unusual error %i.", WSAGetLastError());
								Explode(temp);
								break;
						}
						// If not just "No message" then
						if (iResult != WSAEWOULDBLOCK && iResult != ERROR_SUCCESS)
							// Shutdown thread
							run = false;
						iResult = SOCKET_ERROR;
					}
					else
						Explode("Line 308 occured.");
				}
			}
		}
		// Loop this part if the data isn't empty, else stop
		while (iResult > 0);
		Sleep(100); // Stops CPU killing (hopefully)
	}
	// We're done - Cleanup
	if (OutputTo != "")
		fclose(OutputFile);
	NullStoredHandle(SocketID);
	
	closesocket(MainSocket);
	Report("Server thread exit.");
	
	return 0;
}

DWORD WINAPI ServerThreadIRDA(StructPassThru *Paramters)
{
	MsgBox("IRDA has not yet been programmed due to a storage corruption.");
#if 0
#error You need to edit Common.h to include the header.
#endif
	// Can't report, no Extension-> declared.
	//Report("Client thread exit.");

	return 0;
}

/*
	SOCKADDR_IN ServerAddrIn;
	sockaddr_storage sinServer;
    ZeroMemory(&sinServer, sizeof(sinServer));

    ServerAddrIn.sin_family = AddressFamily;
	ServerAddrIn.sin_addr.S_un.S_addr = WhatEver; // Where to start server?
	ServerAddrIn.sin_port = Port; // Port

#define INADDR_ANY              (ULONG)0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        (ULONG)0xffffffff
#define INADDR_NONE             0xffffffff
*/