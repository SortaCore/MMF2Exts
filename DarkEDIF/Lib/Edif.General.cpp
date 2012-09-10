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

short DLLExport GetRunObjectInfos(mv * mV, kpxRunInfos * infoPtr)
{
	infoPtr->Conditions = &::SDK->ConditionJumps[0];
	infoPtr->Actions = &::SDK->ActionJumps[0];
	infoPtr->Expressions = &::SDK->ExpressionJumps[0];

	infoPtr->NumOfConditions = ::SDK->json[CurLang]["Conditions"].u.object.length;
	infoPtr->NumOfActions = ::SDK->json[CurLang]["Actions"].u.object.length;
	infoPtr->NumOfExpressions = ::SDK->json[CurLang]["Expressions"].u.object.length;
	
	infoPtr->EDITDATASize = sizeof(EDITDATA);
#if 0 //NOPROPS
	{
		const json_value JSON = ::SDK->json[CurLang]["Properties"];
		for(unsigned int i = 0; i < JSON.u.object.length; ++i)
		{
			switch (::SDK->EdittimeProperties[i].Type_ID)
			{
				case Edif::Properties::PROPTYPE_EDIT_STRING:
				case Edif::Properties::PROPTYPE_STATIC:
					infoPtr->editDataSize += sizeof(Prop_AStr)+255;
					break;
				default:
					infoPtr->editDataSize += sizeof(Prop);
			}
			++infoPtr->editDataSize;
		}
	}
#endif //NOPROPS
	
	//+(GetPropertyChbx(edPtr, ::SDK->json[CurLang]["Properties"].u.object.length+1)-&edPtr);

    infoPtr->WindowProcPriority = Extension::WindowProcPriority;

	infoPtr->EditFlags = Extension::OEFLAGS;
	infoPtr->EditPrefs = Extension::OEPREFS;

    memcpy(&infoPtr->Identifier, ::SDK->json["Identifier"], 4);
	
    infoPtr->Version = Extension::Version;
	
	return TRUE;
}

extern "C" unsigned int DLLExport GetInfos(int info)
{
	switch (info)
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

short DLLExport HandleRunObject(RUNDATA * rdPtr)
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
