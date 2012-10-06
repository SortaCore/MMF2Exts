
// ============================================================================
//
// This file contains routines that are handled only during the Edittime,
// under the Frame and Event editors.
//
// Including creating, display, and setting up your object.
// 
// ============================================================================

#include "Common.h"

#if !defined(RUN_ONLY)

// PROPERTIES /////////////////////////////////////////////////////////////////

// Property identifiers
enum {
	PROPID_SETTINGS = PROPID_EXTITEM_CUSTOM_FIRST,

	PROPID_ENABLE_AT_START,
	PROPID_MSGBOX_IF_PATH_INVALID,
	PROPID_INITIALPATH,	
	PROPID_CONSOLE_ENABLED,

};

// Example of content of the PROPID_COMBO combo box
//LPCSTR ComboList[] = {
//	0,	// reserved
//	MAKEINTRESOURCE(IDS_FIRSTOPTION),	
//	MAKEINTRESOURCE(IDS_SECONDOPTION),	
//	MAKEINTRESOURCE(IDS_THIRDOPTION),	
//	NULL
//};



#endif // !defined(RUN_ONLY)


// ============================================================================
//
// ROUTINES USED UNDER FRAME EDITOR
// 
// ============================================================================


// --------------------
// MakeIcon
// --------------------
// Called once object is created or modified, just after setup.
// Also called before showing the "Insert an object" dialog if your object
// has no icon resource

int WINAPI DLLExport MakeIconEx(mv * mV, cSurface* pIconSf, LPTSTR lpName, ObjectInfo * oiPtr, EDITDATA * edPtr )
{
#ifndef RUN_ONLY

    pIconSf->Delete();
    pIconSf->Clone(*SDK->Icon);

    pIconSf->SetTransparentColor(RGB(255, 0, 255));

   return 0;

#endif // !defined(RUN_ONLY)
}


// --------------------
// CreateObject
// --------------------
// Called when you choose "Create new object". It should display the setup box 
// and initialize everything in the datazone.

int WINAPI DLLExport CreateObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr)
{
#ifndef RUN_ONLY

	// Check compatibility
	if ( IS_COMPATIBLE(mV) )
	{
        Edif::Init(mV, edPtr);

		// Set default object settings
		edPtr->DoMsgBoxIfPathNotSet = false;
		edPtr->EnableAtStart = false;
		memset(edPtr->InitialPath, '\0', MAX_PATH+1);
		edPtr->ConsoleEnabled = false;

        return 0;
	}
#endif // !defined(RUN_ONLY)

	// Error
	return -1;
}

// --------------------
// EditObject
// --------------------
// Called when the user selects the Edit command in the object's popup menu
//
BOOL WINAPI EditObject (mv * mV, ObjectInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr)
{
#ifndef RUN_ONLY
	// Check compatibility
	if ( IS_COMPATIBLE(mV) )
	{
	
	}
#endif // !defined(RUN_ONLY)
	return FALSE;
}

// --------------------
// SetEditSize
// --------------------
// Called when the object has been resized
//
// Note: remove the comments if your object can be resized (and remove the comments in the .def file)
/*
BOOL WINAPI SetEditSize(LPMV mv, EDITDATA * edPtr, int cx, int cy)
{
#ifndef RUN_ONLY
	edPtr->swidth = cx;
	edPtr->sheight = cy;
#endif // !defined(RUN_ONLY)
	return TRUE;	// OK
}
*/

// --------------------
// PutObject
// --------------------
// Called when each individual object is dropped in the frame.
//
void WINAPI	DLLExport PutObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr, unsigned short cpt)
{
#ifndef RUN_ONLY
#endif // !defined(RUN_ONLY)
}

// --------------------
// RemoveObject
// --------------------
// Called when each individual object is removed from the frame.
//
void WINAPI	DLLExport RemoveObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr, unsigned short cpt)
{
#ifndef RUN_ONLY
	// Is the last object removed?
    if (0 == cpt)
	{
        Edif::Free(edPtr);

		// Do whatever necessary to remove our data
	}
#endif // !defined(RUN_ONLY)
}

// --------------------
// DuplicateObject
// --------------------
// Called when an object is created from another one (note: should be called CloneObject instead...)
//
void WINAPI DLLExport DuplicateObject(mv * mV, ObjectInfo * oiPtr, EDITDATA * edPtr)
{
#ifndef RUN_ONLY
#endif // !defined(RUN_ONLY)
}

// --------------------
// GetObjectRect
// --------------------
// Returns the size of the rectangle of the object in the frame editor.
//
void WINAPI DLLExport GetObjectRect(mv * mV, RECT * rc, LevelObject * loPtr, EDITDATA * edPtr)
{
#ifndef RUN_ONLY
	rc->right = rc->left + SDK->Icon->GetWidth();	// edPtr->swidth;
	rc->bottom = rc->top + SDK->Icon->GetHeight();	// edPtr->sheight;
#endif // !defined(RUN_ONLY)
	return;
}


// --------------------
// EditorDisplay
// --------------------
// Displays the object under the frame editor
//

cSurface * DLLExport WinGetSurface (int idWin, int surfID = 0);
void WINAPI DLLExport EditorDisplay(mv * mV, ObjectInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr, RECT FAR *rc)
{
#ifndef RUN_ONLY

	cSurface * Surface = WinGetSurface((int) mV->IdEditWin);

	if (!Surface)
        return;

    SDK->Icon->Blit(*Surface, rc->left, rc->top, BMODE_TRANSP, BOP_COPY, 0);

#endif // !defined(RUN_ONLY)
}


// --------------------
// IsTransparent
// --------------------
// This routine tells MMF2 if the mouse pointer is over a transparent zone of the object.
// 

extern "C" BOOL WINAPI DLLExport IsTransparent(mv * mV, LevelObject * loPtr, EDITDATA * edPtr, int dx, int dy)
{
#ifndef RUN_ONLY
	// Write your code here
#endif // !defined(RUN_ONLY)
	return FALSE;
}

// --------------------
// PrepareToWriteObject
// --------------------
// Just before writing the datazone when saving the application, MMF2 calls this routine.
// 
void WINAPI	DLLExport PrepareToWriteObject(mv * mV, EDITDATA * edPtr, ObjectInfo * adoi)
{
#ifndef RUN_ONLY

	// Write your code here
#endif // !defined(RUN_ONLY)
}

// --------------------
// GetFilters
// --------------------

BOOL WINAPI GetFilters(mv * mV, EDITDATA * edPtr, DWORD dwFlags, LPVOID pReserved)
{
#ifndef RUN_ONLY
	// If your extension uses image filters
//	if ( (dwFlags & GETFILTERS_IMAGES) != 0 )
//		return TRUE;

	// If your extension uses sound filters
//	if ( (dwFlags & GETFILTERS_SOUNDS) != 0 )
//		return TRUE;
#endif // RUN_ONLY
	return FALSE;
}

// --------------------
// UsesFile
// --------------------
// Triggers when a file is dropped onto the frame
// Return TRUE if you can create an object from the given file
//
BOOL WINAPI	DLLExport UsesFile (mv * mV, LPTSTR fileName)
{
	BOOL r = FALSE;
#ifndef RUN_ONLY

	// Example: return TRUE if file extension is ".txt"
/*	
	LPSTR	ext, npath;

	if ( fileName != NULL )
	{
		if ( (ext=(LPSTR)malloc(_MAX_EXT)) != NULL )
		{
			if ( (npath=(LPSTR)malloc(_MAX_PATH)) != NULL )
			{
				strcpy(npath, fileName);
				_splitpath(npath, NULL, NULL, NULL, ext);
				if ( _stricmp(ext, ".txt") == 0 )
					r = TRUE;
				free(npath);
			}
			free(ext);
		}
	} */
#endif // !defined(RUN_ONLY)
	return r;
}


// --------------------
// CreateFromFile
// --------------------
// Creates a new object from file
//
void WINAPI DLLExport CreateFromFile (mv * mV, LPTSTR fileName, EDITDATA * edPtr)
{
	#ifndef RUN_ONLY
		if (!edPtr || !fileName)
		{
			MessageBoxA(NULL, "CreateFromFile() failed: invalid calling parameters.", 
						"DebugObject load error", MB_OK | MB_ICONERROR);
			goto ErrorWipe;
		}
		// Initialise your extension data from the given file
		FILE * File = NULL;
		while (fopen_s(&File, fileName, "rb") || !File)
		{
			// Retry/cancel message box so user can choose whether to retry. Loops until cancel pressed or successful open.
			if (IDCANCEL == MessageBoxA(NULL, "CreateFromFile() failed: could not open filename.", 
										"DebugObject load error", MB_RETRYCANCEL | MB_ICONERROR | MB_DEFBUTTON2))
				goto ErrorWipe;
		}

		// Goto end of file
		fseek(File, 0, SEEK_END);
	
		long FileSize = ftell(File);

		// File size should be 2 chars and a null-terminated string, limited to MAX_PATH bytes + null
		if (FileSize < 3+0+1 || FileSize > 3+MAX_PATH+1)
		{
			MessageBoxA(NULL, "CreateFromFile() failed: File size is invalid.", "DebugObject load error", MB_OK | MB_ICONERROR | MB_DEFBUTTON2);
			fclose(File);
			goto ErrorWipe;
		}
		char Betsy[3+MAX_PATH+1];

		// Didn't read all the bytes
		if (FileSize != fread_s(File, 2+MAX_PATH+1, sizeof(char), FileSize, File))
		{
			MessageBoxA(NULL, "CreateFromFile() failed: reading from file encountered error.", "DebugObject load error", MB_OK | MB_ICONERROR | MB_DEFBUTTON2);
			fclose(File);
			goto ErrorWipe;
		}

		// String property is the only read operation that can encounter an error, so do it first
		if (strcpy_s(edPtr->InitialPath, MAX_PATH, &Betsy[3]))
		{
			std::stringstream s; s << "CreateFromFile() failed: reading from file encountered error " << errno << ".";
			MessageBoxA(NULL, s.str().c_str(), "DebugObject load error", MB_OK | MB_ICONERROR | MB_DEFBUTTON2);
			fclose(File);
			goto ErrorWipe;
		}
		
		// Read other variables from file
		edPtr->EnableAtStart = Betsy[0] != 0;
		edPtr->DoMsgBoxIfPathNotSet = Betsy[1] != 0;
		edPtr->ConsoleEnabled = Betsy[2] != 0;

		// Cleanup
		fclose(File);
		return;

		// On error, wipe to default variables
		ErrorWipe:
			edPtr->DoMsgBoxIfPathNotSet = false;
			edPtr->EnableAtStart = false;
			edPtr->ConsoleEnabled = false;
			memset(edPtr->InitialPath, '\0', MAX_PATH+1);


	#endif // #ifndef(RUN_ONLY)
}

// ============================================================================
//
// PROPERTIES
// 
// ============================================================================

// --------------------
// GetProperties
// --------------------
// Inserts properties into the properties of the object.
//
BOOL WINAPI DLLExport GetProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
	#ifndef RUN_ONLY
		mvInsertProps(mV, edPtr, SDK->EdittimeProperties, PROPID_TAB_GENERAL, TRUE);
	#endif // !defined(RUN_ONLY)

	// OK
	return TRUE;
}

// --------------------
// ReleaseProperties
// --------------------
// Called when the properties are removed from the property window.
//
void WINAPI DLLExport ReleaseProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#ifndef RUN_ONLY
	// Write your code here
#endif // !defined(RUN_ONLY)
}

// --------------------
// GetPropCreateParam
// --------------------
// Called when a property is initialized and its creation parameter is NULL (in the PropData).
// Allows you, for example, to change the content of a combobox property according to specific settings in the EDITDATA structure.
//
LPARAM WINAPI DLLExport GetPropCreateParam(mv * mV, EDITDATA * edPtr, UINT nPropID)
{
#ifndef RUN_ONLY
	// Example
	// -------
//	if ( nPropID == PROPID_COMBO )
//	{
//		switch (edPtr->sType)
//		{
//		case TYPE1:
//			return (LPARAM)ComboList1;
//		case TYPE2:
//			return (LPARAM)ComboList2;
//		}
//	}
#endif // !defined(RUN_ONLY)
	return NULL;
}

// ----------------------
// ReleasePropCreateParam
// ----------------------
// Called after a property has been initialized.
// Allows you, for example, to free memory allocated in GetPropCreateParam.
//
void WINAPI DLLExport ReleasePropCreateParam(mv * mV, EDITDATA * edPtr, UINT nPropID, LPARAM lParam)
{
#ifndef RUN_ONLY
#endif // !defined(RUN_ONLY)
}

// --------------------
// GetPropValue
// --------------------
// Returns the value of properties that have a value.
// Note: see GetPropCheck for checkbox properties
//
LPVOID WINAPI DLLExport GetPropValue(mv * mV, EDITDATA * edPtr, UINT nPropID)
{
#ifndef RUN_ONLY
	// Example
	// -------
	switch (nPropID) {
//
//	// Returns a color.
//	case PROPID_COLOR:
//		return new CPropDWordValue(edPtr->dwColor);
//
//	// Returns a string
	case PROPID_INITIALPATH:
		return new Prop_AStr(&edPtr->InitialPath[0]);
//
//	// Returns the value of the combo box
//	case PROPID_COMBO:
//		return new CPropDWordValue(edPtr->nComboIndex);
	}

#endif // !defined(RUN_ONLY)
	return NULL;
}

// --------------------
// GetPropCheck
// --------------------
// Returns the checked state of properties that have a check box.
//
BOOL WINAPI DLLExport GetPropCheck(mv * mV, EDITDATA * edPtr, UINT nPropID)
{
#ifndef RUN_ONLY
	// Example
	// -------
	switch (nPropID) {
	// Return 0 (unchecked) or 1 (checked)
	case PROPID_ENABLE_AT_START:
		return edPtr->EnableAtStart;
	case PROPID_MSGBOX_IF_PATH_INVALID:
		return edPtr->DoMsgBoxIfPathNotSet;
	case PROPID_CONSOLE_ENABLED:
		return edPtr->ConsoleEnabled;
//	case PROPID_TEXT:
//		return &edPtr->szText[0];
	}
	

#endif // !defined(RUN_ONLY)
	return 0;		// Unchecked
}

// --------------------
// SetPropValue
// --------------------
// This routine is called by MMF after a property has been modified.
//
void WINAPI DLLExport SetPropValue(mv * mV, EDITDATA * edPtr, UINT nPropID, LPVOID lParam)
{
#ifndef RUN_ONLY
	// Gets the pointer to the CPropValue structure
	Prop * pValue = (Prop *)lParam;

	// Example
	// -------
	switch (nPropID) {
//
//	case PROPID_COMBO:
//		// Simply grab the value
//		edPtr->nComboIndex = ((CPropDWordValue*)pValue)->m_dwValue;
//		break;

//	case PROPID_COLOR:
//		// Here too, gets the value
//		edPtr->dwColor = ((CPropDWordValue*)pValue)->m_dwValue;
//		break;

		case PROPID_INITIALPATH:
		{
			// Gets the string
			LPSTR pStr = (LPSTR)((Prop_AStr *)pValue)->String;

			// You can simply poke the string if your EDITDATA structure has a fixed size
			
			// If the length is out of bounds
			if (strnlen(pStr, MAX_PATH) != MAX_PATH || strnlen(pStr, 1) != 0)
			{
				if (strcpy_s(edPtr->InitialPath, MAX_PATH, pStr))
				{
					std::stringstream s; s << "SetPropValue() failed: strcpy_s() returned error " << errno << ".";
					MessageBoxA(NULL, s.str().c_str(), "DebugObject - Error", MB_OK | MB_ICONERROR);
				}
			}
			else
			{	
				MessageBoxA(NULL, "SetPropValue() failed: String is not equal or less than 260 bytes", "DebugObject - error", MB_OK | MB_ICONERROR);
			}
			return;
		}
		break;
	}

	// You may want to have your object redrawn in the frame editor after the modifications,
	// in this case, just call this function
	// mvInvalidateObject(mV, edPtr);

#endif // !defined(RUN_ONLY)
}

// --------------------
// SetPropCheck
// --------------------
// This routine is called by MMF when the user modifies a checkbox in the properties.
//
void WINAPI DLLExport SetPropCheck(mv * mV, EDITDATA * edPtr, UINT nPropID, BOOL nCheck)
{
#ifndef RUN_ONLY
	// Example
	// -------
	switch (nPropID)
	{
		case PROPID_ENABLE_AT_START:
			edPtr->EnableAtStart = nCheck != 0;
			// mvInvalidateObject(mV, edPtr);
			// mvRefreshProp(mV, edPtr, PROPID_COMBO, TRUE);
			return;
		case PROPID_MSGBOX_IF_PATH_INVALID:
			edPtr->DoMsgBoxIfPathNotSet = nCheck != 0;
			return;
		case PROPID_CONSOLE_ENABLED:
			edPtr->ConsoleEnabled = nCheck != 0;
			// mvInvalidateObject(mV, edPtr);
			// mvRefreshProp(mV, edPtr, PROPID_COMBO, TRUE);
			return;
	}
#endif // !defined(RUN_ONLY)
}

// --------------------
// EditProp
// --------------------
// This routine is called when the user clicks the button of a Button or EditButton property.
//
BOOL WINAPI DLLExport EditProp(mv * mV, EDITDATA * edPtr, UINT nPropID)
{
#ifndef RUN_ONLY

	// Example
	// -------
/*
	if (nPropID==PROPID_EDITCONTENT)
	{
		if ( EditObject(mV, NULL, NULL, edPtr) )
			return TRUE;
	}
*/

#endif // !defined(RUN_ONLY)
	return FALSE;
}

// --------------------
// IsPropEnabled
// --------------------
// This routine returns the enabled state of a property.
//
BOOL WINAPI IsPropEnabled(mv * mV, EDITDATA * edPtr, UINT nPropID)
{
#ifndef RUN_ONLY
	// Example
	// -------
/*
	switch (nPropID) {

	case PROPID_CHECK:
		return (edPtr->nComboIndex != 0);
	}
*/
#endif // !defined(RUN_ONLY)
	return TRUE;
}


// ============================================================================
//
// TEXT PROPERTIES
// 
// ============================================================================

// --------------------
// GetTextCaps
// --------------------
// Return the text capabilities of the object under the frame editor.
//
DWORD WINAPI DLLExport GetTextCaps(mv * mV, EDITDATA * edPtr)
{
	return 0;	// (TEXT_ALIGN_LEFT|TEXT_ALIGN_HCENTER|TEXT_ALIGN_RIGHT|TEXT_ALIGN_TOP|TEXT_ALIGN_VCENTER|TEXT_ALIGN_BOTTOM|TEXT_FONT|TEXT_COLOR);
}

// --------------------
// GetTextFont
// --------------------
// Return the font used the object.
// Note: the pStyle and cbSize parameters are obsolete and passed for compatibility reasons only.
//
BOOL WINAPI DLLExport GetTextFont(mv * mV, EDITDATA * edPtr, LPLOGFONT plf, LPTSTR pStyle, UINT cbSize)
{
#if !defined(RUN_ONLY)
	// Example: copy LOGFONT structure from EDITDATA
	// memcpy(plf, &edPtr->m_lf, sizeof(LOGFONT));
#endif // !defined(RUN_ONLY)

	return TRUE;
}

// --------------------
// SetTextFont
// --------------------
// Change the font used the object.
// Note: the pStyle parameter is obsolete and passed for compatibility reasons only.
//
BOOL WINAPI DLLExport SetTextFont(mv * mV, EDITDATA * edPtr, LPLOGFONT plf, LPCSTR pStyle)
{
#if !defined(RUN_ONLY)
	// Example: copy LOGFONT structure to EDITDATA
	// memcpy(&edPtr->m_lf, plf, sizeof(LOGFONT));
#endif // !defined(RUN_ONLY)

	return TRUE;
}

// --------------------
// GetTextClr
// --------------------
// Get the text color of the object.
//
COLORREF WINAPI DLLExport GetTextClr(mv * mV, EDITDATA * edPtr)
{
	// Example
	return 0;	// edPtr->fontColor;
}

// --------------------
// SetTextClr
// --------------------
// Set the text color of the object.
//
void WINAPI DLLExport SetTextClr(mv * mV, EDITDATA * edPtr, COLORREF color)
{
	// Example
	//edPtr->fontColor = color;
}

// --------------------
// GetTextAlignment
// --------------------
// Get the text alignment of the object.
//
DWORD WINAPI DLLExport GetTextAlignment(mv * mV, EDITDATA * edPtr)
{
	DWORD dw = 0;
#if !defined(RUN_ONLY)
	// Example
	// -------
/*	if ( (edPtr->eData.dwFlags & ALIGN_LEFT) != 0 )
		dw |= TEXT_ALIGN_LEFT;
	if ( (edPtr->eData.dwFlags & ALIGN_HCENTER) != 0 )
		dw |= TEXT_ALIGN_HCENTER;
	if ( (edPtr->eData.dwFlags & ALIGN_RIGHT) != 0 )
		dw |= TEXT_ALIGN_RIGHT;
	if ( (edPtr->eData.dwFlags & ALIGN_TOP) != 0 )
		dw |= TEXT_ALIGN_TOP;
	if ( (edPtr->eData.dwFlags & ALIGN_VCENTER) != 0 )
		dw |= TEXT_ALIGN_VCENTER;
	if ( (edPtr->eData.dwFlags & ALIGN_BOTTOM) != 0 )
		dw |= TEXT_ALIGN_BOTTOM;
*/
#endif // !defined(RUN_ONLY)
	return dw;
}

// --------------------
// SetTextAlignment
// --------------------
// Set the text alignment of the object.
//
void WINAPI DLLExport SetTextAlignment(mv * mV, EDITDATA * edPtr, DWORD dwAlignFlags)
{
#if !defined(RUN_ONLY)
	// Example
	// -------
/*	DWORD dw = edPtr->eData.dwFlags;

	if ( (dwAlignFlags & TEXT_ALIGN_LEFT) != 0 )
		dw = (dw & ~(ALIGN_LEFT|ALIGN_HCENTER|ALIGN_RIGHT)) | ALIGN_LEFT;
	if ( (dwAlignFlags & TEXT_ALIGN_HCENTER) != 0 )
		dw = (dw & ~(ALIGN_LEFT|ALIGN_HCENTER|ALIGN_RIGHT)) | ALIGN_HCENTER;
	if ( (dwAlignFlags & TEXT_ALIGN_RIGHT) != 0 )
		dw = (dw & ~(ALIGN_LEFT|ALIGN_HCENTER|ALIGN_RIGHT)) | ALIGN_RIGHT;

	if ( (dwAlignFlags & TEXT_ALIGN_TOP) != 0 )
		dw = (dw & ~(ALIGN_TOP|ALIGN_VCENTER|ALIGN_BOTTOM)) | ALIGN_TOP;
	if ( (dwAlignFlags & TEXT_ALIGN_VCENTER) != 0 )
		dw = (dw & ~(ALIGN_TOP|ALIGN_VCENTER|ALIGN_BOTTOM)) | ALIGN_VCENTER;
	if ( (dwAlignFlags & TEXT_ALIGN_BOTTOM) != 0 )
		dw = (dw & ~(ALIGN_TOP|ALIGN_VCENTER|ALIGN_BOTTOM)) | ALIGN_BOTTOM;

	edPtr->eData.dwFlags = dw;
*/
#endif // !defined(RUN_ONLY)
}


// ----------------------------------------------------------
// Custom Parameters
// ----------------------------------------------------------

// --------------------
// InitParameter
// --------------------
// Initialize the parameter.
//
void WINAPI InitParameter(mv * mV, short code, ParamExtension * pExt)
{
#if !defined(RUN_ONLY)
	// Example
	// -------
	// strcpy(&pExt->pextData[0], "Parameter Test");
	// pExt->pextSize = sizeof(paramExt) + strlen(pExt->pextData)+1;
#endif // !defined(RUN_ONLY)
}

// Example of custom parameter setup proc
// --------------------------------------
/*
#if !defined(RUN_ONLY)
BOOL CALLBACK DLLExport SetupProc(HWND hDlg, UINT msgType, WPARAM wParam, LPARAM lParam)
{
	paramExt*			pExt;

	switch (msgType)
	{
		case WM_INITDIALOG: // Init dialog

			// Save edptr
			SetWindowLong(hDlg, DWL_USER, lParam);
			pExt=(paramExt*)lParam;

			SetDlgItemText(hDlg, IDC_EDIT, pExt->pextData);
			return TRUE;

		case WM_COMMAND: // Command

			// Retrieve edptr
			pExt = (paramExt *)GetWindowLong(hDlg, DWL_USER);

			switch (wmCommandID)
			{
			case IDOK:	// Exit
				GetDlgItemText(hDlg, IDC_EDIT, pExt->pextData, 500);
				pExt->pextSize=sizeof(paramExt)+strlen(pExt->pextData)+1;
				EndDialog(hDlg, TRUE);
				return TRUE;

				default:
					break;
			}
			break;

		default:
			break;
	}
	return FALSE;
}
#endif // !defined(RUN_ONLY)
*/

// --------------------
// EditParameter
// --------------------
// Edit the parameter.
//
void WINAPI EditParameter(mv * mV, short code, ParamExtension * pExt)
{
#if !defined(RUN_ONLY)

	// Example
	// -------
	// DialogBoxParam(hInstLib, MAKEINTRESOURCE(DB_TRYPARAM), mV->mvHEditWin, SetupProc, (LPARAM)(LPBYTE)pExt);

#endif // !defined(RUN_ONLY)
}

// --------------------
// GetParameterString
// --------------------
// Initialize the parameter.
//
void WINAPI GetParameterString(mv * mV, short code, ParamExtension * pExt, LPSTR pDest, short size)
{
#if !defined(RUN_ONLY)

	// Example
	// -------
	// wsprintf(pDest, "Super parameter %s", pExt->pextData);

#endif // !defined(RUN_ONLY)
}
