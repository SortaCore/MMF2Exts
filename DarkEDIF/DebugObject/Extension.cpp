
#include "Common.h"

class Extension * GlobalExt = NULL;
///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr)
    : rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr)
{
	if (!GlobalExt)
		GlobalExt = this;
	Data = NULL;
	
	LoadDataVariable();

    /*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
    */

	// Link information
    LinkAction(0, Output);
	LinkAction(1, SetOutputFile);
	LinkAction(2, SetOutputTimeFormat);
	LinkAction(3, SetOutputOnOff);
	LinkAction(4, SetHandler);
	LinkAction(5, CauseCrash_ZeroDivisionInt);
	LinkAction(6, CauseCrash_ZeroDivisionFloat);
	LinkAction(7, CauseCrash_ReadAccessViolation);
	LinkAction(8, CauseCrash_WriteAccessViolation);
	LinkAction(9, CauseCrash_ArrayOutOfBoundsRead);
	LinkAction(10, CauseCrash_ArrayOutOfBoundsWrite);
	LinkAction(11, SetConsoleOnOff);
	LinkAction(12, SetDumpFile);

	LinkCondition(0, OnAnyConsoleInput);
	LinkCondition(1, OnSpecificConsoleInput);
	LinkCondition(2, OnUnhandledException);
	LinkCondition(3, OnCtrlCEvent);
	LinkCondition(4, OnCtrlBreakEvent);
	LinkCondition(5, OnConsoleCloseEvent);

	LinkExpression(0, FullCommand);
	LinkExpression(1, CommandMinusName);

	if (false) //edPtr->PauseForDebugger)
		MessageBoxA(NULL, "Pause for debugger property is enabled; attach your debugger now then continue the process.", "DebugObject - Information", MB_OK | MB_ICONASTERISK);
	
	// Initialise from edittime
	SetOutputOnOff(edPtr->EnableAtStart);
	Data->DoMsgBoxIfPathNotSet = edPtr->DoMsgBoxIfPathNotSet;
	SetConsoleOnOff(edPtr->ConsoleEnabled);

	if (edPtr->InitialPath[0] != '\0') // ""
		SetOutputFile(edPtr->InitialPath, 0);
	
}


Extension::~Extension()
{
	// Nothing to do if we don't have stored information
	// We can't output log closure or free resources, y'see.
	if (!Data)
		return;
	
	// Output closure message and stop any other access to this Extension instance
	if (GlobalExt == this)
	{
		GlobalExt->OutputNow(1, -1, "*** Log closed. ***");
		GlobalExt = NULL;
	}
	
	// Open lock
	OpenLock();

	// Are we the last using this Data?
	if ((--Data->NumUsages) == 0)
	{
		// Close resources
		if (Data->FileHandle)
			fclose(Data->FileHandle);
		Data->FileHandle = NULL;

		// Close MMF pointer to Data
		Runtime.WriteGlobal(GlobalID, NULL);
		
		// Disable console
		if (Data->ConsoleOut)
		{
			if (FreeConsole())
			{
				SetConsoleCtrlHandler(HandlerRoutine, FALSE);
			}
			else
			{
				OutputNow(5, -1, "Console closing function FreeConsole() failed.\r\n"
								 "This generally occurs when a console has already been allocated.");
			}

			Data->ConsoleOut = NULL;
			Data->ConsoleIn = NULL;
		}
		
		// Close lock
		CloseLock();

		// Close container
		delete Data;
		Data = NULL;
		
		// Exception handling - invalidate
		SetUnhandledExceptionFilter(NULL);
	}
	else // Other extensions are using this
	{
		CloseLock();
	}
}


short Extension::Handle()
{
	// If console not enabled, this Handle event is not useful.
	if (!Data->ConsoleEnabled)
		return REFLAG::ONE_SHOT;
	
	// If releasing console input is currently set to false, there is a console input message.
	if (!Data->ReleaseConsoleInput)
	{
		Runtime.GenerateEvent(0);
		Runtime.GenerateEvent(1);
		Data->ConsoleReceived = "";
		Data->ReleaseConsoleInput = true;
	}

	// If console break type is greater than 0, someone's pressed Ctrl+C/Break.
	if (Data->ConsoleBreakType > 0)
	{
		Runtime.GenerateEvent(Data->ConsoleBreakType);
		Data->ConsoleBreakType = 0;
	}

	return REFLAG::ONE_SHOT;
}


short Extension::Display()
{
    return 0;
}

short Extension::Pause()
{

    // Ok
    return 0;
}

short Extension::Continue()
{

    // Ok
    return 0;
}

bool Extension::Save(HANDLE File)
{
	bool OK = false;

    #ifndef VITALIZE

	    // Save the object's data here

	    OK = true;

    #endif

	return OK;
}

bool Extension::Load(HANDLE File)
{
	bool OK = false;

    #ifndef VITALIZE

	    // Load the object's data here

	    OK = true;

    #endif

	return OK;
}


// These are called if there's no function linked to an ID

void Extension::Action(int ID, RUNDATA * rdPtr, long param1, long param2)
{

}

long Extension::Condition(int ID, RUNDATA * rdPtr, long param1, long param2)
{

    return false;
}

long Extension::Expression(int ID, RUNDATA * rdPtr, long param)
{

    return 0;
}

