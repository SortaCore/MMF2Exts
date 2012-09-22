#include "Common.h"


///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(LPRDATA _rdPtr, LPEDATA edPtr, fpcob cobPtr)
    : rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.hoAdRunHeader), Runtime(_rdPtr)
{
    /*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
    */
	unsigned char id = 0;
#ifndef _DEBUG
#define _LinkAction(f) LinkAction(id++, f)
#define _LinkCondition(f) LinkCondition(id++, f)
#define _LinkExpression(f) LinkExpression(id++, f)
#else
	bool b = true;
#define _LinkAction(f) LinkAction(id, f); \
	if (SDK->ActionFunctions[id++] != Edif::MemberFunctionPointer(&Extension::f)) \
	{ \
		MsgBox("Critical error: Action ID failed to be linked!"); \
		b = false; \
	} 
#define _LinkCondition(f) LinkCondition(id, f); \
	if (SDK->ConditionFunctions[id++] != Edif::MemberFunctionPointer(&Extension::f)) \
	{ \
		MsgBox("Critical error: Condition ID failed to be linked!"); \
		b = false; \
	}
#define _LinkExpression(f) LinkExpression(id, f); \
	if (SDK->ExpressionFunctions[id++] != Edif::MemberFunctionPointer(&Extension::f)) \
	{ \
		MsgBox("Critical error: Expression ID failed to be linked!"); \
		b = false; \
	}
#endif
/// ACTIONS
	_LinkAction(TestReportAndExplode)
	_LinkAction(UsePopupMessages)
	// Client
	_LinkAction(ClientInitialise_Basic)
	_LinkAction(ClientInitialise_Advanced)
	_LinkAction(ClientShutdownSocket)
	_LinkAction(ClientSend)
	_LinkAction(ClientGoIndependent)
	_LinkAction(ClientReceiveOnly)
	_LinkAction(ClientLinkFileOutput)
	_LinkAction(ClientUnlinkFileOutput)
	_LinkAction(ClientMMF2Report)
	// Server
	_LinkAction(ServerInitialise_Basic)
	_LinkAction(ServerInitialise_Advanced)
	_LinkAction(ServerShutdownSocket)
	_LinkAction(ServerSend)
	_LinkAction(ServerGoIndependent)
	_LinkAction(ServerAutoAccept)
	_LinkAction(ServerLinkFileOutput)
	_LinkAction(ServerUnlinkFileOutput)
	_LinkAction(ServerMMF2Report)
	// Form packet
	_LinkAction(PacketForm_NewPacket);
	_LinkAction(PacketForm_ResizePacket);
	_LinkAction(PacketForm_SetByte);
	_LinkAction(PacketForm_SetShort);
	_LinkAction(PacketForm_SetInteger);
	_LinkAction(PacketForm_SetLong);
	_LinkAction(PacketForm_SetFloat);
	_LinkAction(PacketForm_SetDouble);
	_LinkAction(PacketForm_SetString);
	_LinkAction(PacketForm_SetWString);
	_LinkAction(PacketForm_SetBankFromBank);

#ifdef _DEBUG
	if (b)
		MsgBox("Actions linked up properly.");
	b = true;
#endif
	id = 0;
/// CONDITIONS
    _LinkCondition(OnError);
	_LinkCondition(OnNewStatus);

 	_LinkCondition(ClientSocketConnected);
	_LinkCondition(ClientSocketDisconnected);
	_LinkCondition(ClientReturnedMessage);
	
	_LinkCondition(ServerReturnedMessage);
	_LinkCondition(ServerSocketDone);
	_LinkCondition(ServerPeerConnected);
	_LinkCondition(ServerPeerDisconnected);

#ifdef _DEBUG
	if (b)
		MsgBox("Conditions linked up properly.");
	b = true;
#endif
	id = 0;
/// EXPRESSIONS
    _LinkExpression(GetErrors);
    _LinkExpression(GetReports);
	_LinkExpression(GetLastMessageSocketID);
	_LinkExpression(GetLastMessageText);
	_LinkExpression(GetLastMessageAddress);
	_LinkExpression(GetLastMessageSize);
	_LinkExpression(GetNewSocketID);
	_LinkExpression(GetSocketIDForLastEvent);
	_LinkExpression(GetPortFromType);

	_LinkExpression(PacketForm_GetAddress);
	_LinkExpression(PacketForm_GetSize);
	_LinkExpression(PacketForm_RunOnesComplement);

	_LinkExpression(LastMessage_GetByte);
	_LinkExpression(LastMessage_GetShort);
	_LinkExpression(LastMessage_GetInteger);
	_LinkExpression(LastMessage_GetLong);
	_LinkExpression(LastMessage_GetFloat);
	_LinkExpression(LastMessage_GetDouble);
	_LinkExpression(LastMessage_GetString);
	_LinkExpression(LastMessage_GetWString);

#ifdef _DEBUG
	if (b)
		MsgBox("Expressions linked up properly.");
	b = true;
#endif	
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
	else
	{
		if (wsaData.wVersion != MAKEWORD(2,2))
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
	if (!PacketFormLocation)
		free(PacketFormLocation);

	WSACleanup();
}


short Extension::Handle()
{
    /*
       If your extension will draw to the MMF window you should first 
       check if anything about its display has changed :

           if (rdPtr->roc.rcChanged)
              return REFLAG_DISPLAY;
           else
              return 0;

       You will also need to make sure you change this flag yourself 
       to 1 whenever you want to redraw your object
     
       If your extension won't draw to the window, but it still needs 
       to do something every MMF loop use :

            return 0;

       If you don't need to do something every loop, use :

            return REFLAG_ONESHOT;

       This doesn't mean this function can never run again. If you want MMF
       to handle your object again (causing this code to run) use this function:

            Runtime.Rehandle();

       At the end of the loop this code will run

    */

	// Will not be called next loop	
	return REFLAG_ONESHOT;
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

void Extension::Action(int ID, LPRDATA rdPtr, long param1, long param2)
{
	char t [50];
	sprintf_s(t, 50, "Action not set up: ID %i.", ID);
	MessageBoxA(NULL, t, "DarkSocket Object Error", MB_OK);
}

long Extension::Condition(int ID, LPRDATA rdPtr, long param1, long param2)
{
	char t [50];
	sprintf_s(t, 50, "Condition not set up: ID %i.", ID);
	MessageBoxA(NULL, t, "DarkSocket Object Error", MB_OK);
    return false;
}

long Extension::Expression(int ID, LPRDATA rdPtr, long param)
{
	char t [50];
	sprintf_s(t, 50, "Expression not set up: ID %i.", ID);
	MessageBoxA(NULL, t, "DarkSocket Object Error", MB_OK);
    return 0;
}

