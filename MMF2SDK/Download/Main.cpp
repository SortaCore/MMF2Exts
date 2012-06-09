// ============================================================================
//
// This file are where the Conditions/Actions/Expressions are defined.
// You can manually enter these, or use CICK (recommended)
// See the Extension FAQ in this SDK for more info and where to download it
//
// ============================================================================

// Common Include
#include	"common.h"
#include	"mmsystem.h"

extern void CloseSlot(LPRDATA rdPtr, int slot);

// Quick memo: content of the eventInformations arrays
// ---------------------------------------------------
// Menu ID
// String ID
// Code
// Flags
// Number_of_parameters
// Parameter_type [Number_of_parameters]
// Parameter_TitleString [Number_of_parameters]

// Definitions of parameters for each condition
short conditionsInfos[]=
		{
		IDMN_CONDITION_DOWNLOADING, M_CONDITION_DOWNLOADING, CND_DOWNLOADING, EVFLAGS_ALWAYS+EVFLAGS_NOTABLE, 1, PARAM_EXPRESSION, M_SLOT,
		IDMN_CONDITION_COMPLETED, M_CONDITION_COMPLETED, CND_COMPLETED, EVFLAGS_ALWAYS+EVFLAGS_NOTABLE, 1, PARAM_EXPRESSION, M_SLOT,
		IDMN_CONDITION_ERROR, M_CONDITION_ERROR, CND_ERROR, EVFLAGS_ALWAYS+EVFLAGS_NOTABLE, 1, PARAM_EXPRESSION, M_SLOT,
		IDMN_CONDITION_ABORTED, M_CONDITION_ABORTED, CND_ABORTED, EVFLAGS_ALWAYS+EVFLAGS_NOTABLE, 1, PARAM_EXPRESSION, M_SLOT,
		};

// Definitions of parameters for each action
short actionsInfos[]=
		{
		IDMN_ACTION_DOWNLOAD, M_ACTION_DOWNLOAD, ACT_ACTION_DOWNLOAD, 0, 3, PARAM_EXPSTRING, PARAM_FILENAME2, PARAM_EXPRESSION, M_URL, M_FILE, M_SLOT,
		IDMN_ACTION_ABORT, M_ACTION_ABORT, ACT_ACTION_ABORT, 0, 1, PARAM_EXPRESSION, M_SLOT,
		IDMN_ACTION_PAUSE, M_ACTION_PAUSE, ACT_ACTION_PAUSE, 0, 1, PARAM_EXPRESSION, M_SLOT,
		IDMN_ACTION_RESUME, M_ACTION_RESUME, ACT_ACTION_RESUME, 0, 1, PARAM_EXPRESSION, M_SLOT,
		};

// Definitions of parameters for each expression
short expressionsInfos[]=
		{
		IDMN_EXP_TOTALSIZE, M_EXP_TOTALSIZE, EXP_TOTALSIZE, 0, 1, EXPPARAM_LONG, M_SLOT,
		IDMN_EXP_CURRENTPOS, M_EXP_CURRENTPOS, EXP_CURRENTPOS, 0, 1, EXPPARAM_LONG, M_SLOT,
		IDMN_EXP_CURRENTPERCENT, M_EXP_CURRENTPERCENT, EXP_CURRENTPERCENT, 0, 1, EXPPARAM_LONG, M_SLOT,
		IDMN_EXP_CURRENTSPEED, M_EXP_CURRENTSPEED, EXP_CURRENTSPEED, 0, 1, EXPPARAM_LONG, M_SLOT,
		IDMN_EXP_TOTALTIME, M_EXP_TOTALTIME, EXP_TOTALTIME, 0, 1, EXPPARAM_LONG, M_SLOT,
		IDMN_EXP_CURRENTURL, M_EXP_CURRENTURL, EXP_CURRENTURL, EXPFLAG_STRING, 1, EXPPARAM_LONG, M_SLOT,
		IDMN_EXP_SIZE, M_EXP_SIZE, EXP_SIZE, 0, 1, EXPPARAM_STRING, M_EXPURL,
		};



// ============================================================================
//
// CONDITION ROUTINES
// 
// ============================================================================

// -----------------
// Sample Condition
// -----------------
// Returns TRUE when the two values are equal!
// 

long WINAPI DLLExport Condition_Downloading(LPRDATA rdPtr, long param1, long param2)
{
	param1--;
	if (param1>=0 && param1<MAX_SLOTS)
	{
		if (rdPtr->slots[param1].hOpenedURL!=0)
		{
			return TRUE;
		}
	}
	return FALSE;
}
long WINAPI DLLExport Condition_Completed(LPRDATA rdPtr, long param1, long param2)
{
	param1--;
	if (param1>=0 && param1<MAX_SLOTS)
	{
		return rdPtr->slots[param1].bCompleted;
	}
	return FALSE;
}
long WINAPI DLLExport Condition_Error(LPRDATA rdPtr, long param1, long param2)
{
	param1--;
	if (param1>=0 && param1<MAX_SLOTS)
	{
		return rdPtr->slots[param1].bError!=0;
	}
	return FALSE;
}
long WINAPI DLLExport Condition_Aborted(LPRDATA rdPtr, long param1, long param2)
{
	param1--;
	if (param1>=0 && param1<MAX_SLOTS)
	{
		return rdPtr->slots[param1].bAborted;
	}
	return FALSE;
}


// ============================================================================
//
// ACTIONS ROUTINES
// 
// ============================================================================

short WINAPI DLLExport Action_Download(LPRDATA rdPtr, long param1, long param2)
{
	LPSTR pURL=(LPSTR)CNC_GetParameter(rdPtr);
	LPSTR pFile=(LPSTR)CNC_GetParameter(rdPtr);
	int slot=CNC_GetParameter(rdPtr);

	slot--;
	if (slot<0 || slot>=MAX_SLOTS)
		return 0;

	if (rdPtr->hWininet!=0 && rdPtr->hInternetSession!=0)
	{
		if (rdPtr->slots[slot].hOpenedURL==0)
		{
			rdPtr->slots[slot].bCompleted=FALSE;
			rdPtr->slots[slot].bError=0;
			rdPtr->slots[slot].bPause=FALSE;
			rdPtr->slots[slot].bAborted=FALSE;
			int l=strlen(pURL);
			if (l>0)
			{
				if (rdPtr->slots[slot].pURL!=NULL)
				{
					free(rdPtr->slots[slot].pURL);
					rdPtr->slots[slot].pURL=NULL;
				}
				rdPtr->slots[slot].pURL=(LPSTR)malloc(l+1);
				if (rdPtr->slots[slot].pURL!=NULL)
				{
					if (!strcpy_s(rdPtr->slots[slot].pURL, l+1, pURL))
					{
						rdPtr->slots[slot].pBuffer=(LPBYTE)malloc(DOWNLOAD_BUFFER_SIZE);
						if ( rdPtr->slots[slot].pBuffer!=NULL )
						{
							if ( (rdPtr->slots[slot].hOpenedURL=rdPtr->lpInternetOpenUrl(rdPtr->hInternetSession, pURL, NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0)) != NULL )
							{
								DWORD dwNumberOfBytesAvailable;
								if ( rdPtr->lpInternetQueryDataAvailable(rdPtr->slots[slot].hOpenedURL, &dwNumberOfBytesAvailable, 0, 0) )
								{ 
									if ( dwNumberOfBytesAvailable != 0 )
									{
										// Get file length
										char buffer[64];
										DWORD dwTaille=64;
										DWORD dwHeader=0;
										if (rdPtr->lpHttpQueryInfo(rdPtr->slots[slot].hOpenedURL, HTTP_QUERY_CONTENT_LENGTH, buffer, &dwTaille, &dwHeader))
										{
											rdPtr->slots[slot].dwFileSize=atoi(buffer);
										
											// Create target file
											rdPtr->slots[slot].hFile = CreateFile(pFile,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
											if ( rdPtr->slots[slot].hFile!=INVALID_HANDLE_VALUE )
											{
												rdPtr->slots[slot].dwTotalRead=0;
												rdPtr->slots[slot].dwSpeed=0;
												rdPtr->slots[slot].dwInitialTime=timeGetTime();
												return 0;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			CloseSlot(rdPtr, slot);
			rdPtr->slots[slot].bAborted=TRUE;
			rdPtr->slots[slot].bError=1;
		}
	}
	return 0;
}

short WINAPI DLLExport Action_Abort(LPRDATA rdPtr, long param1, long param2)
{
	param1--;
	if (param1>=0 && param1<MAX_SLOTS)
	{
		if (rdPtr->slots[param1].hOpenedURL!=0)
		{
			rdPtr->slots[param1].bAborted=TRUE;
			CloseSlot(rdPtr, param1);
		}
	}
	return 0;
}

short WINAPI DLLExport Action_Pause(LPRDATA rdPtr, long param1, long param2)
{
	param1--;
	if (param1>=0 && param1<MAX_SLOTS)
	{
		if (rdPtr->slots[param1].hOpenedURL!=0)
		{
			rdPtr->slots[param1].bPause=TRUE;
		}
	}
	return 0;
}

short WINAPI DLLExport Action_Resume(LPRDATA rdPtr, long param1, long param2)
{
	param1--;
	if (param1>=0 && param1<MAX_SLOTS)
	{
		if (rdPtr->slots[param1].hOpenedURL!=0)
		{
			rdPtr->slots[param1].bPause=FALSE;
		}
	}
	return 0;
}

// ============================================================================
//
// EXPRESSIONS ROUTINES
// 
// ============================================================================
long WINAPI DLLExport Exp_Size(LPRDATA rdPtr,long param1)
{
	LPSTR pURL=(LPSTR)CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_STRING);

	HINTERNET hOpenedURL=NULL;
	int taille=-1;

	if (rdPtr->hWininet!=0 && rdPtr->hInternetSession!=0)
	{
		if ( (hOpenedURL=rdPtr->lpInternetOpenUrl(rdPtr->hInternetSession, pURL, NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0)) != NULL )
		{
			char buffer[64];
			DWORD dwTaille=64;
			DWORD dwHeader=0;
			if (rdPtr->lpHttpQueryInfo(hOpenedURL, HTTP_QUERY_CONTENT_LENGTH, buffer, &dwTaille, &dwHeader))
			{
				taille=atoi(buffer);
			}
			rdPtr->lpInternetCloseHandle(hOpenedURL);
		}
	}
	return taille;
}

long WINAPI DLLExport Exp_TotalSize(LPRDATA rdPtr,long param1)
{
	long slot = CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	slot--;

	if (slot>=0 && slot<MAX_SLOTS)
	{
		return rdPtr->slots[slot].dwFileSize;
	}
	return 0;
}
long WINAPI DLLExport Exp_CurrentPos(LPRDATA rdPtr,long param1)
{
	long slot = CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	slot--;

	if (slot>=0 && slot<MAX_SLOTS)
	{
		return rdPtr->slots[slot].dwTotalRead;
	}
	return 0;
}
long WINAPI DLLExport Exp_CurrentPercent(LPRDATA rdPtr,long param1)
{
	long slot = CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	slot--;

	if (slot>=0 && slot<MAX_SLOTS)
	{
		if (rdPtr->slots[slot].dwFileSize!=0)
		{
//			int percent=(rdPtr->slots[slot].dwTotalRead*100)/rdPtr->slots[slot].dwFileSize;
			if ( rdPtr->slots[slot].dwTotalRead == rdPtr->slots[slot].dwFileSize )
				return 100;
			return (int)((double)(rdPtr->slots[slot].dwTotalRead*100.0)/(double)rdPtr->slots[slot].dwFileSize);
		}
	}
	return 0;
}
long WINAPI DLLExport Exp_CurrentSpeed(LPRDATA rdPtr,long param1)
{
	long slot = CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	slot--;

	if (slot>=0 && slot<MAX_SLOTS)
	{
		return rdPtr->slots[slot].dwSpeed;
	}
	return 0;
}
long WINAPI DLLExport Exp_TotalTime(LPRDATA rdPtr,long param1)
{
	long slot = CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	slot--;

	if (slot>=0 && slot<MAX_SLOTS)
	{
		DWORD bytes=rdPtr->slots[slot].dwFileSize-rdPtr->slots[slot].dwTotalRead;
		if (bytes>0 && rdPtr->slots[slot].dwSpeed>0)
		{
			return bytes/rdPtr->slots[slot].dwSpeed;
		}
	}
	return 0;
}
long WINAPI DLLExport Exp_CurrentURL(LPRDATA rdPtr,long param1)
{
	long slot = CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	slot--;

	rdPtr->rHo.hoFlags |= HOF_STRING;

	if (slot>=0 && slot<MAX_SLOTS)
	{
		if (rdPtr->slots[slot].pURL!=NULL)
		{
			return (long)rdPtr->slots[slot].pURL;
		}
	}
	return 0;
}


// ----------------------------------------------------------
// Condition / Action / Expression jump table
// ----------------------------------------------------------
// Contains the address inside the extension of the different
// routines that handle the action, conditions and expressions.
// Located at the end of the source for convinience
// Must finish with a 0
//
long (WINAPI * ConditionJumps[])(LPRDATA rdPtr, long param1, long param2) = 
			{ 
			Condition_Downloading,
			Condition_Completed,
			Condition_Error,
			Condition_Aborted,
			0
			};
	
short (WINAPI * ActionJumps[])(LPRDATA rdPtr, long param1, long param2) =
			{
			Action_Download,
			Action_Abort,
			Action_Pause,
			Action_Resume,
			0
			};

long (WINAPI * ExpressionJumps[])(LPRDATA rdPtr, long param) = 
			{     
			Exp_TotalSize,
			Exp_CurrentPos,
			Exp_CurrentPercent,
			Exp_CurrentSpeed,
			Exp_TotalTime,
			Exp_CurrentURL,
			Exp_Size,
			0
			};