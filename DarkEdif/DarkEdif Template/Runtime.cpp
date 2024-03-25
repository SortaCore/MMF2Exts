// ============================================================================
// This file contains routines that are handled during runtime; during both
// Run Application and in built EXEs.
// It includes various Fusion-triggered events, such as StartApp and EndApp,
// window message processing, and runtime display of your object.
//
// If your object does not use any of those, a simple non-displaying object,
// you can safely exclude this file.
// ============================================================================
#include "Common.hpp"


// ============================================================================
// RUNTIME DISPLAY: EFFECTS, TRANSISTIONS, COLLISION MASKS
// ============================================================================

// Implement this function instead of DisplayRunObject if your extension
// supports ink effects and transitions. Note: you can support ink effects
// in DisplayRunObject too, but this is automatically done if you implement
// GetRunObjectSurface (MMF applies the ink effect to the surface).
/*
cSurface * FusionAPI GetRunObjectSurface(RUNDATA * rdPtr)
{
	#pragma DllExportHint
	return NULL;
}
*/

// Implement this function if your extension supports fine collision mode (OEPREFS::FINE_COLLISIONS),
// Or if it's a background object and you want Obstacle properties for this object.
//
// You can return NULL if the object is opaque (without semi-transparency)
//
/*
LPSMASK FusionAPI GetRunObjectCollisionMask(RUNDATA * rdPtr, LPARAM lParam)
{
	#pragma DllExportHint
	// Typical example for active objects
	// ----------------------------------
	// Opaque? collide with box
	if ((rdPtr->rs.rsEffect & EFFECTFLAG_TRANSPARENT) == 0)	// Note: only if your object has the OEPREFS_INKEFFECTS option
		return NULL;

	// Transparent? Create mask
	LPSMASK pMask = rdPtr->m_pColMask;
	if (pMask == NULL)
	{
		if (rdPtr->m_pSurface != NULL )
		{
			unsigned int dwMaskSize = rdPtr->m_pSurface->CreateMask(NULL, lParam);
			if(dwMaskSize != 0 )
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


// ============================================================================
// START APP / END APP / START FRAME / END FRAME routines
// ============================================================================

/*
// Called when the application starts or restarts. Also called for subapps.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
void FusionAPI StartApp(mv *mV, CRunApp* pApp)
{
	#pragma DllExportHint
	// Whether this is a subapp can be checked by pApp->ParentApp != NULL
	// Example
	// -------
	// Delete global data (if restarts application)
//	CMyData* pData = (CMyData*)mV->mvGetExtUserData(pApp, hInstLib);
//	if ( pData != NULL )
//	{
//		delete pData;
//		mV->SetExtUserData(pApp, hInstLib, NULL);
//	}
}*/

/*
// Called when the application ends or restarts. Also called for subapps.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
void FusionAPI EndApp(mv *mV, CRunApp* pApp)
{
	#pragma DllExportHint
	// Example
	// -------
	// Delete global data
//	CMyData* pData = (CMyData*)mV->mvGetExtUserData(pApp, hInstLib);
//	if ( pData != NULL )
//	{
//		delete pData;
//		mV->SetExtUserData(pApp, hInstLib, NULL);
//	}
}*/

/*
// Called when the frame starts or restarts.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
void FusionAPI StartFrame(mv *mV, std::uint32_t dwReserved, int nFrameIndex)
{
	#pragma DllExportHint
}*/

/*
// Called when the frame ends.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
void FusionAPI EndFrame(mv *mV, std::uint32_t dwReserved, int nFrameIndex)
{
	#pragma DllExportHint
}*/


// ============================================================================
// TEXT ROUTINES (if OEFLAG_TEXT)
// ============================================================================

// Return the font used by the object.
/*
void FusionAPI GetRunObjectFont(RUNDATA * rdPtr, LOGFONT* pLf)
{
	#pragma DllExportHint
	// Example
	// -------
	// GetObject(rdPtr->m_hFont, sizeof(LOGFONT), pLf);
}

// Change the font used by the object.
void FusionAPI SetRunObjectFont(RUNDATA * rdPtr, LOGFONT* pLf, RECT* pRc)
{
	#pragma DllExportHint
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

// Return the text color of the object.
COLORREF FusionAPI GetRunObjectTextColor(RUNDATA * rdPtr)
{
	#pragma DllExportHint
	// Example
	// -------
	return 0;	// e.g. RGB()
}

// Change the text color of the object.
void FusionAPI SetRunObjectTextColor(RUNDATA * rdPtr, COLORREF rgb)
{
	#pragma DllExportHint
	// Example
	// -------
	rdPtr->m_dwColor = rgb;
	InvalidateRect(rdPtr->m_hWnd, NULL, TRUE);
}
*/


// ============================================================================
// WINDOWPROC (interception of messages sent to hMainWin and hEditWin)
// ============================================================================

/*
// Get the pointer to the object's data from its window handle
// Note: the object's window must have been subclassed with a
// callRunTimeFunction(rdPtr, RFUNCTION_SUBCLASSWINDOW, 0, 0);
// See the documentation and the Simple Control example for more info.
RUNDATA * FusionAPI GetRdPtr(HWND hwnd, RunHeader * rhPtr)
{
	#pragma DllExportHint
	return (RUNDATA *)GetProp(hwnd, (const char *)rhPtr->rh4.rh4AtomRd);
}

// Called from the window proc of hMainWin and hEditWin.
// You can intercept the messages and/or tell the main proc to ignore them.
LRESULT FusionAPI WindowProc(RunHeader * rhPtr, HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	#pragma DllExportHint

#ifndef VISUAL_EXTENSION
	// If you do not define this, DisplayRunObject() isn't exposed to Fusion runtime.
	// When a window is redisplayed, e.g. after resizing, all exts with OEFLAGS::WNDPROC are assumed to need their
	// DisplayRunObject functions called, and if it's missing for an ext, the Fusion runtime crashes.
	//
	// To test if it's working for you, simply make a Fusion application, drop your ext in,
	// change the application property to "resize display to fill window size" to true, then run the app and resize it.
	#error Define VISUAL_EXTENSION in project properties!
#endif

	RUNDATA * rdPtr = NULL;

	switch (nMsg) {

	// Example
	case WM_CTLCOLORSTATIC:
		{
			// Get the handle of the control
			HWND hWndControl = (HWND)lParam;

			// Get a pointer to the RUNDATA structure (see GetRdptr function above for more info)
			rdPtr = GetRdPtr(hWndControl, rhPtr);

			// Check if the rdPtr pointer is valid and points to an object created with this extension
			if ( rdPtr == NULL || rdPtr->rHo.hoIdentifier != IDENTIFIER )
				break;

			// OK, intercept the message
			HDC hDC = (HDC)wParam;
			SetBkColor(hDC, rdPtr->backColor);
			SetTextColor(hDC, rdPtr->fontColor);
			rhPtr->rh4.rh4KpxReturn = (long)rdPtr->hBackBrush;
			return REFLAG_MSGRETURNVALUE;
		}
		break;
	}

	return 0;
}
*/
