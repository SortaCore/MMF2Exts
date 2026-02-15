#include "Common.hpp"

EventToRun::EventToRun(int fusionSocketID, std::shared_ptr<SocketSource> source, Conditions cond) :
	fusionSocketID(fusionSocketID), source(source), eventID(cond)
{
	// Everything is inited in declaration
}
void Thread::Shutdown()
{
	lock.edif_lock();
	isShutdown = true;
	auto closeSocket = [](int fd) {
#ifdef _WIN32
		closesocket(fd);
#else
		close(fd);
#endif
	};

	if (isServer)
	{
		for each (auto & f in sources)
			closeSocket(f->peerSocketFD);
	}

	closeSocket(mainSocketFD);
	lock.edif_unlock();

	// Wait for thread to close itself
	if (thread.joinable() && thread.get_id() != std::this_thread::get_id())
		thread.join();
}

// Handles the error code from a socket op like send(), returns true if ok
bool Thread::HandleSockOpReturn(const char * func, int& sockOpRet, bool ifZero)
{
	// Data was received/sent OK
	if (sockOpRet > 0)
		return true;

	// Socket closed gracefully
	if (sockOpRet == 0)
		return ifZero;

	// Socket had error
	// if (sockOpRet < 0)
	TCHAR errMsg[512];
#if _WIN32
	// sockOpRet will be SOCKET_ERROR, the actual error code is here
	int err = WSAGetLastError();

	DWORD errMsgWrittenSize = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errMsg, std::size(errMsg), 0);
	if (errMsgWrittenSize == 0)
		_stprintf_s(errMsg, _T("(couldn't convert number to text, format error %u)"), GetLastError());
	else // Go backwards to erase the ending ".\r\n"
		errMsg[errMsgWrittenSize - 3] = _T('\0');
	// WSAEFAULT -> EFAULT
	sockOpRet = -(err - WSABASEERR);
#else
	int err = errno;
	const char * errMsg2 = strerror(err);
	if (errMsg2 == NULL)
		sprintf(errMsg, "(couldn't convert number to text, format error %d)", errno);
	else
		strcpy(errMsg, errMsg2);
	sockOpRet = err;
#endif

	globals->CreateError(fusionSocketID, _T("Error running %s; got error %d: %s."), DarkEdif::UTF8ToTString(func).c_str(), err, errMsg);
	return false;
}
SocketSource::SocketSource(std::shared_ptr<Thread> thread, sockaddr_storage& dest, int sockAddrSize)
	: sock(thread), peerSockAddress(dest), peerSockAddressSize(sockAddrSize)
{
}
void SocketSource::LinkFileOutput(const std::tstring_view newPathOrBlank)
{
	lock.edif_lock();
	if (fileWriteToPath != newPathOrBlank)
	{
		fileWriteTo = NULL;
		fileWriteToPath = newPathOrBlank;
		if (!fileWriteToPath.empty() && (fileWriteTo = _tfopen(fileWriteToPath.c_str(), _T("ab"))) == NULL)
		{
			sock->globals->CreateError(sock->fusionSocketID, _T("Failed to open new output file \"%s\", error %d: %s."), fileWriteToPath.c_str(), errno, _tcserror(errno));
			fileWriteToPath.clear();
		}
	}

	if (fileWriteTo != NULL && fclose(fileWriteTo) != 0)
	{
		sock->globals->CreateError(sock->fusionSocketID, _T("Failed to close old file \"%s\", error %d: %s. The ending section of the file may have been discarded."),
			fileWriteToPath.c_str(), errno, _tcserror(errno));
	}
	lock.edif_unlock();
}
Thread::Thread(GlobalInfo * gl) : globals(gl) {
	// Get the current socket ID
	globals->threadsafe.edif_lock();
	fusionSocketID = globals->newSocketID++;
	globals->threadsafe.edif_unlock();
}
Thread::~Thread()
{
	if (thread.joinable())
		thread.join();
		// DarkEdif::MsgBox::Error(_T("Socket thread alive"), _T("Killing socket thread %d, when thread for socket is still alive."));
}

GlobalInfo::GlobalInfo(Extension * firstExt, const EDITDATA * const edPtr)
{
	isGlobal = firstExt->isGlobal;
	extsHoldingGlobals.push_back(firstExt);

#if _WIN32
	WSADATA data;
	int err = WSAStartup(MAKEWORD(2, 2), &data);
	if (err != 0)
		DarkEdif::MsgBox::Error(_T("WSAStartup failed"), _T("WSAStartup() failed, error %d."), err);
#endif
}
GlobalInfo::~GlobalInfo()
{
	threadsafe.edif_lock();
	if (!extsHoldingGlobals.empty())
		DarkEdif::MsgBox::Error(_T("Fatal error"), _T("This GlobalInfo being deleted is still held by %zu extensions!"), extsHoldingGlobals.size());

	// Threads won't exit if their message pointers are still within eventsToRun, so clear it first
	eventsToRun.clear();

	// If non-global object, this won't be true; if it's true, it prevents new events being added
	pendingDelete = true;

	for (auto s : socketThreadList)
	{
		if (!s->shutdownPending)
		{
			s->lock.edif_lock();
			s->shutdownPending = true;
			s->changesPendingForThread = true;
			s->lock.edif_unlock();
		}
	}
	threadsafe.edif_unlock();
	for (auto s : socketThreadList)
	{
		if (s->thread.joinable())
			s->thread.join();
	}

#if _WIN32
	int err = WSACleanup();
	if (err != 0)
		DarkEdif::MsgBox::Error(_T("WSACleanup failed"), _T("WSACleanup() failed, error %d."), err);
#endif
}
void GlobalInfo::CloseSocket(int& socketFD)
{
	closesocket(socketFD);
	socketFD = -1;
}
