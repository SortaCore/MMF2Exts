#include "Common.h"

// ============================================================================
//
// CONDITIONS
// 
// ============================================================================


#define AlwaysTrue(name); bool Extension::name() {return true;}

// ID = 0
AlwaysTrue(OnError);

// ID = 1
AlwaysTrue(OnNewStatus);

// ID = 2
bool Extension::ClientReturnedMessage(int SocketID)
{
	ThreadSafe_Start();
	bool LRT = LastReturnType;
	int LRS = LastReturnSocketID;
	ThreadSafe_End();
	return (LRT == CLIENT_RETURN && (SocketID == -1 || SocketID == LRS));
}

// ID = 4
bool Extension::ClientSocketDisconnected(int SocketID)
{
	ThreadSafe_Start();
	int LRS = LastReturnSocketID;
	ThreadSafe_End();
	return (SocketID == -1 || SocketID == LRS);
}

// ID = 3
bool Extension::ServerReturnedMessage(int SocketID)
{
	ThreadSafe_Start();
	bool LRT = LastReturnType;
	int LRS = LastReturnSocketID;
	ThreadSafe_End();
	return (LRT == SERVER_RETURN && (SocketID == -1 || SocketID == LRS));
}

// ID = 4
bool Extension::ServerSocketDone(int SocketID)
{
	ThreadSafe_Start();
	bool LRT = LastReturnType;
	int LRS = LastReturnSocketID;
	ThreadSafe_End();
	return (LRT == SERVER_RETURN && (SocketID == -1 || SocketID == LRS));
}

// ID = 5
bool Extension::ServerPeerConnected(int SocketID)
{
	ThreadSafe_Start();
	int LRS = LastReturnSocketID;
	ThreadSafe_End();
	return (SocketID == -1 || SocketID == LRS);
}

// ID = 6
bool Extension::ServerPeerDisconnected(int SocketID)
{
	ThreadSafe_Start();
	int LRS = LastReturnSocketID;
	ThreadSafe_End();
	return (SocketID == -1 || SocketID == LRS);
}

// ID = 7
bool Extension::ClientSocketConnected(int SocketID)
{
	ThreadSafe_Start();
	int LRS = LastReturnSocketID;
	ThreadSafe_End();
	return (SocketID == -1 || SocketID == LRS);
}