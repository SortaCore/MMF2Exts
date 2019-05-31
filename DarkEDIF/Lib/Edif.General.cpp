#include "Common.h"

const TCHAR ** Dependencies = 0;

const TCHAR ** DLLExport GetDependencies()
{
	if (!Dependencies)
    {
        const json_value &DependenciesJSON = SDK->json["Dependencies"];

        Dependencies = new const TCHAR * [DependenciesJSON.u.object.length + 2];

        int Offset = 0;

        if (Edif::ExternalJSON)
        {
            TCHAR JSONFilename [MAX_PATH];

            GetModuleFileName (hInstLib, JSONFilename, sizeof (JSONFilename)/sizeof(TCHAR));

            TCHAR * Iterator = JSONFilename + _tcslen(JSONFilename) - 1;

            while(*Iterator != '.')
                -- Iterator;

            _tcscpy(++ Iterator, _T("json"));

            Iterator = JSONFilename + _tcslen(JSONFilename) - 1;

            while(*Iterator != '\\' && *Iterator != '/')
                -- Iterator;

            Dependencies [Offset ++] = ++ Iterator;
        }

        unsigned int i = 0;

        for(; i < DependenciesJSON.u.object.length; ++ i)
		{
			TCHAR* tstr = Edif::ConvertString(DependenciesJSON[i]);
            Dependencies[Offset + i] = tstr;
			Edif::FreeString(tstr);
		}

        Dependencies[Offset + i] = 0;
    }

    return Dependencies;
}

/// <summary> Called every time the extension is being created from nothing.
///           Default property contents should be loaded from JSON. </summary>
short DLLExport GetRunObjectInfos(mv * mV, kpxRunInfos * infoPtr)
{
	infoPtr->Conditions = &::SDK->ConditionJumps[0];
	infoPtr->Actions = &::SDK->ActionJumps[0];
	infoPtr->Expressions = &::SDK->ExpressionJumps[0];

	infoPtr->NumOfConditions = CurLang["Conditions"].u.object.length;
	infoPtr->NumOfActions = CurLang["Actions"].u.object.length;
	infoPtr->NumOfExpressions = CurLang["Expressions"].u.object.length;

	static unsigned short EDITDATASize = 0;
	if (EDITDATASize == 0)
	{
		infoPtr->EDITDATASize = sizeof(EDITDATA);
#ifndef NOPROPS
		const json_value& JSON = CurLang["Properties"];
		size_t fullSize = sizeof(EDITDATA);
		// Store one bit per property, for any checkboxes
		fullSize += (int)ceil(JSON.u.array.length / 8.0);

		for (unsigned int i = 0; i < JSON.u.array.length; ++i)
		{
			const json_value& propjson = *JSON.u.array.values[i];
			const char * curPropType = propjson["Type"];

			if (!_stricmp(curPropType, "Editbox String"))
			{
				const char * defaultText = CurLang["Properties"]["DefaultState"];
				fullSize += (defaultText ? strlen(defaultText) : 0) + 1; // UTF-8
			}
			// Stores a number (in combo box, an index)
			else if (!_stricmp(curPropType, "Editbox Number") || !_stricmp(curPropType, "Combo Box"))
				fullSize += sizeof(int);
			// No content, or already stored in checkbox part before this for loop
			else if (!_stricmp(curPropType, "Text") || !_stricmp(curPropType, "Checkbox") ||
				// Folder or FolderEnd - no data, folders are cosmetic
				!_strnicmp(curPropType, "Folder", sizeof("Folder") - 1) ||
				// Buttons - no data, they're just clickable
				!_stricmp(curPropType, "Edit button"))
			{
				// skip 'em, no changeable data
			}
			else
			{
				std::stringstream err;
				err << "Can't handle property type \"" << curPropType << "\".";
				MessageBoxA(NULL, err.str().c_str(), "DarkEDIF - Error", MB_OK | MB_ICONERROR);
			}
		}
		// Too large for EDITDATASize
		if (fullSize > MAXUINT16)
			MessageBoxA(NULL, "Property default sizes are too large.", "DarkEDIF - Error", MB_OK | MB_ICONERROR);
		else
			infoPtr->EDITDATASize = EDITDATASize = (unsigned short)fullSize;
	}
#endif //NOPROPS
	
	//+(GetPropertyChbx(edPtr, CurLang["Properties"].u.object.length+1)-&edPtr);

    infoPtr->WindowProcPriority = Extension::WindowProcPriority;

	infoPtr->EditFlags = Extension::OEFLAGS;
	infoPtr->EditPrefs = Extension::OEPREFS;

    memcpy(&infoPtr->Identifier, ::SDK->json["Identifier"], 4);
	
    infoPtr->Version = Extension::Version;
	
	return TRUE;
}

extern "C" unsigned int DLLExport GetInfos(int info)
{
	switch ((KGI)info)
	{
		case KGI::VERSION:
			return 0x300; // I'm a MMF2 extension!

		case KGI::PLUGIN:
			return 0x100; // Version 1 type o' plugin

		case KGI::PRODUCT:
			#ifdef PROEXT
				return 3; // MMF Developer
			#endif
			#ifdef TGFEXT
				return 1; // TGF2
			#endif
			return 2;		// MMF Standard

		case KGI::BUILD:
			return Extension::MinimumBuild;

	#ifdef _UNICODE
		case KGI::UNICODE:
			return TRUE;	// I'm building in Unicode
	#endif

		default:
			return 0;
	}
}

short DLLExport CreateRunObject(RUNDATA * rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr)
{
    /* Global to all extensions! Use the constructor of your Extension class (Extension.cpp) instead! */

    rdPtr->pExtension = new Extension(rdPtr, edPtr, cobPtr);
	rdPtr->pExtension->Runtime.ObjectSelection.pExtension = rdPtr->pExtension;
	
	return 0;
}


/* Don't touch any of these, they're global to all extensions! See Extension.cpp */

short DLLExport DestroyRunObject(RUNDATA * rdPtr, long fast)
{
    delete rdPtr->pExtension;
	rdPtr->pExtension = NULL;

	return 0;
}

REFLAG DLLExport HandleRunObject(RUNDATA * rdPtr)
{
    return rdPtr->pExtension->Handle();
}

short DLLExport DisplayRunObject(RUNDATA * rdPtr)
{
	return rdPtr->pExtension->Display();
}

unsigned short DLLExport GetRunObjectDataSize(RunHeader * rhPtr, EDITDATA * edPtr)
{
	return (sizeof(RUNDATA));
}

short DLLExport PauseRunObject(RUNDATA * rdPtr)
{
	return rdPtr->pExtension->Pause();
}

short DLLExport ContinueRunObject(RUNDATA * rdPtr)
{
	return rdPtr->pExtension->Continue();
}

BOOL SaveRunObject(RUNDATA * rdPtr, HANDLE hf)
{            
    return rdPtr->pExtension->Save(hf) ? TRUE : FALSE;
}

BOOL LoadRunObject(RUNDATA * rdPtr, HANDLE hf)
{            
    return rdPtr->pExtension->Load(hf) ? TRUE : FALSE;
}
