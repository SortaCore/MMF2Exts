#include "Common.h"

// ============================================================================
//
// CONDITIONS
//
// ============================================================================


bool Extension::AlwaysTrue() const { return true; }
bool Extension::SocketIDCondition(int socketID) const {
	return (socketID == -1 || socketID == curEvent->fusionSocketID);
}
bool Extension::ServerPeerDisconnected(int SocketID)
{
	return (SocketID == -1 || SocketID == curEvent->fusionSocketID);
}
