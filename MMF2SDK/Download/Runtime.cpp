// ============================================================================
//
// This file contains routines that are handled during the Runtime.
//
// Including creating, display, and handling your object.
// 
// ============================================================================

// Common Include
#include	"common.h"
#include	"mmsystem.h"

// DEBUGGER /////////////////////////////////////////////////////////////////

#if !defined(RUN_ONLY)
// Identifiers of items displayed in the debugger
enum
{
	DB_SLOT0,
	DB_URL0,
	DB_TOTALSIZE0,
	DB_TOTALREAD0,
	DB_SPEED0,
	DB_SLOT1,
	DB_URL1,
	DB_TOTALSIZE1,
	DB_TOTALREAD1,
	DB_SPEED1,
	DB_SLOT2,
	DB_URL2,
	DB_TOTALSIZE2,
	DB_TOTALREAD2,
	DB_SPEED2,
	DB_SLOT3,
	DB_URL3,
	DB_TOTALSIZE3,
	DB_TOTALREAD3,
	DB_SPEED3,
};

// Items displayed in the debugger
WORD DebugTree[]=
{
	DB_PARENT|DB_SLOT0, DB_URL0,
	DB_PARENT|DB_SLOT0, DB_TOTALSIZE0,
	DB_PARENT|DB_SLOT0, DB_TOTALREAD0,
	DB_PARENT|DB_SLOT0, DB_SPEED0,

	DB_PARENT|DB_SLOT1, DB_URL1,
	DB_PARENT|DB_SLOT1, DB_TOTALSIZE1,
	DB_PARENT|DB_SLOT1, DB_TOTALREAD1,
	DB_PARENT|DB_SLOT1, DB_SPEED1,

	DB_PARENT|DB_SLOT2, DB_URL2,
	DB_PARENT|DB_SLOT2, DB_TOTALSIZE2,
	DB_PARENT|DB_SLOT2, DB_TOTALREAD2,
	DB_PARENT|DB_SLOT2, DB_SPEED2,

	DB_PARENT|DB_SLOT3, DB_URL3,
	DB_PARENT|DB_SLOT3, DB_TOTALSIZE3,
	DB_PARENT|DB_SLOT3, DB_TOTALREAD3,
	DB_PARENT|DB_SLOT3, DB_SPEED3,

	// End of table (required)
	DB_END
};

#endif // !defined(RUN_ONLY)


// --------------------
// GetRunObjectDataSize
// --------------------
// Returns the size of the runtime datazone of the object
// 
short WINAPI DLLExport GetRunObjectDataSize(fprh rhPtr, LPEDATA edPtr)
{
	return(sizeof(RUNDATA));
}


// ---------------
// CreateRunObject
// ---------------
// The routine where the object is actually created
// 
short WINAPI DLLExport CreateRunObject(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr)
{	// Initialisation of data
	rdPtr->hWininet=0;
	rdPtr->hInternetSession=0;
	int n;
	for (n=0; n<MAX_SLOTS; n++)
	{
		rdPtr->slots[n].pURL=NULL;
		rdPtr->slots[n].hFile=0;
		rdPtr->slots[n].hOpenedURL=0;
		rdPtr->slots[n].dwFileSize=0;
		rdPtr->slots[n].dwTotalRead=0;
		rdPtr->slots[n].pBuffer=NULL;
		rdPtr->slots[n].bCompleted=FALSE;
		rdPtr->slots[n].dwInitialTime=0;
		rdPtr->slots[n].dwSpeed=0;
		rdPtr->slots[n].bError=0;
		rdPtr->slots[n].bPause=FALSE;
		rdPtr->slots[n].bAborted=FALSE;
	}

	// Open WinINet library
	rdPtr->hWininet = LoadLibrary("wininet.dll");

	// Get proc addresses
	(FARPROC&)rdPtr->lpInternetOpen = GetProcAddress(rdPtr->hWininet, "InternetOpenA");
	(FARPROC&)rdPtr->lpInternetCloseHandle = GetProcAddress(rdPtr->hWininet, "InternetCloseHandle");
	(FARPROC&)rdPtr->lpInternetOpenUrl = GetProcAddress(rdPtr->hWininet, "InternetOpenUrlA");
	(FARPROC&)rdPtr->lpInternetQueryDataAvailable = GetProcAddress(rdPtr->hWininet, "InternetQueryDataAvailable");
	(FARPROC&)rdPtr->lpInternetReadFile = GetProcAddress(rdPtr->hWininet, "InternetReadFile");
	(FARPROC&)rdPtr->lpInternetSetFilePointer = GetProcAddress(rdPtr->hWininet, "InternetSetFilePointer");
	(FARPROC&)rdPtr->lpInternetQueryDataAvailable = GetProcAddress(rdPtr->hWininet, "InternetQueryDataAvailable");
	(FARPROC&)rdPtr->lpHttpQueryInfo=GetProcAddress(rdPtr->hWininet, "HttpQueryInfoA");

	// Error? free library
	if ( *rdPtr->lpInternetOpen == NULL ||
		 *rdPtr->lpInternetCloseHandle == NULL ||
		 *rdPtr->lpInternetOpenUrl == NULL ||
		 *rdPtr->lpInternetQueryDataAvailable == NULL ||
		 *rdPtr->lpInternetReadFile == NULL ||
		 *rdPtr->lpHttpQueryInfo == NULL)
	{
		if ( rdPtr->hWininet != NULL )
		{
			FreeLibrary(rdPtr->hWininet);
			rdPtr->hWininet = NULL;
			return 0;
		}
	}

	// Open Internet session
	LPRH rhPtr=rdPtr->rHo.hoAdRunHeader;
	rdPtr->hInternetSession = rdPtr->lpInternetOpen(rhPtr->rhApp->m_name, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	// No errors
	return 0;
}


// ----------------
// DestroyRunObject
// ----------------
// Destroys the run-time object
// 
void CloseSlot(LPRDATA rdPtr, int slot)
{
	if (rdPtr->slots[slot].pBuffer!=NULL)
	{
		free(rdPtr->slots[slot].pBuffer);
		rdPtr->slots[slot].pBuffer=NULL;
	}
	if (rdPtr->slots[slot].hOpenedURL!=0)
	{
		rdPtr->lpInternetCloseHandle(rdPtr->slots[slot].hOpenedURL);
		rdPtr->slots[slot].hOpenedURL=0;
	}
	if (rdPtr->slots[slot].hFile!=0)
	{
		CloseHandle(rdPtr->slots[slot].hFile);
		rdPtr->slots[slot].hFile=0;
	}
}
short WINAPI DLLExport DestroyRunObject(LPRDATA rdPtr, long fast)
{
	if (rdPtr->hWininet!=0)
	{
		if (rdPtr->hInternetSession!=0)
		{
			int n;
			for (n=0; n<MAX_SLOTS; n++)
			{
				CloseSlot(rdPtr, n);
				if (rdPtr->slots[n].pURL!=NULL)
				{
					free(rdPtr->slots[n].pURL);
				}
			}
			rdPtr->lpInternetCloseHandle(rdPtr->hInternetSession);
		}
		FreeLibrary(rdPtr->hWininet);
	}
	return 0;
}


// ----------------
// HandleRunObject
// ----------------
// Called (if you want) each loop, this routine makes the object live
// 
short WINAPI DLLExport HandleRunObject(LPRDATA rdPtr)
{
	int slot;
	BOOL bError=FALSE;

	for (slot=0; slot<MAX_SLOTS; slot++)
	{
		if (rdPtr->slots[slot].hOpenedURL!=0)
		{
			if (rdPtr->slots[slot].bPause==FALSE)
			{
				// Download file
				DWORD dwNumberOfBytesAvailable;
				rdPtr->lpInternetQueryDataAvailable(rdPtr->slots[slot].hOpenedURL, &dwNumberOfBytesAvailable,0,0);
				if (dwNumberOfBytesAvailable>0)
				{
					DWORD dwRead, dwReadSize, dwWritten;
					dwReadSize = min(DOWNLOAD_BUFFER_SIZE, dwNumberOfBytesAvailable);
							
					if (TRUE==rdPtr->lpInternetReadFile(rdPtr->slots[slot].hOpenedURL, rdPtr->slots[slot].pBuffer, dwReadSize, &dwRead))
					{
						if (FALSE==WriteFile(rdPtr->slots[slot].hFile, rdPtr->slots[slot].pBuffer, dwRead, &dwWritten, NULL))
						{
							CloseSlot(rdPtr, slot);
							rdPtr->slots[slot].bCompleted=FALSE;
							rdPtr->slots[slot].bError=2;
							rdPtr->slots[slot].bAborted=TRUE;
						}
						else
						{
							rdPtr->slots[slot].dwTotalRead += dwRead;

							// Stocke pour le calcul de vitesse
							DWORD time=timeGetTime();
							double delta=(double)(time-rdPtr->slots[slot].dwInitialTime);
							if (delta>0)
							{
								rdPtr->slots[slot].dwSpeed=(DWORD) (( (double)rdPtr->slots[slot].dwTotalRead*1000.0)/delta);
							}
						}
					}
				}
				else
				{
					CloseSlot(rdPtr, slot);
					rdPtr->slots[slot].bCompleted=TRUE;
				}
			}
		}
	}
	return 0;
}

// ----------------
// DisplayRunObject
// ----------------
// Draw the object in the application screen.
// 
short WINAPI DLLExport DisplayRunObject(LPRDATA rdPtr)
{
	return 0;
}


// ----------------
// PauseRunObject
// ----------------
// Enters the pause mode
// 
short WINAPI DLLExport PauseRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}


// -----------------
// ContinueRunObject
// -----------------
// Quits the pause mode
//
short WINAPI DLLExport ContinueRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}


// ============================================================================
//
// START APP / END APP / START FRAME / END FRAME routines
// 
// ============================================================================

// -------------------
// StartApp
// -------------------
// Called when the application starts or restarts.
// Useful for storing global data
// 
void WINAPI DLLExport StartApp(mv _far *mV, CRunApp* pApp)
{
	OutputDebugString("Start app\n");

	// Example
	// -------
	// Delete global data (if restarts application)
//	CMyData* pData = (CMyData*)mV->mvGetExtUserData(pApp, hInstLib);
//	if ( pData != NULL )
//	{
//		delete pData;
//		mV->mvSetExtUserData(pApp, hInstLib, NULL);
//	}
}

// -------------------
// EndApp
// -------------------
// Called when the application ends.
// 
void WINAPI DLLExport EndApp(mv _far *mV, CRunApp* pApp)
{
	OutputDebugString("End app\n");

	// Example
	// -------
	// Delete global data
//	CMyData* pData = (CMyData*)mV->mvGetExtUserData(pApp, hInstLib);
//	if ( pData != NULL )
//	{
//		delete pData;
//		mV->mvSetExtUserData(pApp, hInstLib, NULL);
//	}
}

// -------------------
// StartFrame
// -------------------
// Called when the frame starts or restarts.
// 
void WINAPI DLLExport StartFrame(mv _far *mV, DWORD dwReserved, int nFrameIndex)
{
}

// -------------------
// EndFrame
// -------------------
// Called when the frame ends.
// 
void WINAPI DLLExport EndFrame(mv _far *mV, DWORD dwReserved, int nFrameIndex)
{
}


// ============================================================================
//
// DEBUGGER ROUTINES
// 
// ============================================================================

// -----------------
// GetDebugTree
// -----------------
// This routine returns the address of the debugger tree
//
LPWORD WINAPI DLLExport GetDebugTree(LPRDATA rdPtr)
{
#if !defined(RUN_ONLY)
	return DebugTree;
#else
	return NULL;
#endif // !defined(RUN_ONLY)
}

// -----------------
// GetDebugItem
// -----------------
// This routine returns the text of a given item.
//
void WINAPI DLLExport GetDebugItem(LPSTR pBuffer, LPRDATA rdPtr, int id)
{
#if !defined(RUN_ONLY)

	char temp[DB_BUFFERSIZE];//check the tooltip on this buffer - hover your mouse over it
	int slot=0;
	switch(id)//there
	{
	case DB_SLOT0:
	case DB_URL0:
	case DB_TOTALSIZE0:
	case DB_TOTALREAD0:
	case DB_SPEED0:
		slot=0;
		break;
	case DB_SLOT1:
	case DB_URL1:
	case DB_TOTALSIZE1:
	case DB_TOTALREAD1:
	case DB_SPEED1:
		slot=1;
		break;
	case DB_SLOT2:
	case DB_URL2:
	case DB_TOTALSIZE2:
	case DB_TOTALREAD2:
	case DB_SPEED2:
		slot=2;
		break;
	case DB_SLOT3:
	case DB_URL3:
	case DB_TOTALSIZE3:
	case DB_TOTALREAD3:
	case DB_SPEED3:
		slot=3;
		break;
	}

	switch (id)
	{
	case DB_SLOT0:
	case DB_SLOT1:
	case DB_SLOT2:
	case DB_SLOT3:
		LoadString(hInstLib, IDS_SLOT0+slot, pBuffer, DB_BUFFERSIZE);
		break;
	case DB_URL0:
	case DB_URL1:
	case DB_URL2:
	case DB_URL3:
		LoadString(hInstLib, IDS_URL, temp, DB_BUFFERSIZE);
		if (rdPtr->slots[slot].pURL!=NULL)
			wsprintf(pBuffer, temp, rdPtr->slots[slot].pURL);
		else
			wsprintf(pBuffer, temp, "");
		break;
	case DB_TOTALSIZE0:
	case DB_TOTALSIZE1:
	case DB_TOTALSIZE2:
	case DB_TOTALSIZE3:
		LoadString(hInstLib, IDS_TOTALSIZE, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->slots[slot].dwFileSize);
		break;
	case DB_TOTALREAD0:
	case DB_TOTALREAD1:
	case DB_TOTALREAD2:
	case DB_TOTALREAD3:
		LoadString(hInstLib, IDS_TOTALREAD, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->slots[slot].dwTotalRead);
		break;
	case DB_SPEED0:
	case DB_SPEED1:
	case DB_SPEED2:
	case DB_SPEED3:
		LoadString(hInstLib, IDS_SPEED, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->slots[slot].dwSpeed);
		break;
	}

#endif // !defined(RUN_ONLY)
}

// -----------------
// EditDebugItem
// -----------------
// This routine allows to edit editable items.
//
void WINAPI DLLExport EditDebugItem(LPRDATA rdPtr, int id)
{
#if !defined(RUN_ONLY)

#endif // !defined(RUN_ONLY)
}


