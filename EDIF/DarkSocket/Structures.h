// ============================================================================
//
// STRUCTS
// 
// ============================================================================

// Pass
struct StructPassThru
{
	Extension * para_Ext;
	unsigned short para_Port;
	int para_ProtocolType;
	int para_AddressFamily;
	int para_SocketType;
	/* Client only */
	tchar * para_client_hostname;
	void * para_client_InitialSend;
	/* Server only */
	unsigned long para_server_InAddr;
};

// Struct for passing server/client -> MMF2
struct CarryMsg
{
	int Socket;
	bool ClientOrServer;
	void * Message;
	int MessageSize;
};

// Struct for passing MMF2 -> server/client
struct RevCarryMsg
{
	char Cmd;
	int Socket;
	void * Message;
	int MessageSize;
	tstring Client;
};

struct ClientAccessNode
{
	SOCKET socket;
	SOCKADDR_STORAGE sockaddr;
	tstring FriendlyName;
};

#define CLIENT_RETURN 0
#define SERVER_RETURN 1

enum Commands {
	SHUTDOWNTHREAD,
	GOINDEPENDENT,
	RECEIVEONLY,
	SENDMSG,
	AUTOACCEPTSETON,
	AUTOACCEPTSETOFF,
	LINKOUTPUTTOFILE,
	UNLINKFILEOUTPUT,
	MMFREPORTOFF,
	MMFREPORTON,
};

// Conditions
enum Conditions {
	MF2C_DEBUG_ON_ERROR,
	MF2C_DEBUG_ON_NEW_STATUS,
	MF2C_CLIENT_ON_CONNECT,
	MF2C_CLIENT_ON_DISCONNECT,
	MF2C_CLIENT_RECEIVED_MESSAGE,
	MF2C_SERVER_RECEIVED_MESSAGE,
	MF2C_SERVER_SOCKET_DONE,
	MF2C_SERVER_CLIENT_CONNECTED,
	MF2C_SERVER_CLIENT_DISCONNECTED,
	
};

DWORD WINAPI ClientThread(StructPassThru * Parameters);
DWORD WINAPI ClientThreadIRDA(StructPassThru * Parameters);
DWORD WINAPI ServerThread(StructPassThru *Parameters);
DWORD WINAPI ServerThreadIRDA(StructPassThru *Paramters);
