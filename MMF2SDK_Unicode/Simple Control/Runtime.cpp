// ============================================================================
//
// This file contains routines that are handled during the Runtime.
//
// Including creating, display, and handling your object.
//
// ============================================================================

// Common Include
#include	"common.h"

// DEBUGGER /////////////////////////////////////////////////////////////////

#if !defined(RUN_ONLY)
// Identifiers of items displayed in the debugger
enum
{
// Example
// -------
//	DB_CURRENTSTRING,
//	DB_CURRENTVALUE,
//	DB_CURRENTCHECK,
//	DB_CURRENTCOMBO
};

// Items displayed in the debugger
WORD DebugTree[]=
{
// Example
// -------
//	DB_CURRENTSTRING|DB_EDITABLE,
//	DB_CURRENTVALUE|DB_EDITABLE,
//	DB_CURRENTCHECK,
//	DB_CURRENTCOMBO,

	// End of table (required)
	DB_END
};

#endif // !defined(RUN_ONLY)


// --------------------
// GetRunObjectDataSize
// --------------------
// Returns the size of the runtime datazone of the object
//
ushort WINAPI DLLExport GetRunObjectDataSize(fprh rhPtr, LPEDATA edPtr)
{
	return(sizeof(RUNDATA));
}


// ---------------
// CreateRunObject
// ---------------
// The routine where the object is actually created
//
short WINAPI DLLExport CreateRunObject(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr)
{
/*
	This routine runs when your object is created, as you might have guessed.
	It is here that you must transfer any data you need in rdPtr from edPtr,
	because after this has finished you cannot access it again!
	Also, if you have anything to initialise (e.g. dynamic arrays, surface objects)
	you should do it here, and free your resources in DestroyRunObject.
*/
	LPRH rhPtr = rdPtr->rHo.hoAdRunHeader;

	rdPtr->rHo.hoX = cobPtr->cobX;
	rdPtr->rHo.hoY = cobPtr->cobY;
	rdPtr->rHo.hoImgWidth = edPtr->nWidth;
	rdPtr->rHo.hoImgHeight = edPtr->nHeight;

	// Initialize RUNDATA members
	rdPtr->textColor = edPtr->textColor;
	rdPtr->backColor = edPtr->backColor;
	rdPtr->dwAlignFlags = edPtr->dwAlignFlags;
	rdPtr->dwFlags = edPtr->dwFlags;
	rdPtr->hFont = CreateFontIndirect(&edPtr->textFont);
	rdPtr->dwLastChangedLoopNumber = -1;
	rdPtr->dwEvtFlags = 0;

	// Create control window
	DWORD dwStyle = WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE;
	if ((edPtr->dwFlags & SCTRL_BORDER) != 0)
		dwStyle |= WS_BORDER;
	rdPtr->hWnd = CreateWindowEx(0, _T("EDIT"), NULL, dwStyle,
									rdPtr->rHo.hoX - rhPtr->rhWindowX, rdPtr->rHo.hoY - rhPtr->rhWindowY,
									rdPtr->rHo.hoImgWidth, rdPtr->rHo.hoImgHeight,
									rhPtr->rhHEditWin, NULL, rhPtr->rh4.rh4Instance, NULL);

	// Set text & font
	if ( rdPtr->hWnd != NULL )
	{
		// Let MMF subclass window
		rdPtr->nWnds = 1;
		rdPtr->rHo.hoOffsetToWindows = (short)((LPBYTE)&rdPtr->nWnds - (LPBYTE)rdPtr);
		callRunTimeFunction(rdPtr, RFUNCTION_SUBCLASSWINDOW, 0, 0);

		// Create background brush
		rdPtr->hBrush = NULL;
		if ( (rdPtr->dwFlags & SCTRL_SYSTEMCOLORS) == 0 )
			rdPtr->hBrush = CreateSolidBrush(rdPtr->backColor);

		// Set font
		SendMessage(rdPtr->hWnd, WM_SETFONT, (WPARAM)rdPtr->hFont, MAKELONG(TRUE, 0));

		// Set Text
		SetWindowText(rdPtr->hWnd, edPtr->sText);
	}

	// No errors
	return 0;
}


// ----------------
// DestroyRunObject
// ----------------
// Destroys the run-time object
//
short WINAPI DLLExport DestroyRunObject(LPRDATA rdPtr, long fast)
{
/*
	When your object is destroyed (either with a Destroy action or at the end of
	the frame) this routine is called. You must free any resources you have allocated!
*/
	// Destroy
	if (rdPtr->hWnd != NULL)
		DestroyWindow(rdPtr->hWnd);

	// Destroy font
	if (rdPtr->hFont != NULL)
		DeleteObject(rdPtr->hFont);

	// Destroy brush
	if (rdPtr->hBrush != NULL)
		DeleteObject(rdPtr->hBrush);

	// No errors
	return 0;
}


// ----------------
// HandleRunObject
// ----------------
// Called (if you want) each loop, this routine makes the object live
//
short WINAPI DLLExport HandleRunObject(LPRDATA rdPtr)
{
	// If text has been modified, generate a CND_CHANGED condition
	if ( (rdPtr->dwEvtFlags & EVTFLAG_CHANGED) != 0 )
	{
		callRunTimeFunction(rdPtr, RFUNCTION_PUSHEVENTSTOP, CND_CHANGED, 0);
		rdPtr->dwEvtFlags &= ~EVTFLAG_CHANGED;
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
void WINAPI GetRunObjectFont(LPRDATA rdPtr, LOGFONT* pLf)
{
	GetObject(rdPtr->hFont, sizeof(LOGFONT), pLf);
}

// -------------------
// SetRunObjectFont
// -------------------
// Change the font used by the object.
//
void WINAPI SetRunObjectFont(LPRDATA rdPtr, LOGFONT* pLf, RECT* pRc)
{
	HFONT hFont = CreateFontIndirect(pLf);
	if ( hFont != NULL )
	{
		// Change font
		if (rdPtr->hFont!=0)
			DeleteObject(rdPtr->hFont);
		rdPtr->hFont = hFont;

		// Redraw object
		callRunTimeFunction(rdPtr, RFUNCTION_REDRAW, 0, 0);
	}

}

// ---------------------
// GetRunObjectTextColor
// ---------------------
// Return the text color of the object.
//
COLORREF WINAPI GetRunObjectTextColor(LPRDATA rdPtr)
{
	return rdPtr->textColor;
}

// ---------------------
// SetRunObjectTextColor
// ---------------------
// Change the text color of the object.
//
void WINAPI SetRunObjectTextColor(LPRDATA rdPtr, COLORREF rgb)
{
	rdPtr->textColor = rgb;

	// Redraw object
	callRunTimeFunction(rdPtr, RFUNCTION_REDRAW, 0, 0);
}



// ============================================================================
//
// WINDOWPROC (interception of messages sent to hMainWin and hEditWin)
//
// Do not forget to enable the WindowProc function in the .def file if you implement it
//
// ============================================================================

// Get the pointer to the object's data from its window handle
// Note: the object's window must have been subclassed with a
// callRunTimeFunction(rdPtr, RFUNCTION_SUBCLASSWINDOW, 0, 0);
// See the documentation and the Simple Control example for more info.
//
LPRDATA GetRdPtr(HWND hwnd, LPRH rhPtr)
{
	return (LPRDATA)GetProp(hwnd, (LPCTSTR)rhPtr->rh4.rh4AtomRd);
}

// Called from the window proc of hMainWin and hEditWin.
// You can intercept the messages and/or tell the main proc to ignore them.
//
LRESULT CALLBACK DLLExport WindowProc(fprh rhPtr, HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	LPRDATA rdPtr = NULL;

	switch (nMsg) {

	// Custom colors
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		{
			// Get the handle of the control
			HWND hWndControl = (HWND)lParam;

			// Get a pointer to the RUNDATA structure (see GetRdptr function above for more info)
			rdPtr = GetRdPtr(hWndControl, rhPtr);

			// Check if the rdPtr pointer is valid and points to an object created with this extension
			if ( rdPtr == NULL || rdPtr->rHo.hoIdentifier != IDENTIFIER )
				break;

			// OK, intercept the message if the object doesn't use system colors
			if ( (rdPtr->dwFlags & SCTRL_SYSTEMCOLORS) == 0 )
			{
				HDC hDC = (HDC)wParam;
				SetBkColor(hDC, rdPtr->backColor);
				SetTextColor(hDC, rdPtr->textColor);
				rhPtr->rh4.rh4KpxReturn = (long)rdPtr->hBrush;
				return REFLAG_MSGRETURNVALUE;
			}
		}
		break;

	case WM_COMMAND:
		// The text in an edit box has been modified ?
		if ( HIWORD(wParam) == EN_CHANGE )
		{
			// Is it this edit control?
			if ( (rdPtr = GetRdPtr((HWND)lParam, rhPtr)) != NULL && rdPtr->rHo.hoIdentifier == IDENTIFIER )
			{
				// Yes => set flag to trigger a CND_CHANGED condition in the next call to HandleRunObject
				rdPtr->dwLastChangedLoopNumber = rhPtr->rh4.rh4EventCount;
				rdPtr->dwEvtFlags |= EVTFLAG_CHANGED;
			}
		}
		break;
	}

	return 0;
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
void WINAPI DLLExport GetDebugItem(LPTSTR pBuffer, LPRDATA rdPtr, int id)
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
			wsprintf(pBuffer, temp, _T("TRUE"));
		else
			wsprintf(pBuffer, temp, _T("FALSE"));
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
			TCHAR buffer[256];

			dbi.pText=buffer;
			dbi.lText=TEXT_MAX;
			dbi.pTitle=NULL;

			_tcscpy(buffer, rdPtr->text);
			long ret=callRunTimeFunction(rdPtr, RFUNCTION_EDITTEXT, 0, (LPARAM)&dbi);
			if (ret)
				_tcscpy(rdPtr->text, dbi.pText);
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


