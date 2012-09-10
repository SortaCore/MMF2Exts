
// ============================================================================
//
// This file contains routines that are handled only during the Edittime,
// under the Frame and Event editors.
//
// Including creating, display, and setting up your object.
// 
// ============================================================================

#include "Common.h"

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

int DLLExport MakeIconEx ( mv *mV, cSurface* pIconSf, TCHAR * lpName, ObjInfo * oiPtr, EDITDATA * edPtr )
{
	#ifndef RUN_ONLY

		pIconSf->Delete();
		pIconSf->Clone(*SDK->Icon);

		pIconSf->SetTransparentColor(RGB(255, 0, 255));

	#endif // !defined(RUN_ONLY)
   return 0;
}


// --------------------
// CreateObject
// --------------------
// Called when you choose "Create new object". It should display the setup box 
// and initialize everything in the datazone.

int DLLExport CreateObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr)
{
	// Check compatibility
	if ( IS_COMPATIBLE(mV) )
	{
		Edif::Init(mV, edPtr);

		// Set default object settings from DefaultState.
		edPtr->AutomaticClear = ::SDK->json[CurLang]["Properties"][1]["DefaultState"];
		edPtr->Global = ::SDK->json[CurLang]["Properties"][2]["DefaultState"];
		if (strcpy_s(edPtr->edGlobalID, 255, ::SDK->json[CurLang]["Properties"][3]["DefaultState"]))
			MessageBoxA(NULL, "Error initialising property 3; error copying string.", "DarkEDIF - CreateObject() error", MB_OK);
		edPtr->MultiThreading = ::SDK->json[CurLang]["Properties"][4]["DefaultState"];
		
		//InitialisePropertiesFromJSON(mV, edPtr);

		return 0;
	}
	else // Error
		return -1;
}

// --------------------
// EditObject
// --------------------
// Called when the user selects the Edit command in the object's popup menu
//
BOOL DLLExport EditObject (mv *mV, ObjInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr)
{
	// Check compatibility
	if ( IS_COMPATIBLE(mV) )
	{
		return true;
	}
	else
		return FALSE;
}

// --------------------
// SetEditSize
// --------------------
// Called when the object has been resized
//
// Note: remove the comments if your object can be resized (and remove the comments in the .def file)
/*
BOOL WINAPI SetEditSize(LPMV mv, LPEDATA edPtr, int cx, int cy)
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
void WINAPI	DLLExport PutObject(mv *mV, LevelObject * loPtr, EDITDATA * edPtr, unsigned short count)
{
	#ifndef RUN_ONLY
	#endif // !defined(RUN_ONLY)
}

// --------------------
// RemoveObject
// --------------------
// Called when each individual object is removed from the frame.
//
void WINAPI	DLLExport RemoveObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr, unsigned short count)
{
	#ifndef RUN_ONLY
		// Is the last object removed?
		if (0 == count)
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
void DLLExport DuplicateObject(mv * mV, ObjectInfo * oiPtr, EDITDATA * edPtr)
{
	#ifndef RUN_ONLY
	
	#endif // !defined(RUN_ONLY)
}

// --------------------
// GetObjectRect
// --------------------
// Returns the size of the rectangle of the object in the frame editor.
//
void DLLExport GetObjectRect(mv * mV, RECT * rc, LevelObject * loPtr, EDITDATA * edPtr)
{
	#ifndef RUN_ONLY
		if (!mV || !rc || !edPtr)
			return;
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
void DLLExport EditorDisplay(mv *mV, ObjectInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr, RECT * rc)
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

extern "C" BOOL DLLExport IsTransparent(mv *mV, LevelObject * loPtr, EDITDATA * edPtr, int dx, int dy)
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
void DLLExport PrepareToWriteObject(mv * mV, EDITDATA * edPtr, ObjectInfo * adoi)
{
#ifndef RUN_ONLY

	// Write your code here
#endif // !defined(RUN_ONLY)
}

// --------------------
// GetFilters
// --------------------

BOOL WINAPI GetFilters(mv * mV, EDITDATA * edPtr, unsigned int Flags, void * Reserved)
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
BOOL DLLExport UsesFile(mv * mV, TCHAR * fileName)
{
	BOOL r = FALSE;
	#ifndef RUN_ONLY

		// Example: return TRUE if file extension is ".txt"
	/*	
		char *	ext, npath;

		if ( fileName != NULL )
		{
			if ( (ext=(char *)malloc(_MAX_EXT)) != NULL )
			{
				if ( (npath=(char *)malloc(_MAX_PATH)) != NULL )
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
void WINAPI	DLLExport CreateFromFile (mv * mV, TCHAR * fileName, EDITDATA * edPtr)
{
	#ifndef RUN_ONLY
		// Initialize your extension data from the given file
	//	edPtr->swidth = 48;
	//	edPtr->sheight = 48;

		// Example: store the filename
		// strcpy(edPtr->myFileName, fileName);
	#endif // !defined(RUN_ONLY)
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
BOOL DLLExport GetProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
	#ifndef RUN_ONLY
		mvInsertProps(mV, edPtr, ::SDK->EdittimeProperties, PROPID_TAB_GENERAL, TRUE);
	#endif // !defined(RUN_ONLY)

	// OK
	return TRUE;
}

// --------------------
// ReleaseProperties
// --------------------
// Called when the properties are removed from the property window.
//
void DLLExport ReleaseProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
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
LPARAM DLLExport GetPropCreateParam(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	#ifndef RUN_ONLY
		// Example
		// -------
	//	if ( PropID == PROPID_COMBO )
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
void DLLExport ReleasePropCreateParam(mv * mV, EDITDATA * edPtr, unsigned int PropID, LPARAM lParam)
{
	#ifndef RUN_ONLY
	#endif // !RUN_ONLY
}

// --------------------
// GetPropValue
// --------------------
// Returns the value of properties that have a value.
// Note: see GetPropCheck for checkbox properties
//
Prop * DLLExport GetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	#ifndef RUN_ONLY
		unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;

		if ((unsigned int) ::SDK->json[CurLang]["Properties"].u.object.length > ID)
		{
			if (::SDK->EdittimeProperties[ID].Type_ID != Edif::Properties::PROPTYPE_LEFTCHECKBOX)
			{
				if (ID == 0)
					return new Prop_AStr(::SDK->json[CurLang]["Properties"][ID]["DefaultState"]);
				if (ID == 3)
					return new Prop_AStr(edPtr->edGlobalID);
			}

			// Override invalid property warning
			// See request for change: http://community.clickteam.com/showthread.php?t=72152
			return NULL;
		}
		

		MessageBoxA(NULL, "Invalid property ID given to GetPropValue() call.", "DarkEDIF - Invalid property", MB_OK);
	#endif // !RUN_ONLY
		
	return NULL;
}

// --------------------
// GetPropCheck
// --------------------
// Returns the checked state of properties that have a check box.
//
BOOL DLLExport GetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	#ifndef RUN_ONLY
		unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;
		
		if ((unsigned int) ::SDK->json[CurLang]["Properties"].u.object.length > ID)
		{
			if (ID == 1)
				return edPtr->AutomaticClear;
			if (ID == 2)
				return edPtr->Global;
			if (ID == 4)
				return edPtr->MultiThreading;
		}
		
		MessageBoxA(NULL, "Invalid property ID given to GetPropCheck() call.", "DarkEDIF - Invalid property", MB_OK);
	#endif // !RUN_ONLY

	return 0;		// Unchecked
}

// --------------------
// SetPropValue
// --------------------
// This routine is called by MMF after a property has been modified.
//
void DLLExport SetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID, Prop * NewParam)
{
	#ifndef RUN_ONLY
	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;
	if (ID == 3)
	{
		if (strcpy_s(edPtr->edGlobalID, 255, ((Prop_AStr *)NewParam)->String))
			MessageBoxA(NULL, "Error setting new property 3; error copying string.", "DarkEDIF - SetPropValue() error", MB_OK);
	}

#if 0
	Prop * LocalParam = ID == 0 ? : &edPtr->Prop3;
	// Gets the pointer to the Prop structure
	switch (LocalParam->GetClassID())
	{
		case 'INT ':
			((Prop_SInt *)LocalParam)->Value = ((Prop_SInt *)NewParam)->Value;
			break;

		case 'DWRD':
			((Prop_UInt *)LocalParam)->Value = ((Prop_UInt *)NewParam)->Value;
			break;

		case 'FLOT':
			((Prop_Float *)LocalParam)->Value = ((Prop_Float *)NewParam)->Value;
			break;

		case 'DBLE':
			((Prop_Double *)LocalParam)->Value = ((Prop_Double *)NewParam)->Value;
			break;

		case 'SIZE':
			((Prop_Size *)LocalParam)->X = ((Prop_Size *)NewParam)->X;
			((Prop_Size *)LocalParam)->Y = ((Prop_Size *)NewParam)->Y;
			break;

		case 'INT2':
			((Prop_Int64 *)LocalParam)->Value = ((Prop_Int64 *)NewParam)->Value;
			break;

		case 'LPTR':
			if (!((Prop_Ptr *)LocalParam)->Address)
				free(((Prop_Ptr *)LocalParam)->Address);

			((Prop_Ptr *)LocalParam)->Address = ((Prop_Ptr *)NewParam)->Address;
			break;

		case 'DATA':
			if (!((Prop_Buff *)LocalParam)->Address)
				free(((Prop_Buff *)LocalParam)->Address);

			((Prop_Buff *)LocalParam)->Address = ((Prop_Buff *)NewParam)->Address;
			((Prop_Buff *)LocalParam)->Size = ((Prop_Buff *)NewParam)->Size;
			break;

	#ifdef _UNICODE
		case 'STRA':
			MessageBoxA(NULL, "Unexpected ANSI string given to a Unicode extension.", "DarkEDIF - Edittime SetPropValue() error.", MB_OK);
			break;
		case 'STRW':
	#else
		case 'STRW':
			MessageBoxA(NULL, "Unexpected Unicode string given to a ANSI extension.", "DarkEDIF - Edittime SetPropValue() error.", MB_OK);
			break;
		case 'STRA':
	#endif
			if (!((Prop_Str *)LocalParam)->String)
				free(((Prop_Str *)LocalParam)->String);

			((Prop_Str *)LocalParam)->String = _tcsdup(((Prop_AStr *)NewParam)->String);
			break;
		
		// Custom/unrecognised parameter
		default:
			((Prop_Custom *)LocalParam)->SetPropValue(NewParam);
			break;
	}
#endif // NOPROPS!!

		// Example
		// -------
//		switch (nPropID) {
//	
//		case PROPID_COMBO:
//			// Simply grab the value
//			edPtr->nComboIndex = ((CPropDWordValue*)pValue)->m_dwValue;
//			break;
//
//		case PROPID_COLOR:
//			// Here too, gets the value
//			edPtr->dwColor = ((CPropDWordValue*)pValue)->m_dwValue;
//			break;
//
//		case PROPID_TEXT:
//			{
//				// Gets the string
//				char * pStr = (char *)((CPropDataValue*)pValue)->m_pData;
//	
//				// You can simply poke the string if your EDITDATA structure has a fixed size,
//				// or have an adaptive size of structure like below
//	
//				// If the length is different
//				if (strlen(pStr)!=strlen(edPtr->text))
//				{
//					// Asks MMF to reallocate the structure with the new size
//					LPEDATA pNewPtr = (LPEDATA)mvReAllocEditData(mV, edPtr, sizeof(EDITDATA)+strlen(pStr));
//					
//					// If reallocation worked
//					if (pNewPtr!=NULL)
//					{
//						// Copy the string
//						edPtr=pNewPtr;
//						strcpy(edPtr->text, pStr);
//					}
//				}
//				else
//				{	
//					// Same size : simply copy
//					strcpy(edPtr->text, pStr);
//				}
//			}
//			break;
//		}

		
		// mvInvalidateObject(mV, edPtr);
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
void DLLExport SetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID, BOOL Check)
{
#ifndef RUN_ONLY
		unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;

		if (::SDK->json[CurLang]["Properties"].u.object.length > ID)
		{
			if (ID == 1)
			{
				edPtr->AutomaticClear = (Check != 0);
				return;
			}
			if (ID == 2)
			{
				edPtr->Global = (Check != 0);
				return;
			}
			if (ID == 4)
			{
				edPtr->MultiThreading = (Check != 0);
				return;
			}
		}
		
		MessageBoxA(NULL, "Invalid property ID given to SetPropCheck() call.", "DarkEDIF - Invalid property", MB_OK);
		//mvRefreshProp(mV, edPtr, PropID, TRUE);
	// Example
	// -------
//		switch (nPropID)
//		{
//		case PROPID_CHECK:
//			edPtr->nCheck = nCheck;
//			mvInvalidateObject(mV, edPtr);
//			mvRefreshProp(mV, edPtr, PROPID_COMBO, TRUE);
//			break;
//		}
#endif // !defined(RUN_ONLY)
}

// --------------------
// EditProp
// --------------------
// This routine is called when the user clicks the button of a Button or EditButton property.
//
BOOL DLLExport EditProp(mv * mV, EDITDATA * edPtr, unsigned int PropID)
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
BOOL WINAPI IsPropEnabled(mv * mV, EDITDATA * edPtr, unsigned int PropID)
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
unsigned int DLLExport GetTextCaps(mv * mV, EDITDATA * edPtr)
{
	return 0;	// (TEXT_ALIGN_LEFT|TEXT_ALIGN_HCENTER|TEXT_ALIGN_RIGHT|TEXT_ALIGN_TOP|TEXT_ALIGN_VCENTER|TEXT_ALIGN_BOTTOM|TEXT_FONT|TEXT_COLOR);
}

// --------------------
// GetTextFont
// --------------------
// Return the font used the object.
// Note: the pStyle and cbSize parameters are obsolete and passed for compatibility reasons only.
//
BOOL DLLExport GetTextFont(mv * mV, EDITDATA * edPtr, LOGFONT * Font, TCHAR * pStyle, unsigned int cbSize)
{
	#ifndef RUN_ONLY
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
BOOL DLLExport SetTextFont(mv * mV, EDITDATA * edPtr, LOGFONT * Font, const char * pStyle)
{
	#ifndef RUN_ONLY
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
COLORREF DLLExport GetTextClr(mv * mV, EDITDATA * edPtr)
{
	// Example
	return 0;	// edPtr->fontColor;
}

// --------------------
// SetTextClr
// --------------------
// Set the text color of the object.
//
void DLLExport SetTextClr(mv *mV, EDITDATA * edPtr, COLORREF color)
{
	// Example
	//edPtr->fontColor = color;
}

// --------------------
// GetTextAlignment
// --------------------
// Get the text alignment of the object.
//
unsigned int DLLExport GetTextAlignment(mv *mV, EDITDATA * edPtr)
{
	unsigned int dw = 0;
	#ifndef RUN_ONLY
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
void DLLExport SetTextAlignment(mv *mV, EDITDATA * edPtr, unsigned int AlignFlags)
{
	#ifndef RUN_ONLY
		// Example
		// -------
	/*	unsigned int dw = edPtr->eData.dwFlags;

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
	#ifndef RUN_ONLY
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
void WINAPI EditParameter(mv *mV, short code, ParamExtension * pExt)
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
void WINAPI GetParameterString(mv *mV, short code, ParamExtension * pExt, char * pDest, short size)
{
#if !defined(RUN_ONLY)

	// Example
	// -------
	// wsprintf(pDest, "Super parameter %s", pExt->pextData);

#endif // !defined(RUN_ONLY)
}
