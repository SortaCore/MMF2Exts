
#include "Common.h"

// Called when the application starts or restarts.
// Useful for storing global data
void FusionAPI StartApp(mv *mV, CRunApp *pApp)
{
#pragma DllExportHint
	if (GlobalExt)
		GlobalExt->LoadDataVariable();
}

void Extension::LoadDataVariable()
{
	data = (GlobalData *)Runtime.ReadGlobal(_T("DebugObject"));

	// Not initialized
	if (!data)
	{
		// Create new container
		data = new GlobalData;

		// Open lock
		data->readingThis = true; // Open lock without checking if the handle is valid first

		// Mark data position
		Runtime.WriteGlobal(GlobalID, data);

		// Initialize data
		data->fileHandle = NULL;
		data->debugEnabled = false;
		memset(data->timeFormat, 0, sizeof(data->timeFormat));
		memset(data->realTime, 0, sizeof(data->realTime));
		_tcscpy_s(data->timeFormat, std::size(data->timeFormat), _T("%X"));
		data->numUsages = 1;
		data->doMsgBoxIfPathNotSet = false;
		data->consoleIn = NULL;
		data->consoleOut = NULL;
		data->consoleEnabled = false;
		data->consoleReceived = std::tstring();
		data->releaseConsoleInput = true;
		data->consoleBreakType = 0;
		data->miniDumpPath = std::tstring();
		data->miniDumpType = MiniDumpNormal;

		// Exception handling (container)
		data->continuesRemaining = -1;
		data->continuesMax = -1;
		CloseLock();

		// Exception handling (WinAPI call)
		SetUnhandledExceptionFilter(UnhandledExceptionCatcher);
	}
	else // Already initialized
	{
		OpenLock();
		++data->numUsages;
		CloseLock();
	}
}


// Called when the application ends or restarts. Also called for subapps.
void FusionAPI EndApp(mv *mV, CRunApp* pApp)
{
	#pragma DllExportHint
}
