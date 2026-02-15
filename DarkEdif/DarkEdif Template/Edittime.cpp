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
#if DARKEDIF_DISPLAY_TYPE > DARKEDIF_DISPLAY_ANIMATIONS
	edPtr->objSize = DarkEdif::Size { 32, 32 }; // size of icon
#endif
#if TEXT_OEFLAG_EXTENSION
	edPtr->font.Initialize(mV);
#endif

	return DarkEdif::DLL::DLL_CreateObject(mV, loPtr, edPtr);
}

// Displays the object under the frame editor
void FusionAPI EditorDisplay(mv * mV, ObjectInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr, DarkEdif::Rect * rc)
{
#pragma DllExportHint
	auto frameSurf = DarkEdif::Surface::CreateFromFrameEditorWindow();

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
	Edif::SDK->ExtIcon->CopyToPoint(frameSurf, ((DarkEdif::Rect *)rc)->GetTopLeft());
#if DARKEDIF_DISPLAY_TYPE > DARKEDIF_DISPLAY_NONE && IS_DARKEDIF_TEMPLATE==0
	#error Add your drawing tech here, by accesing frameSurf, for example:
	frameSurf.FillImageWith(DarkEdif::SurfaceFill::Solid(DarkEdif::ColorRGB(0, 0, 128)));
#endif
}

// This routine tells Fusion if the mouse pointer is over a transparent zone of the object.
// If not exported, the entire display is assumed to be opaque.
/* BOOL FusionAPI IsTransparent(mv* mV, LevelObject* loPtr, EDITDATA* edPtr, int dx, int dy)
{
#pragma DllExportHint
	// const DarkEdif::Point pt { dx, dy };

	// Don't forget to pre-set the size in CreateObject() above,
	// and uncomment GetObjectRect() below so Fusion can read the current size.
	// You should display in EditorDisplay() above.
	return FALSE;
}
*/

#if DARKEDIF_DISPLAY_TYPE > DARKEDIF_DISPLAY_NONE
// Called when the object has been resized
// If this function is not exported, your object will not be resizeable
/*
BOOL FusionAPI SetEditSize(mv* mV, EDITDATA* edPtr, int cx, int cy)
{
#pragma DllExportHint
	// Check compatibility
	if (!Edif::IS_COMPATIBLE(mV))
		return FALSE;

	// Don't forget to pre-set the size in CreateObject() above,
	// and uncomment GetObjectRect() below so Fusion can read the current size.
	// You should display in EditorDisplay() above.
	edPtr->objSize = DarkEdif::Size { cx, cy };
	return TRUE;
}
*/

// Returns the size of the rectangle of the object in the frame editor.
// If this function isn't defined, a size of 32x32 is assumed.
/*
void FusionAPI GetObjectRect(mv * mV, DarkEdif::Rect * rc, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	if (!mV || !rc || !edPtr)
		return;

	// Don't forget to pre-set the size in CreateObject() above,
	// and uncomment SetEditSize() if you want the user to be able to resize it.
	// You should display it in EditorDisplay() above.
	rc->right = rc->left + edPtr->objSize.width;
	rc->bottom = rc->top + edPtr->objSize.height;
}
*/

#endif // drawing ext

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

// Called by Fusion when the user clicks the button of a Button, Image List or Font property.
BOOL FusionAPI EditProp(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	if (PropID < PROPID_EXTITEM_CUSTOM_FIRST)
		return FALSE;
	return DarkEdif::DLL::DLL_EditProp(mV, edPtr, PropID);
}

// Called by Fusion to request the enabled state of a property.
BOOL FusionAPI IsPropEnabled(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_IsPropEnabled(mV, edPtr, PropID);
}

// Called when a property is initialized and its creation parameter is NULL (in the PropData).
// Allows you, for example, to change the content of a combobox property according to specific settings in the EDITDATA structure.
LPARAM FusionAPI GetPropCreateParam(mv *mV, EDITDATA *edPtr, unsigned int PropID)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_GetPropCreateParam(mV, edPtr, PropID);
}

// Called after a property has been initialized.
// Allows you, for example, to free memory allocated in GetPropCreateParam.
void FusionAPI ReleasePropCreateParam(mv *mV, EDITDATA *edPtr, unsigned int PropID, LPARAM lParam)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_ReleasePropCreateParam(mV, edPtr, PropID, lParam);
}

// ============================================================================
// ROUTINES USED WHEN BUILDING
// ============================================================================

/*
// This routine is called by Fusion when an Android build is prepared before building.
// It enables you to modify the Android files to add your own content, or otherwise check the Android build.
// It is called in the Extensions[\Unicode] MFX, for any extension in the MFA that defines PrepareAndroidBuild,
// including for exts that have no corresponding Data\Runtime\Android file and would create a not-compatible build warning.
void FusionAPI PrepareAndroidBuild(mv* mV, EDITDATA* edPtr, const TCHAR * androidDirectoryPathname)
{
#pragma DllExportHint
	// Erase the manifest file so the build will fail
	std::tstring manifestPath = androidDirectoryPathname;
	manifestPath += _T("app\\src\\main\\AndroidManifest.xml"sv);
	// Open manifestPath as a file, e.g. with
	// _tfopen(manifestPath.c_str(), _T("ab"))
	// and you're free to edit the manifest.
	// You can also add a DarkEdif::MsgBox call, look at the path while the popup is open.
}
*/

#endif // EditorBuild
