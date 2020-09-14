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
int FusionAPI MakeIconEx(mv * mV, cSurface * pIconSf, TCHAR * lpName, ObjInfo * oiPtr, EDITDATA * edPtr)
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
	if (!IS_COMPATIBLE(mV))
		return -1;

	Edif::Init(mV, edPtr);
	return 0;
}

cSurface * FusionAPI WinGetSurface(int idWin, int surfID = 0);
// Displays the object under the frame editor
void FusionAPI EditorDisplay(mv *mV, ObjectInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr, RECT * rc)
{
#pragma DllExportHint
	cSurface * Surface = WinGetSurface((int) mV->IdEditWin);
	if (!Surface)
		return;

	// If you don't have this function run in Edittime.cpp, SDK Updater will be disabled for your ext
	// Don't comment or preprocessor-it out if you're removing it; delete the line entirely.
	DarkEdif::SDKUpdater::RunUpdateNotifs(mV, edPtr);

	::SDK->Icon->Blit(*Surface, rc->left, rc->top, BMODE_TRANSP, BOP_COPY, 0);
}

// This routine tells MMF2 if the mouse pointer is over a transparent zone of the object.
// If not exported, the entire display is assumed to be opaque.
/*
BOOL FusionAPI IsTransparent(mv *mV, LevelObject * loPtr, EDITDATA * edPtr, int dx, int dy)
{
#pragma DllExportHint
	return FALSE;
}
*/

// Called when the object has been resized
/*
BOOL FusionAPI SetEditSize(mv * mv, EDITDATA * edPtr, int cx, int cy)
{
#pragma DllExportHint
	// Check compatibility
	if (!IS_COMPATIBLE(mV))
		return FALSE;

	edPtr->swidth = cx;
	edPtr->sheight = cy;
	return TRUE;
}
*/

// Returns the size of the rectangle of the object in the frame editor.
// If this function isn't define, a size of 32x32 is assumed.
/* void FusionAPI GetObjectRect(mv * mV, RECT * rc, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	if (!mV || !rc || !edPtr)
		return;

	rc->right = rc->left + SDK->Icon->GetWidth();	// edPtr->swidth;
	rc->bottom = rc->top + SDK->Icon->GetHeight();	// edPtr->sheight;
}*/

// Called when the user selects the Edit command in the object's popup menu
/*
BOOL FusionAPI EditObject(mv *mV, ObjInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	// Check compatibility
	if (!IS_COMPATIBLE(mV))
		return FALSE;

	// do stuff
	return TRUE;
}
*/


// ============================================================================
// PROPERTIES
// ============================================================================

// Inserts properties into the properties of the object.
BOOL FusionAPI GetProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
	mvInsertProps(mV, edPtr, SDK->EdittimeProperties, PROPID_TAB_GENERAL, TRUE);

	if (edPtr->DarkEdif_Prop_Size == 0)
	{
		InitialisePropertiesFromJSON(mV, edPtr);
		mvInvalidateObject(mV, edPtr);
	}

	// OK
	return TRUE;
}

// Called when the properties are removed from the property window.
void FusionAPI ReleaseProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
}

// Returns the value of properties that have a value.
// Note: see GetPropCheck for checkbox properties
Prop * FusionAPI GetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID_)
{
#pragma DllExportHint
	std::uint32_t PropID = (PropID_ - 0x80000) % 1000;
	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
		return NULL;

	return GetProperty(edPtr, PropID);
}

// Returns the checked state of properties that have a check box.
BOOL FusionAPI GetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID_)
{
#pragma DllExportHint
	std::uint32_t PropID = (PropID_ - 0x80000) % 1000;

	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
		return FALSE;

	return (edPtr->DarkEdif_Props[PropID >> 3] >> (PropID % 8) & 1);
}

// Called by Fusion after a property has been modified.
void FusionAPI SetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID_, void * Param)
{
#pragma DllExportHint
	Prop * prop = (Prop *)Param;

	unsigned int i = prop->GetClassID(), PropID = (PropID_ - 0x80000) % 1000;

	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
	{
#ifdef _DEBUG
		std::stringstream str;
		str << "Accessed property ID " << PropID << ", outside of custom extension range; ignoring it.\n";
		OutputDebugStringA(str.str().c_str());
#endif
		return;
	}

	switch (i)
	{
		case 'DATA': // Buffer or string
		{
			const json_value & propjson = CurLang["Properties"][PropID];
			// Buff can be used for a string property
			if (!_stricmp(propjson["Type"], "Editbox String"))
			{
				std::string utf8Str = TStringToUTF8(((Prop_Str *)prop)->String);
				PropChange(mV, edPtr, PropID, utf8Str.c_str(), utf8Str.size() + 1);
			}
			// If we get a Buff and it's not a string property, DarkEdif doesn't know how to handle it.
			else
				MessageBoxA(NULL, "ERROR: Got Buff type for non-string property.", "DarkEdif - Property error", MB_OK);
			break;
		}
		case 'STRA': // ANSI string
		{
			std::string utf8Str = ANSIToUTF8(((Prop_AStr *)prop)->String);
			PropChange(mV, edPtr, PropID, utf8Str.c_str(), utf8Str.size() + 1);
			break;
		}
		case 'STRW': // Unicode string
		{
			std::string utf8Str = WideToUTF8(((Prop_WStr *)prop)->String);
			PropChange(mV, edPtr, PropID, utf8Str.c_str(), utf8Str.size() + 1);
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
			// PropChange(mV, edPtr, PropID, prop2->GetPropValue(), prop2->GetPropValueSize());

			MessageBoxA(NULL, "Assuming class ID is custom - but no custom code yet written.", "DarkEdif - Error", MB_OK);
			break;
		}
	}
}

// Called by Fusion when the user modifies a checkbox in the properties.
void FusionAPI SetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID_, BOOL checked)
{
#pragma DllExportHint
	std::uint32_t PropID = (PropID_ - 0x80000) % 1000;
	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
		return;

	PropChangeChkbox(edPtr, PropID, checked != FALSE);
}

// Called by Fusion when the user clicks the button of a Button or EditButton property.
/*BOOL FusionAPI EditProp(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	// Example
	// -------
/*
	if (nPropID==PROPID_EDITCONTENT)
	{
		if ( EditObject(mV, NULL, NULL, edPtr) )
			return TRUE;
	}
*\/

	return FALSE;
}*/

// Called by Fusion to request the enabled state of a property.
/*BOOL FusionAPI IsPropEnabled(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	// Example
	// -------
/*
	switch (nPropID) {

	case PROPID_CHECK:
		return (edPtr->nComboIndex != 0);
	}
*\/
	return TRUE;
}*/


// Called when a property is initialized and its creation parameter is NULL (in the PropData).
// Allows you, for example, to change the content of a combobox property according to specific settings in the EDITDATA structure.
/*LPARAM FusionAPI GetPropCreateParam(mv *mV, EDITDATA *edPtr, unsigned int PropID)
{
#pragma DllExportHint
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
}*/

// Called after a property has been initialized.
// Allows you, for example, to free memory allocated in GetPropCreateParam.
/*void FusionAPI ReleasePropCreateParam(mv *mV, EDITDATA *edPtr, unsigned int PropID, LPARAM lParam)
{
#pragma DllExportHint
}*/

// ============================================================================
// TEXT PROPERTIES
// ============================================================================

// Return the text capabilities of the object under the frame editor.
/*std::uint32_t FusionAPI GetTextCaps(mv * mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	return 0;	// (TEXT_ALIGN_LEFT|TEXT_ALIGN_HCENTER|TEXT_ALIGN_RIGHT|TEXT_ALIGN_TOP|TEXT_ALIGN_VCENTER|TEXT_ALIGN_BOTTOM|TEXT_FONT|TEXT_COLOR);
}*/

// Return the font used the object.
// Note: the pStyle and cbSize parameters are obsolete and passed for compatibility reasons only.
/*BOOL FusionAPI GetTextFont(mv * mV, EDITDATA * edPtr, LOGFONT * Font, TCHAR * pStyle, unsigned int cbSize)
{
#pragma DllExportHint
	// Example: copy LOGFONT structure from EDITDATA
	// memcpy(plf, &edPtr->m_lf, sizeof(LOGFONT));

	return TRUE;
}*/

// Change the font used the object.
// Note: the pStyle parameter is obsolete and passed for compatibility reasons only.
//
/*BOOL FusionAPI SetTextFont(mv * mV, EDITDATA * edPtr, LOGFONT * Font, [[deprecated]] const char * pStyle)
{
#pragma DllExportHint
	// Example: copy LOGFONT structure to EDITDATA
	// memcpy(&edPtr->m_lf, plf, sizeof(LOGFONT));

	return TRUE;
}*/

// Get the text color of the object.
/*COLORREF FusionAPI GetTextClr(mv * mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	return 0; // try RGB()
}*/

// Called by Fusion to set the text color of the object.
/*void FusionAPI SetTextClr(mv *mV, EDITDATA * edPtr, COLORREF color)
{
#pragma DllExportHint
	// Example
	// edPtr->fontColor = color;
}*/

// Get the text alignment of the object.
/*std::uint32_t FusionAPI GetTextAlignment(mv *mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	return 0;
}*/

// Set the text alignment of the object.
/*void FusionAPI SetTextAlignment(mv *mV, EDITDATA * edPtr, unsigned int AlignFlags)
{
#pragma DllExportHint
}*/

#endif // EditorBuild
