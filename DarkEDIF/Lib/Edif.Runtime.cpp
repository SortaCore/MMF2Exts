#include "Common.h"

Edif::Runtime::Runtime(RUNDATA * _rdPtr) : rdPtr(_rdPtr), ObjectSelection(_rdPtr->rHo.AdRunHeader)
{
}

Edif::Runtime::~Runtime()
{
}

void Edif::Runtime::Rehandle()
{
    CallRunTimeFunction(rdPtr, RFUNCTION::REHANDLE, 0, 0);
}

void Edif::Runtime::GenerateEvent(int EventID)
{
    CallRunTimeFunction(rdPtr, RFUNCTION::GENERATE_EVENT, EventID, 0);
}

void Edif::Runtime::PushEvent(int EventID)
{
    CallRunTimeFunction(rdPtr, RFUNCTION::PUSH_EVENT, EventID, 0);
}

void * Edif::Runtime::Allocate(size_t Size)
{
    return (void *) CallRunTimeFunction(rdPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, Size * sizeof(TCHAR));
}

TCHAR * Edif::Runtime::CopyString(const TCHAR * String)
{
	TCHAR * New = NULL;
	try {
    New = (TCHAR *) Allocate(_tcslen(String) + 1);
    _tcscpy(New, String);
	}
	catch(...)
	{
		if (New)
			free(New);
		New = (TCHAR *)Allocate(1);
		New[0] = 0;
	}
    
    return New;
}

void Edif::Runtime::Pause()
{
    CallRunTimeFunction(rdPtr, RFUNCTION::PAUSE, 0, 0);
}

void Edif::Runtime::Resume()
{
    CallRunTimeFunction(rdPtr, RFUNCTION::CONTINUE, 0, 0);
}

void Edif::Runtime::Redisplay()
{
    CallRunTimeFunction(rdPtr, RFUNCTION::REDISPLAY, 0, 0);
}

void Edif::Runtime::GetApplicationDrive(TCHAR * Buffer)
{
    CallRunTimeFunction(rdPtr, RFUNCTION::GET_FILE_INFOS, FILEINFO::DRIVE, (long) Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
    CallRunTimeFunction(rdPtr, RFUNCTION::GET_FILE_INFOS, FILEINFO::DIR, (long) Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
    CallRunTimeFunction(rdPtr, RFUNCTION::GET_FILE_INFOS, FILEINFO::PATH, (long) Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
    CallRunTimeFunction(rdPtr, RFUNCTION::GET_FILE_INFOS, FILEINFO::APP_NAME, (long) Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
    CallRunTimeFunction(rdPtr, RFUNCTION::GET_FILE_INFOS, FILEINFO::TEMP_PATH, (long) Buffer);
}

void Edif::Runtime::Redraw()
{
    CallRunTimeFunction(rdPtr, RFUNCTION::REDRAW, 0, 0);
}

void Edif::Runtime::Destroy()
{
    CallRunTimeFunction(rdPtr, RFUNCTION::DESTROY, 0, 0);
}

void Edif::Runtime::ExecuteProgram(ParamProgram * Program)
{
    CallRunTimeFunction(rdPtr, RFUNCTION::EXECUTE_PROGRAM, 0, (long) Program);
}

long Edif::Runtime::EditInteger(EditDebugInfo * EDI)
{
    return CallRunTimeFunction(rdPtr, RFUNCTION::EDIT_INT, 0, (long) EDI);
}

long Edif::Runtime::EditText(EditDebugInfo * EDI)
{
    return CallRunTimeFunction(rdPtr, RFUNCTION::EDIT_TEXT, 0, (long) EDI);
}

void Edif::Runtime::CallMovement(int ID, long Parameter)
{
    CallRunTimeFunction(rdPtr, RFUNCTION::CALL_MOVEMENT, ID, Parameter);
}

void Edif::Runtime::SetPosition(int X, int Y)
{
    CallRunTimeFunction(rdPtr, RFUNCTION::SET_POSITION, X, Y);
}

CallTables * Edif::Runtime::GetCallTables()
{
    return (CallTables *)CallRunTimeFunction(rdPtr, RFUNCTION::GET_CALL_TABLES, 0, 0);
}

bool Edif::Runtime::IsHWA()
{
    return rdPtr->rHo.AdRunHeader->rh4.rh4Mv->CallFunction(NULL, EF_ISHWA, 0, 0, 0) == 1;
}

bool Edif::Runtime::IsUnicode()
{
    return rdPtr->rHo.AdRunHeader->rh4.rh4Mv->CallFunction(NULL, EF_ISUNICODE, 0, 0, 0) == 1;
}

event &Edif::Runtime::CurrentEvent()
{
    return *(event *) (((char *) param1) - CND_SIZE);
}

RunObject * Edif::Runtime::RunObjPtrFromFixed(int fixedvalue)
{
	objectsList * objList = rdPtr->rHo.AdRunHeader->ObjectList;
	int index = 0x0000FFFF & fixedvalue;

	if (index < 0 || index >= rdPtr->rHo.AdRunHeader->MaxObjects)
		return NULL;

	RunObject * theObject = (RunObject *)objList[0x0000FFFF & fixedvalue].oblOffset;

	if (theObject == NULL)
		return NULL;
	else if (FixedFromRunObjPtr(theObject) != fixedvalue)
		return NULL;

	return theObject;
}

int Edif::Runtime::FixedFromRunObjPtr(RunObject * object)
{
	if (object != NULL)
		return (object->roHo.CreationId << 16) + object->roHo.Number;
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
    RunHeader * rhPtr = rdPtr->rHo.AdRunHeader;

//	while (rhPtr->App->ParentApp)
//		rhPtr = rhPtr->App->ParentApp->Frame->rhPtr;

    EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->GetExtUserData(rhPtr->App, hInstLib);

    if (!Global)
    {
        Global = new EdifGlobal;

        _tcscpy(Global->Name, Name);
        Global->Value = Value;

        Global->Next = 0;

        rhPtr->rh4.rh4Mv->SetExtUserData(rhPtr->App, hInstLib, Global);

        return;
    }

    while (Global)
    {
        if (!_tcsicmp(Global->Name, Name))
        {
            Global->Value = Value;
            return;
        }

        if (!Global->Next)
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
    RunHeader * rhPtr = rdPtr->rHo.AdRunHeader;

//	while(rhPtr->App->ParentApp)
//		rhPtr = rhPtr->App->ParentApp->Frame->rhPtr;

    EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->GetExtUserData(rhPtr->App, hInstLib);

    while(Global)
    {
        if (!_tcsicmp(Global->Name, Name))
            return Global->Value;

        Global = Global->Next;
    }

    return 0;
}
