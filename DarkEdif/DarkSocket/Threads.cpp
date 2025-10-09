#include "Common.hpp"

// ============================================================================
//
// THREADS
//
// ============================================================================

void GlobalInfo::ClientThread(std::shared_ptr<Thread> self, std::unique_ptr<StructPassThru> params)
{
	Extension * ext = params->ext;

	ReportInfo(self->fusionSocketID, _T("Finding a DNS address"));
	// Declare two pointers to addrinfo, and one hints that's used to indicate to OS what protocol we expect
#ifndef _UNICODE
	struct addrinfo *result = NULL, *addrInfo = NULL, hints = {};
#else // Windows Unicode only
	struct addrinfoW *result = NULL, *addrInfo = NULL, hints = {};
#define freeaddrinfo(x) FreeAddrInfoW(x)
#endif

	hints.ai_protocol = params->protoType;
	hints.ai_family = params->addressFamily;
	hints.ai_socktype = params->socketType;
	// default flags:
	// AI_ALL and V4_MAPPED queries for IPv6, and if fails, queries for IPv4 and returns a IPv6-mapped-result
	// AI_ADDRCONFIG prevents returning addresses that are valid but no adapter allows them
	// for example, returning IPv6 addresses on an IPv4-adapter-only system
	// https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
	hints.ai_flags = AI_ALL | AI_V4MAPPED | AI_ADDRCONFIG;

#if _WIN32
	// AI_XX flags are only supported in Vista+
	if (Internal_GetWinVer() < _WIN32_WINNT_VISTA)
		hints.ai_flags = 0;
#endif

	// If service name is not given, pass the port as the service name
	if (params->client_serviceName.empty() && params->port != 0)
		params->client_serviceName = std::to_tstring(params->port);

	// Note: client_hostname is edited in a certain way of using getaddrinfo, we don't use that way,
	// but that's why client_hostname.data() is used instead of c_str()

	int err;
	do {
#if _WIN32
		// Windows likes its UTF-16 variants
		err = GetAddrInfo(params->client_hostname.data(),
			params->client_serviceName.empty() ? NULL : params->client_serviceName.c_str(),
			(PADDRINFOT)&hints, (PADDRINFOT *)&addrInfo);
#else
		err = getaddrinfo(params->client_hostname.data(),
			params->client_serviceName.empty() ? NULL : params->client_serviceName.c_str(),
			&hints, &addrInfo);
#endif

		if (!self->HandleSockOpReturn("getaddrinfo()", err, true))
		{
			if (err == 1002 /* WSATRYAGAIN */)
			{
				ReportInfo(self->fusionSocketID, _T("getaddrinfo() returned \"try again\" error, will retry in 5 seconds."));
				std::this_thread::sleep_for(std::chrono::seconds(5));
				continue;
			};
			if (err == 1004 /* WSANO_DATA */)
			{
				return CreateError(self->fusionSocketID, _T("Error with getaddrinfo(): hostname \"%s\" with service name \"%s\" has DNS records, but no records that can be used to connect to. Thread aborting."),
					params->client_hostname.c_str());
			}
			return CreateError(self->fusionSocketID, _T("Error with getaddrinfo(): [%i] %s. Thread aborting."), err, gai_strerror(err));
		}

		ReportInfo(self->fusionSocketID, _T("getaddrinfo() success."));
		break;
	} while (true);

	ReportInfo(self->fusionSocketID, _T("DNS list found, trying to connect..."));
	// Create a SOCKET for connecting to server
	int addrInfoNum = 0;
	for (auto * curAddrInfo = addrInfo; curAddrInfo != NULL; curAddrInfo = curAddrInfo->ai_next) {
		++addrInfoNum;
		self->mainSocketFD = socket(curAddrInfo->ai_family, curAddrInfo->ai_socktype, curAddrInfo->ai_protocol);
		if (!self->HandleSockOpReturn("socket()", self->mainSocketFD))
			continue; // Move to next one

		ReportInfo(self->fusionSocketID, _T("socket() for address #%i okay, moving on to connect()"), addrInfoNum);
		// Connect to server.
		err = connect(self->mainSocketFD, curAddrInfo->ai_addr, (int)curAddrInfo->ai_addrlen);
		if (!self->HandleSockOpReturn("connect()", err, true))
		{
			CloseSocket(self->mainSocketFD);
			continue;
		};
		ReportInfo(self->fusionSocketID, _T("connect() for address #%i worked, done connecting!"), addrInfoNum);
		self->mainSockAddressSize = curAddrInfo->ai_addrlen;
		if (memcpy_s(&self->mainSockAddress, sizeof(self->mainSockAddress), curAddrInfo->ai_addr, curAddrInfo->ai_addrlen))
			CreateError(self->fusionSocketID, _T("Failed to copy memory; error %d."), errno);

		freeaddrinfo(addrInfo);
		addrInfo = nullptr;
		goto found;
	}

	CreateError(self->fusionSocketID, _T("Couldn't connect to any of the %d matching addresses. Closing client."), addrInfoNum);
	CloseSocket(self->mainSocketFD);
	freeaddrinfo(addrInfo);
#if defined(_WIN32) && defined(_UNICODE)
#undef freeaddrinfo
#endif
	return;

found:

	std::shared_ptr<SocketSource> socketSource = std::make_shared<SocketSource>(self, self->mainSockAddress, self->mainSockAddressSize);
	self->sources.push_back(socketSource);

	ReportInfo(self->fusionSocketID, _T("Switching to non-blocking socket"));
	{
		unsigned long UL = 1;
		// Set to non-blocking
		int error = ioctlsocket(self->mainSocketFD, FIONBIO, &UL);
		if (error != 0)
			CreateError(self->fusionSocketID, _T("Warning: non-blocking mode could not be set."));
	}
	ReportInfo(self->fusionSocketID, _T("Socket connected and set to non-blocking, now main loop start"));
	AddEvent(EventToRun(self->fusionSocketID, socketSource, Conditions::OnClientConnect));

	bool receiveOnly = false;
	// Default stuff
	size_t recvbufsize = 1 * 1024 * 1024; // 1MB
	std::unique_ptr<char[]> recvbuf = std::make_unique<char[]>(recvbufsize);

	// Main receiving loop
	int lastErr = 0, curReturn;
	while (true)
	{
		if (self->changesPendingForThread)
		{
			self->lock.edif_lock();
			ReportInfo(self->fusionSocketID, _T("Thread signaled for changes; looking for them."));

			if (self->shutdownPending)
			{
				ReportInfo(self->fusionSocketID, _T("Thread responding to shutdown request."));
				self->Shutdown();
				self->lock.edif_unlock();
				break;
			}

			if (!receiveOnly && self->receiveOnly)
			{
				ReportInfo(self->fusionSocketID, _T("Thread responding to receive-only request. Now read-only."));
				curReturn = shutdown(self->mainSocketFD, SD_SEND);
				receiveOnly = true;
				self->HandleSockOpReturn("shutdown(SD_SEND)", curReturn);
			}
			ReportInfo(self->fusionSocketID, _T("Thread change signal processed and reset."));
			self->changesPendingForThread = false;
			self->lock.edif_unlock();
		}

		fd_set readFDs = {}, exceptFDs = {};
		FD_ZERO(&readFDs);
		FD_ZERO(&exceptFDs);
		FD_SET(self->mainSocketFD, &readFDs);
		FD_SET(self->mainSocketFD, &exceptFDs);

		// select() burns CPU until timeout, so make it short
		struct timeval timeout = { 0, 100 * 1000 };

		curReturn = select(1, &readFDs, NULL, &exceptFDs, &timeout);

		// No FDs set; nothing to do
		if (curReturn == 0)
		{
			if (!self->changesPendingForThread)
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}

		if (!self->HandleSockOpReturn("select()", curReturn))
		{
			ReportInfo(self->fusionSocketID, _T("Got error %d when looking for received data."), curReturn);

			// Same error as last loop; give up
			if (lastErr == curReturn)
			{
				ReportInfo(self->fusionSocketID, _T("Error %d was repeated from last loop, stopping thread."), lastErr);
				break;
			}
			lastErr = curReturn;
			continue;
		}


		// This apparently only happens when out-of-band (OOB) data is returned.
		if (exceptFDs.fd_count > 0)
			ReportInfo(self->fusionSocketID, _T("Received out-of-band data."));

		if (readFDs.fd_count > 0)
		{
			// Receive data until the server closes the connection
			curReturn = recv(self->mainSocketFD, recvbuf.get(), recvbufsize, 0);
			if (self->HandleSockOpReturn("recv", curReturn, true))
			{
				if (curReturn > 0)
				{
					if (recvbufsize == curReturn)
						CreateError(self->fusionSocketID, _T("Too much data left to receive."));

					ReportInfo(self->fusionSocketID, _T("Msg recv: %i bytes."), curReturn);
					++socketSource->numPacketsIn;
					socketSource->bytesIn += curReturn;
					EventToRun etr(self->fusionSocketID, socketSource, Conditions::OnClientReceivedPacket);
					etr.msg.assign(recvbuf.get(), curReturn);
					this->AddEvent(std::move(etr));
					continue;
				}
				else // curReturn == 0, socket was gracefully closed
				{
					ReportInfo(self->fusionSocketID, _T("Client connection was closed. Thread exiting."));
					break;
				}
			}

			// error reported already, we check if continuable later
		}

		// OOB data to receive
		if (exceptFDs.fd_count > 0)
		{
			ULONG atMark;
			curReturn = ioctlsocket(self->mainSocketFD, SIOCATMARK, &atMark);
			if (curReturn < 0)
				CreateError(self->fusionSocketID, _T("Couldn't check if OOB data was present."));

			// If true, we're at mark; if not, got some more regular data to receive before OOB.
			// Worth noting recv() is guaranteed to not read past OOB in a recv(0) call...
			// unless MSG_OOBINLINE is used, which would prevent exceptFDs triggering anyway.
			if (atMark != 0)
			{
				curReturn = recv(self->fusionSocketID, recvbuf.get(), recvbufsize, MSG_OOB);
				if (!self->HandleSockOpReturn("recv for OOB", curReturn))
				{
					ReportInfo(self->fusionSocketID, _T("recv() for OOB failed with error %d, continuing thread"), curReturn);
					continue;
				}

				// TCP only supports one OOB byte at a time, weirdly.
				if (curReturn > 0)
				{
					++socketSource->numPacketsIn;
					socketSource->bytesIn += curReturn;
					EventToRun etr(self->fusionSocketID, socketSource, Conditions::OnClientReceivedPacket);
					etr.msg.assign(recvbuf.get(), curReturn);
					etr.msgIsOOB = true;
					this->AddEvent(std::move(etr));
					continue;
				}
			}
		}

		// Sockets are okay, time to loop again
		if (curReturn > 0)
			continue;

		// Error happened, and it was same as last time
		if (curReturn == lastErr)
		{
			ReportInfo(self->fusionSocketID, _T("Error %d was repeated from last loop, stopping thread."), lastErr);
			break;
		}
		else
		{
			lastErr = curReturn;
			continue;
		}

		static int conDedErrors[]{ ENOTCONN, 58 /* WSAESHUTDOWN */, ECONNABORTED, WSAECONNABORTED - WSABASEERR, WSAECONNRESET - WSABASEERR, ENETRESET};
		for (std::size_t i = 0; i < std::size(conDedErrors); ++i)
		{
			if (-lastErr == conDedErrors[i])
			{
				ReportInfo(self->fusionSocketID, _T("Client connection was closed. Thread exiting."));
				break;
			}
		}
		// continue;
	}


	ReportInfo(self->fusionSocketID, _T("* Main loop ended, beginning cleanup"));

	self->lock.edif_lock();
	if (!self->shutdownPending)
		self->Shutdown();

	AddEvent(EventToRun(self->fusionSocketID, socketSource, Conditions::OnClientDisonnect));

	self->lock.edif_unlock();
	while (true)
	{
		threadsafe.edif_lock();
		bool anyFound = std::any_of(eventsToRun.cbegin(), eventsToRun.cend(),
			[&](const std::unique_ptr<EventToRun>& e) { return e->fusionSocketID == self->fusionSocketID; });
		threadsafe.edif_unlock();
		if (!anyFound)
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	AddEvent(EventToRun(self->fusionSocketID, socketSource, Conditions::CleanupSocket));

	ReportInfo(self->fusionSocketID, _T("Client thread exit."));
}

void GlobalInfo::ServerThread(std::shared_ptr<Thread> self, std::unique_ptr<StructPassThru> params)
{
	Extension* ext = params->ext;

	struct sockaddr_storage addrInfo = {};
	addrInfo.ss_family = params->addressFamily;

	// Address family 0 indicates IPv4 + IPv6, but that only works for client connections.
	// We'll manipulate the server to give the same functionality.
	bool isIPv6And4 = params->addressFamily == AF_UNSPEC;
	size_t sockaddrSize = sizeof(sockaddr_storage);
	if (isIPv6And4)
		addrInfo.ss_family = AF_INET6;

	self->mainSocketFD = socket(addrInfo.ss_family, params->socketType, params->protoType);
	if (!self->HandleSockOpReturn("socket()", self->mainSocketFD))
		return CreateError(self->fusionSocketID, _T("Couldn't create a socket with address %d, socket %d, protocol %d."), params->addressFamily, params->socketType, params->protoType);

	// With this disabled, new server applications won't be able to re-use the port immediately - the OS
	// has to free the port for reusing, which can be several minutes later, equating to several minutes of downtime.
	unsigned long on = 1;
	int curReturn = setsockopt(self->mainSocketFD, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, 4); // sizeof int, we reuse as 8 byte
	if (!self->HandleSockOpReturn("setsockopt(reuse)", curReturn, true))
		return;

	if (isIPv6And4)
	{
		ReportInfo(self->fusionSocketID, _T("Switching to IPv6-and-IPv4"));
		int no = 0;
		curReturn = setsockopt(self->mainSocketFD, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&no, sizeof(no));
		self->HandleSockOpReturn("setsockopt(IPv6 only = no)", curReturn, true);
	}

	auto serverSocketSource = std::make_shared<SocketSource>(self, self->mainSockAddress, self->mainSockAddressSize);
	self->sources.push_back(serverSocketSource);


	ReportInfo(self->fusionSocketID, _T("Switching to non-blocking socket"));
	{
		// Set to non-blocking
		int error = ioctlsocket(self->mainSocketFD, FIONBIO, &on);
		if (error != 0)
			CreateError(self->fusionSocketID, _T("Warning: non-blocking mode could not be set."));
	}
	ReportInfo(self->fusionSocketID, _T("Socket prepared and set to non-blocking, now main loop start"));

	if (addrInfo.ss_family == AF_INET6)
	{
		// sockaddr_in and sockaddr_in6 has port as second member
		// ((unsigned short*)&addrInfo)[1] = params->port;
		((sockaddr_in6*)&addrInfo)->sin6_port = htons(params->port);
		((sockaddr_in6*)&addrInfo)->sin6_addr = params->server_InAddr;
		sockaddrSize = sizeof(sockaddr_in6);
	}
	else if (addrInfo.ss_family == AF_INET) {
		((sockaddr_in*)&addrInfo)->sin_port = htons(params->port);
		((sockaddr_in*)&addrInfo)->sin_addr = *(in_addr*)&params->server_InAddr;
		sockaddrSize = sizeof(sockaddr_in);
	}

	self->mainSockAddressSize = sockaddrSize;
	if (memcpy_s(&self->mainSockAddress, sizeof(self->mainSockAddress), &addrInfo, sockaddrSize))
		CreateError(self->fusionSocketID, _T("Failed to copy memory; error %d."), errno);

	curReturn = bind(self->mainSocketFD,
		(struct sockaddr*)&addrInfo, sockaddrSize);
	if (!self->HandleSockOpReturn("bind()", curReturn, true))
		return;
	ReportInfo(self->fusionSocketID, _T("Starting to listen"));

	curReturn = listen(self->mainSocketFD, 32);
	if (!self->HandleSockOpReturn("listen()", curReturn, true))
		return;

	// Master set holds both server and all the client FD sockets, working_set is returned by listening to changes
	fd_set master_set = {}, readFDs = {}, exceptFDs = {};
	FD_SET(self->mainSocketFD, &master_set);

	// select() burns CPU until timeout, so make it short
	struct timeval timeout = { 0, 100 * 1000 };

	AddEvent(EventToRun(self->fusionSocketID, self->sources[0], Conditions::OnServerStartHosting));

	bool receiveOnly = false;
	// Default stuff
	size_t recvbufsize = 1 * 1024 * 1024; // 1MB
	std::unique_ptr<char[]> recvbuf = std::make_unique<char[]>(recvbufsize);

	std::shared_ptr<SocketSource> socketSource;
	size_t newSockID = 0;
	int max_sd = self->mainSocketFD;
	// Main receiving loop
	int lastErr = 0;
	while (true)
	{
		if (self->changesPendingForThread)
		{
			self->lock.edif_lock();
			ReportInfo(self->fusionSocketID, _T("Thread signaled for changes; looking for them."));

			if (self->shutdownPending)
			{
				ReportInfo(self->fusionSocketID, _T("Thread responding to shutdown request."));
				self->Shutdown();
				self->lock.edif_unlock();
				break;
			}

			if (!receiveOnly && self->receiveOnly)
			{
				ReportInfo(self->fusionSocketID, _T("Thread responding to receive-only request. Now read-only."));
				curReturn = shutdown(self->mainSocketFD, SD_SEND);
				receiveOnly = true;
				self->HandleSockOpReturn("shutdown(SD_SEND)", curReturn);
			}
			ReportInfo(self->fusionSocketID, _T("Thread change signal processed and reset."));
			self->changesPendingForThread = false;
			self->lock.edif_unlock();
		}

		if (memcpy_s(&readFDs, sizeof(readFDs), &master_set, sizeof(master_set)) ||
			memcpy_s(&exceptFDs, sizeof(exceptFDs), &master_set, sizeof(master_set)))
		{
			CreateError(self->fusionSocketID, _T("Couldn't copy data, error %d."), errno);
			break;
		}

		curReturn = select(max_sd + 1, &readFDs, NULL, &exceptFDs, &timeout);

		// No FDs set; nothing to do
		if (curReturn == 0)
		{
			if (!self->changesPendingForThread)
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}

		if (!self->HandleSockOpReturn("select()", curReturn))
		{
			ReportInfo(self->fusionSocketID, _T("Got error %d when looking for received data."), curReturn);

			// Same error as last loop; give up
			if (lastErr == curReturn)
			{
				ReportInfo(self->fusionSocketID, _T("Error %d was repeated from last loop, stopping thread."), lastErr);
				break;
			}
			lastErr = curReturn;
			continue;
		}

		// This apparently only happens when out-of-band (OOB) data is returned.
		if (exceptFDs.fd_count > 0)
			ReportInfo(self->fusionSocketID, _T("Received out-of-band data."));

		int desc_ready = curReturn;
		for (int i = 0; i <= max_sd && desc_ready > 0; ++i)
		{
			if (FD_ISSET(i, &readFDs))
			{
				// Server socket was triggered - new incoming connection(s)
				if (i == self->mainSocketFD)
				{
					sockaddr_storage newSockAddr;
					int new_sd;
					do
					{
						int sockaddr_len = sizeof(newSockAddr);
						new_sd = accept(self->mainSocketFD, (sockaddr*)&newSockAddr, &sockaddr_len);
						if (new_sd < 0)
						{
							// Would block if no connection waiting - so ignore EWOULDBLOCK
							if (errno != EWOULDBLOCK)
								self->HandleSockOpReturn("accept()", curReturn, true);
							break; // no connection waiting
						}

						auto newSockSource = std::make_shared<SocketSource>(self, newSockAddr, sockaddr_len);
						newSockSource->peerSocketFD = new_sd;
						newSockSource->peerSocketID = newSockID++;
						ReportInfo(self->fusionSocketID, _T("New connection with peer sock ID %i was created."), newSockSource->peerSocketID);
						self->lock.edif_lock();
						self->sources.push_back(newSockSource);
						self->lock.edif_unlock();

						FD_SET(new_sd, &master_set);
						if (new_sd > max_sd)
							max_sd = new_sd;
						AddEvent(EventToRun(self->fusionSocketID, newSockSource, Conditions::OnServerPeerConnected));
					} while (new_sd != -1);

					goto exceptCheck; // jump to next FD
				}

				// It's an existing connection; find it
				auto socketSourceIt = std::find_if(self->sources.cbegin(), self->sources.cend(),
					[i](const std::shared_ptr<SocketSource>& src) {
						return src->peerSocketFD == i;
					});
				assert(socketSourceIt != self->sources.cend());
				socketSource = *socketSourceIt;

				// Receive data
				curReturn = recv(i, recvbuf.get(), recvbufsize, 0);
				if (self->HandleSockOpReturn("recv", curReturn, true))
				{
					if (curReturn > 0)
					{
						if (recvbufsize == curReturn)
							CreateError(self->fusionSocketID, _T("Too much data left to receive from client %d."), i);

						ReportInfo(self->fusionSocketID, _T("Msg recv: %i bytes."), curReturn);
						++socketSource->numPacketsIn;
						socketSource->bytesIn += curReturn;
						++serverSocketSource->numPacketsIn;
						serverSocketSource->bytesIn += curReturn;
						EventToRun etr(self->fusionSocketID, socketSource, Conditions::OnServerReceivedPacket);
						etr.msg.assign(recvbuf.get(), curReturn);
						AddEvent(std::move(etr));
						continue;
					}
					else // curReturn == 0, socket was gracefully closed
					{
						ReportInfo(self->fusionSocketID, _T("Client connection %i (peer sock ID %i) was closed."), i, socketSource->peerSocketID);
						AddEvent(EventToRun(self->fusionSocketID, socketSource, Conditions::OnServerPeerDisconnected));
						CloseSocket(socketSource->peerSocketFD);
						FD_CLR(i, &master_set);
						if (i == max_sd)
						{
							while (FD_ISSET(max_sd, &master_set) == FALSE)
								max_sd -= 1;
						}
						continue;
					}
				}
				// else  error reported already, we check if continuable later
			}
		exceptCheck:
			// OOB data to receive
			if (FD_ISSET(i, &exceptFDs))
			{
				ULONG atMark;
				curReturn = ioctlsocket(i, SIOCATMARK, &atMark);
				if (curReturn < 0)
					CreateError(self->fusionSocketID, _T("Couldn't check if OOB data was present."));

				// If true, we're at mark; if not, got some more regular data to receive before OOB.
				// Worth noting recv() is guaranteed to not read past OOB in a recv(0) call...
				// unless MSG_OOBINLINE is used, which would prevent exceptFDs triggering anyway.
				if (atMark != 0)
				{
					curReturn = recv(i, recvbuf.get(), recvbufsize, MSG_OOB);
					if (!self->HandleSockOpReturn("recv for OOB", curReturn))
					{
						ReportInfo(self->fusionSocketID, _T("recv() for OOB (peer socket %d) failed with error %d, continuing thread"), i, curReturn);
						continue;
					}

					// TCP only supports one OOB byte at a time, weirdly.
					if (curReturn > 0)
					{
						++socketSource->numPacketsIn;
						socketSource->bytesIn += curReturn;
						++serverSocketSource->numPacketsIn;
						serverSocketSource->bytesIn += curReturn;
						EventToRun etr(self->fusionSocketID, socketSource, Conditions::OnClientReceivedPacket);
						etr.msg.assign(recvbuf.get(), curReturn);
						etr.msgIsOOB = true;
						this->AddEvent(std::move(etr));
						continue;
					}
				}
			}
		}

		// Sockets are okay, time to loop again
		if (curReturn >= 0)
			continue;

		// Error happened, and it was same as last time
		if (curReturn == lastErr)
		{
			ReportInfo(self->fusionSocketID, _T("Error %d was repeated from last loop, stopping thread."), lastErr);
			break;
		}
		else
		{
			lastErr = curReturn;
			continue;
		}

		static int conDedErrors[]{ ENOTCONN, 58 /* WSAESHUTDOWN */, ECONNABORTED, WSAECONNABORTED - WSABASEERR, WSAECONNRESET - WSABASEERR, ENETRESET };
		for (std::size_t i = 0; i < std::size(conDedErrors); ++i)
		{
			if (-lastErr == conDedErrors[i])
			{
				ReportInfo(self->fusionSocketID, _T("Client connection %i was forcibly closed. Thread exiting."), socketSource->peerSocketID);
				break;
			}
		}
		// continue;
	}


	ReportInfo(self->fusionSocketID, _T("* Main loop ended, beginning cleanup"));

	self->lock.edif_lock();
	if (!self->shutdownPending)
		self->Shutdown();

	AddEvent(EventToRun(self->fusionSocketID, self->sources[0], Conditions::OnServerStoppedHosting));

	self->lock.edif_unlock();
	while (true)
	{
		threadsafe.edif_lock();
		bool anyFound = std::any_of(eventsToRun.cbegin(), eventsToRun.cend(),
			[&](const std::unique_ptr<EventToRun>& e) { return e->fusionSocketID == self->fusionSocketID; });
		threadsafe.edif_unlock();
		if (!anyFound)
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	AddEvent(EventToRun(self->fusionSocketID, self->sources[0], Conditions::CleanupSocket));

	ReportInfo(self->fusionSocketID, _T("Client thread exit."));
}

void GlobalInfo::ClientThreadIRDA(std::shared_ptr<Thread> self, std::unique_ptr<StructPassThru> Parameters)
{
	// not implemented
	CreateError(self->fusionSocketID, _T("IRDA client/server not programmed."));
}
void GlobalInfo::ServerThreadIRDA(std::shared_ptr<Thread> self, std::unique_ptr<StructPassThru> Parameters)
{
	// not implemented
	CreateError(self->fusionSocketID, _T("IRDA client/server not programmed."));
}

void GlobalInfo::AddEvent(EventToRun && etr)
{
	threadsafe.edif_lock();
	if (!pendingDelete)
		eventsToRun.push_back(std::make_unique<EventToRun>(etr));
	threadsafe.edif_unlock();
}
