
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
	data = NULL;
	
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

	LinkCondition(0, AlwaysTrue /* OnAnyConsoleInput */);
	LinkCondition(1, OnSpecificConsoleInput);
	LinkCondition(2, AlwaysTrue /* OnUnhandledException */);
	LinkCondition(3, AlwaysTrue /* OnCtrlCEvent */);
	LinkCondition(4, AlwaysTrue /* OnCtrlBreakEvent */);
	LinkCondition(5, AlwaysTrue /* OnConsoleCloseEvent */);

	LinkExpression(0, FullCommand);
	LinkExpression(1, CommandMinusName);

	if (edPtr->IsPropChecked(4))
		MessageBoxA(NULL, "Pause for debugger property is enabled; attach your debugger now then continue the process.", "DebugObject - Information", MB_OK | MB_ICONASTERISK);
	
	// Initialise from edittime
	SetOutputOnOff(edPtr->IsPropChecked(0));
	data->doMsgBoxIfPathNotSet = edPtr->IsPropChecked(1);
	SetConsoleOnOff(edPtr->IsPropChecked(3));

	std::tstring outputFile = edPtr->GetPropertyStr(2);
	if (!outputFile.empty()) // if not blank
		SetOutputFile(outputFile.c_str(), 0);
}


Extension::~Extension()
{
	// Nothing to do if we don't have stored information
	// We can't output log closure or free resources, y'see.
	if (!data)
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
	if ((--data->numUsages) == 0)
	{
		// Close resources
		if (data->fileHandle)
			fclose(data->fileHandle);
		data->fileHandle = NULL;

		// Close MMF pointer to Data
		Runtime.WriteGlobal(GlobalID, NULL);
		
		// Disable console
		if (data->consoleOut)
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

			data->consoleOut = NULL;
			data->consoleIn = NULL;
		}
		
		// Close lock
		CloseLock();

		// Close container
		delete data;
		data = NULL;
		
		// Exception handling - invalidate
		SetUnhandledExceptionFilter(NULL);
	}
	else // Other extensions are using this
	{
		CloseLock();
	}
}


REFLAG Extension::Handle()
{
	// If console not enabled, this Handle event is not useful.
	if (!data->consoleEnabled)
		return REFLAG::ONE_SHOT;
	
	// If releasing console input is currently set to false, there is a console input message.
	if (!data->releaseConsoleInput)
	{
		Runtime.GenerateEvent(0);
		Runtime.GenerateEvent(1);
		data->consoleReceived = std::tstring();
		data->releaseConsoleInput = true;
	}

	// If console break type is greater than 0, someone's pressed Ctrl+C/Break.
	if (data->consoleBreakType > 0)
	{
		Runtime.GenerateEvent(data->consoleBreakType);
		data->consoleBreakType = 0;
	}

	return REFLAG::ONE_SHOT;
}


REFLAG Extension::Display()
{
	return REFLAG::NONE;
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

