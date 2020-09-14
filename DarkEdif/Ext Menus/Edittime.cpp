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

	::SDK->Icon->Blit(*Surface, rc->left, rc->top, BMODE_TRANSP, BOP_COPY, 0);
}


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

#endif // EditorBuild
