
#include "Common.h"

LPCTSTR * Dependencies = 0;

LPCTSTR * WINAPI DLLExport GetDependencies()
{
	if(!Dependencies)
    {
        const json_value &DependenciesJSON = SDK->json["About"]["Dependencies"];

        Dependencies = new LPCTSTR [DependenciesJSON.u.array.length + 2];

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

        for(; i < DependenciesJSON.u.array.length; ++ i)
		{
			TCHAR* tstr = Edif::ConvertString(DependenciesJSON[i]);
            Dependencies[Offset + i] = tstr;
			Edif::FreeString(tstr);
		}

        Dependencies[Offset + i] = 0;
    }

    return Dependencies;
}

short WINAPI DLLExport GetRunObjectInfos(mv _far *mV, fpKpxRunInfos infoPtr)
{
	infoPtr->conditions = &SDK->ConditionJumps[0];
	infoPtr->actions = &SDK->ActionJumps[0];
	infoPtr->expressions = &SDK->ExpressionJumps[0];

	infoPtr->numOfConditions = SDK->json["Conditions"].u.array.length;
	infoPtr->numOfActions = SDK->json["Actions"].u.array.length;
	infoPtr->numOfExpressions = SDK->json["Expressions"].u.array.length;

	infoPtr->editDataSize = sizeof(EDITDATA);

    infoPtr->windowProcPriority = Extension::WindowProcPriority;

	infoPtr->editFlags = Extension::OEFLAGS;
	infoPtr->editPrefs = Extension::OEPREFS;

    memcpy(&infoPtr->identifier, SDK->json["About"]["Identifier"], 4);
	
    infoPtr->version = Extension::Version;
	
	return TRUE;
}

extern "C" 
{
	DWORD WINAPI DLLExport GetInfos(int info)
	{
		
		switch (info)
		{
			case KGI_VERSION:
				return EXT_VERSION2;

			case KGI_PLUGIN:
				return EXT_PLUGIN_VERSION1;

			case KGI_PRODUCT:

                #ifdef PROEXT
				    return PRODUCT_VERSION_DEV;
                #endif

                #ifdef TGFEXT
				    return PRODUCT_VERSION_HOME;
                #endif

                return PRODUCT_VERSION_STANDARD;

			case KGI_BUILD:
				return Extension::MinimumBuild;

            case KGI_UNICODE:

                #ifdef _UNICODE
                    return TRUE;
                #else
                    return FALSE;
                #endif

			default:
				return 0;
		}
	}
}

short WINAPI DLLExport CreateRunObject(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr)
{
    /* Global to all extensions! Use the constructor of your Extension class (Extension.cpp) instead! */

    rdPtr->pExtension = new Extension(rdPtr, edPtr, cobPtr);
	rdPtr->pExtension->Runtime.ObjectSelection.pExtension = rdPtr->pExtension;

	return 0;
}


/* Don't touch any of these, they're global to all extensions! See Extension.cpp */

short WINAPI DLLExport DestroyRunObject(LPRDATA rdPtr, long fast)
{
    delete rdPtr->pExtension;

	return 0;
}

short WINAPI DLLExport HandleRunObject(LPRDATA rdPtr)
{
    return rdPtr->pExtension->Handle();
}

short WINAPI DLLExport DisplayRunObject(LPRDATA rdPtr)
{
	return rdPtr->pExtension->Display();
}

ushort WINAPI DLLExport GetRunObjectDataSize(fprh rhPtr, LPEDATA edPtr)
{
	return(sizeof(RUNDATA));
}

short WINAPI DLLExport PauseRunObject(LPRDATA rdPtr)
{
	return rdPtr->pExtension->Pause();
}

short WINAPI DLLExport ContinueRunObject(LPRDATA rdPtr)
{
	return rdPtr->pExtension->Continue();
}

BOOL WINAPI SaveRunObject(LPRDATA rdPtr, HANDLE hf)
{            
    return rdPtr->pExtension->Save(hf) ? TRUE : FALSE;
}

BOOL WINAPI LoadRunObject(LPRDATA rdPtr, HANDLE hf)
{            
    return rdPtr->pExtension->Load(hf) ? TRUE : FALSE;
}

LPEVENTINFOS2 GetEventInformations(LPEVENTINFOS2 eiPtr, short code)
{
	while(eiPtr->infos.code != code)
		eiPtr = EVINFO2_NEXT(eiPtr);
	
	return eiPtr;
}
