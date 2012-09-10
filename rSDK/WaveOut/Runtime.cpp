// ============================================================================
//
// This file contains routines that are handled during the Runtime
// 
// ============================================================================

// Common Include
#include	"common.h"

// --------------------
// GetRunObjectDataSize
// --------------------
// Returns the size of the runtime datazone of the object
// 
short WINAPI DLLExport GetRunObjectDataSize(fprh rhPtr, LPEDATA edPtr)
{
	return(sizeof(RUNDATA));
}

volatile float iii = 0; // Remove this, it's used for the frequency modulation
unsigned long WINAPI audioThreadProc(void* argument)
{
	RUNDATA* rdPtr = (RUNDATA*)argument;

	/* Keep busy */
	while(!rdPtr->shuttingDown)
	{
		/* Find all buffers that aren't currently queued */
		for(int i = 0; i < NUM_BUFFERS; ++i)
		{
			/* Not in use, fill it up and queue it! */
			if((rdPtr->buffers[i].dwFlags & WHDR_INQUEUE) == 0)
			{
				/**** Right here, you'd normally ask the sound engine to give you some data *****/

				for(unsigned int j = 0; j < rdPtr->buffers[i].dwBufferLength/sizeof(short); ++j)
				{
					/* Simple frequency modulation */
					/***** This is the part where you would fill in the values *****/
					((short*)rdPtr->buffers[i].lpData)[j] = sin(iii += 0.3+0.2*sin(iii*0.001))*32000;
				}

				/* Queue the buffer */
				waveOutWrite(rdPtr->device, &rdPtr->buffers[i], sizeof(WAVEHDR));
			}
		}

		/*	Do nothing for 100ms - With NUM_BUFFERS set to 10, each covering a 60th second,
			that's 0.166ms, so we should have about 0.66ms left each time the stream
			attempts to queue new buffers. You should sleep a bit shorter than the playback
			of all buffers in a row takes so that the computer actually has time to keep up
			and doesn't need to rush. Some older computers will appreciate this!			*/
		Sleep(100);
	}

	return 0;
}

// ---------------
// CreateRunObject
// ---------------
// The routine where the object is actually created
// 
short WINAPI DLLExport CreateRunObject(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr)
{
	#include "rCreateRunObject.h"

	/* You can remove this, but if you build your extension as Debug you can printf to a console! */
	#ifdef _DEBUG
		AllocConsole();
		freopen("conin$","r",stdin);
		freopen("conout$","w",stdout);
		freopen("conout$","w",stderr);
	#endif

	/* Set up stream format */
	WAVEFORMATEX format;
	format.cbSize = sizeof(WAVEFORMATEX);
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;			// Mono
	format.wBitsPerSample = 16;		// 16-bit
	format.nSamplesPerSec = 44100;	// 44.1 khz
	format.nBlockAlign = format.nChannels*format.wBitsPerSample/8;
	format.nAvgBytesPerSec = format.nBlockAlign*format.nSamplesPerSec;

	/* Open device with callback */
	if(MMSYSERR_NOERROR != waveOutOpen(&rdPtr->device, WAVE_MAPPER, &format, 0, 0, 0))
		return -1;

	/* Prepare audio buffers used for streaming */
	for(int i = 0; i < NUM_BUFFERS; ++i)
	{
		memset(&rdPtr->buffers[i], 0, sizeof(WAVEHDR));
		rdPtr->buffers[i].dwBufferLength = format.nBlockAlign*format.nSamplesPerSec; // Exactly one second per buffer since 44100 samples at 44100 khz is one

		/* Allocate PCM data buffer */
		rdPtr->buffers[i].lpData = new char[rdPtr->buffers[i].dwBufferLength];

		/* Prepare for usage */
		if(MMSYSERR_NOERROR != waveOutPrepareHeader(rdPtr->device, &rdPtr->buffers[i], sizeof(WAVEHDR)))
			return -1;
	}

	/* Thread signal to shut down */
	rdPtr->shuttingDown = false;

	/* Run the actual streaming thread */
	if(NULL == (rdPtr->thread = CreateThread(0, 0, audioThreadProc, rdPtr, 0, 0)))
		return -1;

	return 0;
}



// ----------------
// DestroyRunObject
// ----------------
// Destroys the run-time object
// 
short WINAPI DLLExport DestroyRunObject(LPRDATA rdPtr, long fast)
{
	/* Immediately stop playback because the buffers will be gone soon */
	waveOutPause(rdPtr->device);

	/* Tell thread to stop */
	rdPtr->shuttingDown = true;

	/* Wait for thread to finish */
	WaitForSingleObject(rdPtr->thread, INFINITE);
	CloseHandle(rdPtr->thread);
	
	/* Free all buffers */
	for(int i = 0; i < NUM_BUFFERS; ++i)
	{
		waveOutUnprepareHeader(rdPtr->device, &rdPtr->buffers[i], sizeof(WAVEHDR));
		delete[] rdPtr->buffers[i].lpData;
	}

	/* Close audio device */
	waveOutClose(rdPtr->device);

	/* Good bye! */
	delete rdPtr->rRd;
	return 0;
}


///* This callback makes sure we get to know once a queued audio buffer (via waveOutWrite) has finished! */
//void CALLBACK waveOutProc(HWAVEOUT device, UINT msg, DWORD instance, DWORD param1, DWORD param2)
//{
//	/* Buffer playback completed, decrease use count */
//	if(msg == WOM_DONE)
//		((RUNDATA*)instance)->queuedBuffers--;
//	printf("%d\n", ((RUNDATA*)instance)->queuedBuffers);
//}

// ----------------
// HandleRunObject
// ----------------
// Called (if you want) each loop, this routine makes the object live
// 
short WINAPI DLLExport HandleRunObject(LPRDATA rdPtr)
{
	return REFLAG_ONESHOT;
}

// ----------------
// DisplayRunObject
// ----------------
// Draw the object in the application screen.
// 
short WINAPI DLLExport DisplayRunObject(LPRDATA rdPtr)
{
/*
   If you return REFLAG_DISPLAY in HandleRunObject this routine will run.
   See Graphic_Object_Ex.txt for an example of what you may put here.
*/
	// Ok
	return 0;
}

// -------------------
// GetRunObjectSurface
// -------------------
// Implement this function instead of DisplayRunObject if your extension
// supports ink effects and transitions. Note: you can support ink effects
// in DisplayRunObject too, but this is automatically done if you implement
// GetRunObjectSurface (MMF applies the ink effect to the transition).
//
// Note: do not forget to enable the function in the .def file 
// if you remove the comments below.
/*
cSurface* WINAPI DLLExport GetRunObjectSurface(LPRDATA rdPtr)
{
	return NULL;
}
*/

// -------------------------
// GetRunObjectCollisionMask
// -------------------------
// Implement this function if your extension supports fine collision mode (OEPREFS_FINECOLLISIONS),
// Or if it's a background object and you want Obstacle properties for this object.
//
// Should return NULL if the object is not transparent.
//
// Note: do not forget to enable the function in the .def file 
// if you remove the comments below.
//
/*
cSurface* WINAPI DLLExport GetRunObjectCollisionMask(LPRDATA rdPtr, LPARAM lParam)
{
	// Typical example for active objects
	// ----------------------------------
	// Opaque? collide with box
	if ( (rdPtr->rs.rsEffect & EFFECTFLAG_TRANSPARENT) == 0 )	// Note: only if your object has the OEPREFS_INKEFFECTS option
		return NULL;

	// Transparent? Create mask
	LPSMASK pMask = rdPtr->m_pColMask;
	if ( pMask == NULL )
	{
		if ( rdPtr->m_pSurface != NULL )
		{
			DWORD dwMaskSize = rdPtr->m_pSurface->CreateMask(NULL, lParam);
			if ( dwMaskSize != 0 )
			{
				pMask = (LPSMASK)calloc(dwMaskSize, 1);
				if ( pMask != NULL )
				{
					rdPtr->m_pSurface->CreateMask(pMask, lParam);
					rdPtr->m_pColMask = pMask;
				}
			}
		}
	}

	// Note: for active objects, lParam is always the same.
	// For background objects (OEFLAG_BACKGROUND), lParam maybe be different if the user uses your object
	// as obstacle and as platform. In this case, you should store 2 collision masks
	// in your data: one if lParam is 0 and another one if lParam is different from 0.

	return pMask;
}
*/

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
// TEXT ROUTINES (if OEFLAG_TEXT)
// 
// ============================================================================

// -------------------
// GetRunObjectFont
// -------------------
// Return the font used by the object.
// 
/*

  // Note: do not forget to enable the functions in the .def file 
  // if you remove the comments below.

void WINAPI GetRunObjectFont(LPRDATA rdPtr, LOGFONT* pLf)
{
	// Example
	// -------
	// GetObject(rdPtr->m_hFont, sizeof(LOGFONT), pLf);
}

// -------------------
// SetRunObjectFont
// -------------------
// Change the font used by the object.
// 
void WINAPI SetRunObjectFont(LPRDATA rdPtr, LOGFONT* pLf, RECT* pRc)
{
	// Example
	// -------
//	HFONT hFont = CreateFontIndirect(pLf);
//	if ( hFont != NULL )
//	{
//		if (rdPtr->m_hFont!=0)
//			DeleteObject(rdPtr->m_hFont);
//		rdPtr->m_hFont = hFont;
//		SendMessage(rdPtr->m_hWnd, WM_SETFONT, (WPARAM)rdPtr->m_hFont, FALSE);
//	}

}

// ---------------------
// GetRunObjectTextColor
// ---------------------
// Return the text color of the object.
// 
COLORREF WINAPI GetRunObjectTextColor(LPRDATA rdPtr)
{
	// Example
	// -------
	return 0;	// rdPtr->m_dwColor;
}

// ---------------------
// SetRunObjectTextColor
// ---------------------
// Change the text color of the object.
// 
void WINAPI SetRunObjectTextColor(LPRDATA rdPtr, COLORREF rgb)
{
	// Example
	// -------
	rdPtr->m_dwColor = rgb;
	InvalidateRect(rdPtr->m_hWnd, NULL, TRUE);
}
*/


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

	// Example
	// -------
/*
	char temp[DB_BUFFERSIZE];

	switch (id)
	{
	case DB_CURRENTSTRING:
		LoadString(hInstLib, IDS_CURRENTSTRING, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->text);
		break;
	case DB_CURRENTVALUE:
		LoadString(hInstLib, IDS_CURRENTVALUE, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->value);
		break;
	case DB_CURRENTCHECK:
		LoadString(hInstLib, IDS_CURRENTCHECK, temp, DB_BUFFERSIZE);
		if (rdPtr->check)
			wsprintf(pBuffer, temp, "TRUE");
		else
			wsprintf(pBuffer, temp, "FALSE");
		break;
	case DB_CURRENTCOMBO:
		LoadString(hInstLib, IDS_CURRENTCOMBO, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->combo);
		break;
	}
*/

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

	// Example
	// -------
/*
	switch (id)
	{
	case DB_CURRENTSTRING:
		{
			EditDebugInfo dbi;
			char buffer[256];

			dbi.pText=buffer;
			dbi.lText=TEXT_MAX;
			dbi.pTitle=NULL;

			strcpy(buffer, rdPtr->text);
			long ret=callRunTimeFunction(rdPtr, RFUNCTION_EDITTEXT, 0, (LPARAM)&dbi);
			if (ret)
				strcpy(rdPtr->text, dbi.pText);
		}
		break;
	case DB_CURRENTVALUE:
		{
			EditDebugInfo dbi;

			dbi.value=rdPtr->value;
			dbi.pTitle=NULL;

			long ret=callRunTimeFunction(rdPtr, RFUNCTION_EDITINT, 0, (LPARAM)&dbi);
			if (ret)
				rdPtr->value=dbi.value;
		}
		break;
	}
*/
#endif // !defined(RUN_ONLY)
}


