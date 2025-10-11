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
int FusionAPI MakeIconEx(mv *mV, cSurface *pIconSf, TCHAR *lpName, ObjInfo *oiPtr, EDITDATA *edPtr)
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
	// Check compatibility
	if (!Edif::IS_COMPATIBLE(mV))
		return -1;
	Edif::Init(mV, edPtr);

	if (edPtr->eHeader.extSize < sizeof(EDITDATA))
	{
		void* newEd = mvReAllocEditData(mV, edPtr, sizeof(EDITDATA));
		if (!newEd)
			return DarkEdif::MsgBox::Error(_T("Invalid properties"), _T("Failed to allocate %zu bytes for properties."), sizeof(EDITDATA)), -1;
		edPtr = (EDITDATA *)newEd;
	}

	// Zero the padding and all data
	memset(((char *)edPtr) + sizeof(edPtr->eHeader), 0, sizeof(EDITDATA) - sizeof(EDITDATA::eHeader));

	// Set default object settings from DefaultState.
	const auto &propsJSON = CurLang["Properties"sv];
	edPtr->automaticClear = propsJSON[1]["DefaultState"sv];
	edPtr->isGlobal = propsJSON[2]["DefaultState"sv];
	if (strcpy_s(edPtr->edGlobalID, std::size(edPtr->edGlobalID), propsJSON[3]["DefaultState"sv].c_str()))
		DarkEdif::MsgBox::Error(_T("CreateObject() error"), _T("Error initialising property 3; error %i copying string."), errno);
	edPtr->multiThreading = propsJSON[4]["DefaultState"sv];
	edPtr->timeoutWarningEnabled = propsJSON[5]["DefaultState"sv];
	edPtr->fullDeleteEnabled = propsJSON[6]["DefaultState"sv];
	edPtr->enableInactivityTimer = propsJSON[7]["DefaultState"sv];

	// ext version 1 = before Unicode port, 2 = after. Does not mean the properties are UTF16; they're UTF8.
	edPtr->eHeader.extVersion = 2;

	// Since we're echoing Relay's property layout, we don't use DarkEdif's property manager.
	// InitializePropertiesFromJSON(mV, edPtr);

	return 0;
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

	Edif::SDK->Icon->Blit(*Surface, rc->left, rc->top, BMODE_TRANSP, BOP_COPY, 0);
}

// ============================================================================
// PROPERTIES
// ============================================================================

// Inserts properties into the properties of the object.
BOOL FusionAPI GetProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
	if (!Edif::IS_COMPATIBLE(mV))
		return FALSE;

	mvInsertProps(mV, edPtr, Edif::SDK->EdittimeProperties.get(), PROPID_TAB_GENERAL, TRUE);

	return TRUE; // OK
}

// Called when the properties are removed from the property window.
void FusionAPI ReleaseProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
}

// Returns the value of properties that have a value.
// Note: see GetPropCheck for checkbox properties
Prop *FusionAPI GetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	if (PropID < PROPID_EXTITEM_CUSTOM_FIRST)
		return NULL; // built-in properties, not managed by ext

	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;

	if ((unsigned int) CurLang["Properties"sv].u.object.length > ID)
	{
		if (ID == 0)
		{
			char extVerBuffer[256];
			sprintf_s(extVerBuffer, CurLang["Properties"sv][ID]["DefaultState"sv].c_str(), lacewing::relayserver::buildnum, STRIFY(CONFIG));
			return new Prop_Str(DarkEdif::UTF8ToTString(extVerBuffer).c_str());
		}
		if (ID == 3)
			return new Prop_Str(DarkEdif::UTF8ToTString(edPtr->edGlobalID).c_str());

		// Note that checkbox-only properties call GetPropValue() too for no reason, so
		return NULL;
	}

	DarkEdif::MsgBox::Error(_T("Unknown property"), _T("Unknown property ID %u given to GetPropValue() call."), ID);
	return NULL;
}

// Returns the checked state of properties that have a check box.
BOOL FusionAPI GetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	if (PropID < PROPID_EXTITEM_CUSTOM_FIRST)
		return FALSE; // built-in properties, not managed by ext

	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;

	if ((unsigned int) CurLang["Properties"sv].u.object.length > ID)
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
		if (ID == 7)
			return edPtr->enableInactivityTimer;
	}

	DarkEdif::MsgBox::Error(_T("Unknown property"), _T("Unknown property ID %u given to GetPropCheck() call."), ID);
	return FALSE; // Unchecked
}

// This routine is called by Fusion after a property has been modified.
void FusionAPI SetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID, Prop * NewParam)
{
#pragma DllExportHint
	if (PropID < PROPID_EXTITEM_CUSTOM_FIRST)
		return; // built-in properties, not managed by ext

	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;
	if (ID == 3)
	{
		const std::string newValAsU8 = DarkEdif::TStringToUTF8(((Prop_Str *)NewParam)->String);
		if (strcpy_s(edPtr->edGlobalID, newValAsU8.c_str()))
			DarkEdif::MsgBox::Error(_T("SetPropValue() error"), _T("Error %i setting global ID; too long?"), errno);
	}

	// You may want to have your object redrawn in the frame editor after the modifications,
	// in this case, just call this function
	// mvInvalidateObject(mV, edPtr);
}

// This routine is called by Fusion when the user modifies a checkbox in the properties.
void FusionAPI SetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID, BOOL Check)
{
#pragma DllExportHint
	if (PropID < PROPID_EXTITEM_CUSTOM_FIRST)
		return; // built-in properties, not managed by ext

	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;

	if (CurLang["Properties"sv].u.object.length > ID)
	{
		// The ending ", (void)0" means that the expression evaluates to void overall
		if (ID == 1)
			return edPtr->automaticClear = (Check != 0), (void)0;
		if (ID == 2)
			return edPtr->isGlobal = (Check != 0), (void)0;
		if (ID == 4)
			return edPtr->multiThreading = (Check != 0), (void)0;
		if (ID == 5)
			return edPtr->timeoutWarningEnabled = (Check != 0), (void)0;
		if (ID == 6)
			return edPtr->fullDeleteEnabled = (Check != 0), (void)0;
		if (ID == 7)
			return edPtr->enableInactivityTimer = (Check != 0), (void)0;
	}

	DarkEdif::MsgBox::Error(_T("Unknown property"), _T("Unknown property ID %u given to SetPropCheck() call."), ID);
}

// This routine is called by Fusion when an Android build happens, in the Extensions[\Unicode] MFX.
// It enables you to modify the manifest file to add your own content, or otherwise check the Android build.
void FusionAPI PrepareAndroidBuild(mv* mV, EDITDATA* edPtr, LPCTSTR androidDirectoryPathname)
{
#pragma DllExportHint

	// Android permissions: 4 is access network state, 6 access wifi state, 12 bluetooth, 13 bluetooth admin, 49 internet,
	// 114 nfc, 24 change network state, 25 change wifi multicast state, 26 change wifi state
	//
	// The permissions do not 1:1 match the index in the Fusion properties window, so you'll have to loop through them to work out the ID.
	// They match between CF2.5 and MMF2.0, but 2.0 lacks some permissions (105+, so including NFC).

	DWORD hasINTERNETPerm = mvGetAppPropCheck(mV, edPtr, PROPID_APP_ANDROID_PERM_FIRST + 49);
	if (hasINTERNETPerm == TRUE)
		return;

	DarkEdif::MsgBox::Error(_T("Invalid Android properties!"), _T("To use Bluewing Server, please enable the INTERNET permission in application "
		"properties under the Android tab.\nAborting build with a SAXParseException."));

	// Erase the manifest file so the build will fail
	std::tstring manifestPath = androidDirectoryPathname;
	manifestPath += _T("app\\src\\main\\AndroidManifest.xml"sv);
	FILE * manifest = _tfopen(manifestPath.c_str(), _T("wb"));
	fputs("<!-- Enable the INTERNET permission! ~love from " PROJECT_NAME " -->", manifest);
	fclose(manifest);
}

#endif // EditorBuild
