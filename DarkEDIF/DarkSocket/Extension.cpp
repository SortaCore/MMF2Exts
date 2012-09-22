#include "Common.h"


///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr)
    : rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr)
{
    /*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
	*/
/// ACTIONS
	LinkAction(0, TestReportAndExplode)
	LinkAction(1, UsePopupMessages)
	// Client
	LinkAction(2, ClientInitialise_Basic)
	LinkAction(3, ClientInitialise_Advanced)
	LinkAction(4, ClientShutdownSocket)
	LinkAction(5, ClientSend)
	LinkAction(6, ClientGoIndependent)
	LinkAction(7, ClientReceiveOnly)
	LinkAction(8, ClientLinkFileOutput)
	LinkAction(9, ClientUnlinkFileOutput)
	LinkAction(10, ClientMMF2Report)
	// Server
	LinkAction(11, ServerInitialise_Basic)
	LinkAction(12, ServerInitialise_Advanced)
	LinkAction(13, ServerShutdownSocket)
	LinkAction(14, ServerSend)
	LinkAction(15, ServerGoIndependent)
	LinkAction(16, ServerAutoAccept)
	LinkAction(17, ServerLinkFileOutput)
	LinkAction(18, ServerUnlinkFileOutput)
	LinkAction(19, ServerMMF2Report)
	// Form packet
	LinkAction(20, PacketForm_NewPacket);
	LinkAction(21, PacketForm_ResizePacket);
	LinkAction(22, PacketForm_SetByte);
	LinkAction(23, PacketForm_SetShort);
	LinkAction(24, PacketForm_SetInteger);
	LinkAction(25, PacketForm_SetLong);
	LinkAction(26, PacketForm_SetFloat);
	LinkAction(27, PacketForm_SetDouble);
	LinkAction(28, PacketForm_SetString);
	LinkAction(29, PacketForm_SetWString);
	LinkAction(30, PacketForm_SetBankFromBank);
	
/// CONDITIONS
	LinkCondition(0, OnError);
	LinkCondition(1, OnNewStatus);

 	LinkCondition(2, ClientSocketConnected);
	LinkCondition(3, ClientSocketDisconnected);
	LinkCondition(4, ClientReturnedMessage);
	
	LinkCondition(5, ServerReturnedMessage);
	LinkCondition(6, ServerSocketDone);
	LinkCondition(7, ServerPeerConnected);
	LinkCondition(8, ServerPeerDisconnected);
	
/// EXPRESSIONS
	LinkExpression(0, GetErrors);
	LinkExpression(1, GetReports);
	LinkExpression(2, GetLastMessageSocketID);
	LinkExpression(3, GetLastMessageText);
	LinkExpression(4, GetLastMessageAddress);
	LinkExpression(5, GetLastMessageSize);
	LinkExpression(6, GetNewSocketID);
	LinkExpression(7, GetSocketIDForLastEvent);
	LinkExpression(8, GetPortFromType);

	LinkExpression(9, PacketForm_GetAddress);
	LinkExpression(10, PacketForm_GetSize);
	LinkExpression(11, PacketForm_RunOnesComplement);

	LinkExpression(12, LastMessage_GetByte);
	LinkExpression(13, LastMessage_GetShort);
	LinkExpression(14, LastMessage_GetInteger);
	LinkExpression(15, LastMessage_GetLong);
	LinkExpression(16, LastMessage_GetFloat);
	LinkExpression(17, LastMessage_GetDouble);
	LinkExpression(18, LastMessage_GetString);
	LinkExpression(19, LastMessage_GetWString);

	/*
		This is where you'd do anything you'd do in CreateRunObject in the original SDK

		It's the only place you'll get access to edPtr at runtime, so you should transfer
		anything from edPtr to the extension class here.
	
	*/
	threadsafe = false;
	nogevent = false;
	LastReturnSocketID = -1;
	LastReturnType = false;
	NewSocketID = 0;
	UsePopups = false;
	PacketFormSize = 0;
	PacketFormLocation = NULL;
	LastError = _T("");
	CompleteStatus = _T("");
	LastLockFile = _T("");
	LastLockLine = -1;
	LastReturnMsg = _T("");

	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		MsgBox("WSAStartup() failed! There is something seriously off with your computer.");
		
	}
	else if (wsaData.wVersion != MAKEWORD(2,2))
	{
		MsgBox("WSAStartup() returned an older version of WinSock.\n"
				"DarkSocket does not support old versions directly;\n"
				"This means possible undefined behaviour, including crashes.");
	}

}

Extension::~Extension()
{
    /*
        This is where you'd do anything you'd do in DestroyRunObject in the original SDK.
        (except calling destructors and other such atrocities, because that's automatic in Edif)
    */

#if defined(AllowTermination) && defined(TerminateOnEnd)
	for (unsigned char i = 0; i < SocketThreadList.size(); i++)
	{
		if (SocketThreadList[i] != NULL)
		{
			// TerminateThread is dangerous and will not deallocate properly.
			if (TerminateThread(SocketThreadList[i], 0))
				MsgBox("Error: Thread could not be terminated!");
		}
	}
#endif
	if (PacketFormLocation)
		free(PacketFormLocation);

	WSACleanup();
}


short Extension::Handle()
{
	// Will not be called next loop	
	return REFLAG::ONE_SHOT;
}


short Extension::Display()
{
    /*
       If you return REFLAG_DISPLAY in Handle() this routine will run.
    */

    // Ok
    return 0;
}

short Extension::Pause()
{

    // Ok
    return 0;
}

short Extension::Continue()
{

    // Ok
    return 0;
}

bool Extension::Save(HANDLE File)
{
	bool OK = false;

    #ifndef VITALIZE

	    // Save the object's data here

	    OK = true;

    #endif

	return OK;
}

bool Extension::Load(HANDLE File)
{
	bool OK = false;

    #ifndef VITALIZE

	    // Load the object's data here

	    OK = true;

    #endif

	return OK;
}


// These are called if there's no function linked to an ID

void Extension::Action(int ID, RUNDATA * rdPtr, long param1, long param2)
{
	char t [50];
	sprintf_s(t, 50, "Action not set up: ID %i.", ID);
	MessageBoxA(NULL, t, "DarkSocket Object Error", MB_OK);
}

long Extension::Condition(int ID, RUNDATA * rdPtr, long param1, long param2)
{
	char t [50];
	sprintf_s(t, 50, "Condition not set up: ID %i.", ID);
	MessageBoxA(NULL, t, "DarkSocket Object Error", MB_OK);
    return false;
}

long Extension::Expression(int ID, RUNDATA * rdPtr, long param)
{
	char t [50];
	sprintf_s(t, 50, "Expression not set up: ID %i.", ID);
	MessageBoxA(NULL, t, "DarkSocket Object Error", MB_OK);
    return 0;
}

