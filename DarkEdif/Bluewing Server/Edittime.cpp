// ============================================================================
// This file contains routines that are handled only during the Edittime,
// under the Frame and Event editors.
//
// Including creating, display, and setting up your object.
// ============================================================================

#include "Common.h"
#include "DarkEdif.h"

// ============================================================================
// ROUTINES USED UNDER FRAME EDITOR
// ============================================================================

#if EditorBuild

// Called once object is created or modified, just after setup.
// Also called before showing the "Insert an object" dialog if your object
// has no icon resource
int FusionAPI MakeIconEx(mv *mV, cSurface *pIconSf, TCHAR *lpName, ObjInfo *oiPtr, EDITDATA *edPtr)
{
#pragma DllExportHint
	pIconSf->Delete();
	pIconSf->Clone(*SDK->Icon);

	pIconSf->SetTransparentColor(RGB(255, 0, 255));
	return 0;
}


// Called when you choose "Create new object". It should display the setup box 
// and initialize everything in the datazone.
int FusionAPI CreateObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	// Check compatibility
	if (!IS_COMPATIBLE(mV))
		return -1;
	Edif::Init(mV, edPtr);

	if (edPtr->eHeader.extSize < sizeof(EDITDATA))
	{
		void* newEd = mvReAllocEditData(mV, edPtr, sizeof(EDITDATA));
		if (!newEd) {
			MessageBoxA(NULL, "Failed to allocate enough size for properites.", PROJECT_NAME " error", MB_ICONERROR);
			return -1;
		}
		edPtr = (EDITDATA*)newEd;
	}

	// Set default object settings from DefaultState.
	const auto &propsJSON = CurLang["Properties"];
	edPtr->automaticClear = propsJSON[1]["DefaultState"];
	edPtr->isGlobal = propsJSON[2]["DefaultState"];
	if (strcpy_s(edPtr->edGlobalID, 255, propsJSON[3]["DefaultState"]))
		MessageBoxA(NULL, "Error initialising property 3; error copying string.", "DarkEdif - CreateObject() error", MB_OK);
	edPtr->multiThreading = propsJSON[4]["DefaultState"];
	edPtr->timeoutWarningEnabled = propsJSON[5]["DefaultState"];
	edPtr->fullDeleteEnabled = propsJSON[6]["DefaultState"];

	// Since we're echoing Relay's property layout, we don't use DarkEdif's property manager.
	// InitialisePropertiesFromJSON(mV, edPtr);

	return 0;
}


cSurface * FusionAPI WinGetSurface (int idWin, int surfID = 0);

// Displays the object under the frame editor
void FusionAPI EditorDisplay(mv *mV, ObjectInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr, RECT * rc)
{
#pragma DllExportHint
	cSurface * Surface = WinGetSurface((int) mV->IdEditWin);
	if (!Surface)
		return;

	DarkEdif::SDKUpdater::RunUpdateNotifs(mV, edPtr);

	SDK->Icon->Blit(*Surface, rc->left, rc->top, BMODE_TRANSP, BOP_COPY, 0);
}

// ============================================================================
// PROPERTIES
// ============================================================================

// Inserts properties into the properties of the object.
BOOL FusionAPI GetProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
	if (!IS_COMPATIBLE(mV))
		return FALSE;

	mvInsertProps(mV, edPtr, ::SDK->EdittimeProperties, PROPID_TAB_GENERAL, TRUE);

	return TRUE; // OK
}

// Called when the properties are removed from the property window.
void FusionAPI ReleaseProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
}

// Called when a property is initialized and its creation parameter is NULL (in the PropData).
// Allows you, for example, to change the content of a combobox property according to specific settings in the EDITDATA structure.
LPARAM FusionAPI GetPropCreateParam(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	return NULL;
}

// Called after a property has been initialized.
// Allows you, for example, to free memory allocated in GetPropCreateParam.
void FusionAPI ReleasePropCreateParam(mv * mV, EDITDATA * edPtr, unsigned int PropID, LPARAM lParam)
{
#pragma DllExportHint
}

// Returns the value of properties that have a value.
// Note: see GetPropCheck for checkbox properties
Prop * FusionAPI GetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;

	if ((unsigned int) CurLang["Properties"].u.object.length > ID)
	{
		if (::SDK->EdittimeProperties[ID].Type_ID != Edif::Properties::PROPTYPE_LEFTCHECKBOX)
		{
			if (ID == 0)
			{
				char extVerBuffer[256];
				sprintf_s(extVerBuffer, CurLang["Properties"][ID]["DefaultState"], lacewing::relayserver::buildnum, STRIFY(CONFIG));
				return new Prop_AStr(extVerBuffer);
			}
			if (ID == 3)
				return new Prop_AStr(edPtr->edGlobalID);
		}

		// Override invalid property warning
		// See request for change: http://community.clickteam.com/showthread.php?t=72152
		return NULL;
	}

	MessageBoxA(NULL, "Invalid property ID given to GetPropValue() call.", "DarkEdif - Invalid property", MB_OK);
	return NULL;
}

// Returns the checked state of properties that have a check box.
BOOL FusionAPI GetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;
		
	if ((unsigned int) CurLang["Properties"].u.object.length > ID)
	{
		if (ID == 1)
			return edPtr->automaticClear;
		if (ID == 2)
			return edPtr->isGlobal;
		if (ID == 4)
			return edPtr->multiThreading;
		if (ID == 5)
			return edPtr->timeoutWarningEnabled;
		if (ID == 6)
			return edPtr->fullDeleteEnabled;
	}

	MessageBoxA(NULL, "Invalid property ID given to GetPropCheck() call.", "DarkEdif - Invalid property", MB_OK);
	return 0; // Unchecked
}

// This routine is called by MMF after a property has been modified.
void FusionAPI SetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID, Prop * NewParam)
{
#pragma DllExportHint
	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;
	if (ID == 3)
	{ 
		if (strcpy_s(edPtr->edGlobalID, 255, ((Prop_AStr *)NewParam)->String))
			MessageBoxA(NULL, "Error setting new property 3; error copying string.", "DarkEdif - SetPropValue() error", MB_OK);
	}

	// You may want to have your object redrawn in the frame editor after the modifications,
	// in this case, just call this function
	// mvInvalidateObject(mV, edPtr);
}

// This routine is called by MMF when the user modifies a checkbox in the properties.
void FusionAPI SetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID, BOOL Check)
{
#pragma DllExportHint
	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;

	if (CurLang["Properties"].u.object.length > ID)
	{
		if (ID == 1)
		{
			edPtr->automaticClear = (Check != 0);
			return;
		}
		if (ID == 2)
		{
			edPtr->isGlobal = (Check != 0);
			return;
		}
		if (ID == 4)
		{
			edPtr->multiThreading = (Check != 0);
			return;
		}
		if (ID == 5)
		{
			edPtr->timeoutWarningEnabled = (Check != 0);
			return;
		}
		if (ID == 6)
		{
			edPtr->fullDeleteEnabled = (Check != 0);
			return;
		}
	}
		
	MessageBoxA(NULL, "Invalid property ID given to SetPropCheck() call.", "DarkEdif - Invalid property", MB_OK);
}

// This routine is called when the user clicks the button of a Button or EditButton property.
BOOL FusionAPI EditProp(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	return FALSE;
}

// This routine returns the enabled state of a property.
BOOL FusionAPI IsPropEnabled(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	return TRUE;
}


// ============================================================================
// TEXT PROPERTIES
// ============================================================================

// Return the text capabilities of the object under the frame editor.
std::uint32_t FusionAPI GetTextCaps(mv * mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	return 0;	// (TEXT_ALIGN_LEFT|TEXT_ALIGN_HCENTER|TEXT_ALIGN_RIGHT|TEXT_ALIGN_TOP|TEXT_ALIGN_VCENTER|TEXT_ALIGN_BOTTOM|TEXT_FONT|TEXT_COLOR);
}

// Return the font used the object.
// Note: the pStyle and cbSize parameters are obsolete and passed for compatibility reasons only.
BOOL FusionAPI GetTextFont(mv * mV, EDITDATA * edPtr, LOGFONT * Font, TCHAR * pStyle, unsigned int cbSize)
{
#pragma DllExportHint
	// Example: copy LOGFONT structure from EDITDATA
	// memcpy(plf, &edPtr->m_lf, sizeof(LOGFONT));

	return TRUE;
}

// Change the font used the object.
// Note: the pStyle parameter is obsolete and passed for compatibility reasons only.
BOOL FusionAPI SetTextFont(mv * mV, EDITDATA * edPtr, LOGFONT * Font, const char * pStyle)
{
#pragma DllExportHint
	// Example: copy LOGFONT structure to EDITDATA
	// memcpy(&edPtr->m_lf, plf, sizeof(LOGFONT));

	return TRUE;
}

// Get the text color of the object.
COLORREF FusionAPI GetTextClr(mv * mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	// Example
	return 0;	// edPtr->fontColor;
}

// Set the text color of the object.
void FusionAPI SetTextClr(mv *mV, EDITDATA * edPtr, COLORREF color)
{
#pragma DllExportHint
	//edPtr->fontColor = color;
}

// Get the text alignment of the object.
std::uint32_t FusionAPI GetTextAlignment(mv *mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	unsigned int dw = 0;
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

// Set the text alignment of the object.
void FusionAPI SetTextAlignment(mv *mV, EDITDATA * edPtr, unsigned int AlignFlags)
{
#pragma DllExportHint
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

// Initialize custom parameter.
void FusionAPI InitParameter(mv * mV, short code, ParamExtension * pExt)
{
#pragma DllExportHint
	// strcpy(&pExt->pextData[0], "Parameter Test");
	// pExt->pextSize = sizeof(paramExt) + strlen(pExt->pextData)+1;
}

// Example of custom parameter setup proc
// --------------------------------------
/*
BOOL FusionAPI SetupProc(HWND hDlg, UINT msgType, WPARAM wParam, LPARAM lParam)
{
#pragma DllExportHint
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
*/

// Edit the custom parameter.
void FusionAPI EditParameter(mv *mV, short code, ParamExtension * pExt)
{
#pragma DllExportHint
	// Example
	// -------
	// DialogBoxParam(hInstLib, MAKEINTRESOURCE(DB_TRYPARAM), mV->mvHEditWin, SetupProc, (LPARAM)(LPBYTE)pExt);
}

// Initialize the custom parameter.
void FusionAPI GetParameterString(mv *mV, short code, ParamExtension * pExt, char * pDest, short size)
{
#pragma DllExportHint
	// Example
	// -------
	// wsprintf(pDest, "Super parameter %s", pExt->pextData);
}

#endif // EditorBuild
