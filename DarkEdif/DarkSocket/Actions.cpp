#include "Common.hpp"

// ============================================================================
//
// ACTIONS
//
// ============================================================================

void Extension::DEPRECATED_TestReportAndErrors()
{
	globals->CreateError(-1, _T("Testing report functions in Fusion event line %d is no longer used. Remove it."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::DEPRECATED_UsePopupMessages(int OnOrOff)
{
	globals->CreateError(-1, _T("Use Popup Messages in Fusion event line %d is no longer used. Remove it, and roll your "
		"own with Powerful Dialogs object, or Popup Message Object 2."), DarkEdif::GetCurrentFusionEventNum(this));
}

// ============================================================================
//
// CLIENT
//
// ============================================================================

void Extension::DEPRECATED_ClientInitialize_Basic(const TCHAR * hostname, int port, const TCHAR * protocol, const TCHAR * initialText)
{
	globals->CreateError(-1, _T("Old Client Initalize Basic action used on Fusion event line %d. Re-create the action."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::ClientInitialize_Basic(const TCHAR * hostnameParam, int port, const TCHAR * protocol)
{
	if (port < 1 || port > UINT16_MAX)
		return globals->CreateError(-1, _T("Basic client initialize error: port must be greater than 0 and less than 65535; you passed %d."), _T(__FUNCTION__), port);

	// This is the basic action, so limit it to 3 protocols
	std::string protoASCIIUpper = DarkEdif::TStringToANSI(protocol);
	std::transform(protoASCIIUpper.begin(), protoASCIIUpper.end(), protoASCIIUpper.begin(),
		[](unsigned char c) { return std::toupper(c); });

	if (protoASCIIUpper != "TCP"sv && protoASCIIUpper != "UDP"sv && protoASCIIUpper != "ICMP"sv)
		return globals->CreateError(-1, _T("Basic client initialize error: Protocol \"%s\" unrecognised. Use \"TCP\", \"UDP\", or \"ICMP\", or advanced client initialize."), protocol);

	std::tstring_view hostname(hostnameParam);
	if (hostname.empty())
		return globals->CreateError(-1, _T("Hostname cannot be blank."));

	// Move text to a number from a macro
	int Protocol2 = GlobalInfo::ProtocolTypeTextToNum(protoASCIIUpper);
	// If error'd out report it.
	if (Protocol2 == -1)
		return globals->CreateError(-1, _T("Protocol unrecognised. Use the help file to see the available protocols."));

	std::unique_ptr<StructPassThru> threadParams = std::make_unique<StructPassThru>();

	// Contains colon, is it IPv6 address?
	if (hostname.find(':') != std::tstring::npos)
	{
		// It's invalid to have https://, so anything with ":" will be IPv6
		if (hostname.find_first_not_of(_T("0123456789abcdefABCDEF"sv)) != std::tstring::npos)
			return globals->CreateError(-1, _T("Hostname %s has invalid characters. You should only be passing a domain or IP address, don't include protocol like \"http://\"."), hostname);
		threadParams->addressFamily = AF_INET6;
	}
	else // Assume IPv4
		threadParams->addressFamily = AF_INET; // INET = INTERNET = IPv4

	threadParams->ext = this;
	threadParams->client_hostname = hostname;
	threadParams->port = port;
	threadParams->protoType = Protocol2;
	if (protoASCIIUpper == "TCP"sv)
		threadParams->socketType = SOCK_STREAM;	// Default for TCP
	if (protoASCIIUpper == "UDP"sv)
		threadParams->socketType = SOCK_DGRAM;	// Default for UDP
	if (protoASCIIUpper == "ICMP"sv)
		threadParams->socketType = SOCK_RAW;		// Default for ICMP

	globals->ReportInfo(-1, _T("Client regular thread booting with basic parameters..."));
	std::shared_ptr<Thread> newThread = std::make_shared<Thread>(globals);
	newThread->isServer = false;
	newThread->thread = std::thread(&GlobalInfo::ClientThread, globals, newThread, std::move(threadParams));
	globals->socketThreadList.push_back(std::move(newThread));
}
void Extension::DEPRECATED_ClientInitialize_Advanced(const TCHAR * Hostname, int Port, const TCHAR * Protocol, const TCHAR * AddressFamily, const TCHAR * SocketType, const TCHAR * initialText)
{
	globals->CreateError(-1, _T("Old Client Initialize Advanced action in Fusion event line %d. Recreate the action."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::ClientInitialize_Advanced(const TCHAR * hostnameParam, int port, const TCHAR * Protocol, const TCHAR * AddressFamily, const TCHAR * SocketType)
{
	if (port < 1 || port > UINT16_MAX)
		return globals->CreateError(-1, _T("Basic client initialize error: port must be greater than 0 and less than 65535; you passed %d."), _T(__FUNCTION__), port);

	std::tstring hostname(hostnameParam);
	if (hostname.empty())
		return globals->CreateError(-1, _T("Hostname must not be blank."));

	// Move text to a number from a macro
	int Protocol2 = GlobalInfo::ProtocolTypeTextToNum(DarkEdif::TStringToANSI(Protocol));
	int AddressFamily2 = GlobalInfo::AddressFamilyTextToNum(DarkEdif::TStringToANSI(AddressFamily));
	int SocketType2 = GlobalInfo::SocketTypeTextToNum(DarkEdif::TStringToANSI(SocketType));
	// If error'd out report it
	if (Protocol2 == -1)
		return globals->CreateError(-1, _T("Protocol unrecognised. Use the help file to see the available protocols."));
	if (AddressFamily2 == -1)
		return globals->CreateError(-1, _T("Address Family unrecognised. Use the help file to see the available families."));
	if (SocketType2 == -1)
		return globals->CreateError(-1, _T("Socket type unrecognised. Use the help file to see the available socket types."));

	// If no errors, go for it. This is a seperate if, rather than
	// if-else-if, so several errors with the parameters can be reported.
	if (Protocol2 != -1 && AddressFamily2 != -1 && SocketType2 != -1)
	{
		std::unique_ptr<StructPassThru> Parameters = std::make_unique<StructPassThru>();
		Parameters->addressFamily = AddressFamily2;
		Parameters->ext = this;
		Parameters->client_hostname = hostname;
		Parameters->port = port;
		Parameters->protoType = Protocol2;
		Parameters->socketType = SocketType2;

		std::shared_ptr<Thread> newThread = std::make_shared<Thread>(globals);
		newThread->isServer = false;

		// IRDA uses different struct to other protocols
		if (Protocol2 != AF_IRDA)
		{
			globals->ReportInfo(-1, _T("Client regular thread booting with advanced parameters..."));
			newThread->thread = std::thread(&GlobalInfo::ClientThread, globals, newThread, std::move(Parameters));
		}
		else
		{
			globals->ReportInfo(-1, _T("Client IRDA thread booting with advanced parameters..."));
			newThread->thread = std::thread(&GlobalInfo::ClientThreadIRDA, globals, newThread, std::move(Parameters));
		}
		globals->socketThreadList.push_back(std::move(newThread));
	}
}
void Extension::ClientCloseSocket(int socketID)
{
	auto sock = globals->GetSocket("Client shutdown socket", SocketType::Client, socketID);
	if (!sock)
		return;

	sock->lock.edif_lock();
	sock->shutdownPending = true;
	sock->changesPendingForThread = true;
	sock->lock.edif_unlock();
}
void Extension::DEPRECATED_ClientSend(int socketID, const TCHAR * packet)
{
	size_t max = _tcslen(packet);
	max = std::min(max, 15U);
	globals->CreateError(socketID, _T("Old Client Send action used for text \"%.*s...\" on Fusion event line %d. Replace the action."),
		max, packet, DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::ClientSend(int socketID, const TCHAR * packet, const TCHAR * encodingParam, int flags)
{
	const std::string encoding(DarkEdif::TStringToUTF8(encodingParam));

	auto sock = globals->GetSocket("Client send action", SocketType::Client, socketID);
	if (!sock)
		return;

	sock->lock.edif_lock();

	if (sock->mainSocketFD == -1)
	{
		sock->lock.edif_unlock();
		return globals->CreateError(socketID, _T("Client socket is not valid, cannot send."));
	}

	std::string msgTo;
	if (!Internal_SetTextWithEncoding(encoding, packet, msgTo, sock->mainSocketFD))
	{
		sock->lock.edif_unlock();
		return;
	}

#if _WIN32
	// Only two recognised WinSock flags
	if (!(flags & MSG_DONTROUTE | MSG_OOB))
	{
		sock->lock.edif_unlock();
		return globals->CreateError(socketID, _T("Flags %i are not recognised."), flags);
	}
#endif

	// sendto is equivalent to send, with address ignored for one-destination anyway
	int ret = sendto(sock->mainSocketFD, msgTo.data(), msgTo.size(), flags, (sockaddr *)&sock->mainSockAddress, sock->mainSockAddressSize);
	sock->lock.edif_unlock();

	// sock->sources[0] may be invalid
	if (ret > 0)
	{
		++sock->sources[0]->numPacketsOut;
		sock->sources[0]->bytesOut += ret;
	}

	// Incomplete send. If this is common, I'll get the ticking to handle packet sending too, so it can send the rest.
	if (sock->HandleSockOpReturn("sending packet on client socket", ret) && ret < (int)msgTo.size())
		globals->CreateError(socketID, _T("Only sent %d bytes of packet instead of %zu bytes; sending overloaded?"), ret, msgTo.size());
}
void Extension::REMOVED_ClientGoIndependent(int socketID)
{
	globals->CreateError(socketID, _T("Going independent action is no longer available. Remove the action from Fusion event line %d."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::ClientReceiveOnly(int socketID)
{
	auto sock = globals->GetSocket("Setting client socket to receive-only", SocketType::Client, socketID);
	if (!sock)
		return;

	sock->lock.edif_lock();
	sock->receiveOnly = true;
	sock->changesPendingForThread = true;
	sock->lock.edif_unlock();
}
void Extension::DEPRECATED_ClientLinkFileOutput(int socketID, const TCHAR* filename)
{
	globals->CreateError(socketID, _T("Deprecated \"link file output\" action used on Fusion event line %d. Saving to file should now be done by other extensions."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::DEPRECATED_ClientUnlinkFileOutput(int socketID)
{
	globals->CreateError(socketID, _T("Deprecated \"unlink file output\" action used on Fusion event line %d. Saving to file should now be done by other extensions."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::REMOVED_ClientFusionReport(int socketID, int OnOrOff)
{
	globals->CreateError(socketID, _T("Fusion reporting is now always enabled. Remove the action from Fusion event line %d."),
		DarkEdif::GetCurrentFusionEventNum(this));
}


// ============================================================================
//
/// SERVER
//
// ============================================================================

void Extension::ServerInitialize_Basic(const TCHAR * protocol, int port)
{
	if (port < 1 || port > UINT16_MAX)
		return globals->CreateError(-1, _T("Basic server initialize error: Port must be greater than 0 and less than 65535, you passed %d."), port);

	// This is the basic action, so limit it to 3 protocols
	std::string protoASCIIUpper = DarkEdif::TStringToANSI(protocol);
	std::transform(protoASCIIUpper.begin(), protoASCIIUpper.end(), protoASCIIUpper.begin(),
		[](unsigned char c) { return std::toupper(c); });

	if (protoASCIIUpper != "TCP"sv && protoASCIIUpper != "UDP"sv)
		return globals->CreateError(-1, _T("Basic server initialize error: Protocol \"%s\" unrecognised. Use \"TCP\" or \"UDP\", or advanced server initialize."), protocol);

	// Move text to a number from a macro
	int Protocol2 = GlobalInfo::ProtocolTypeTextToNum(protoASCIIUpper);
	// If error'd out report it.
	if (Protocol2 == -1)
		return globals->CreateError(-1, _T("Basic server initialize error: Protocol \"%s\" unrecognised. Use the help file to see the available protocols."), protocol);

	std::unique_ptr<StructPassThru> threadParams = std::make_unique<StructPassThru>();
	threadParams->addressFamily = AF_UNSPEC; // both AF_INET and AF_INET6
	threadParams->ext = this;
	threadParams->port = port;
	threadParams->protoType = Protocol2;
	if (protoASCIIUpper == "TCP"sv)
		threadParams->socketType = SOCK_STREAM;	// Default for TCP
	else // if (protoASCIIUpper == "UDP"sv)
		threadParams->socketType = SOCK_DGRAM;	// Default for UDP
	threadParams->server_InAddr = {}; // IN6ADDR_INIT; INADDR_ANY, accepts all incoming addresses

	globals->ReportInfo(-1, _T("Server regular thread booting with basic parameters..."));
	std::shared_ptr<Thread> newThread = std::make_shared<Thread>(globals);
	newThread->isServer = true;
	newThread->thread = std::thread(&GlobalInfo::ServerThread, globals, newThread, std::move(threadParams));
	globals->socketThreadList.push_back(std::move(newThread));
}
void Extension::ServerInitialize_Advanced(const TCHAR * protocol, const TCHAR * addressFamily, const TCHAR * socketType, int port, const TCHAR * InAddr)
{
	if (port < 1 || port > UINT16_MAX)
		return globals->CreateError(-1, _T("Advanced server initialize error: Port must be greater than 0 and less than 65535, you passed %d."), port);

	// Move text to a number from a macro
	int protocolNum = GlobalInfo::ProtocolTypeTextToNum(DarkEdif::TStringToANSI(protocol));
	int addressFamilyNum = GlobalInfo::AddressFamilyTextToNum(DarkEdif::TStringToANSI(addressFamily));
	int socketTypeNum = GlobalInfo::SocketTypeTextToNum(DarkEdif::TStringToANSI(socketType));
	in6_addr InAddr2 = {}; // all zeros is IN6ADDR_INIT or INADDR_ANY, accepts all incoming addresses

	// If error'd out report it.
	if (protocolNum == -1)
		return globals->CreateError(-1, _T("Protocol unrecognised. Use the help file to see the available protocols."), protocol);
	if (addressFamilyNum == -1)
		return globals->CreateError(-1, _T("Address family \"%s\" unrecognised. Use the help file to see the available families."), addressFamily);
	if (socketTypeNum == -1)
		return globals->CreateError(-1, _T("Socket type \"%s\" unrecognised. Use the help file to see the available socket types."), socketType);
	if (!GlobalInfo::InAddrTextToStruct(DarkEdif::TStringToANSI(InAddr), &InAddr2))
		return globals->CreateError(-1, _T("InAddr type \"%s\" unrecognised. Use the help file to see the available InAddr types."), InAddr);

	std::unique_ptr<StructPassThru> threadParams = std::make_unique<StructPassThru>();
	threadParams->addressFamily = addressFamilyNum;
	threadParams->ext = this;
	threadParams->port = port;
	threadParams->protoType = protocolNum;
	threadParams->socketType = socketTypeNum;
	threadParams->server_InAddr = InAddr2;

	globals->ReportInfo(-1, _T("Server regular thread booting with basic parameters..."));
	std::shared_ptr<Thread> newThread = std::make_shared<Thread>(globals);
	newThread->isServer = true;

	// IRDA uses different struct to other protocols
	if (protocolNum != AF_IRDA)
	{
		globals->ReportInfo(-1, _T("Server regular thread booting with advanced parameters..."));
		newThread->thread = std::thread(&GlobalInfo::ServerThread, globals, newThread, std::move(threadParams));
	}
	else
	{
		globals->ReportInfo(-1, _T("Server IRDA thread booting with advanced parameters..."));
		newThread->thread = std::thread(&GlobalInfo::ServerThreadIRDA, globals, newThread, std::move(threadParams));
	}
	globals->socketThreadList.push_back(std::move(newThread));
}
void Extension::ServerShutdown(int socketID)
{
	auto sock = globals->GetSocket("shutdown socket", SocketType::Server, socketID);
	if (!sock)
		return;

	sock->lock.edif_lock();
	sock->shutdownPending = true;
	sock->changesPendingForThread = true;
	sock->lock.edif_unlock();
}
void Extension::ServerShutdownPeerSocket(int socketID, int peerSocketID, int immediate)
{
	auto sock = globals->GetSocketSource("shutdown peer socket", socketID, peerSocketID);
	if (!sock)
		return;

	sock->lock.edif_lock();
	if (immediate == 1)
	{
		const int yes = 1;
		int ret = setsockopt(sock->peerSocketFD, SOL_SOCKET, SO_DONTLINGER, (const char*)&yes, sizeof(yes));
		sock->sock->HandleSockOpReturn("shutdown peer socket - setting SO_DONTLINGER", ret, true);
	}
#if _WIN32
	closesocket(sock->peerSocketFD);
#else
	close(sock->peerSocketFD);
#endif
	sock->lock.edif_unlock();
}
void Extension::DEPRECATED_ServerSend(int socketID, const TCHAR * packet, const TCHAR* clientID)
{
	size_t max = _tcslen(packet);
	max = std::min(max, 15U);
	globals->CreateError(socketID, _T("Old Server send action used for text \"%.*s...\" on Fusion event line %d. Re-create the action."),
		max, packet, DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::ServerSend(int socketID, int peerSocketID, const TCHAR * packet, const TCHAR * encodingParam,  int flags)
{
	const std::string encoding(DarkEdif::TStringToUTF8(encodingParam));

	auto sock = globals->GetSocketSource("Server send action", socketID, peerSocketID);
	if (!sock)
		return;

	std::string msgTo;
	if (!Internal_SetTextWithEncoding(encoding, packet, msgTo, socketID))
		return;

#if _WIN32
	// Only two recognised WinSock flags
	if (!(flags & MSG_DONTROUTE | MSG_OOB))
		return globals->CreateError(socketID, _T("Flags %i are not recognised."), flags);
#endif

	sock->lock.edif_lock();
	// sendto is equivalent to send, with address ignored for one-destination anyway
	int ret = sendto(sock->peerSocketFD, msgTo.data(), msgTo.size(), flags, (sockaddr *)&sock->peerSockAddress, sock->peerSockAddressSize);
	sock->lock.edif_unlock();

	++sock->numPacketsOut;
	sock->bytesOut += ret;
	++sock->sock->sources[0]->numPacketsOut;
	sock->sock->sources[0]->bytesOut += ret;

	// Incomplete send! If this is common, I'll get the ticking to handle packet sending too, so it can send the rest.
	if (sock->sock->HandleSockOpReturn("sending packet on server socket", ret) && ret < (int)msgTo.size())
		globals->CreateError(socketID, _T("Only sent %d bytes instead of %zu bytes; sending overloaded?"), ret, msgTo.size());
}
void Extension::REMOVED_ServerGoIndependent(int socketID)
{
	globals->CreateError(socketID, _T("Going independent action is no longer available. Remove the action from Fusion event line %d."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::ServerAutoAccept(int socketID, int OnOrOff)
{
	auto sock = globals->GetSocket("Server auto-accept action", SocketType::Server, socketID);
	if (!sock)
		return;

	sock->lock.edif_lock();
	sock->autoAcceptOn = true;
	sock->changesPendingForThread = true;
	sock->lock.edif_unlock();
}
void Extension::DEPRECATED_ServerLinkFileOutput(int socketID, const TCHAR * File)
{
	globals->CreateError(socketID, _T("Old server link file output action used on Fusion event line %d. Re-create the action."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::DEPRECATED_ServerUnlinkFileOutput(int socketID)
{
	globals->CreateError(socketID, _T("Old server unlink file output action used on Fusion event line %d. Replace the action with \"link to file output\" with a blank filename."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::REMOVED_ServerFusionReport(int socketID, int OnOrOff)
{
	globals->CreateError(socketID, _T("Disabling Fusion reporting action is no longer available. Remove the action from Fusion event line %d."),
		DarkEdif::GetCurrentFusionEventNum(this));
}


// ============================================================================
//
/// PACKET BEING BUILT
//
// ============================================================================


void Extension::PacketBeingBuilt_NewPacket(int size)
{
	if (size < 0)
		return globals->CreateError(-1, _T("Could not begin new packet; size %d is too small."), size);
	if (size > 10 * 1024 * 1024)
		return globals->CreateError(-1, _T("Could not begin new packet; size %d is too large. Break up your packet into smaller ones."), size);

	// Clear existing content
	memset(packetBeingBuilt.data(), 0, packetBeingBuilt.size());
	packetBeingBuilt.resize(size);
}
void Extension::PacketBeingBuilt_ResizePacket(int sizeInBytes)
{
	// For sanity reasons, we cap the packet to 10MB. The network will likely break it up sooner than that.
	if (sizeInBytes < 0 || sizeInBytes > 10 * 1024 * 1024)
		return globals->CreateError(-1, _T("Built packet can't be resized to %d bytes!"), sizeInBytes);

	packetBeingBuilt.resize(sizeInBytes);
}
void Extension::Internal_BuiltPacketSizeSet(int& builtPacketOffset)
{
	if (builtPacketOffset == -1)
		builtPacketOffset = (int)packetBeingBuilt.size();
}

void Extension::PacketBeingBuilt_SetByte(int byteParam, int packetBeingBuiltOffset)
{
	Internal_BuiltPacketSizeSet(packetBeingBuiltOffset);
	if (byteParam < INT8_MIN || byteParam > UINT8_MAX)
		return globals->CreateError(-1, _T("Can't set byte: a byte cannot contain the number %i."), byteParam);
	if (packetBeingBuiltOffset < 0 || (size_t)packetBeingBuiltOffset > packetBeingBuilt.size())
		return globals->CreateError(-1, _T("Can't set byte: Packet offset %i is invalid."), packetBeingBuiltOffset);

	const char byteToAdd = *(char *)&byteParam;

	if ((size_t)packetBeingBuiltOffset < packetBeingBuilt.size() && (packetBeingBuilt.size() - (size_t)packetBeingBuiltOffset) < sizeof(char))
		return globals->CreateError(-1, _T("Can't set byte: packet index %i is beyond the packet's index range 0 to %d. Resize the packet first."),
			packetBeingBuiltOffset, ((int)packetBeingBuilt.size()) - 1);

	if ((size_t)packetBeingBuiltOffset == packetBeingBuilt.size())
		packetBeingBuilt += byteToAdd;
	else
		packetBeingBuilt[(size_t)packetBeingBuiltOffset] = byteToAdd;
}
void Extension::PacketBeingBuilt_SetShort(int shortParam, int packetBeingBuiltOffset, int runHTON)
{
	Internal_BuiltPacketSizeSet(packetBeingBuiltOffset);
	if (shortParam < INT16_MIN || shortParam > UINT16_MAX)
		return globals->CreateError(-1, _T("Can't set short: a short cannot contain the number %i."), shortParam);
	if (packetBeingBuiltOffset < 0 || (size_t)packetBeingBuiltOffset > packetBeingBuilt.size() || (size_t)packetBeingBuiltOffset == packetBeingBuilt.size() - 1)
		return globals->CreateError(-1, _T("Can't set short: Packet being built index %i is invalid."), packetBeingBuiltOffset);
	if ((runHTON & 1) != runHTON)
		return globals->CreateError(-1, _T("Can't set short: Run htons() parameter is %i, can only be 0 or 1."), runHTON);

	unsigned short shortToAdd = *(unsigned short *)&shortParam;
	if (runHTON != 0)
	{
		// htons() requires unsigned short - we checked for UINT16_MAX above
		if (shortParam < 0)
			return globals->CreateError(-1, _T("Can't set short: Run htons() is enabled, so it should be an unsigned short; between 0 and 65535, inclusive."), shortParam);
		shortToAdd = htons(shortToAdd);
	}

	if ((size_t)packetBeingBuiltOffset < packetBeingBuilt.size() && (packetBeingBuilt.size() - (size_t)packetBeingBuiltOffset) < sizeof(unsigned short))
		return globals->CreateError(-1, _T("Can't set short: index %i - %i is beyond the packet's index range 0 to %d. Resize the packet first."),
			packetBeingBuiltOffset, packetBeingBuiltOffset + 1, ((int)packetBeingBuilt.size()) - 2);

	if ((size_t)packetBeingBuiltOffset == packetBeingBuilt.size())
		packetBeingBuilt.append((char*)&shortToAdd, sizeof(shortToAdd));
	else
		packetBeingBuilt.replace((size_t)packetBeingBuiltOffset, sizeof(shortToAdd), std::string_view((char *)&shortToAdd, sizeof(shortToAdd)));
}

void Extension::PacketBeingBuilt_SetInteger(int intParam, int packetBeingBuiltOffset, int runHTONL)
{
	Internal_BuiltPacketSizeSet(packetBeingBuiltOffset);
	if (intParam < INT32_MIN || intParam > UINT32_MAX)
		return globals->CreateError(-1, _T("Can't set integer: a integer cannot contain the number %i."), intParam);
	if (packetBeingBuiltOffset < 0 || ((size_t)packetBeingBuiltOffset > packetBeingBuilt.size() - 4 && (size_t)packetBeingBuiltOffset != packetBeingBuilt.size()))
		return globals->CreateError(-1, _T("Can't set integer: Packet offset %i is invalid."), packetBeingBuiltOffset);
	if ((runHTONL & 1) != runHTONL)
		return globals->CreateError(-1, _T("Can't set integer: Run htonl() parameter is %i, can only be 0 or 1."), runHTONL);

	if (runHTONL == 1)
		intParam = htonl(intParam);

	if ((size_t)packetBeingBuiltOffset < packetBeingBuilt.size() && (packetBeingBuilt.size() - (size_t)packetBeingBuiltOffset) < sizeof(int))
		return globals->CreateError(-1, _T("Can't set integer: index %i - %i is beyond the packet's index range 0 to %d. Resize the packet first."),
			packetBeingBuiltOffset, packetBeingBuiltOffset + 1, ((int)packetBeingBuilt.size()) - 1);

	if ((size_t)packetBeingBuiltOffset == packetBeingBuilt.size())
		packetBeingBuilt.append((char*)&intParam, sizeof(intParam));
	else
		packetBeingBuilt.replace((size_t)packetBeingBuiltOffset, sizeof(intParam), std::string_view((char*)&intParam, sizeof(intParam)));
}
void Extension::DEPRECATED_PacketBeingBuilt_SetLong(int LongP, int packetBeingBuiltOffset, int runHTONL)
{
	globals->CreateError(-1, _T("Can't set long: this action is ambiguous and was removed. Replace the action on Fusion event line %d with the 32-bit integer or 64-bit integer actions instead."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::PacketBeingBuilt_SetFloat(float floatParam, int packetBeingBuiltOffset)
{
	Internal_BuiltPacketSizeSet(packetBeingBuiltOffset);
	if (packetBeingBuiltOffset < 0 || (size_t)packetBeingBuiltOffset >= packetBeingBuilt.size())
		return globals->CreateError(-1, _T("Can't set float: Packet being built offset %i is invalid."), packetBeingBuiltOffset);

	if ((size_t)packetBeingBuiltOffset < packetBeingBuilt.size() && (packetBeingBuilt.size() - (size_t)packetBeingBuiltOffset) < sizeof(float))
		return globals->CreateError(-1, _T("Can't set float: index %i - %i is beyond the packet being built's index range 0 to %d. Resize the packet first."),
			packetBeingBuiltOffset, packetBeingBuiltOffset + 1, ((int)packetBeingBuilt.size()) - 1);

	if ((size_t)packetBeingBuiltOffset == packetBeingBuilt.size())
		packetBeingBuilt.append(std::string_view((char*)&floatParam, sizeof(floatParam)), sizeof(floatParam));
	else
		packetBeingBuilt.replace((size_t)packetBeingBuiltOffset, sizeof(floatParam), std::string_view((char*)&floatParam, sizeof(floatParam)));
}
void Extension::PacketBeingBuilt_SetDouble(float doubleParam, int packetBeingBuiltOffset)
{
	Internal_BuiltPacketSizeSet(packetBeingBuiltOffset);
	if (packetBeingBuiltOffset < 0 || (size_t)packetBeingBuiltOffset > packetBeingBuilt.size())
		return globals->CreateError(-1, _T("Can't set double: Packet being built offset %i is invalid."), packetBeingBuiltOffset);

	if ((size_t)packetBeingBuiltOffset < packetBeingBuilt.size() && (packetBeingBuilt.size() - (size_t)packetBeingBuiltOffset) < sizeof(double))
		return globals->CreateError(-1, _T("Can't set double: index %i - %i is beyond the packet's index range 0 to %d. Resize the packet first."),
			packetBeingBuiltOffset, packetBeingBuiltOffset + 1, ((int)packetBeingBuilt.size()) - 1);

	if ((size_t)packetBeingBuiltOffset == packetBeingBuilt.size())
		packetBeingBuilt.append((char*)&doubleParam, sizeof(doubleParam));
	else
		packetBeingBuilt.replace((size_t)packetBeingBuiltOffset, sizeof(doubleParam), std::string_view((char*)&doubleParam, sizeof(doubleParam)));
}
void Extension::DEPRECATED_PacketBeingBuilt_SetString(const TCHAR * text, int packetBeingBuiltOffset, int sizeOfString)
{
	globals->CreateError(-1, _T("Can't set string: this action was encoding-ambiguous and was removed. Replace the action on Fusion event line %d."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::DEPRECATED_PacketBeingBuilt_SetWString(const TCHAR* text, int packetBeingBuiltOffset, int sizeOfString)
{
	globals->CreateError(-1, _T("Can't set wide strin1g: this action was encoding-ambiguous and was removed. Replace the action on Fusion event line %d."),
		DarkEdif::GetCurrentFusionEventNum(this));
}
void Extension::DEPRECATED_PacketBeingBuilt_SetBuffer(int memoryAddress, int packetBeingBuiltOffset, int sizeOfBank)
{
	globals->CreateError(-1, _T("Setting buffer action needs to be re-created. Memory addresses can be 64-bit "
		"and Fusion only supports 32-bit integers. Use the new action, and pass Str$(number_address) if needed."));
}

void Extension::PacketBeingBuilt_SetBuffer(const TCHAR * memoryAddress, int sizeOfBank, int packetBeingBuiltOffset)
{
	Internal_BuiltPacketSizeSet(packetBeingBuiltOffset);
	if (packetBeingBuiltOffset < 0 || (size_t)packetBeingBuiltOffset > packetBeingBuilt.size())
		return globals->CreateError(-1, _T("Can't copy memory into packet: Packet being built offset %i is invalid."), packetBeingBuiltOffset);
	if (sizeOfBank <= 0)
		return globals->CreateError(-1, _T("Can't copy memory into packet; size %i is too small."), sizeOfBank);

	// Inserting at end: we're using memcpy, so pre-reserve the space
	if ((size_t)packetBeingBuiltOffset == packetBeingBuilt.size())
		packetBeingBuilt.resize(packetBeingBuilt.size() + sizeOfBank);
	else if (packetBeingBuilt.size() - packetBeingBuiltOffset < (size_t)sizeOfBank)
	{
		return globals->CreateError(-1, _T("Can't copy %i bytes of memory: the requested index %i - %i is beyond the packet's index range 0 to %d. Resize the packet first."),
			sizeOfBank, packetBeingBuiltOffset, packetBeingBuiltOffset + sizeOfBank, ((int)packetBeingBuilt.size()) - 1);
	}

	// The size of "long" is 64-bit on 64-bit platforms, 32-bit on 32-bit platforms
	std::uint64_t conv;
	if constexpr (sizeof (unsigned long) == sizeof(std::uint64_t))
		conv = (std::uint64_t)_tcstoul(memoryAddress, NULL, 0); // 64-bit
	else
		conv = (std::uint64_t)_tcstoull(memoryAddress, NULL, 0); // 32-bit

	if (errno == ERANGE || errno == EINVAL)
		return globals->CreateError(-1, _T("Couldn't set buffer from memory address \"%s\", the address was too small or too big to be an address."), memoryAddress);

	if (!memcpy(&packetBeingBuilt[packetBeingBuiltOffset], (void *)conv, (size_t)sizeOfBank))
		globals->CreateError(-1, _T("Set bank to bank failed; error %d: %s."), errno, _tcserror(errno));
}


void Extension::PacketBeingBuilt_SetInt64(const TCHAR* int64AsText, int packetBeingBuiltOffset)
{
	Internal_BuiltPacketSizeSet(packetBeingBuiltOffset);
	if (packetBeingBuiltOffset < 0 || (size_t)packetBeingBuiltOffset > packetBeingBuilt.size())
		return globals->CreateError(-1, _T("Can't set int64: Packet being built offset %i is invalid. Must be 0 to %zu."), packetBeingBuiltOffset, packetBeingBuilt.size());

	if ((packetBeingBuilt.size() - (size_t)packetBeingBuiltOffset) < sizeof(std::int64_t))
		return globals->CreateError(-1, _T("Can't set int64: index %i - %i is beyond the being built's index range 0 to %zu. Resize the packet being built first."),
			packetBeingBuiltOffset, packetBeingBuiltOffset + 1, packetBeingBuilt.size() - 1U);

	std::string int64AsANSIText = DarkEdif::TStringToANSI(int64AsText);
	if (int64AsText[0] == '\0')
		return globals->CreateError(-1, _T("Can't set int64: \"\" is not an int64."));
	std::int64_t conv;
	errno = 0;
	if (int64AsText[0] == '-')
	{
		// The size of "long" is 64-bit on 64-bit platforms, 32-bit on 32-bit platforms
		if constexpr (sizeof(long) == sizeof(std::int64_t))
			conv = (std::int64_t)strtol(int64AsANSIText.c_str(), NULL, 0); // 64-bit
		else
			conv = (std::int64_t)strtoll(int64AsANSIText.c_str(), NULL, 0); // 32-bit
	}
	else
	{
		if constexpr (sizeof(unsigned long) == sizeof(std::uint64_t))
			*(std::uint64_t *)&conv = (std::uint64_t)strtoul(int64AsANSIText.c_str(), NULL, 0); // 64-bit
		else
			*(std::uint64_t *)&conv = (std::uint64_t)strtoull(int64AsANSIText.c_str(), NULL, 0); // 32-bit
	}

	if (errno == ERANGE || errno == EINVAL)
		return globals->CreateError(-1, _T("Can't set int64: \"%s\" is too small or too big to fit in an int64."), int64AsText);

	if ((size_t)packetBeingBuiltOffset == packetBeingBuilt.size())
		packetBeingBuilt.append((char*)&conv, sizeof(conv));
	else
		packetBeingBuilt.replace((size_t)packetBeingBuiltOffset, sizeof(conv), (char*)&conv, sizeof(conv));
}
void Extension::PacketBeingBuilt_SetString(const TCHAR * text, const TCHAR * encoding, int packetBeingBuiltOffset, int includeNull)
{
	Internal_BuiltPacketSizeSet(packetBeingBuiltOffset);
	if (encoding == _T("PACKET"sv))
	{
		return globals->CreateError(-1, _T("Can't set string inside packet being built with encoding \"PACKET\", \"PACKET\" is a keyword used for sending messages only."
			" Use the encoding of the text \".15%s...\" that you're intending to add."), text);
	}
	if ((includeNull & 1) != includeNull)
		return globals->CreateError(-1, _T("Can't set text inside packet being built: \"include null\" parameter is %i, can only be 0 or 1."), includeNull);

	std::string outputBytes;
	if (!Internal_SetTextWithEncoding(DarkEdif::TStringToANSI(encoding), text, outputBytes, -1))
		return; // assume it reported an error

	if (packetBeingBuiltOffset < 0 || (size_t)packetBeingBuiltOffset > packetBeingBuilt.size() - outputBytes.size())
		return globals->CreateError(-1, _T("Can't set string of %zu bytes inside packet being built at index %d. The range is 0 to %zu."), outputBytes.size(), packetBeingBuiltOffset, packetBeingBuilt.size() - 1U);

	// Expand to add two null bytes if UTF-16, one byte if UTF-8, ANSI, etc
	if (includeNull == 1)
		outputBytes.resize(outputBytes.size() + (encoding == _T("UTF-16"sv) ? 2 : 1));

	if ((size_t)packetBeingBuiltOffset == packetBeingBuilt.size())
		packetBeingBuilt.append(outputBytes);
	else
		packetBeingBuilt.replace((size_t)packetBeingBuiltOffset, outputBytes.size(), outputBytes);
}

void Extension::PendingData_DiscardBytes(int positionFrom, int numBytes)
{
	if (!curEvent->source)
		return globals->CreateError(curEvent->fusionSocketID, _T("Can't discard part of pending data, no source selected."));

	// Is start position in range?
	if (positionFrom < 0 || positionFrom > (int)curEvent->source->pendingDataToRead.size() - 1)
		return globals->CreateError(curEvent->fusionSocketID, _T("Can't discard %d bytes, position %d is invalid."), numBytes, positionFrom);

	// -1 becomes all after position
	if (numBytes == -1)
		numBytes = (int)curEvent->source->pendingDataToRead.size() - positionFrom;
	// Less than 0 (but else, so not -1), is too low
	else if (numBytes < 0)
		return globals->CreateError(curEvent->fusionSocketID, _T("Can't discard %d bytes, negative."), numBytes);
	// Too many bytes to discard, we don't have enough
	else if ((size_t)numBytes > curEvent->source->pendingDataToRead.size() - positionFrom)
		return globals->CreateError(curEvent->fusionSocketID, _T("Can't remove %d bytes, at position %d there is only %zu pending bytes."), numBytes, curEvent->source->pendingDataToRead.size());
	// Discard 0 = nothing to do
	else if (numBytes == 0)
		return;

	// We've eaten part of this message; mark it as unreadable
	if (curEvent->source->pendingDataToRead.size() - curEvent->msg.size() > (size_t)positionFrom + (size_t)numBytes)
		curEvent->msgIsFullyInPendingData = false;

	// Only A/C/E and Handle - main thread - will modify sock->pendingData, so it's safe to change without lock
	curEvent->source->pendingDataToRead.erase(positionFrom, numBytes);
	if (curEvent->source->pendingDataToReadCursor >= (size_t)positionFrom)
		curEvent->source->pendingDataToReadCursor = std::max((int)curEvent->source->pendingDataToReadCursor - numBytes, 0);
}
