
// ============================================================================
//
// This file contains routines that are handled only during the Edittime,
// under the Frame and Event editors.
//
// Including creating, display, and setting up your object.
// 
// ============================================================================

#include "Common.h"
#include "DarkEdif.h"

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

// Use var args to fix warning 4001
#ifdef RUN_ONLY
	#define NoOpInRuntime(...) return __VA_ARGS__
#else
	#define NoOpInRuntime(...)
#endif

int DLLExport MakeIconEx ( mv *mV, cSurface* pIconSf, TCHAR * lpName, ObjInfo * oiPtr, EDITDATA * edPtr )
{
	NoOpInRuntime(0);
	pIconSf->Delete();
	pIconSf->Clone(*SDK->Icon);

	pIconSf->SetTransparentColor(RGB(255, 0, 255));
	return 0;
}


// --------------------
// CreateObject
// --------------------
// Called when you choose "Create new object". It should display the setup box 
// and initialize everything in the datazone.

int DLLExport CreateObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr)
{
	NoOpInRuntime(-1);

	if (!IS_COMPATIBLE(mV))
		return -1;

	Edif::Init(mV, edPtr);

	// Set default object settings
	//		edPtr->swidth = 48;
	//		edPtr->sheight = 48;

	return 0;
}

// --------------------
// EditObject
// --------------------
// Called when the user selects the Edit command in the object's popup menu
//
BOOL DLLExport EditObject (mv *mV, ObjInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr)
{
	NoOpInRuntime(FALSE);

	// Check compatibility
	if (!IS_COMPATIBLE(mV) )
		return FALSE;

	return TRUE;
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
void WINAPI	DLLExport PutObject(mv *mV, LevelObject * loPtr, EDITDATA * edPtr, unsigned short count)
{
	NoOpInRuntime();
}

// --------------------
// RemoveObject
// --------------------
// Called when each individual object is removed from the frame.
//
void WINAPI	DLLExport RemoveObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr, unsigned short count)
{
	NoOpInRuntime();

	// Is the last object removed?
	if (0 == count)
	{
		Edif::Free(edPtr);

		// Do whatever necessary to remove our data
	}
}

// --------------------
// DuplicateObject
// --------------------
// Called when an object is created from another one (note: should be called CloneObject instead...)
//
void DLLExport DuplicateObject(mv * mV, ObjectInfo * oiPtr, EDITDATA * edPtr)
{
	NoOpInRuntime();
}

// --------------------
// GetObjectRect
// --------------------
// Returns the size of the rectangle of the object in the frame editor.
//
void DLLExport GetObjectRect(mv * mV, RECT * rc, LevelObject * loPtr, EDITDATA * edPtr)
{
	NoOpInRuntime();
	if (!mV || !rc || !edPtr)
		return;
	
	rc->right = rc->left + SDK->Icon->GetWidth();	// edPtr->swidth;
	rc->bottom = rc->top + SDK->Icon->GetHeight();	// edPtr->sheight;
}


cSurface * DLLExport WinGetSurface (int idWin, int surfID = 0);
// --------------------
// EditorDisplay
// --------------------
// Displays the object under the frame editor
//
void DLLExport EditorDisplay(mv *mV, ObjectInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr, RECT * rc)
{
	NoOpInRuntime();

	cSurface * Surface = WinGetSurface((int) mV->IdEditWin);
	if (!Surface)
		return;

	SDK->Icon->Blit(*Surface, rc->left, rc->top, BMODE_TRANSP, BOP_COPY, 0);
}


// --------------------
// IsTransparent
// --------------------
// This routine tells MMF2 if the mouse pointer is over a transparent zone of the object.
// 

extern "C" BOOL DLLExport IsTransparent(mv *mV, LevelObject * loPtr, EDITDATA * edPtr, int dx, int dy)
{
	NoOpInRuntime(FALSE);
	return FALSE;
}

// --------------------
// PrepareToWriteObject
// --------------------
// Just before writing the datazone when saving the application, MMF2 calls this routine.
// 
void DLLExport PrepareToWriteObject(mv * mV, EDITDATA * edPtr, ObjectInfo * adoi)
{
	NoOpInRuntime();
}

// --------------------
// GetFilters
// --------------------

BOOL WINAPI GetFilters(mv * mV, EDITDATA * edPtr, unsigned int Flags, void * Reserved)
{
	NoOpInRuntime(FALSE);
	// If your extension uses image filters
//	if ( (Flags & GETFILTERS_IMAGES) != 0 )
//		return TRUE;

	// If your extension uses sound filters
//	if ( (Flags & GETFILTERS_SOUNDS) != 0 )
//		return TRUE;
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
	NoOpInRuntime(FALSE);
	
	BOOL r = FALSE;
	/*	Example: return TRUE if file extension is ".txt"
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
		}
	*/
	return r;
}


// --------------------
// CreateFromFile
// --------------------
// Creates a new object from file
//
void WINAPI	DLLExport CreateFromFile (mv * mV, TCHAR * fileName, EDITDATA * edPtr)
{
	NoOpInRuntime();
	// Initialize your extension data from the given file
	//	edPtr->swidth = 48;
	//	edPtr->sheight = 48;

	// Example: store the filename
	// strcpy(edPtr->myFileName, fileName);
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
void InitialisePropertiesFromJSON(mv * mV, EDITDATA * edPtr);
BOOL DLLExport GetProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
	NoOpInRuntime(FALSE);

	mvInsertProps(mV, edPtr, SDK->EdittimeProperties, PROPID_TAB_GENERAL, TRUE);

	if (edPtr->DarkEdif_Prop_Size == 0)
	{
		InitialisePropertiesFromJSON(mV, edPtr);
		mvInvalidateObject(mV, edPtr);
	}

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
	NoOpInRuntime();
}

// --------------------
// GetPropCreateParam
// --------------------
// Called when a property is initialized and its creation parameter is NULL (in the PropData).
// Allows you, for example, to change the content of a combobox property according to specific settings in the EDITDATA structure.
//
LPARAM DLLExport GetPropCreateParam(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	NoOpInRuntime(NULL);
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
	NoOpInRuntime();
}

// --------------------
// GetPropValue
// --------------------
// Returns the value of properties that have a value.
// Note: see GetPropCheck for checkbox properties
//
using namespace Edif::Properties;
Prop * GetProperty(EDITDATA * edPtr, size_t ID);
void * DLLExport GetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID_)
{
	NoOpInRuntime(NULL);

	unsigned int PropID = (PropID_ - 0x80000) % 1000;
	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
		return NULL;

	return GetProperty(edPtr, PropID);
}

// --------------------
// GetPropCheck
// --------------------
// Returns the checked state of properties that have a check box.
//
BOOL DLLExport GetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID_)
{
	NoOpInRuntime(FALSE);
	unsigned int PropID = (PropID_ - 0x80000) % 1000;

	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
		return FALSE;
	
	return (edPtr->DarkEdif_Props[PropID >> 3] >> (PropID % 8) & 1);
}

// --------------------
// SetPropValue
// --------------------
// This routine is called by MMF after a property has been modified.
//
void DLLExport SetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID_, void * Param)
{
	NoOpInRuntime();

	Prop * prop = (Prop *)Param;

	unsigned int i = prop->GetClassID(), PropID = (PropID_ - 0x80000) % 1000;

	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
		return;

	const json_value & propjson = CurLang["Properties"][PropID];

	switch (i)
	{
		case 'DATA': // Buffer or string
			if (strcmp(propjson["Type"], "Editbox String"))
			{
				Prop_Buff * prop2 = (Prop_Buff *)prop;
				PropChange(mV, edPtr, PropID, prop2->Address, prop2->Size);
				break;
			}
			else
				MessageBoxA(NULL, "ERROR: Got Buff type instead of string-based for string property.", "DarkEdif - Property error", MB_OK);
			break;
		case 'STRA': // ANSI string
			{
				Prop_Buff * prop2 = (Prop_Buff *)prop; // see note

				// Serialise string to UTF-8
				const char * fromStr = static_cast<const char *>(prop2->Address);

				std::string utf8Str(1, '\0');
				size_t numBytes = 1;
				// String is blank?
				if (fromStr[0] != '\0')
				{
					std::stringstream str;
					// First convert to Unicode UCS-2
					size_t numCharsInclNull = MultiByteToWideChar(CP_ACP, 0, fromStr, -1, 0, 0);
					if (numCharsInclNull == 0)
					{
						str << "Failed to convert new property text to UCS-2 (error " << GetLastError() << "). Change will be ignored.";
						MessageBoxA(NULL, str.str().c_str(), "DarkEdif - Property Error", MB_OK);
						break;
					}
					std::wstring wideStr(numCharsInclNull, L'\0');
					if (MultiByteToWideChar(CP_ACP, 0, fromStr, -1, &wideStr.front(), wideStr.size()) == 0)
					{
						str << "Failed to convert new property text to UCS-2 (error " << GetLastError() << "). Change will be ignored.";
						MessageBoxA(NULL, str.str().c_str(), "DarkEdif - Property Error", MB_OK);
						break;
					}

					// Then back to Unicode UTF-8
					numBytes = WideCharToMultiByte(CP_UTF8, 0, &wideStr.front(), -1, 0, 0, NULL, NULL);
					if (numBytes == 0)
					{
						str << "Failed to convert new UCS-2 property text to UTF-8 (error " << GetLastError() << "). Change will be ignored.";
						MessageBoxA(NULL, str.str().c_str(), "DarkEdif - Property Error", MB_OK);
						break;
					}
					utf8Str.resize(numBytes + 1);
					if (WideCharToMultiByte(CP_UTF8, 0, &wideStr.front(), -1, &utf8Str.front(), utf8Str.size(), NULL, NULL) == 0)
					{
						str << "Failed to convert new UCS-2 property text to UTF-8 (error " << GetLastError() << "). Change will be ignored.";
						MessageBoxA(NULL, str.str().c_str(), "DarkEdif - Property Error", MB_OK);
						break;
					}
				}

				PropChange(mV, edPtr, PropID, &utf8Str.front(), numBytes);
				break;
			}
		case 'STRW': // Unicode string
			{
				Prop_Buff * prop2 = (Prop_Buff *)prop; // see note

				// Serialise string to UTF-8
				const wchar_t * fromStr = static_cast<const wchar_t *>(prop2->Address);

				std::string utf8Str(1, '\0');
				size_t numBytes = 1;
				// String is blank?
				if (fromStr[0] != L'\0')
				{
					std::stringstream str;
					numBytes = WideCharToMultiByte(CP_UTF8, 0, fromStr, -1, 0, 0, NULL, NULL);
					if (numBytes == 0)
					{
						str << "Failed to convert new property text to UTF-8 (error " << GetLastError() << "). Change will be ignored.";
						MessageBoxA(NULL, str.str().c_str(), "DarkEdif - Property Error", MB_OK);
						break;
					}
					utf8Str.resize(numBytes + 1);
					if (WideCharToMultiByte(CP_UTF8, 0, fromStr, -1, &utf8Str.front(), utf8Str.size(), NULL, NULL) == 0)
					{
						str << "Failed to convert new property text to UTF-8 (error " << GetLastError() << "). Change will be ignored.";
						MessageBoxA(NULL, str.str().c_str(), "DarkEdif - Property Error", MB_OK);
						break;
					}
				}

				PropChange(mV, edPtr, PropID, &utf8Str.front(), utf8Str.size());
				break;
			}
		case 'INT ': // 4-byte signed int
			{
				Prop_SInt * prop2 = (Prop_SInt *)prop;
				PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(int));
				break;
			}
		case 'DWRD': // 4-byte unsigned int
			{
				Prop_UInt * prop2 = (Prop_UInt *)prop;
				PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(unsigned int));
				break;
			}
		case 'INT2': // 8-byte signed int
			{
				Prop_Int64 * prop2 = (Prop_Int64 *)prop;
				PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(__int64));
				break;
			}

		case 'DBLE': // 8-byte floating point var
			{
				Prop_Double * prop2 = (Prop_Double *)prop;
				PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(double));
				break;
			}
		case 'FLOT': // 4-byte floating point var
			{
				Prop_Float * prop2 = (Prop_Float *)prop;
				PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(float));
				break;
			}
		case 'SIZE': // Two ints depicting a size
			{
				Prop_Size * prop2 = (Prop_Size *)prop;
				PropChange(mV, edPtr, PropID, &prop2->X, sizeof(int)*2);
				break;
			}
		default: // Custom property
			{
				Prop_Custom * prop2 = (Prop_Custom *)prop;
				//PropChange(mV, edPtr, PropID, prop2->GetPropValue(), prop2->GetPropValueSize());
			}
			MessageBoxA(NULL, "Assuming class ID is custom - but no custom code yet written.", "DarkEdif - Error", MB_OK);
			break;
	}
}

// --------------------
// SetPropCheck
// --------------------
// This routine is called by MMF when the user modifies a checkbox in the properties.
//
void DLLExport SetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID_, BOOL checked)
{
	NoOpInRuntime();

	unsigned int PropID = (PropID_ - 0x80000) % 1000;
	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
		return;

	PropChangeChkbox(edPtr, PropID, checked != FALSE);
}

// --------------------
// EditProp
// --------------------
// This routine is called when the user clicks the button of a Button or EditButton property.
//
BOOL DLLExport EditProp(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	NoOpInRuntime(FALSE);
	// Example
	// -------
/*
	if (nPropID==PROPID_EDITCONTENT)
	{
		if ( EditObject(mV, NULL, NULL, edPtr) )
			return TRUE;
	}
*/

	return FALSE;
}

// --------------------
// IsPropEnabled
// --------------------
// This routine returns the enabled state of a property.
//
BOOL WINAPI IsPropEnabled(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	NoOpInRuntime(FALSE);
	// Example
	// -------
/*
	switch (nPropID) {

	case PROPID_CHECK:
		return (edPtr->nComboIndex != 0);
	}
*/
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
	NoOpInRuntime(FALSE);
	// Example: copy LOGFONT structure from EDITDATA
	// memcpy(plf, &edPtr->m_lf, sizeof(LOGFONT));

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
	NoOpInRuntime(FALSE);
	// Example: copy LOGFONT structure to EDITDATA
	// memcpy(&edPtr->m_lf, plf, sizeof(LOGFONT));

	return TRUE;
}

// --------------------
// GetTextClr
// --------------------
// Get the text color of the object.
//
COLORREF DLLExport GetTextClr(mv * mV, EDITDATA * edPtr)
{
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
	NoOpInRuntime(0);
	unsigned int dw = 0;
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
	return dw;
}

// --------------------
// SetTextAlignment
// --------------------
// Set the text alignment of the object.
//
void DLLExport SetTextAlignment(mv *mV, EDITDATA * edPtr, unsigned int AlignFlags)
{
	NoOpInRuntime();
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
	NoOpInRuntime();
	// Example
	// -------
	// strcpy(&pExt->pextData[0], "Parameter Test");
	// pExt->pextSize = sizeof(paramExt) + strlen(pExt->pextData)+1;
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
	NoOpInRuntime();

	// Example
	// -------
	// DialogBoxParam(hInstLib, MAKEINTRESOURCE(DB_TRYPARAM), mV->mvHEditWin, SetupProc, (LPARAM)(LPBYTE)pExt);
}

// --------------------
// GetParameterString
// --------------------
// Initialize the parameter.
//
void WINAPI GetParameterString(mv *mV, short code, ParamExtension * pExt, char * pDest, short size)
{
	NoOpInRuntime();
	// Example
	// -------
	// wsprintf(pDest, "Super parameter %s", pExt->pextData);
	return;
}

