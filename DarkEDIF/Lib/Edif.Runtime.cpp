#include "Common.h"

Edif::Runtime::Runtime(LPRDATA _rdPtr) : rdPtr(_rdPtr), ObjectSelection(_rdPtr->rHo.hoAdRunHeader)
{
}

Edif::Runtime::~Runtime()
{
}

void Edif::Runtime::Rehandle()
{
    callRunTimeFunction(rdPtr, RFUNCTION_REHANDLE, 0, 0);
}

void Edif::Runtime::GenerateEvent(int EventID)
{
    callRunTimeFunction(rdPtr, RFUNCTION_GENERATEEVENT, EventID, 0);
}

void Edif::Runtime::PushEvent(int EventID)
{
    callRunTimeFunction(rdPtr, RFUNCTION_PUSHEVENT, EventID, 0);
}

void * Edif::Runtime::Allocate(size_t Size)
{
    return (void *) callRunTimeFunction(rdPtr, RFUNCTION_GETSTRINGSPACE_EX, 0, Size * sizeof(TCHAR));
}

TCHAR * Edif::Runtime::CopyString(const TCHAR * String)
{
    TCHAR * New = (TCHAR *) Allocate(_tcslen(String) + 1);
    _tcscpy(New, String);
    
    return New;
}

void Edif::Runtime::Pause()
{
    callRunTimeFunction(rdPtr, RFUNCTION_PAUSE, 0, 0);
}

void Edif::Runtime::Resume()
{
    callRunTimeFunction(rdPtr, RFUNCTION_CONTINUE, 0, 0);
}

void Edif::Runtime::Redisplay()
{
    callRunTimeFunction(rdPtr, RFUNCTION_REDISPLAY, 0, 0);
}

void Edif::Runtime::GetApplicationDrive(TCHAR * Buffer)
{
    callRunTimeFunction(rdPtr, RFUNCTION_GETFILEINFOS, FILEINFO_DRIVE, (long) Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
    callRunTimeFunction(rdPtr, RFUNCTION_GETFILEINFOS, FILEINFO_DIR, (long) Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
    callRunTimeFunction(rdPtr, RFUNCTION_GETFILEINFOS, FILEINFO_PATH, (long) Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
    callRunTimeFunction(rdPtr, RFUNCTION_GETFILEINFOS, FILEINFO_APPNAME, (long) Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
    callRunTimeFunction(rdPtr, RFUNCTION_GETFILEINFOS, FILEINFO_TEMPPATH, (long) Buffer);
}

void Edif::Runtime::Redraw()
{
    callRunTimeFunction(rdPtr, RFUNCTION_REDRAW, 0, 0);
}

void Edif::Runtime::Destroy()
{
    callRunTimeFunction(rdPtr, RFUNCTION_DESTROY, 0, 0);
}

void Edif::Runtime::ExecuteProgram(prgParam * Program)
{
    callRunTimeFunction(rdPtr, RFUNCTION_EXECPROGRAM, 0, (long) Program);
}

long Edif::Runtime::EditInteger(EditDebugInfo * EDI)
{
    return callRunTimeFunction(rdPtr, RFUNCTION_EDITINT, 0, (long) EDI);
}

long Edif::Runtime::EditText(EditDebugInfo * EDI)
{
    return callRunTimeFunction(rdPtr, RFUNCTION_EDITTEXT, 0, (long) EDI);
}

void Edif::Runtime::CallMovement(int ID, long Parameter)
{
    callRunTimeFunction(rdPtr, RFUNCTION_CALLMOVEMENT, ID, Parameter);
}

void Edif::Runtime::SetPosition(int X, int Y)
{
    callRunTimeFunction(rdPtr, RFUNCTION_SETPOSITION, X, Y);
}

CallTables* Edif::Runtime::GetCallTables()
{
    return (CallTables*)callRunTimeFunction(rdPtr, RFUNCTION_GETCALLTABLES, 0, 0);
}

bool Edif::Runtime::IsHWA()
{
    return rdPtr->rHo.hoAdRunHeader->rh4.rh4Mv->mvCallFunction(NULL, 112, 0, 0, 0) == 1;
}

bool Edif::Runtime::IsUnicode()
{
    return rdPtr->rHo.hoAdRunHeader->rh4.rh4Mv->mvCallFunction(NULL, 113, 0, 0, 0) == 1;
}

event &Edif::Runtime::CurrentEvent()
{
    return *(event *) (((char *) param1) - CND_SIZE);
}

LPRO Edif::Runtime::LPROFromFixed(int fixedvalue)
{
	LPOBL objList = rdPtr->rHo.hoAdRunHeader->rhObjectList;
	int index = 0x0000FFFF & fixedvalue;

	if (index < 0 || index >= rdPtr->rHo.hoAdRunHeader->rhMaxObjects)
		return NULL;

	LPRO theObject = (LPRO)objList[0x0000FFFF & fixedvalue].oblOffset;

	if (theObject == NULL)
		return NULL;
	else if (FixedFromLPRO(theObject) != fixedvalue)
		return NULL;

	return theObject;
}

int Edif::Runtime::FixedFromLPRO(LPRO object)
{
	if(object != NULL)
		return (object->roHo.hoCreationId << 16) + object->roHo.hoNumber;
	return 0;
}

extern HINSTANCE hInstLib;

struct EdifGlobal
{
    TCHAR Name[256];
    void * Value;

    EdifGlobal * Next;
};

void Edif::Runtime::WriteGlobal(const TCHAR * Name, void * Value)
{
    LPRH rhPtr = rdPtr->rHo.hoAdRunHeader;

	while(rhPtr->rhApp->m_pParentApp)
		rhPtr = rhPtr->rhApp->m_pParentApp->m_Frame->m_rhPtr;

    EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->mvGetExtUserData(rhPtr->rhApp, hInstLib);

    if(!Global)
    {
        Global = new EdifGlobal;

        _tcscpy(Global->Name, Name);
        Global->Value = Value;

        Global->Next = 0;

        rhPtr->rh4.rh4Mv->mvSetExtUserData(rhPtr->rhApp, hInstLib, Global);

        return;
    }

    while(Global)
    {
        if(!_tcsicmp(Global->Name, Name))
        {
            Global->Value = Value;
            return;
        }

        if(!Global->Next)
            break;

        Global = Global->Next;
    }

    Global->Next = new EdifGlobal;
    Global = Global->Next;

    _tcscpy(Global->Name, Name);

    Global->Value = Value;
    Global->Next = 0;
}

void * Edif::Runtime::ReadGlobal(const TCHAR * Name)
{
    LPRH rhPtr = rdPtr->rHo.hoAdRunHeader;

	while(rhPtr->rhApp->m_pParentApp)
		rhPtr = rhPtr->rhApp->m_pParentApp->m_Frame->m_rhPtr;

    EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->mvGetExtUserData(rhPtr->rhApp, hInstLib);

    while(Global)
    {
        if(!_tcsicmp(Global->Name, Name))
            return Global->Value;

        Global = Global->Next;
    }

    return 0;
}

#ifdef EdifUseJS

    /* Breaking versions of Edif should change these so that newer extensions don't
       try to work with an older JS context. */

    const char * const ContextName = "JSContext-V1";


    JSContext * Edif::Runtime::GetJSContext()
    {
        JSContext * Context = ReadGlobal(ContextName);

        if(!Context)
        {
            Context = JS_NewContext(JS_NewRuntime(8 * 1024 * 1024), 8192);

            JS_SetOptions(Context, JSOPTION_VAROBJFIX | JSOPTION_JIT | JSOPTION_METHODJIT | JSOPTION_NO_SCRIPT_RVAL);
            JS_SetVersion(Context, JSVERSION_LATEST);
            
            JSObject * Global = JS_NewGlobalObject(context, &GlobalClass);

            JS_SetGlobalObject    (Context, Global);
            JS_InitStandardClasses(Context, Global);

            WriteGlobal(ContextName, Context);
        }

        return Context;
    }

#endif
