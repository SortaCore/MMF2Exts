
#include "Common.h"

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



// -------------------
// GetRunObjectSurface
// -------------------
// Implement this function instead of DisplayRunObject if your extension
// supports ink effects and transitions. Note: you can support ink effects
// in DisplayRunObject too, but this is automatically done if you implement
// GetRunObjectSurface (MMF applies the ink effect to the surface).
//
// Note: do not forget to enable the function in the .def file 
// if you remove the comments below.
/*
cSurface* WINAPI DLLExport GetRunObjectSurface(RUNDATA * rdPtr)
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
LPSMASK WINAPI DLLExport GetRunObjectCollisionMask(RUNDATA * rdPtr, LPARAM lParam)
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
static LONG WINAPI UnhandledExceptionCatcher(PEXCEPTION_POINTERS pExceptionPtrs)
{
	if (!pExceptionPtrs || !pExceptionPtrs->ExceptionRecord)
	{
		GlobalExt->OutputNow(5, -1, "Failed to catch crash, invalid pointers supplied.");
		return EXCEPTION_CONTINUE_SEARCH;
	}
	
	std::stringstream OutputThis;
	// Starting statment
	OutputThis << "*** Unhandled exception occured in the MMF2 program at address 0x"
			   << (void *)pExceptionPtrs->ExceptionRecord->ExceptionAddress
			   << " ***";
	GlobalExt->OutputNow(5, -1, OutputThis.str().c_str());
	if (Data->FileHandle)
		OutputThis.str("");
	else
		OutputThis << "\r\n";
	
	bool NoHandling = false;
	switch (pExceptionPtrs->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			OutputThis << "The thread tried to read from or write to a virtual address for which "
						  "it does not have the appropriate access.";
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			OutputThis << "The thread tried to access an array element that is out of bounds (and "
						  "the underlying hardware supports bounds checking)."; 
			break;
		case EXCEPTION_BREAKPOINT:
			OutputThis << "A breakpoint exception was encountered - the object will pass the "
						  "exception on.";
			NoHandling = true;
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			OutputThis << "The thread tried to read or write data that is misaligned on hardware "
						  "that does not provide alignment. For example, 16-bit values must be "
						  "aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and "
						  "so on.";
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			OutputThis << "One of the operands in a floating-point operation is denormal. A "
						  "denormal value is one that is too small to represent as a standard "
						  "floating-point value.";
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			OutputThis << "The thread tried to divide a floating-point value by a floating-point "
						  "divisor of zero.";
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			OutputThis << "The result of a floating-point operation cannot be represented exactly "
						  "as a decimal fraction.";
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			OutputThis << "This exception is an uncommon floating-point exception.";
			break;
		case EXCEPTION_FLT_OVERFLOW:
			OutputThis << "The exponent of a floating-point operation is greater than the magnitude "
						  "allowed by the corresponding type.";
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			OutputThis << "The stack overflowed or underflowed as the result of a floating-point "
						  "operation.";
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			OutputThis << "The exponent of a floating-point operation is less than the magnitude "
						  "allowed by the corresponding type.";
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			OutputThis << "The thread tried to execute an invalid instruction.";
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			OutputThis << "The thread tried to access a page that was not present, and the system "
						  "was unable to load the page. For example, this exception might occur if "
						  "a network connection is lost while running a program over the network.";
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			OutputThis << "The thread tried to divide an integer value by an integer divisor of zero.";
			break;
		case EXCEPTION_INT_OVERFLOW:
			OutputThis << "The result of an integer operation caused a carry out of the most significant "
						  "bit of the result.";
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			OutputThis << "An exception handler returned an invalid disposition to the exception dispatcher. "
						  "This is incredibly rare for C/C++ programs."; 
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			OutputThis << "The thread tried to continue after an exception, but this exception type makes it "
						  "impossible."/* " DebugObject will not attempt to continue after this error."*/;
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			OutputThis << "The thread tried to execute an instruction whose operation is not allowed in the "
						  "current machine mode.";
			break;
		case EXCEPTION_SINGLE_STEP:
			OutputThis << "A trace trap or other single-instruction mechanism signaled that one instruction "
						  "has been executed.";
			NoHandling = true;
			break;
		case EXCEPTION_STACK_OVERFLOW:
			OutputThis << "The thread used up its allocated stack memory.";
			break;
		default:
			OutputThis << "Unrecognised exception code [" << pExceptionPtrs->ExceptionRecord->ExceptionCode << "].";
			break;
	}
	if (!Data->FileHandle)
	{
		if (Data->DoMsgBoxIfPathNotSet)
			MessageBoxA(NULL, OutputThis.str().c_str(), "DebugObject - caught exception.", MB_OK | MB_ICONERROR);
	}
	else
		GlobalExt->OutputNow(5, -2, OutputThis.str().c_str());

	if (NoHandling || !Data)
		return EXCEPTION_CONTINUE_SEARCH;
	else
	{
		DWORD d = 0;
		switch (Data->HandleExceptionVia)
		{
			case GlobalData::HANDLE_VIA_QUIT:
				GlobalExt->OutputNow(5, -1, "Handling via quit.");
				ExitProcess(-1);
				return EXCEPTION_CONTINUE_SEARCH;

			case GlobalData::HANDLE_VIA_INFINITE_WAIT:
				GlobalExt->OutputNow(5, -1, "Handling via infinite sleep.");
				Sleep(INFINITE);
				GlobalExt->OutputNow(5, -1, "Passed infinite sleep.");
				return EXCEPTION_CONTINUE_SEARCH;

			case GlobalData::HANDLE_VIA_IGNORE:
				GlobalExt->OutputNow(5, -1, "Handling via passing on.");
				return EXCEPTION_CONTINUE_SEARCH;

			case GlobalData::HANDLE_VIA_CONTINUE:
				if (Data->ContinuesCount > 0)
				{
					--(Data->ContinuesCount);
				}
				else
				{
					if (Data->ContinuesCount == 0)
					{
						GlobalExt->OutputNow(5, -1, "Max continues expired; passing exception to debugger.");
						Data->ContinuesCount = Data->ContinuesMax;
						return EXCEPTION_CONTINUE_SEARCH;
					}
				}
				// No return, pass onto default

			default:
				GlobalExt->OutputNow(5, -1, "Handling via continuing execution.");
				return EXCEPTION_CONTINUE_EXECUTION;
		}
	}
/*
	// Otherwise
	DWORD rv;

    EXCEPTION_RECORD* per = pep->ExceptionRecord;

    if ( ( per->ExceptionCode == EXCEPTION_ACCESS_VIOLATION ) &&
         ( per->ExceptionInformation[0] != 0 ) )
    {
        rv = BasepCheckForReadOnlyResource( per->ExceptionInformation[1] );

        if ( rv == EXCEPTION_CONTINUE_EXECUTION )
            return EXCEPTION_CONTINUE_EXECUTION;
    }

    DWORD DebugPort = 0;

    rv = NtQueryInformationProcess( GetCurrentProcess(), ProcessDebugPort,
                                    &DebugPort, sizeof( DebugPort ), 0 );

    if ( ( rv >= 0 ) && ( DebugPort != 0 ) )
    {
        // Yes, it is -> Pass exception to the debugger
		OutputNow(5, -1, "*** Unhandled exception. Detected that a debug mode is enabled; passing control of unhandled exception to debugger. ***");
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Is custom filter for unhandled exceptions registered ?

    if ( BasepCurrentTopLevelFilter != 0 )
    {
        // Yes, it is -> Call the custom filter

        rv = (BasepCurrentTopLevelFilter)(pep);

        if ( rv == EXCEPTION_EXECUTE_HANDLER )
		{
			OutputNow(5, -1, "*** Unhandled exception. Caught and reported. ***");
            return EXCEPTION_EXECUTE_HANDLER;
		}

        if ( rv == EXCEPTION_CONTINUE_EXECUTION )
		{
			OutputNow(5, -1, "*** Unhandled exception. Detected that a debug mode is enabled; passing control of unhandled exception to debugger. ***");
            return EXCEPTION_CONTINUE_EXECUTION;
		}
    }   */

}
void WINAPI DLLExport StartApp(mv * mV, CRunApp* pApp)
{
	// Example
	// -------
	// Delete global data (if restarts application)
	// Initialise variables
	Data = (GlobalData *)mV->GetExtUserData(pApp, hInstLib);
	
	// Not initialised
	if (!Data)
	{
		// Create new container
		Data = new GlobalData;
		
		// Open lock
		Data->ReadingThis = true; // Open lock without checking if the handle is valid first
		
		// Mark data position
		mV->SetExtUserData(pApp, hInstLib, Data);
		
		// Initialise data
		Data->FileHandle = NULL;
		Data->DebugEnabled = false;
		Data->TimeFormat = (char *)calloc(255, sizeof(char));
		Data->RealTime = (char *)calloc(255, sizeof(char));
		strcpy_s(Data->TimeFormat, 255, "%X");
		Data->NumUsages = 1;
		Data->DoMsgBoxIfPathNotSet = false;
		Data->ConsoleEnabled = false;
		
		// Exception handling (container)
		Data->ContinuesCount = -1;
		Data->ContinuesMax = -1;
		CloseLock();

		// Exception handling (WinAPI call)
		SetUnhandledExceptionFilter(UnhandledExceptionCatcher);
	}
	else // Already initialised
	{
		OpenLock();
		++Data->NumUsages;
		CloseLock();
	}
}

// -------------------
// EndApp
// -------------------
// Called when the application ends.
// 
void WINAPI DLLExport EndApp(mv * mV, CRunApp* pApp)
{
	// Example
	// -------
	// Delete global data
	
	// Nothing to do if we don't have stored information
	// We can't output log closure or free resources, y'see.
	if (!Data)
	{
		SetUnhandledExceptionFilter(NULL);
		return;
	}
	
	// Output closure message
	if (GlobalExt)
		GlobalExt->OutputNow(1, -1, "*** Log closed. ***");
    
	// Open lock
	OpenLock();

	// Are we the last using this Data?
	if ((--Data->NumUsages) == 0)
	{
		// Close resources
		if (Data->FileHandle)
			fclose(Data->FileHandle);
		Data->FileHandle = NULL;

		// Close MMF pointer to Data
		mV->SetExtUserData(pApp, hInstLib, NULL);
		
		// Close lock
		CloseLock();

		// Close container
		delete Data;
		
		// Exception handling - invalidate
		SetUnhandledExceptionFilter(NULL);
	}
	else // Other extensions are using this
	{
		CloseLock();
	}
}

// -------------------
// StartFrame
// -------------------
// Called when the frame starts or restarts.
// 
void WINAPI DLLExport StartFrame(mv * mV, DWORD dwReserved, int nFrameIndex)
{
}

// -------------------
// EndFrame
// -------------------
// Called when the frame ends.
// 
void WINAPI DLLExport EndFrame(mv * mV, DWORD dwReserved, int nFrameIndex)
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

void WINAPI GetRunObjectFont(RUNDATA * rdPtr, LOGFONT* pLf)
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
void WINAPI SetRunObjectFont(RUNDATA * rdPtr, LOGFONT* pLf, RECT* pRc)
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
COLORREF WINAPI GetRunObjectTextColor(RUNDATA * rdPtr)
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
void WINAPI SetRunObjectTextColor(RUNDATA * rdPtr, COLORREF rgb)
{
	// Example
	// -------
	rdPtr->m_dwColor = rgb;
	InvalidateRect(rdPtr->m_hWnd, NULL, TRUE);
}
*/


// ============================================================================
//
// WINDOWPROC (interception of messages sent to hMainWin and hEditWin)
//
// Do not forget to enable the WindowProc function in the .def file if you implement it
// 
// ============================================================================
/*
// Get the pointer to the object's data from its window handle
// Note: the object's window must have been subclassed with a
// callRunTimeFunction(rdPtr, RFUNCTION_SUBCLASSWINDOW, 0, 0);
// See the documentation and the Simple Control example for more info.
//
RUNDATA * GetRdPtr(HWND hwnd, LPRH rhPtr)
{
	return (RUNDATA *)GetProp(hwnd, (LPCSTR)rhPtr->rh4.rh4AtomRd);
}

// Called from the window proc of hMainWin and hEditWin.
// You can intercept the messages and/or tell the main proc to ignore them.
//
LRESULT CALLBACK DLLExport WindowProc(LPRH rhPtr, HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
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
LPWORD WINAPI DLLExport GetDebugTree(RUNDATA * rdPtr)
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
void WINAPI DLLExport GetDebugItem(LPSTR pBuffer, RUNDATA * rdPtr, int id)
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
void WINAPI DLLExport EditDebugItem(RUNDATA * rdPtr, int id)
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


