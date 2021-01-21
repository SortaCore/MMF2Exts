#include "Common.h"
#include "DarkEdif.h"

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

void * Edif::Runtime::Allocate(size_t size)
{
	return (void *) CallRunTimeFunction(rdPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, size * sizeof(TCHAR));
}

TCHAR * Edif::Runtime::CopyString(const TCHAR * String)
{
	TCHAR * New = NULL;
	New = (TCHAR *) Allocate(_tcslen(String) + 1);
	_tcscpy(New, String);

	return New;
}

char * Edif::Runtime::CopyStringEx(const char * String)
{
	char * New = NULL;
	New = (char *)CallRunTimeFunction(rdPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, (strlen(String) + 1) * sizeof(char));
	strcpy(New, String);

	return New;
}

wchar_t * Edif::Runtime::CopyStringEx(const wchar_t * String)
{
	wchar_t * New = NULL;
	New = (wchar_t *)CallRunTimeFunction(rdPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, (wcslen(String) + 1) * sizeof(wchar_t));
	wcscpy(New, String);

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
	CallRunTimeFunction(rdPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DRIVE, (long) Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
	CallRunTimeFunction(rdPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DIR, (long) Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
	CallRunTimeFunction(rdPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::PATH, (long) Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
	CallRunTimeFunction(rdPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::APP_NAME, (long) Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
	CallRunTimeFunction(rdPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::TEMP_PATH, (long) Buffer);
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
	static bool isHWA, inited = false;
	if (!inited) {
		isHWA = rdPtr->rHo.AdRunHeader->rh4.rh4Mv->CallFunction(NULL, CallFunctionIDs::ISHWA, 0, 0, 0) == 1;
		inited = true;
	}
	return isHWA;
}

bool Edif::Runtime::IsUnicode()
{
	static bool isUni, inited = false;
	if (!inited) {
		isUni = rdPtr->rHo.AdRunHeader->rh4.rh4Mv->CallFunction(NULL, CallFunctionIDs::ISUNICODE, 0, 0, 0) == 1;
		inited = true;
	}
	return isUni;
}

event2 &Edif::Runtime::CurrentEvent()
{
	return *(event2 *) (((char *) param1) - CND_SIZE);
}

RunObject * Edif::Runtime::RunObjPtrFromFixed(int fixedvalue)
{
	objectsList * objList = rdPtr->rHo.AdRunHeader->ObjectList;
	int index = fixedvalue & 0x0000FFFF;

	if (index < 0 || index >= rdPtr->rHo.AdRunHeader->MaxObjects)
		return NULL;

	RunObject * theObject = (RunObject *)objList[index].oblOffset;

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
	TCHAR name[256];
	void * Value;

	EdifGlobal * Next;
};

void Edif::Runtime::WriteGlobal(const TCHAR * name, void * Value)
{
	RunHeader * rhPtr = rdPtr->rHo.AdRunHeader;

	while (rhPtr->App->ParentApp)
		rhPtr = rhPtr->App->ParentApp->Frame->rhPtr;

	EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->GetExtUserData(rhPtr->App, hInstLib);

	if (!Global)
	{
		Global = new EdifGlobal;

		_tcscpy(Global->name, name);
		Global->Value = Value;

		Global->Next = 0;

		rhPtr->rh4.rh4Mv->SetExtUserData(rhPtr->App, hInstLib, Global);

		return;
	}

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
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

	_tcscpy(Global->name, name);

	Global->Value = Value;
	Global->Next = 0;
}

void * Edif::Runtime::ReadGlobal(const TCHAR * name)
{
	RunHeader * rhPtr = rdPtr->rHo.AdRunHeader;

	while (rhPtr->App->ParentApp)
		rhPtr = rhPtr->App->ParentApp->Frame->rhPtr;

	EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->GetExtUserData(rhPtr->App, hInstLib);

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
			return Global->Value;

		Global = Global->Next;
	}

	return 0;
}

