// ============================================================================
//
// STRUCTS
//
// ============================================================================
#pragma once
#include "DarkEdif.hpp"
#include <map>
#include <WinSock2.h>
#include <in6addr.h>

// Conditions
enum class Conditions : int {
	// Run on end of socket thread, notifies main thread to destroy the thread object.
	// This allows the socket to keep its messages and disconnect event in queue.
	CleanupSocket = -2,

	Unset = -1,
	OnError,
	OnInfo,
	OnClientConnect,
	OnClientDisonnect,
	OnClientReceivedPacket,
	OnServerReceivedPacket,
	OnServerStoppedHosting,
	OnServerPeerConnected,
	OnServerPeerDisconnected,
	OnServerPeerAttemptedConnection,
	OnServerStartHosting,
};


// Pass
struct StructPassThru
{
	Extension * ext = nullptr;
	std::uint16_t port = 0;
	int protoType = 0;
	int addressFamily = 0;
	int socketType = 0;
	/* Client only */
	std::tstring client_hostname;
	std::tstring client_serviceName;
	/* Server only */
	struct in6_addr server_InAddr = {};
};

struct SocketSource;

struct EventToRun {
	// This specific event's error/info message.
	std::tstring info;
	int fusionSocketID = -1;
	Conditions eventID = Conditions::Unset;

	std::shared_ptr<SocketSource> source;

	// This specific event's received data. When the event runs, it is appended to sock->pendingData.
	std::string msg;

	// Is this specific event received data out of bound data?
	bool msgIsOOB = false;
	// Is this specific event's received data still intact within the source's pendingData?
	bool msgIsFullyInPendingData = true;

	EventToRun(int fusionSocketID, std::shared_ptr<SocketSource> source, Conditions cond);
};
enum class SocketType
{
	Client,
	Server,
	Either
};

class Extension;
struct Thread;
struct GlobalInfo
{
	Edif::recursive_mutex threadsafe;		// Handles whether GlobalInfo can be read from/written to.
	int newSocketID = 0;					// The new socket ID is assigned here. 0+, not reused.
	std::vector<Extension *> extsHoldingGlobals;
	std::vector<std::shared_ptr<Thread>> socketThreadList;
	std::vector<std::unique_ptr<EventToRun>> eventsToRun;

	void ClientThread(std::shared_ptr<Thread> self, std::unique_ptr<StructPassThru> params);
	void ServerThread(std::shared_ptr<Thread> self, std::unique_ptr<StructPassThru> params);
	void ClientThreadIRDA(std::shared_ptr<Thread> self, std::unique_ptr<StructPassThru> params);
	void ServerThreadIRDA(std::shared_ptr<Thread> self, std::unique_ptr<StructPassThru> params);
	void AddEvent(EventToRun && etr);

	// Returns win version as a _WINNT_WIN32 (Win10 on non-Windows)
	static int Internal_GetWinVer();

	// Returns pointer or null, reports error if not found
	std::shared_ptr<Thread> GetSocket(const char * func, SocketType socketType, int socketID);
	std::shared_ptr<SocketSource> GetSocketSource(const char* func, int socketID, int peerSocketID);
	static int AddressFamilyTextToNum(const std::string_view addrFamAsASCIIUpper);
	static int SocketTypeTextToNum(const std::string_view sockTypeAsASCIIUpper);
	static int ProtocolTypeTextToNum(const std::string_view protoTypeAsASCIIUpper);
	static bool InAddrTextToStruct(const std::string_view inaddrAsASCIIUpper, struct in6_addr *writeTo);

	void CreateError(int SocketID, PrintFHintInside const TCHAR * error, ...) PrintFHintAfter(3, 4);
	void ReportInfo(int SocketID, PrintFHintInside const TCHAR * report, ...) PrintFHintAfter(3, 4);

	// Thread handle; Thread checking whether a client extension has not regained control of connection in a reasonable time, i.e. slow frame transition
	std::thread timeoutThread;
	// Event; counterpart to AppWasClosed, but used for cancelling the timeout thread from Ext ctor when a new ext is taking over
	std::atomic<bool> cancelTimeoutThread;

	// If true, the timeout thread has set up a delete for this thread, and is waiting for the main thread to perform it.
	bool pendingDelete = false;
	// If false, when last ext removes itself from extsHolding, this GlobalInfo will be deleted.
	bool isGlobal = false;
	// If true, when output addresses, will unwrap IPv4-mapped-IPv6 to look like IPv4
	bool unwrapIPv6 = true;

	//void ReturnToFusion(int ReturnAsI, int SocketID, const void * Msg, int MsgLength);
	GlobalInfo(Extension * ext, const EDITDATA * const edPtr);
	~GlobalInfo();

private:
	static void CloseSocket(int& socketFD);
};

// Represents a source of messages - server for client socket, clients for server sockets
struct SocketSource
{
	Edif::recursive_mutex lock;

	// There is no pendingDataToWrite, as the sent messages are queued by OS anyway.
	std::string pendingDataToRead;

	size_t pendingDataToReadCursor = 0;

	size_t numPacketsIn = 0, numPacketsOut = 0;
	std::uint32_t bytesIn = 0, bytesOut = 0;

	// FD/SOCKET for an accepted socket of a client. Only used when the Thread is a server.
	int peerSocketFD = -1;
	// Socket ID (0+), not affected by closed sockets.
	int peerSocketID = -1;

	// Socket address for sending to/receiving from
	struct sockaddr_storage peerSockAddress = {};
	size_t peerSockAddressSize = sizeof(sockaddr_storage); // sockaddr_storage is IPv6 sized; so this may need changing.

	// user key-value data
	std::map<std::tstring, std::tstring> sourceData;

	std::shared_ptr<Thread> sock;

	FILE* fileWriteTo = NULL;
	std::tstring fileWriteToPath;
	void LinkFileOutput(const std::tstring_view newPathOrBlank);
	SocketSource(std::shared_ptr<Thread> thread, sockaddr_storage& dest, int sockAddrSize);
};
struct Thread
{
	Edif::recursive_mutex lock;
	std::vector<std::shared_ptr<SocketSource>> sources;

	// Fusion's socket ID (0+, not reused)
	int fusionSocketID = -1;

	// If true, this is a server Thread. Otherwise, client Thread.
	bool isServer = false;

	// Main socket FD/SOCKET (shared with sources[0] for client Threads)
	int mainSocketFD = -1;
	// Main socket address for sending to/receiving from
	struct sockaddr_storage mainSockAddress = {};
	int mainSockAddressSize = sizeof(mainSockAddress); // sockaddr_storage is IPv6 sized; so this may need changing.

	// Only write to this if lock is held! Indicates Fusion has changed something and thread needs to handle it.
	std::atomic<bool> changesPendingForThread;

	// Used to signal thread to close
	bool shutdownPending = false;
	// Used to signal thread to shut down sending side and receive only
	bool receiveOnly = false;
	// Used to indicate the server should accept incoming connections, rather than waiting for Fusion to OK it.
	bool autoAcceptOn = true;

	// Number of bytes read from start of thread's receive buffer.
	size_t trimIncoming = 0;

	// Owning thread
	std::thread thread;

	// Main data
	GlobalInfo * globals;

	// Shuts down, called by main thread or by a different one
	void Shutdown();
	// Handles the error code from a socket op like send(), returns true if ok.
	// Alters the return code out of the WSAEXXX errors into EXXX errors on Windows.
	bool HandleSockOpReturn(const char * func, int& sockOpRet, bool ifZero = false);

	Thread(GlobalInfo* gl);
	~Thread();
private:
	// Is shutdown already
	bool isShutdown = false;
};
