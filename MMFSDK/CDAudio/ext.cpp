//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		Multimedia Fusion: cd-audio object																						//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TGFEXT

#define MinimumBuild 251
#if defined(MMFEXT)
#define	IS_COMPATIBLE(v) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFBUILD_MASK) >= MinimumBuild && (v->mvGetVersion() & MMFVERSION_MASK) >= MMFVERSION_20 && ((v->mvGetVersion() & MMFVERFLAG_MASK) & MMFVERFLAG_HOME) == 0)
#elif defined(PROEXT)
#define IS_COMPATIBLE(v) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFBUILD_MASK) >= MinimumBuild && (v->mvGetVersion() & MMFVERSION_MASK) >= MMFVERSION_20 && ((v->mvGetVersion() & MMFVERFLAG_MASK) & MMFVERFLAG_PRO) != 0)
#else
#define	IS_COMPATIBLE(v) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFBUILD_MASK) >= MinimumBuild && (v->mvGetVersion() & MMFVERSION_MASK) >= MMFVERSION_20)
#endif

// General CCX includes
#include	"ccxhdr.h"
#include	"ccx.h"

// Specific to this kcx
#include	<assert.h>
#include	"ext.h"
#include	"props.h"
#include	"extrc.h"

//#include "..\..\hlp\cnc2hlpobj.hm"


// Instance of kcx
HINSTANCE	hInstLib;

// Object identifier "CDA0"
#define IDENTIFIER	0x43444130

// Data for cd audio objects
char	achCDAudio []="CDAudio";


// Function prototypes
short WINAPI DLLExport Pause(fprdata rdPtr, long param1, long param2);
short WINAPI DLLExport Resume(fprdata rdPtr, long param1, long param2);
long  WINAPI DLLExport GetPosition(fprdata rdPtr, long param1);
short WINAPI DLLExport TimePlay(fprdata rdPtr, long param1, long param2);
short WINAPI DLLExport Stop(fprdata rdPtr, long param1, long param2);



// Definitions of parameters for each condition
infosEvents	conditionsInfos[]=
		{
		{CND_CMPOSINTRACK, 	EVFLAGS_ALWAYS+EVFLAGS_NOTABLE, PARAM_CMPTIME, 		0,  	0, 0},
		{CND_CMPOSITION,	EVFLAGS_ALWAYS+EVFLAGS_NOTABLE, PARAM_CMPTIME,		0,      0, 0}
		};

// Definitions of parameters for each action
infosEvents	actionsInfos[]=
		{
		{ACT_PLAY,     		0,				PARAM_EXPRESSION,   0,		0, 0},
		{ACT_STOP,          0,              0,					0,		0, 0},
		{ACT_PAUSE,         0,              0,					0,		0, 0},
		{ACT_RESUME,        0,				0,					0,		0, 0},
		{ACT_PREVIOUS,      0,              0,					0,		0, 0},
		{ACT_NEXT,          0,              0,					0,		0, 0},
		{ACT_TIMEPLAY,      0,				PARAM_TIME,			0,		0, 0},
		{ACT_OPENDOOR,      0,				0,					0,		0, 0},
		{ACT_CLOSEDOOR,     0,              0,					0,		0, 0},
		{ACT_PLAYTRACK, 	0,				PARAM_EXPRESSION,   0,		0, 0},
		};

// Definitions of parameters for each expression
infosEvents	expressionsInfos[]=
		{
		{EXP_TRACK,			0,              0,					0,		0, 0},
		{EXP_MAXTRACK,		0,              0,					0,		0, 0},
		{EXP_TRACKPOS,		0,              0,					0,		0, 0},
		{EXP_TRACKLEN,		0,              0,					0,		0, 0},
		{EXP_POS,			0,              0,					0,		0, 0},
		{EXP_LEN,			0,              0,					0,		0, 0},
		{EXP_LASTERROR,		0,              0,					0,		0, 0}
		};

#ifndef RUN_ONLY

// Properties /////////////////////////////////////////////////////////////////
enum {
	PROPID_SETTINGS = PROPID_EXTITEM_CUSTOM_FIRST,
	PROPID_OPENDRIVER,
	PROPID_PLAYATSTART,
	PROPID_STOPATEND,
	PROPID_PLAYMINIMIZED,
};

PropData Properties[] = {

	PropData_CheckBox	(PROPID_OPENDRIVER,		IDS_PROP_OPENDRIVER,IDS_PROP_OPENDRIVER_INFO),
	PropData_EditNumber_Check(PROPID_PLAYATSTART,IDS_PROP_PLAYATSTART, IDS_PROP_PLAYATSTART_INFO),
	PropData_CheckBox	(PROPID_STOPATEND,		IDS_PROP_STOPATEND,	IDS_PROP_STOPATEND_INFO),
	PropData_CheckBox	(PROPID_PLAYMINIMIZED,	IDS_PROP_PLAYMINIMIZED,	IDS_PROP_PLAYMINIMIZED_INFO),

	PropData_End()
};

// Debugger
enum
{
	DB_CURRENTTRACK,
	DB_NUMBEROFTRACKS,
	DB_POSITIONINTRACK,
	DB_TRACKLENGTH,
	DB_LENGTH,
	DB_LASTERROR,
};
WORD DebugTree[]=
{
	DB_CURRENTTRACK,
	DB_NUMBEROFTRACKS,
	DB_POSITIONINTRACK,
	DB_TRACKLENGTH,
	DB_LENGTH,
	DB_LASTERROR,
	DB_END
};

#endif	// RUN_ONLY







//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		OBJECT HANDLING ROUTINES WHEN GAME IS RUNNED																		//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		GetRunObjectDataSize																	//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

ushort WINAPI DLLExport GetRunObjectDataSize(fprh rhPtr, fpedata edPtr)
{
	// Debug assertion
    assert(rhPtr != NULL);
    assert(edPtr != NULL);

	// Return the size of the run data structure
	return (sizeof(runData));
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		CreateRunObject									    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport CreateRunObject(fprdata rdPtr, fpedata edPtr, fpcob cobPtr)
{            

	MCI_OPEN_PARMS		MCIOpenParms;
    MCI_STATUS_PARMS	MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);
    assert(edPtr != NULL);
    assert(cobPtr != NULL);

	// Get flags
	rdPtr->rcdFlags = edPtr->ecdaFlags;

	// Open cd
    MCIOpenParms.lpstrDeviceType = achCDAudio;
    rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms);

	// Store device number if open on start
	if ((rdPtr->rcdFlags & CD_OPENONSTART) != 0)
	{
		if (rdPtr->rcdError != 0)
			return 0;
		rdPtr->rcdDevice = MCIOpenParms.wDeviceID;
	}

	// Process play on start flag
	MCIStatusParms.dwItem = MCI_STATUS_MODE;
	rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms);
	if ((rdPtr->rcdFlags & CD_PLAYONSTART) != 0)
	{
		if ((MCI_MODE_STOP == MCIStatusParms.dwReturn) || (MCI_MODE_PAUSE == MCIStatusParms.dwReturn))
		{
    		MCI_PLAY_PARMS		MCIPlayParms;
			MCI_SET_PARMS		MCISetParms;

			// Set time format
			MCISetParms.dwTimeFormat = MCI_FORMAT_TMSF;
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms);

			// To be compatible
			if (0 == edPtr->ecdaTrackOnStart)
				edPtr->ecdaTrackOnStart = 1;

			// Play start track
			MCIPlayParms.dwFrom = edPtr->ecdaTrackOnStart;
			rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, MCI_FROM, (DWORD)(LPVOID)&MCIPlayParms);
		}
	}
	else
	{
		if ((MCI_MODE_PLAY == MCIStatusParms.dwReturn) || (MCI_MODE_SEEK == MCIStatusParms.dwReturn) || (MCI_MODE_PAUSE == MCIStatusParms.dwReturn))
			rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STOP, 0, 0);
	}

	rdPtr->rcdStopAtTrackEnd = FALSE;

	// Close if not open on start
	if (0 == (rdPtr->rcdFlags & CD_OPENONSTART))
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		DestroyRunObject								    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport DestroyRunObject(fprdata rdPtr, long fast)
{

	MCI_OPEN_PARMS				MCIOpenParms;
    MCI_STATUS_PARMS          	MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);

	// Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms);
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Process stop on close flag
	MCIStatusParms.dwItem = MCI_STATUS_MODE;
	rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms);
	if ((rdPtr->rcdFlags & CD_STOPONCLOSE) != 0)
	{
		if ((MCI_MODE_PLAY == MCIStatusParms.dwReturn) || (MCI_MODE_SEEK == MCIStatusParms.dwReturn) || (MCI_MODE_PAUSE == MCIStatusParms.dwReturn))
			rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STOP, 0, 0);
	}

	// Close cd
    if (0 == (rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, 0)))
    {
    	rdPtr->rcdDevice = 0;
    	return 0;
    }
    else
    	return -1;
}










//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		HandleRunObject									    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport HandleRunObject(fprdata rdPtr)
{      
	// Debug assertion
    assert(rdPtr != NULL);
    
	return REFLAG_ONESHOT;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		DisplayRunObject								    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport DisplayRunObject(fprdata rdPtr)
{
	// Debug assertion
    assert(rdPtr != NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		ReInitRunObject									   				 						//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport ReInitRunObject(fprdata rdPtr)
{
	// Debug assertion
    assert(rdPtr != NULL);

	return 0;
}
                   





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		PauseRunObject									  				  						//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport PauseRunObject(fprdata rdPtr)
{
    MCI_STATUS_PARMS	MCIStatusParms;
	MCI_OPEN_PARMS		MCIOpenParms;

	fprh rhPtr = rdPtr->rHo.hoAdRunHeader;

	if (IsIconic(rhPtr->rhHMainWin))
	{
		if (rdPtr->rcdFlags & CD_PLAYWHENMINIMIZED)
			return(0);
	} 
	
	// Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if (mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms))
	    {
    		rdPtr->rcdLastPosition = 0;
    		rdPtr->rcdStatus = MCI_MODE_STOP;
	    	return -1;
	    }
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Get status
	do
	{
		MCIStatusParms.dwItem = MCI_STATUS_MODE;
		mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms);
	}
	while (MCI_MODE_SEEK == MCIStatusParms.dwReturn);
    rdPtr->rcdStatus = (short)MCIStatusParms.dwReturn;
	if (rdPtr->rcdStatus!=MCI_MODE_PAUSE)
		Pause(rdPtr,0,0);
	// ok
	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		ContinueRunObject											    						//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport ContinueRunObject(fprdata rdPtr)
{
	MCI_OPEN_PARMS				MCIOpenParms;
	MCI_STATUS_PARMS			MCIStatusParms;
	
	// Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if (mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms))
	    {
    		rdPtr->rcdLastPosition = 0;
    		rdPtr->rcdStatus = MCI_MODE_STOP;
	    	return -1;
	    }
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	if (rdPtr->rcdStatus!=MCI_MODE_PAUSE)
		Resume(rdPtr,0,0);

	// Get status
	do
	{
		MCIStatusParms.dwItem = MCI_STATUS_MODE;
		mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms);
	}
	while (MCI_MODE_SEEK == MCIStatusParms.dwReturn);
    rdPtr->rcdStatus = (short)MCIStatusParms.dwReturn;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		SaveRunObject	     										    						//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tabSaveCD
{
	short			version;
	short			rcdFlags;
	short			rcdStopAtTrackEnd;
	short			mode;
	DWORD			position;
}SaveCD;
#define SAVEVERSION_CURRENT 1

BOOL WINAPI DLLExport SaveRunObject(fprdata rdPtr, HANDLE hf)
{
	BOOL bOK=FALSE;

#ifndef VITALIZE

	SaveCD save;
	DWORD written;
	while (TRUE)
	{
		save.version=SAVEVERSION_CURRENT;
		save.rcdFlags=rdPtr->rcdFlags;
		save.rcdStopAtTrackEnd=rdPtr->rcdStopAtTrackEnd;

		MCI_OPEN_PARMS				MCIOpenParms;
	    MCI_STATUS_PARMS          	MCIStatusParms;

		// Open cd
		if (0 == rdPtr->rcdDevice)
		{
			MCIOpenParms.lpstrDeviceType = achCDAudio;
			rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms);
		}
		else
		{
			MCIOpenParms.wDeviceID = rdPtr->rcdDevice;
		}
		
		// Current play flag
		MCIStatusParms.dwItem = MCI_STATUS_MODE;
		mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms);
		save.mode=(short)MCIStatusParms.dwReturn;
		
		// Close cd
		if (0 == rdPtr->rcdDevice)
			rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

		save.position=GetPosition(rdPtr, 0);
		if (WriteFile(hf, &save, sizeof(SaveCD), &written, NULL)==0)
			break;
		
		bOK=TRUE;
		break;
	}

#endif // VITALIZE

	return bOK;
}
BOOL WINAPI DLLExport LoadRunObject(fprdata rdPtr, HANDLE hf)
{
	SaveCD save;
	BOOL bOK=FALSE;
	DWORD read;
	while (TRUE)
	{
		if (ReadFile(hf, &save, sizeof(SaveCD), &read, NULL)==0 || read<sizeof(SaveCD))
			break;

		if (save.version!=SAVEVERSION_CURRENT)
			break;
		rdPtr->rcdFlags=save.rcdFlags;
		rdPtr->rcdStopAtTrackEnd=save.rcdStopAtTrackEnd;

		TimePlay(rdPtr, save.position, 0);
		switch(save.mode)
		{
		case MCI_MODE_PAUSE:
			Pause(rdPtr, 0, 0);
			break;
		case MCI_MODE_STOP:
			Stop(rdPtr, 0, 0);
			break;
		default:
			break;
		}
		bOK=TRUE;
		break;
	}
	return bOK;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		CONDITION ROUTINES																									//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		CmpPosition										    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long WINAPI DLLExport CmpPosition(fprdata rdPtr, long param1, long param2)
{

	MCI_OPEN_PARMS				MCIOpenParms;
	MCI_SET_PARMS 				MCISetParms;
    MCI_STATUS_PARMS          	MCIStatusParms;
    
	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// set time format
	MCISetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	
	// Get postion
	MCIStatusParms.dwItem = MCI_STATUS_POSITION;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	// Return position
	return MCIStatusParms.dwReturn;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		CmpPositionInTrack								    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long WINAPI DLLExport CmpPositionInTrack(fprdata rdPtr, long param1, long param2)
{
	MCI_OPEN_PARMS		MCIOpenParms;
	MCI_SET_PARMS		MCISetParms;
    MCI_STATUS_PARMS	MCIStatusParms;
    ulong				TrackPosition;
    ulong				Position;
    
	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// get current track number
	MCIStatusParms.dwItem = MCI_STATUS_CURRENT_TRACK;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Set time format
	MCISetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Get current track position
	MCIStatusParms.dwTrack = MCIStatusParms.dwReturn;
	MCIStatusParms.dwItem = MCI_STATUS_POSITION;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	TrackPosition = MCIStatusParms.dwReturn;

	// Get position in current track
	MCIStatusParms.dwItem = MCI_STATUS_POSITION;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms. wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	Position = MCIStatusParms.dwReturn - TrackPosition;
		
	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	// Return position in current track
	return Position;
}










//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		ACTION ROUTINES																										//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Play										    										//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport Play(fprdata rdPtr, long param1, long param2)
{

	MCI_OPEN_PARMS			MCIOpenParms;
	MCI_SET_PARMS			MCISetParms;
    MCI_PLAY_PARMS			MCIPlayParms;
    MCI_STATUS_PARMS		MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);

	if (param1 < 1)
		param1 = 1;

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Set time format
	MCISetParms.dwTimeFormat = MCI_FORMAT_TMSF;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Test track number validity
	MCIStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	if (param1 < 1)
		param1 = 1;
	if ((unsigned long)param1 > MCIStatusParms.dwReturn)
		param1 = MCIStatusParms.dwReturn;

	// Play track
	MCIPlayParms.dwFrom = param1;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, MCI_FROM, (DWORD)(LPVOID)&MCIPlayParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	rdPtr->rcdStopAtTrackEnd = FALSE;

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		PlayTrack										    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport PlayTrack(fprdata rdPtr, long param1, long param2)
{
	MCI_OPEN_PARMS			MCIOpenParms;
	MCI_SET_PARMS			MCISetParms;
    MCI_PLAY_PARMS			MCIPlayParms;
    MCI_STATUS_PARMS		MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);

	if (param1 < 1)
		param1 = 1;

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Set time format
	MCISetParms.dwTimeFormat = MCI_FORMAT_TMSF;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Test track number validity
	MCIStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	if (param1 < 1)
		param1 = 1;
	if ((unsigned long)param1 > MCIStatusParms.dwReturn)
		param1 = MCIStatusParms.dwReturn;

	// Play track
	MCIPlayParms.dwFrom = param1;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, MCI_FROM, (DWORD)(LPVOID)&MCIPlayParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	MCIPlayParms.dwTo = param1 + 1;
	rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, MCI_TO, (DWORD)(LPVOID)&MCIPlayParms);

	rdPtr->rcdStopAtTrackEnd = TRUE;

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		TimePlay										    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport TimePlay(fprdata rdPtr, long param1, long param2)
{
	MCI_OPEN_PARMS				MCIOpenParms;
	MCI_SET_PARMS 				MCISetParms;
    MCI_PLAY_PARMS 				MCIPlayParms;
    MCI_STATUS_PARMS          	MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);

	if (param1 < 0)
		param1 = 0;

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Set time format
	MCISetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Test track number validity
	MCIStatusParms.dwItem = MCI_STATUS_LENGTH;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	if (param1 < 1)
		param1 = 1;
	if ((unsigned long)param1 > MCIStatusParms.dwReturn)
		param1 = MCIStatusParms.dwReturn;

	// Play track
	MCIPlayParms.dwFrom = param1;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, MCI_FROM, (DWORD)(LPVOID)&MCIPlayParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	rdPtr->rcdStopAtTrackEnd = FALSE;

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Pause											    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport Pause(fprdata rdPtr, long param1, long param2)
{
	MCI_OPEN_PARMS				MCIOpenParms;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Pause
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PAUSE, 0, (DWORD)(LPVOID)NULL)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Resume											    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport Resume(fprdata rdPtr, long param1, long param2)
{
	MCI_OPEN_PARMS			MCIOpenParms;
	MCI_SET_PARMS			MCISetParms;
	MCI_STATUS_PARMS		MCIStatusParms;
	MCI_PLAY_PARMS			MCIPlayParms;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Resume if supported
	if (MCIERR_UNSUPPORTED_FUNCTION == (rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_RESUME, 0, (DWORD)(LPVOID)NULL)))
	{
		// Else, play
		if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, 0, (DWORD)(LPVOID)NULL)) != 0)
		{
			if (0 == rdPtr->rcdDevice)
				mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
			return 0;
		}
		if (TRUE == rdPtr->rcdStopAtTrackEnd)
		{
			// Set time format
			MCISetParms.dwTimeFormat = MCI_FORMAT_TMSF;
			if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
			{
				if (0 == rdPtr->rcdDevice)
					mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
				return 0;
			}
	
			// Get current track number
			MCIStatusParms.dwItem = MCI_STATUS_CURRENT_TRACK;
			if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
			{
				if (0 == rdPtr->rcdDevice)
					mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
				return 0;
			}
	
			// To next track
			MCIPlayParms.dwTo = MCIStatusParms.dwReturn + 1;
			if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, MCI_TO, (DWORD)(LPVOID)&MCIPlayParms)) != 0)
			{
				if (0 == rdPtr->rcdDevice)
					mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
				return 0;
			}
		}
		
	}
	else
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Stop											    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport Stop(fprdata rdPtr, long param1, long param2)
{
	MCI_OPEN_PARMS		MCIOpenParms;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Stop
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STOP, 0, (DWORD)(LPVOID)NULL)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Close cd 
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Previous										    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport Previous(fprdata rdPtr, long param1, long param2)
{
	MCI_OPEN_PARMS			MCIOpenParms;
	MCI_SET_PARMS			MCISetParms;
    MCI_PLAY_PARMS			MCIPlayParms;
    MCI_STATUS_PARMS		MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Get current track
	MCIStatusParms.dwItem = MCI_STATUS_CURRENT_TRACK;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// If not first track		
	if (MCIStatusParms.dwReturn > 1)
	{
		// Set time format
		MCISetParms.dwTimeFormat = MCI_FORMAT_TMSF;
		if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
		{
			if (0 == rdPtr->rcdDevice)
				mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
			return 0;
		}

		// Play previous track
		MCIPlayParms.dwFrom = MCIStatusParms.dwReturn - 1;
		if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, MCI_FROM, (DWORD)(LPVOID)&MCIPlayParms)) != 0)
		{
			if (0 == rdPtr->rcdDevice)
				mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
			return 0;
		}
		if (TRUE == rdPtr->rcdStopAtTrackEnd)
		{
			MCIPlayParms.dwTo = MCIStatusParms.dwReturn;
			rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, MCI_TO, (DWORD)(LPVOID)&MCIPlayParms);
		}
	}
		
	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Next											    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport Next(fprdata rdPtr, long param1, long param2)
{
	MCI_OPEN_PARMS				MCIOpenParms;
	MCI_SET_PARMS 				MCISetParms;
    MCI_PLAY_PARMS 				MCIPlayParms;
    MCI_STATUS_PARMS          	MCIStatusParms;
	DWORD 					   	Nombre_Pistes;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Get number of track
	MCIStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	Nombre_Pistes = MCIStatusParms.dwReturn;
		
	// Get current track number
	MCIStatusParms.dwItem = MCI_STATUS_CURRENT_TRACK;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
		
	// If not last track
	if (MCIStatusParms.dwReturn < Nombre_Pistes)
	{
		// Set time format
		MCISetParms.dwTimeFormat = MCI_FORMAT_TMSF;
		if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
		{
			if (0 == rdPtr->rcdDevice)
				mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
			return 0;
		}

		// Play next track
		MCIPlayParms.dwFrom = MCIStatusParms.dwReturn + 1;
		if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, MCI_FROM, (DWORD)(LPVOID)&MCIPlayParms)) != 0)
		{
			if (0 == rdPtr->rcdDevice)
				mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
			return 0;
		}
		if (TRUE == rdPtr->rcdStopAtTrackEnd)
		{
			MCIPlayParms.dwTo = MCIStatusParms.dwReturn+2;
			rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_PLAY, MCI_TO, (DWORD)(LPVOID)&MCIPlayParms);
		}
	}
		
	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		OpenDoor										    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport OpenDoor(fprdata rdPtr, long param1, long param2)
{
	MCI_OPEN_PARMS				MCIOpenParms;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Open door (Sesame, ouvre toi)
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, (WORD)(LPVOID)NULL)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		CloseDoor										    									//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport CloseDoor(fprdata rdPtr, long param1, long param2)
{
	MCI_OPEN_PARMS				MCIOpenParms;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Close door (Sesame, ferme toi)
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_DOOR_CLOSED, (WORD)(LPVOID)NULL)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	return 0;
}










//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		EXPRESSION ROUTINES																									//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		GetNumberOfTracks					 													//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long WINAPI DLLExport GetNumberOfTracks(fprdata rdPtr, long param1)
{
	MCI_OPEN_PARMS				MCIOpenParms;
    MCI_STATUS_PARMS          	MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Get number of track
	MCIStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	// Return number of track
	return MCIStatusParms.dwReturn;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		GetCurrentTrack						 													//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long WINAPI DLLExport GetCurrentTrack(fprdata rdPtr, long param1)
{
	MCI_OPEN_PARMS				MCIOpenParms;
    MCI_STATUS_PARMS          	MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Get current track number
	MCIStatusParms.dwItem = MCI_STATUS_CURRENT_TRACK;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		
	// Return current track number
	return MCIStatusParms.dwReturn;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		GetPositionInTrack					 													//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long WINAPI DLLExport GetPositionInTrack(fprdata rdPtr, long param1)
{
	MCI_OPEN_PARMS				MCIOpenParms;
	MCI_SET_PARMS 				MCISetParms;
    MCI_STATUS_PARMS          	MCIStatusParms;
    ulong						TrackPosition;
    ulong						Position;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Get current track number
	MCIStatusParms.dwItem = MCI_STATUS_CURRENT_TRACK;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Set time format
	MCISetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Get current current track position
	MCIStatusParms.dwTrack = MCIStatusParms.dwReturn;
	MCIStatusParms.dwItem = MCI_STATUS_POSITION;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	TrackPosition = MCIStatusParms.dwReturn;

	// Get position in track
	MCIStatusParms.dwItem = MCI_STATUS_POSITION;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	Position = MCIStatusParms.dwReturn - TrackPosition;
		
	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	// Return position in track
	return Position;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		GetTrackLength						 													//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long WINAPI DLLExport GetTrackLength(fprdata rdPtr, long param1)
{
	MCI_OPEN_PARMS				MCIOpenParms;
	MCI_SET_PARMS 				MCISetParms;
    MCI_STATUS_PARMS          	MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);

    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Get current track number
	MCIStatusParms.dwItem = MCI_STATUS_CURRENT_TRACK;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Set time format
	MCISetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	
	// Get current track position
	MCIStatusParms.dwTrack = MCIStatusParms.dwReturn;
	MCIStatusParms.dwItem = MCI_STATUS_LENGTH;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
    
    // Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	// Return current track length
	return MCIStatusParms.dwReturn;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		GetPosition							 													//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long  WINAPI DLLExport GetPosition(fprdata rdPtr, long param1)
{
	MCI_OPEN_PARMS				MCIOpenParms;
	MCI_SET_PARMS 				MCISetParms;
    MCI_STATUS_PARMS          	MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);
    
    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Set time format
	MCISetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	
	// Get position
	MCIStatusParms.dwItem = MCI_STATUS_POSITION;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}

	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	// Return position
	return MCIStatusParms.dwReturn;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		GetLength							 													//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long WINAPI DLLExport GetLength(fprdata rdPtr, long param1)
{
	MCI_OPEN_PARMS				MCIOpenParms;
	MCI_SET_PARMS 				MCISetParms;
    MCI_STATUS_PARMS          	MCIStatusParms;

	// Debug assertion
    assert(rdPtr != NULL);
    
    // Open cd
	if (0 == rdPtr->rcdDevice)
	{
	    MCIOpenParms.lpstrDeviceType = achCDAudio;
	    if ((rdPtr->rcdError = (short)mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&MCIOpenParms)) != 0)
	    	return 0;
	}
	else
		MCIOpenParms.wDeviceID = rdPtr->rcdDevice;

	// Set time format
	MCISetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&MCISetParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
	
	// Get length
	MCIStatusParms.dwItem = MCI_STATUS_LENGTH;
	if ((rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&MCIStatusParms)) != 0)
	{
		if (0 == rdPtr->rcdDevice)
			mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);
		return 0;
	}
		
	// Close cd
	if (0 == rdPtr->rcdDevice)
		rdPtr->rcdError = (short)mciSendCommand(MCIOpenParms.wDeviceID, MCI_CLOSE, 0, (WORD)(LPVOID)NULL);

	// Return length
	return MCIStatusParms.dwReturn;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		GetLastError2							 												//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long WINAPI DLLExport GetLastError2(fprdata rdPtr, long param1)
{
	// Debug assertion
    assert(rdPtr != NULL);

	return (rdPtr->rcdError);
}










//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		RETURN THE POPUP MENUS TO OPEN UNDER EVENT EDITOR																	//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef RUN_ONLY


//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		menucpy																				    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

void menucpy(HMENU hTargetMenu, HMENU hSourceMenu)
{
	int			n, id, nMn;
	NPSTR		strBuf;
	HMENU		hSubMenu;
	
	nMn = GetMenuItemCount(hSourceMenu);
	strBuf = (NPSTR)LocalAlloc(LPTR, 80);
	for (n = 0; n < nMn; n++)
	{
		if (0 == (id = GetMenuItemID(hSourceMenu, n)))
			AppendMenu(hTargetMenu, MF_SEPARATOR, 0, 0L);
		else
		{
			GetMenuString(hSourceMenu, n, strBuf, 80, MF_BYPOSITION);
			if (id != -1)
				AppendMenu(hTargetMenu, GetMenuState(hSourceMenu, n, MF_BYPOSITION), id, strBuf);
			else
			{
				hSubMenu = CreatePopupMenu();
				AppendMenu(hTargetMenu, MF_POPUP | MF_STRING, (uint)hSubMenu, strBuf);
				menucpy(hSubMenu, GetSubMenu(hSourceMenu, n));
			}
		}
	}
	LocalFree((HLOCAL)strBuf);
}






//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetPopupMenu																		    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

HMENU GetPopupMenu(short mn)
{
	HMENU	hMn, hSubMenu, hPopup = NULL;

	if ((hMn = LoadMenu(hInstLib, MAKEINTRESOURCE(mn))) != NULL)
	{
		if ((hSubMenu = GetSubMenu(hMn, 0)) != NULL)
		{
			if ((hPopup = CreatePopupMenu()) != NULL)
				menucpy(hPopup, hSubMenu);
		}
		DestroyMenu(hMn);
	}
	return hPopup;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetConditionMenu																	    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

HMENU WINAPI DLLExport GetConditionMenu(mv _far *knpV, fpObjInfo oiPtr, fpedata edPtr)
{
	// Debug assertion
    assert(knpV != NULL);
    assert(oiPtr != NULL);

	return GetPopupMenu(MN_CONDITIONS);
}




//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetActionMenu																		    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

HMENU WINAPI DLLExport GetActionMenu(mv _far *knpV, fpObjInfo oiPtr, fpedata edPtr)
{
	// Debug assertion
    assert(knpV != NULL);
    assert(oiPtr != NULL);
	return GetPopupMenu(MN_ACTIONS);
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetExpressionMenu																	    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

HMENU WINAPI DLLExport GetExpressionMenu(mv _far *knpV, fpObjInfo oiPtr, fpedata edPtr)
{
	// Debug assertion
    assert(knpV != NULL);
    assert(oiPtr != NULL);
	return GetPopupMenu(MN_EXPRESSIONS);
}










//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		RETURN THE TEXT TO BE DISPLAYED AS TITLE FOR PARAMETERS INPUT														//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetCodeTitle																		    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

void GetCodeTitle(WORD s, short mn, short entryId, LPSTR strBuf, WORD maxLen)
{
	HMENU		hMn;

	// Debug assertion
    assert(strBuf != NULL);

	// If a special string is to be returned
	if (s != 0)
		LoadString(hInstLib, s, strBuf, maxLen);

	// Otherwise, returns the menu option 
	else
	{
		if ((hMn = LoadMenu(hInstLib, MAKEINTRESOURCE(mn))) != NULL)
		{
			GetMenuString(hMn, entryId, strBuf, maxLen, MF_BYCOMMAND);
			DestroyMenu(hMn);
		}
	}
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetConditionTitle																	    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

void WINAPI DLLExport GetConditionTitle(mv _far *knpV, short code, short param, fpchar strBuf, short maxLen)
{
	// Debug assertion
    assert(knpV != NULL);
    assert(strBuf != NULL);

	GetCodeTitle(conditionsInfos[code].paramTitle[param], MN_CONDITIONS, (short)(ID_MENUCONDITIONS + code), strBuf, maxLen);
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetActionTitle																		    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

void WINAPI DLLExport GetActionTitle(mv _far *knpV, short code, short param, fpchar strBuf, short maxLen)
{
	// Debug assertion
    assert(knpV != NULL);
    assert(strBuf != NULL);

	GetCodeTitle(actionsInfos[code].paramTitle[param], MN_ACTIONS, (short)(ID_MENUACTIONS + code), strBuf, maxLen);
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetExpressionTitle																	    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

void WINAPI DLLExport GetExpressionTitle(mv _far *knpV, short code, fpchar strBuf, short maxLen)
{
	// Debug assertion
    assert(knpV != NULL);
    assert(strBuf != NULL);

	GetCodeTitle(expressionsInfos[code].paramTitle[0], MN_EXPRESSIONS, (short)(ID_MENUEXPRESSIONS + code), strBuf, maxLen);
}










//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		CONVERTS A MENU ID TO INTERNAL EVENT CODE																			//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetConditionCodeFromMenu															    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport GetConditionCodeFromMenu(mv _far *knpV, short menuId)
{
	// Debug assertion
    assert(knpV != NULL);

	if (menuId >= ID_MENUCONDITIONS && menuId < ID_MENUCONDITIONS + CND_LAST)
	{
		return (menuId - ID_MENUCONDITIONS);
	}
	return -1;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetActionCodeFromMenu																    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport GetActionCodeFromMenu(mv _far *knpV, short menuId)
{
	 // Debug assertion
    assert(knpV != NULL);

	if (menuId >= ID_MENUACTIONS && menuId < ID_MENUACTIONS + ACT_LAST)
	{
		return (menuId - ID_MENUACTIONS);
	}
	return -1;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetExpressionCodeFromMenu															    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport GetExpressionCodeFromMenu(mv _far *knpV, short menuId)
{
	// Debug assertion
    assert(knpV != NULL);

	if (menuId >= ID_MENUEXPRESSIONS && menuId < ID_MENUEXPRESSIONS + EXP_LAST)
	{
		return (menuId - ID_MENUEXPRESSIONS);
	}
	return -1;
}










//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		RETURNS INFORMATIONS ABOUT ONE EVENT																				//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetConditionInfos																	    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

infosEvents _far * WINAPI DLLExport GetConditionInfos(mv _far *knpV, short code)
{
	// Debug assertion
    assert(knpV != NULL);

	return &conditionsInfos[code];
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetActionInfos																		    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

infosEvents _far * WINAPI DLLExport GetActionInfos(mv _far *knpV, short code)
{
	// Debug assertion
    assert(knpV != NULL);

	return &actionsInfos[code];
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetExpressionInfos																	    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

infosEvents _far * WINAPI DLLExport GetExpressionInfos(mv _far *knpV, short code)
{
	// Debug assertion
    assert(knpV != NULL);

	return &expressionsInfos[code];
}










//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		RETURN THE DISPLAY STRING OF AN EVENT																				//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetConditionString																	    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

void WINAPI DLLExport GetConditionString(mv _far *knpV, short code, fpchar strPtr, short maxLen)
{
	// Debug assertion
    assert(knpV != NULL);
    assert(strPtr != NULL);

	// Check compatibility
	if ( !IS_COMPATIBLE(knpV) )
	{
		strcpy_s(strPtr, maxLen, "Incompatible version");
		return;
	}

	LoadString(hInstLib, M_DISPCONDITIONS + code, strPtr, maxLen);
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetActionString																		    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

void WINAPI DLLExport GetActionString(mv _far *knpV, short code, fpchar strPtr, short maxLen)
{            
	// Debug assertion
    assert(knpV != NULL);
    assert(strPtr != NULL);

	// Check compatibility
	if ( !IS_COMPATIBLE(knpV) )
	{
		strcpy_s(strPtr, maxLen, "Incompatible version");
		return;
	}

	LoadString(hInstLib, M_DISPACTIONS + code, strPtr, maxLen);
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		GetExpressionString																	    //
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

void WINAPI DLLExport GetExpressionString(mv _far *knpV, short code, fpchar strPtr, short maxLen)
{            
	// Debug assertion
    assert(knpV != NULL);
    assert(strPtr != NULL);

	// Check compatibility
	if ( !IS_COMPATIBLE(knpV) )
	{
		strcpy_s(strPtr, maxLen, "Incompatible version");
		return;
	}

	LoadString(hInstLib, M_DISPEXPRESSIONS + code, strPtr, maxLen);
}







//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		SUB-ROUTINES																										//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Create object																			//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI DLLExport CreateObject(mv _far *knpV, fpLevObj loPtr, fpedata edPtr)
{
	// Check compatibility
	if ( !IS_COMPATIBLE(knpV) )
		return -1;

	// Set default object flags
	edPtr->ecdaFlags = CD_OPENONSTART | CD_PLAYONSTART | CD_STOPONCLOSE | CD_PLAYWHENMINIMIZED;
	edPtr->ecdaTrackOnStart = 1;
	return 0;
}




//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Get object rectangle																	//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

void WINAPI DLLExport GetObjectRect(mv _far *knpV, RECT FAR *rc, fpLevObj loPtr, fpedata edPtr)
{
	// Debug assertion
    assert(knpV != NULL);
    assert(rc != NULL);
    assert(loPtr != NULL);
    assert(edPtr != NULL);

	// Returns size of the icon
	rc->right = rc->left + 32;
	rc->bottom = rc->top + 32;

	return;
}

void WINAPI	DLLExport GetObjInfos (mv _far *knpV,fpedata edPtr,fpchar ObjName,fpchar ObjAuteur,fpchar ObjCopyright,fpchar ObjComment,fpchar ObjHttp)
{
	LoadString(hInstLib, IDST_OBJNAME, (LPSTR)ObjName, 255);
	LoadString(hInstLib, IDST_AUTHOR,ObjAuteur,255);
	LoadString(hInstLib, IDST_COPYRIGHT,ObjCopyright,255);
	LoadString(hInstLib, IDST_COMMENT,ObjComment,255);
	LoadString(hInstLib, IDST_HTTP,ObjHttp,255);
}

LPCSTR WINAPI DLLExport GetHelpFileName()
{
	return "CDAudio.chm";
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		PROPERTIES  																			//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DLLExport GetProperties(LPMV mV, fpedata edPtr, BOOL bMasterItem)
{
	mV->mvCallFunction(edPtr, EF_INSERTPROPS, (LPARAM)Properties, PROPID_TAB_GENERAL, TRUE);

	return TRUE;
}


LPVOID WINAPI DLLExport GetPropValue(LPMV mV, fpedata edPtr, UINT nPropID)
{
	switch (nPropID) 
	{
	case PROPID_PLAYATSTART:
		return new CPropDWordValue(edPtr->ecdaTrackOnStart);
	}
	return NULL;
}

int WINAPI DLLExport GetPropCheck (LPMV mV, fpedata edPtr, UINT nPropID)
{
	switch (nPropID)
	{
	case PROPID_PLAYATSTART:
		return edPtr->ecdaFlags&CD_PLAYONSTART?TRUE:FALSE;
	case PROPID_OPENDRIVER:
		return edPtr->ecdaFlags&CD_OPENONSTART?TRUE:FALSE;
	case PROPID_STOPATEND:
		return edPtr->ecdaFlags&CD_STOPONCLOSE?TRUE:FALSE;
	case PROPID_PLAYMINIMIZED:
		return edPtr->ecdaFlags&CD_PLAYWHENMINIMIZED?TRUE:FALSE;
	}
	return FALSE;
}

void WINAPI DLLExport SetPropValue(LPMV mV, fpedata edPtr, UINT nPropID, LPVOID lParam)
{
	CPropValue* pValue = (CPropValue*)lParam;

	switch (nPropID) 
	{
	case PROPID_PLAYATSTART:
		edPtr->ecdaTrackOnStart=(short)((CPropDWordValue*)pValue)->m_dwValue;
		break;
	}
}

void WINAPI DLLExport SetPropCheck (LPMV mV, fpedata edPtr, UINT nPropID, int nCheck)
{
	switch (nPropID)
	{
	case PROPID_PLAYATSTART:
		edPtr->ecdaFlags&=~CD_PLAYONSTART;
		if (nCheck)
			edPtr->ecdaFlags|=CD_PLAYONSTART;
		break;
	case PROPID_OPENDRIVER:
		edPtr->ecdaFlags&=~CD_OPENONSTART;
		if (nCheck)
			edPtr->ecdaFlags|=CD_OPENONSTART;
		break;
	case PROPID_STOPATEND:
		edPtr->ecdaFlags&=~CD_STOPONCLOSE;
		if (nCheck)
			edPtr->ecdaFlags|=CD_STOPONCLOSE;
		break;
	case PROPID_PLAYMINIMIZED:
		edPtr->ecdaFlags&=~CD_PLAYWHENMINIMIZED;
		if (nCheck)
			edPtr->ecdaFlags|=CD_PLAYWHENMINIMIZED;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		DEBUGGER	  																			//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

LPWORD WINAPI DLLExport GetDebugTree(fprdata rdPtr)
{
	return DebugTree;
}

void WINAPI DLLExport GetDebugItem(LPSTR pBuffer, fprdata rdPtr, int id)
{
	char pTemp[DB_BUFFERSIZE];

	switch (id)
	{
	case DB_CURRENTTRACK:
		{
			LoadString(hInstLib, IDS_CURRENTTRACK, pTemp, DB_BUFFERSIZE);
			int value=GetCurrentTrack(rdPtr, 0);
			wsprintf(pBuffer, pTemp, value);
		}
		break;
	case DB_NUMBEROFTRACKS:
		{
			LoadString(hInstLib, IDS_NUMBEROFTRACKS, pTemp, DB_BUFFERSIZE);
			int value=GetNumberOfTracks(rdPtr, 0);
			wsprintf(pBuffer, pTemp, value);
		}
		break;
	case DB_POSITIONINTRACK:
		{
			LoadString(hInstLib, IDS_POSITIONINTRACK, pTemp, DB_BUFFERSIZE);
			int position=GetPositionInTrack(rdPtr, 0);
			int minutes=position/60000;
			int secondes=(position-minutes*60000)/1000;
			wsprintf(pBuffer, pTemp, minutes, secondes);
		}
		break;
	case DB_TRACKLENGTH:
		{
			LoadString(hInstLib, IDS_TRACKLENGTH, pTemp, DB_BUFFERSIZE);
			int position=GetTrackLength(rdPtr, 0);
			int minutes=position/60000;
			int secondes=(position-minutes*60000)/1000;
			wsprintf(pBuffer, pTemp, minutes, secondes);
		}
		break;
	case DB_LENGTH:
		{
			LoadString(hInstLib, IDS_LENGTH, pTemp, DB_BUFFERSIZE);
			int position=GetLength(rdPtr, 0);
			int heures=position/3600000;
			int minutes=(position-heures*3600000)/60000;
			int secondes=(position-heures*3600000-minutes*60000)/1000;
			wsprintf(pBuffer, pTemp, heures, minutes, secondes);
		}
		break;
	case DB_LASTERROR:
		LoadString(hInstLib, IDS_LASTERROR, pTemp, DB_BUFFERSIZE);
		wsprintf(pBuffer, pTemp, rdPtr->rcdError);
		break;
	}
}





//==================================================================//
//																	//
//				Functions not called in run only					//
//																	//
//==================================================================//
#else	// RUN_ONLY


void WINAPI	DLLExport GetObjInfos (mv _far *knpV,fpedata edPtr,fpchar ObjName,fpchar ObjAuteur,fpchar ObjCopyright,fpchar ObjComment,fpchar ObjHttp)
{
}

LPCSTR WINAPI DLLExport GetHelpFileName()
{
	return NULL;
}

HMENU WINAPI DLLExport GetConditionMenu(mv _far *knpV, fpObjInfo oiPtr, fpedata edPtr)
{
	return NULL;
}

HMENU WINAPI DLLExport GetActionMenu(mv _far *knpV, fpObjInfo oiPtr, fpedata edPtr)
{
	return NULL;
}

HMENU WINAPI DLLExport GetExpressionMenu(mv _far *knpV, fpObjInfo oiPtr, fpedata edPtr)
{
	return NULL;
}

void WINAPI DLLExport GetConditionTitle(mv _far *knpV, short code, short param, fpchar strBuf, short maxLen)
{
}

void WINAPI DLLExport GetActionTitle(mv _far *knpV, short code, short param, fpchar strBuf, short maxLen)
{
}

void WINAPI DLLExport GetExpressionTitle(mv _far *knpV, short code, fpchar strBuf, short maxLen)
{
}

short WINAPI DLLExport GetConditionCodeFromMenu(mv _far *knpV, short menuId)
{
	return -1;
}

short WINAPI DLLExport GetActionCodeFromMenu(mv _far *knpV, short menuId)
{
	return -1;
}

short WINAPI DLLExport GetExpressionCodeFromMenu(mv _far *knpV, short menuId)
{
	return -1;
}

infosEvents _far * WINAPI DLLExport GetConditionInfos(mv _far *knpV, short code)
{
	return NULL;
}

infosEvents _far * WINAPI DLLExport GetActionInfos(mv _far *knpV, short code)
{
	return NULL;
}

infosEvents _far * WINAPI DLLExport GetExpressionInfos(mv _far *knpV, short code)
{
	return NULL;
}

void WINAPI DLLExport GetConditionString(mv _far *knpV, short code, fpchar strPtr, short maxLen)
{
}

void WINAPI DLLExport GetActionString(mv _far *knpV, short code, fpchar strPtr, short maxLen)
{
}

void WINAPI DLLExport GetExpressionString(mv _far *knpV, short code, fpchar strPtr, short maxLen)
{
}

int WINAPI DLLExport CreateObject(mv _far *knpV, fpLevObj loPtr, fpedata edPtr)
{
	return 0;
}

void WINAPI DLLExport GetObjectRect(mv _far *knpV, RECT FAR *rc, fpLevObj loPtr, fpedata edPtr)
{
}

BOOL WINAPI DLLExport GetProperties(LPMV mV, fpedata edPtr, BOOL bMasterItem)
{
	return FALSE;
}

LPVOID WINAPI DLLExport GetPropValue(LPMV mV, fpedata edPtr, UINT nPropID)
{
	return NULL;
}

int WINAPI DLLExport GetPropCheck (LPMV mV, fpedata edPtr, UINT nPropID)
{
	return 0;
}

void WINAPI DLLExport SetPropValue(LPMV mV, fpedata edPtr, UINT nPropID, LPVOID lParam)
{
}

void WINAPI DLLExport SetPropCheck (LPMV mV, fpedata edPtr, UINT nPropID, int nCheck)
{
}

LPWORD WINAPI DLLExport GetDebugTree(fprdata rdPtr)
{
	return NULL;
}

void WINAPI DLLExport GetDebugItem(LPSTR pBuffer, fprdata rdPtr, int id)
{
}

#endif	// RUN_ONLY




//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Library entry point																		//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		// DLL is attaching to the address space of the current process.
		case DLL_PROCESS_ATTACH:
			hInstLib = hDLL; // Store HINSTANCE
			break;

		// A new thread is being created in the current process.
		case DLL_THREAD_ATTACH:
			break;

		// A thread is exiting cleanly.
		case DLL_THREAD_DETACH:
			break;

		// The calling process is detaching the DLL from its address space.
	    case DLL_PROCESS_DETACH:
	    	// Virer les objets??
			break;
	}

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Demande d'infos a propos de l'objet														//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" DWORD WINAPI DLLExport GetInfos(int info)
{
	switch (info)
	{
		case KGI_VERSION:
			return EXT_VERSION1;
		case KGI_PRODUCT:
			return PRODUCT_VERSION_HOME;
		case KGI_BUILD:
			return MMF_CURRENTBUILD-1;
#ifdef VITALIZE
		case KGI_PLUGIN:
			return EXT_PLUGIN_VERSION2;		// Return plug-in version
		case KGI_ATX_CONTROL:
			return EXT_ATXCTRL_VERSION1;		// Return ActiveX version
#endif

		default:
			return 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
//		Update Object's Version																	//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////
	
HGLOBAL WINAPI DLLExport UpdateEditStructure(mv __far *knpV, void __far * OldEdPtr)
{
	return NULL;
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		RETURNS ADRESSES OF JUMP TABLES AND INFORMATIONS ABOUT THE RUN-OBJECT												//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		Jump table to conditions																//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long ( WINAPI * ConditionJumps[] ) (fprdata rdPtr,long param1,long param2) = 
				{ 
				CmpPositionInTrack,	
				CmpPosition,
				0
				};
	




//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		Jump table to actions																	//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short ( WINAPI * ActionJumps[] ) (fprdata rdPtr,long param1,long param2) =
				{
				Play,
				Stop,
				Pause,
				Resume,
				Previous,
				Next,
				TimePlay,
				OpenDoor,
				CloseDoor,
				PlayTrack,
				0
				};
	




//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		Jump table to expressions																//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

long ( WINAPI * ExpressionJumps[] ) (fprdata rdPtr,long param) = 
				{     
				GetCurrentTrack,
				GetNumberOfTracks,
				GetPositionInTrack,
				GetTrackLength,
				GetPosition,
				GetLength,
				GetLastError2,
				0
				};





//////////////////////////////////////////////////////////////////////////////////////////////////
// 																							    //
// 		get infos on object																		//
// 																							    //
//////////////////////////////////////////////////////////////////////////////////////////////////

short WINAPI DLLExport GetRunObjectInfos(mv _far *knpV, fpKpxRunInfos infoPtr)
{            
	// Debug assertion
    assert(infoPtr != NULL);

	infoPtr->conditions = (LPBYTE)ConditionJumps;
	infoPtr->actions = (LPBYTE)ActionJumps;
	infoPtr->expressions = (LPBYTE)ExpressionJumps;
	
	infoPtr->numOfConditions = CND_LAST;
	infoPtr->numOfActions = ACT_LAST;
	infoPtr->numOfExpressions = EXP_LAST;
	
	infoPtr->editDataSize = MAX_EDITSIZE;
	infoPtr->editFlags = OEFLAGS;

	// Voir la documentation des KCX
	infoPtr->editPrefs = OEPREFS;

	// Un identifier de nom, qui sera reperable en mode runtime
	infoPtr->identifier = IDENTIFIER;
	
	// La version courante de l'extension, voir plus loin
	infoPtr->version = KCX_CURRENT_VERSION;
	
	return TRUE;
}



