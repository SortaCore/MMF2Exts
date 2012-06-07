// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================
#include "common.h"
string EventLastError;
void ChangeLastError(string NewError,LPRDATA rdPtr)
{
	if (EventLastError.length()>0)
		EventLastError.append(" and "); //Join error strings
	else
		rdPtr->rRd->PushEvent(4); //Push event if not already called
	EventLastError.append(NewError);
}

// ============================================================================
//
// CONDITIONS
// 
// ============================================================================

CONDITION(
	/* ID */			0,
	/* Name */			"%o: Download in progress in slot %0",
	/* Flags */			EVFLAGS_ALWAYS+EVFLAGS_NOTABLE,
	/* Params */		(1,PARAM_NUMBER,"Enter the slot number (1-32):")
) {
	int n=Param(PARAM_NUMBER)-1; //n-1 because 0-based.
	if (n>=0&&n<MAX_SLOTS) {
		return rdPtr->slots[n].bPause||	//If Paused, No
		   rdPtr->slots[n].bCompleted;		//If Completed, No
		 //If paused or completed, download is not in progress.
		 }
	else
		return 0;
}

CONDITION(
	/* ID */			1,
	/* Name */			"%o: Download successfully completed in slot %0",
	/* Flags */			EVFLAGS_ALWAYS+EVFLAGS_NOTABLE,
	/* Params */		(1,PARAM_NUMBER,"Enter the slot number (1-32):")
) {
	int n=Param(PARAM_NUMBER)-1; //n-1 because 0-based.
	if (n>=0&&n<MAX_SLOTS)
		return rdPtr->slots[n].bCompleted;
	else 
	{
		ChangeLastError("Invalid port number for \"Download Complete\" condition",rdPtr);
		return 0;
	}
}

CONDITION(
	/* ID */			2,
	/* Name */			"%o: Connection error in slot %0",
	/* Flags */			EVFLAGS_ALWAYS+EVFLAGS_NOTABLE,
	/* Params */		(1,PARAM_NUMBER,"Enter the slot number (1-32):")
) {
	int n=Param(PARAM_NUMBER)-1; //n-1 because 0-based.
	if (n>=0&&n<MAX_SLOTS)
		return rdPtr->slots[n].bError; //If error
	else
		return 0;
}

CONDITION(
	/* ID */			3,
	/* Name */			"%o: Download aborted in slot %0",
	/* Flags */			EVFLAGS_ALWAYS+EVFLAGS_NOTABLE,
	/* Params */		(1,PARAM_NUMBER,"Enter the slot number (1-32):")
) {
	int n=Param(PARAM_NUMBER)-1; //n-1 because 0-based.
	if (n>=0&&n<MAX_SLOTS)
		return rdPtr->slots[n].bAborted; //If aborted...
	else
		return 0;
}



CONDITION(
	/* ID */			4,
	/* Name */			"%o: On error",
	/* Flags */			0,
	/* Params */		(0)
) {
	return true;
}
// ============================================================================
//
// ACTIONS
// 
// ============================================================================

#include <MMSystem.h>
ACTION(
	/* ID */			0,
	/* Name */			"Download file, URL=%0, destination=%1, slot %2",
	/* Flags */			0,
	/* Params */		(4,PARAM_STRING, "Enter the URL of the file. It must begin with HTTP:// or HTTPS://",
						   PARAM_FILENAME, "Destination filename:",
						   PARAM_NUMBER, "Slot number (1-32):",
						   PARAM_NUMBER, "Download initial position (0 for new download, -1 to automatically find out, else number of bytes to start from)")
) {
	string URL;
	URL=(char *)Param(PARAM_STRING);
	HANDLE DownloadTo=fopen((char *)Param(PARAM_STRING),"ab"); //a=append, b=binary
	int n=Param(PARAM_NUMBER)-1;
	if (n>=0&&n<MAX_SLOTS)
	{
		char Buffer[64];
		DWORD dwTaille=64;
		DWORD dwHeader=0;
		if (rdPtr->lpHttpQueryInfo(rdPtr->slots[n].hOpenedURL, HTTP_QUERY_CONTENT_LENGTH, Buffer, &dwTaille, &dwHeader)>0) //Check size of file
		{
			rdPtr->slots[n].FileSize=atoi(Buffer);
			rdPtr->slots[n].InitialTime=timeGetTime(); //timeGetSystemTime()?
		}
		else
			ChangeLastError("Bad URL, size <1",rdPtr);
	}
	else
		ChangeLastError("Bad slot!",rdPtr);
}

ACTION(
	/* ID */			1,
	/* Name */			"Abort download slot %0",
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER, "Slot number (1-32):")
) {
	int n=Param(PARAM_NUMBER)-1;
	if (n>=0&&n<MAX_SLOTS) {
		if (rdPtr->slots[n].bAborted==true)
			ChangeLastError("Already aborted.",rdPtr);
		else {
			if (rdPtr->slots[n].pURL.length()>0)
				rdPtr->slots[n].bAborted=true;}	}
	else
		ChangeLastError("Bad slot",rdPtr);
}

ACTION(
	/* ID */			2,
	/* Name */			"Pause download slot %0",
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER, "Slot number (1-32):")
) {
	int n=Param(PARAM_NUMBER)-1;
	if (n>=0&&n<MAX_SLOTS) {
		if (rdPtr->slots[n].bPause==true)
			ChangeLastError("Already paused.",rdPtr);
		else {
			if (rdPtr->slots[n].bAborted==true)
				ChangeLastError("Download cannot be paused, is currently aborted.",rdPtr);
			else
				rdPtr->slots[n].bPause=true;}	}
	else
		ChangeLastError("Bad slot",rdPtr);
}

ACTION(
	/* ID */			3,
	/* Name */			"Resume download slot %0",
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER, "Slot number (1-32):")
) {
	int n=Param(PARAM_NUMBER)-1;
	if (n>=0&&n<MAX_SLOTS) {
		if (rdPtr->slots[n].bPause==false)
			ChangeLastError("Not paused.",rdPtr);
		else {
			if (rdPtr->slots[n].bAborted==true)
				ChangeLastError("Download cannot be resumed, is currently aborted.",rdPtr);
			else
				rdPtr->slots[n].bPause=false;}	}
	else
		ChangeLastError("Bad slot",rdPtr);
}

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================


EXPRESSION(
	/* ID */			0,
	/* Name */			"InstantFileSize(",
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_STRING,"Enter file URL")
) {
	return 0;
}

EXPRESSION(
	/* ID */			1,
	/* Name */			"CurrentURL$(",
	/* Flags */			EXPFLAG_STRING,
	/* Params */		(1,EXPPARAM_LONG,"Enter the slot number (1-32)")
) {
	int n=Param(EXPPARAM_NUMBER)-1; //n-1 because 0-based.
	if (n>=0&&n<MAX_SLOTS) {
		if (rdPtr->slots[n].pURL.length()>0) {
			char temp[255];
			sprintf_s(temp,rdPtr->slots[n].pURL.length(),"%s",rdPtr->slots[n].pURL);
			ReturnString(*temp); }//If in range and there is a URL
		else {
			ChangeLastError("Unknown URL",rdPtr); //If in range but no URL
			ReturnString(""); }	}
	else {
		ChangeLastError("Invalid slot",rdPtr); //If out of range
		ReturnString(""); }
}


EXPRESSION(
	/* ID */			3,
	/* Name */			"CurrentPos(",
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_NUMBER,"Enter the slot number (1-32)")
) {
	int n=Param(EXPPARAM_NUMBER)-1; //n-1 because 0-based.
	if (n>=0&&n<MAX_SLOTS) {
		if (rdPtr->slots[n].TotalRead>0)
			return rdPtr->slots[n].TotalRead; //If in range and size saved
		else {
			ChangeLastError("Download failed or not initiated", rdPtr); //If in range but no size
			ReturnString(""); }	}
	else {
		ChangeLastError("Invalid slot",rdPtr); //If out of range
		ReturnString(""); }
}

EXPRESSION(
	/* ID */			4,
	/* Name */			"CurrentPercent(",
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_NUMBER,"Enter the slot number (1-32)")
) {
	int n=Param(EXPPARAM_NUMBER)-1; //n-1 because 0-based.
	if (n>=0&&n<MAX_SLOTS) {
		if (rdPtr->slots[n].TotalRead>0)
		{ReturnFloat(((rdPtr->slots[n].FileSize*1.0)/(rdPtr->slots[n].TotalRead*1.0))*100.0);}//If in range and size saved
		else {
			ChangeLastError("Download failed or not initiated",rdPtr); //If in range but no size
			ReturnString(""); }	}
	else {
		ChangeLastError("Invalid slot",rdPtr); //If out of range
		ReturnString(""); }
}

EXPRESSION(
	/* ID */			5,
	/* Name */			"CurrentSpeed(",
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_NUMBER,"Enter the slot number (1-32)")
) {
	int n=Param(EXPPARAM_NUMBER)-1; //n-1 because 0-based.
	if (n>=0&&n<MAX_SLOTS) {
		if (rdPtr->slots[n].TotalRead>0)
			return rdPtr->slots[n].Speed; //If in range and size saved
		else {
			ChangeLastError("Download failed, not initiated, or paused", rdPtr); //If in range but no size
			ReturnString(""); }	}
	else {
		ChangeLastError("Invalid slot",rdPtr); //If out of range
		ReturnString(""); }
}


EXPRESSION(
	/* ID */			6,
	/* Name */			"TotalTime(",
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_NUMBER,"Enter the slot number (1-32)")
) {
	int n=Param(EXPPARAM_NUMBER)-1; //n-1 because 0-based.
	if (n>=0&&n<MAX_SLOTS) {
		if (rdPtr->slots[n].TotalRead>0)
			{ReturnFloat(((rdPtr->slots[n].TotalRead-rdPtr->slots[n].FileSize)*1.0)/(rdPtr->slots[n].Speed*1.0));} //If in range and size saved
		else {
			ChangeLastError("Download failed, not initiated, or paused", rdPtr); //If in range but no size
			ReturnString(""); }	}
	else {
		ChangeLastError("Invalid slot",rdPtr); //If out of range
		ReturnString(""); }
}

EXPRESSION(
	/* ID */			7,
	/* Name */			"LastError$(",
	/* Flags */			EXPFLAG_STRING,
	/* Params */		(0)
) {
	char temp[255];
	sprintf_s(temp,EventLastError.length(),"%s.",EventLastError);
	ReturnString(*temp);
	EventLastError=""; //Resets error string
}
