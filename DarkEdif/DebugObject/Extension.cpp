#include "Common.hpp"

class Extension * GlobalExt = NULL;
///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RunObject* const _rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->get_rHo()->get_AdRunHeader()), Runtime(this)
{
	if (!GlobalExt)
		GlobalExt = this;
	data = NULL;

	LoadDataVariable();

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

	// Could use AttachDebugger(), but if the debugger isn't one of the defaults, it could close the app,
	// instead of opening a Just-In-Time debugger-choosing dialog
	if (edPtr->Props.IsPropChecked(4))
		DarkEdif::MsgBox::Info(_T("Pausing for debugger"), _T("Pause for debugger property is enabled; attach your debugger now, then press OK."));

	// Initialise from edittime
	SetOutputOnOff(edPtr->Props.IsPropChecked(0));
	data->doMsgBoxIfPathNotSet = edPtr->Props.IsPropChecked(1);
	SetConsoleOnOff(edPtr->Props.IsPropChecked(3));

	std::tstring outputFile = edPtr->Props.GetPropertyStr(2);
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

// These are called if there's no function linked to an ID

void Extension::UnlinkedAction(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedAction() called"), _T("Running a fallback for action ID %d. Make sure you ran LinkAction()."), ID);
}

long Extension::UnlinkedCondition(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedCondition() called"), _T("Running a fallback for condition ID %d. Make sure you ran LinkCondition()."), ID);
	return 0;
}

long Extension::UnlinkedExpression(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedExpression() called"), _T("Running a fallback for expression ID %d. Make sure you ran LinkExpression()."), ID);
	// Unlinked A/C/E is fatal error , but try not to return null string and definitely crash it
	if ((size_t)ID < Edif::SDK->ExpressionInfos.size() && Edif::SDK->ExpressionInfos[ID]->Flags.ef == ExpReturnType::String)
		return (long)Runtime.CopyString(_T(""));
	return 0;
}
