// ============================================================================
//
// STRUCTS
//
// ============================================================================
#pragma once
#include "DarkEdif.h"

#define CLIENT_RETURN 0
#define SERVER_RETURN 1

enum class Commands : unsigned char {
	UNSET, // unusable
	SHUTDOWNTHREAD,
	RECEIVEONLY,
	SENDMSG,
	AUTOACCEPTSETON,
	AUTOACCEPTSETOFF,
	LINKOUTPUTTOFILE,
	UNLINKFILEOUTPUT,
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


// Pass
struct StructPassThru
{
	Extension * para_Ext = nullptr;
	unsigned short para_Port = 0;
	int para_ProtocolType = 0;
	int para_AddressFamily = 0;
	int para_SocketType = 0;
	/* Client only */
	TCHAR * para_client_hostname = nullptr;
	void * para_client_InitialSend = nullptr;
	/* Server only */
	unsigned long para_server_InAddr = 0;
};

// Struct for passing server/client->MMF2
struct CarryMsg
{
	int Socket = -1;
	bool ClientOrServer = false;
	void * Message = nullptr;
	int MessageSize = 0;
};

// Struct for passing MMF2->server/client
struct RevCarryMsg
{
	Commands Cmd = Commands::UNSET;
	int Socket = -1; // Ext socket ID, not a SOCKET
	std::string Message;
	std::tstring Client;

	RevCarryMsg(Commands Cmd, int SocketID) : Cmd(Cmd), Socket(SocketID) {}
	~RevCarryMsg() {
	}
};

struct ClientAccessNode
{
	SOCKET socket;
	SOCKADDR_STORAGE sockaddr;
	std::tstring FriendlyName;
};

void ClientThread(StructPassThru * Parameters);
void ClientThreadIRDA(StructPassThru * Parameters);
void ServerThread(StructPassThru *Parameters);
void ServerThreadIRDA(StructPassThru *Paramters);
