// ============================================================================
// This file contains routines that are handled only during the Edittime,
// under the Frame and Event editors.
//
// Including creating, display, and setting up your object.
// ============================================================================

#include "Common.hpp"

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
	pIconSf->Clone(*Edif::SDK->Icon);

	pIconSf->SetTransparentColor(RGB(255, 0, 255));
	return 0;
}

// Called when you choose "Create new object". It should display the setup box
// and initialize everything in the datazone.
int FusionAPI CreateObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	if (!Edif::IS_COMPATIBLE(mV))
		return -1;

	Edif::Init(mV, edPtr);

	// Init any custom EDITDATA stuff here, between Init and DLL_CreateObject

	return DarkEdif::DLL::DLL_CreateObject(mV, loPtr, edPtr);
}

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

	// Draw to surface here with Surface->Rectangle(), Surface->Line(), Surface->SetPixel(), etc.
	// Note writing text is not possible in Direct3D display mode, unless you create a
	// software + DC surface type, write the text there, then blit it into Surface.
	// RunUpdateNotifs does this, to write "minor/major update" text onto the ext icon.
	//
	// Positions are absolute, and refer to the whole Fusion editor window, so to draw
	// on your object's left, draw at X = rc->left, and be careful to only use the
	// Surface functions that take X/Y positions.
	//
	// If you're making your object different to 32x32, don't forget to
	// pre-set the size in CreateObject() above, and uncomment GetObjectRect() below
	// so Fusion can read the current size.
	// If it's partially transparent, implement IsTransparent() below as well.
	// If it's resizeable, also implement SetEditSize() below.

	// If you don't want to draw the icon, you can take this out.
	// To display updates in a custom way, you can use DarkEdif::SDKUpdater::ReadUpdateStatus(NULL);
	// to get the type of update.
	// Note the message box for major updates will still show, and cannot be disabled.
	Edif::SDK->Icon->Blit(*Surface, rc->left, rc->top, BMODE_TRANSP, BOP_COPY, 0);
}

// This routine tells Fusion if the mouse pointer is over a transparent zone of the object.
// If not exported, the entire display is assumed to be opaque.
/*
BOOL FusionAPI IsTransparent(mv *mV, LevelObject * loPtr, EDITDATA * edPtr, int dx, int dy)
{
#pragma DllExportHint
	// Don't forget to pre-set the size in CreateObject() above,
	// and uncomment GetObjectRect() below so Fusion can read the current size.
	// You should display in EditorDisplay() above.
	return FALSE;
}
*/

// Called when the object has been resized
/*
BOOL FusionAPI SetEditSize(mv * mv, EDITDATA * edPtr, int cx, int cy)
{
#pragma DllExportHint
	// Check compatibility
	if (!Edif::IS_COMPATIBLE(mV))
		return FALSE;

	// Don't forget to pre-set the size in CreateObject() above,
	// and uncomment GetObjectRect() below so Fusion can read the current size.
	// You should display in EditorDisplay() above.
	edPtr->swidth = cx;
	edPtr->sheight = cy;
	return TRUE;
}
*/

// Returns the size of the rectangle of the object in the frame editor.
// If this function isn't defined, a size of 32x32 is assumed.
/* void FusionAPI GetObjectRect(mv * mV, RECT * rc, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	if (!mV || !rc || !edPtr)
		return;

	// Don't forget to pre-set the size in CreateObject() above,
	// and uncomment SetEditSize() if you want the user to be able to resize it.
	// You should display it in EditorDisplay() above.
	rc->right = rc->left + Edif::SDK->Icon->GetWidth();	// edPtr->swidth;
	rc->bottom = rc->top + Edif::SDK->Icon->GetHeight();	// edPtr->sheight;
}*/

// Called when the user selects the Edit command in the object's popup menu
/*
BOOL FusionAPI EditObject(mv *mV, ObjInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	// Check compatibility
	if (!Edif::IS_COMPATIBLE(mV))
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
	return DarkEdif::DLL::DLL_GetProperties(mV, edPtr, bMasterItem);
}

// Called when the properties are removed from the property window.
void FusionAPI ReleaseProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_ReleaseProperties(mV, edPtr, bMasterItem);
}

// Returns the value of properties that have a value.
// Note: see GetPropCheck for checkbox properties
void * FusionAPI GetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_GetPropValue(mV, edPtr, PropID);
}

// Returns the checked state of properties that have a check box.
BOOL FusionAPI GetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_GetPropCheck(mV, edPtr, PropID);
}

// Called by Fusion after a property has been modified.
void FusionAPI SetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID, void * Param)
{
#pragma DllExportHint
	DarkEdif::DLL::DLL_SetPropValue(mV, edPtr, PropID, Param);
}

// Called by Fusion when the user modifies a checkbox in the properties.
void FusionAPI SetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID, BOOL checked)
{
#pragma DllExportHint
	DarkEdif::DLL::DLL_SetPropCheck(mV, edPtr, PropID, checked);
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
BOOL FusionAPI IsPropEnabled(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	// Example
	// -------
/*
	switch (nPropID) {

	case PROPID_CHECK:
		return (edPtr->nComboIndex != 0);
	}
*/
	return DarkEdif::DLL::DLL_IsPropEnabled(mV, edPtr, PropID);
}


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

	return DarkEdif::DLL::DLL_GetPropCreateParam(mV, edPtr, PropID);
}*/

// Called after a property has been initialized.
// Allows you, for example, to free memory allocated in GetPropCreateParam.
/*void FusionAPI ReleasePropCreateParam(mv *mV, EDITDATA *edPtr, unsigned int PropID, LPARAM lParam)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_ReleasePropCreateParam(mV, edPtr, PropID, lParam);
}*/

// ============================================================================
// TEXT PROPERTIES
// ============================================================================

// Return the text capabilities of the object under the frame editor; affects what options
// appear if you right-click the object in frame editor and use Text submenu.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
/*std::uint32_t FusionAPI GetTextCaps(mv * mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	return 0;	// (TEXT_ALIGN_LEFT|TEXT_ALIGN_HCENTER|TEXT_ALIGN_RIGHT|TEXT_ALIGN_TOP|TEXT_ALIGN_VCENTER|TEXT_ALIGN_BOTTOM|TEXT_FONT|TEXT_COLOR);
}*/

// Return the font used in the object.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
//			Must be defined if GetTextCaps() returns TEXT_FONT.
//			The pStyle and cbSize parameters are obsolete in MMF2 and passed for compatibility reasons only.
/*BOOL FusionAPI GetTextFont(mv * mV, EDITDATA * edPtr, LOGFONT * Font, TCHAR * pStyle, unsigned int cbSize)
{
#pragma DllExportHint
	// Example: copy LOGFONT structure from EDITDATA
	// memcpy(plf, &edPtr->m_lf, sizeof(LOGFONT));

	return TRUE;
}*/

// Change the font used the object.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
//			Must be defined if GetTextCaps() returns TEXT_FONT.
//			The pStyle and cbSize parameters are obsolete in MMF2 and passed for compatibility reasons only.
/*BOOL FusionAPI SetTextFont(mv * mV, EDITDATA * edPtr, LOGFONT * Font, [[deprecated]] const char * pStyle)
{
#pragma DllExportHint
	// Example: copy LOGFONT structure to EDITDATA
	// memcpy(&edPtr->m_lf, plf, sizeof(LOGFONT));

	return TRUE;
}*/

// Get the text color of the object.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
//			Must be defined if GetTextCaps() returns TEXT_COLOR.
/*COLORREF FusionAPI GetTextClr(mv * mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	return 0; // try RGB()
}*/

// Called by Fusion to set the text color of the object.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
//			Must be defined if GetTextCaps() returns TEXT_COLOR.
/*void FusionAPI SetTextClr(mv *mV, EDITDATA * edPtr, COLORREF color)
{
#pragma DllExportHint
	// Example
	// edPtr->fontColor = color;
}*/

// Get the text alignment of the object.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
//			Must be defined if GetTextCaps() returns any TEXT_ALIGN_XXX flags
/*std::uint32_t FusionAPI GetTextAlignment(mv *mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	return 0;
}*/

// Set the text alignment of the object.
// @remarks Introduced in MMF1.5, missing in MMF1.2 and below.
//			Must be defined if GetTextCaps() returns any TEXT_ALIGN_XXX flags
/*void FusionAPI SetTextAlignment(mv *mV, EDITDATA * edPtr, unsigned int AlignFlags)
{
#pragma DllExportHint
}*/


// ============================================================================
// ROUTINES USED WHEN BUILDING
// ============================================================================

// This routine is called by Fusion when an Android build is prepared before building.
// It enables you to modify the Android manifest file to add your own content, or otherwise check the Android build.
// It is called in the Extensions[\Unicode] MFX, for any extension in the MFA that defines PrepareAndroidBuild,
// including exts that have no corresponding Data\Runtime\Android file and would create a not-compatible build warning.
/*void FusionAPI PrepareAndroidBuild(mv* mV, EDITDATA* edPtr, LPCTSTR androidDirectoryPathname)
{
#pragma DllExportHint
	// Erase the manifest file so the build will fail
	std::tstring manifestPath = androidDirectoryPathname;
	manifestPath += _T("app\\src\\main\\AndroidManifest.xml"sv);
	// Open manifestPath as a file, say with
	// _tfopen(manifestPath.c_str(), _T("ab"))
	// and you're free to edit the manifest
}*/
#endif // EditorBuild
