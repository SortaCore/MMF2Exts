#include "Common.hpp"

// Called by action only
void Extension::Output(int intensity, int line, const TCHAR * textP)
{
	// Output (OutputNow will acquire lock)
	OutputNow(intensity, line, DarkEdif::TStringToUTF8(textP).c_str());
}


void Extension::SetOutputFile(const TCHAR * fileP, int describeAppI = 0)
{
	// Can't continue if Data failed to initialize
	if (!data)
		return;

	// Duplicate variables
	std::tstring file(fileP);

	// If file handle is valid, output closing message
	if (data->fileHandle)
	{
		// No grab of file handle - OutputNow() will need it
		OutputNow(1, -1, "*** Log closed. ***");

		// Acquire lock
		OpenLock();

		// Close file
		fclose(data->fileHandle);
		data->fileHandle = NULL;
	}
	else // File handle invalid
	{
		// Just acquire lock
		OpenLock();
	}

	// Get handle to file
	while (true)
	{
		// Returns false on success
		if ((data->fileHandle = _tfsopen(file.c_str(), _T("wb"), _SH_DENYNO)) != NULL)
			break;

		if (!data->doMsgBoxIfPathNotSet)
		{
			CloseLock();
			return;
		}

		const int ret = DarkEdif::MsgBox::Custom(MB_RETRYCANCEL | MB_ICONERROR, _T("Error"),
			_T("Failed to open log file:\n%sReturned error: %i: %s\nLook for \"errno errors\" on Google for more information."),
			file, errno, _tcserror(errno));
		if (ret == IDCANCEL)
		{
			CloseLock();
			return;
		}
	}

	// If file already exists, add blank line
	fseek(data->fileHandle, 0, SEEK_END);
	if (ftell(data->fileHandle) > 0)
		fputs("\r\n", data->fileHandle);

	fflush(data->fileHandle);

	// Close lock
	CloseLock();

	// Report success
	OutputNow(1, -1, "*** Log opened. ***");
	bool describeApp = (describeAppI != 0);
	if (describeApp)
	{
		// Re-acquire lock
		OpenLock();

		// Print application description
		std::tstringstream str;
		str << _T("\"") << rhPtr->rhApp->name
			<< _T("\" executing from \"") << rhPtr->rhApp->appFileName << _T("\"\r\n")
			<< _T("Original MFA path: \"") << rhPtr->rhApp->editorFileName << _T("\"\r\n")
			<< _T("File first opened from frame #") << rhPtr->rhApp->nCurrentFrame
			<< _T("(") << rhPtr->rhFrame->name << _T(")\r\n");
		std::string str2 = DarkEdif::TStringToUTF8(str.str());
		fputs(str2.c_str(), data->fileHandle);

		// Close lock
		CloseLock();
	}
}

void Extension::SetHandler(int reaction, int continues)
{
	// Check reaction is a valid value (enum starts at 0, ends with HANDLE_VIA_MAX)
	if (reaction < GlobalData::HandleType::HANDLE_VIA_MAX && reaction >= 0)
	{
		// If reaction is Continue, check continues is valid, and set continues
		if (reaction == GlobalData::HandleType::HANDLE_VIA_CONTINUE && (continues <= 100 && continues != 0))
		{
			OpenLock();

			data->handleExceptionVia = GlobalData::HandleType::HANDLE_VIA_CONTINUE;
			data->continuesMax = continues;
			data->continuesRemaining = continues;

			CloseLock();
		}
		else
		{
			OpenLock();

			data->handleExceptionVia = (GlobalData::HandleType)reaction;
			data->continuesMax = -1;
			data->continuesRemaining = -1;

			CloseLock();
		}
	}

}

// Called by non-action only
void Extension::OutputNow(int intensity, int line, std::string textToOutputU8)
{
	// Can't output if Data failed to initialize
	if (!data)
	{
		DebugBreak();
		return;
	}

	// Get lock
	OpenLock();

	// Can't output if debug is off or no debug method is enabled
	if (!data->debugEnabled || (!data->consoleEnabled && !data->fileHandle))
	{
		CloseLock();
		return;
	}

	// Get time (if blank, remove tab for time also)
	if (data->timeFormat[0] != _T('\0'))
	{
		time(&data->rawtime);
		data->timeinfo = localtime(&data->rawtime);
		_tcsftime(data->realTime, std::size(data->realTime), data->timeFormat, data->timeinfo);
		std::string realTimeU8 = DarkEdif::TStringToUTF8(data->realTime);

		// Output
		if (data->fileHandle)
			fprintf_s(data->fileHandle, "%i\t%i\t%s\t%s\r\n", intensity, line, realTimeU8.c_str(), textToOutputU8.c_str());

		// Console wants colourisin'
		if (data->consoleEnabled)
		{
			SetConsoleTextAttribute(data->consoleOut, 0x0A);
			wprintf_s(L"%i\t%i\t%s\t", intensity, line, DarkEdif::TStringToWide(data->realTime).c_str());
			SetConsoleTextAttribute(data->consoleOut, 0x0B);
			wprintf_s(L"%s\r\n", DarkEdif::UTF8ToWide(textToOutputU8).c_str());
			SetConsoleTextAttribute(data->consoleOut, 0x07);
		}
	}
	else
	{
		if (data->fileHandle)
			fprintf_s(data->fileHandle, "%i\t%i\t%s\r\n", intensity, line, textToOutputU8.c_str());

		// Console wants colourisin'
		if (data->consoleEnabled)
		{
			SetConsoleTextAttribute(data->consoleOut, 0x0A);
			wprintf_s(L"%i\t%i\t", intensity, line);
			SetConsoleTextAttribute(data->consoleOut, 0x0B);
			wprintf_s(L"%s\r\n", DarkEdif::UTF8ToWide(textToOutputU8).c_str());
			SetConsoleTextAttribute(data->consoleOut, 0x07);
		}
	}

	// Cleanup
	if (data->fileHandle)
		fflush(data->fileHandle);
	if (data->consoleEnabled)
		std::wcout.flush();
	CloseLock();
}
void Extension::SetOutputTimeFormat(TCHAR * format)
{
	// Check size
	if (_tcslen(format) > 100)
		return DarkEdif::MsgBox::Error(_T("Time format error"), _T("Could not change time format: too large (100 chars is the maximum)."));

	// Acquire lock, so we don't get fights over TimeFormat
	OpenLock();

	_tcscpy_s(data->timeFormat, 255, format);

	// Close lock
	CloseLock();
}

void Extension::SetOutputOnOff(int OnOff)
{
	// Can't continue if Data failed to initialize
	if (!data)
		return;

	// Acquire lock
	OpenLock();

	// Set debug to boolean equivalent
	data->debugEnabled = (OnOff != 0);

	// Close lock
	CloseLock();
}

void Extension::SetConsoleOnOff(int OnOff)
{
	// Can't continue if Data failed to initialize
	if (!data)
		return;

	// Acquire lock
	OpenLock();

	// AllocConsole() will fail if called >1 times.
	if (data->consoleEnabled == (OnOff != 0))
	{
		// Nothing to do!
		CloseLock();
		return;
	}
	// Set debug to boolean equivalent
	data->consoleEnabled = (OnOff != 0);
	if (data->debugEnabled)
	{
		// Start up a console
		if (data->consoleEnabled)
		{
			// success
			if (AllocConsole())
			{
				// Get console handle for log output
				data->consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
				data->consoleIn = GetStdHandle(STD_INPUT_HANDLE);
				// Default to shutdown off
				// Shutdown = false;
				std::wcout << std::setfill(L' ');	// Set fill character to spaces
				std::right(std::wcout);				// Set padding to right

				// Invoke the system to move the standard console streams
				// to the new allocated console
				freopen("CONOUT$", "w", stderr);
				freopen("CONIN$", "r", stdin);
				freopen("CONOUT$", "w", stdout);

				// Handle all console events (Ctrl+C pressed, etc)
				SetConsoleCtrlHandler(HandlerRoutine, TRUE);

				// Disable close window button (this terminates the process after a delay, so cannot be handled)
				HWND hwnd = ::GetConsoleWindow();
				if (hwnd != NULL)
				{
					HMENU hMenu = ::GetSystemMenu(hwnd, FALSE);
					if (hMenu != NULL)
						DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
				}

				data->releaseConsoleInput = true;
				data->consoleReceived = std::tstring();
				data->consoleBreakType = 0;

				CreateThread(NULL, NULL, ReceiveConsoleInput, NULL, NULL, NULL);

				CloseLock();
			}
			else
			{
				CloseLock();
				OutputNow(5, -1, "Console opening function AllocConsole() failed.\r\n"
					"This generally occurs when a console has already been allocated.");
			}
		}
		else // Close down a console
		{
			SetConsoleCtrlHandler(HandlerRoutine, FALSE);

			data->releaseConsoleInput = true;

			if (FreeConsole())
			{
				data->consoleOut = NULL;
				data->consoleIn = NULL;
				CloseLock();
			}
			else
			{
				CloseLock();
				OutputNow(5, -1, "Console closing function FreeConsole() failed."
					"This generally occurs when a console has already been allocated.");
			}

		}

	}
	else
		CloseLock();
}

#pragma optimize( "", off )
void Extension::CauseCrash_ZeroDivisionInt(void)
{
	int a = 0, b = 0, c = 0;
	++a;
	c = a/b;
}

void Extension::CauseCrash_ZeroDivisionFloat(void)
{
	float a = 0.0f, b = 0.0f, c = 0.0f;
	a = 1.0f;
	c = a/b;
}

void Extension::CauseCrash_WriteAccessViolation(void)
{
	*((int *) 0) = 1;
}

void Extension::CauseCrash_ReadAccessViolation(void)
{
	int i = *((int *) 0);
}

void Extension::CauseCrash_ArrayOutOfBoundsRead(void)
{
	// This warning is correct; a crash will occur since i[2] is not
	// initialized - in fact, i[2] doesn't even exist.
	#pragma warning (push)
	#pragma warning (disable:4700)

		int i[2] = {0,0};
		__pragma(warning(suppress:4700)) \
		int j = i[2];
	#pragma warning (pop)
}

void Extension::CauseCrash_ArrayOutOfBoundsWrite(void)
{
	int i[2];
	i[2] = 0;
}
#pragma optimize( "", on )

void Extension::SetDumpFile(const TCHAR * path, int flags)
{
	if ((flags & MiniDumpValidTypeFlags) != flags)
		OutputNow(5, -1, "Invalid minidump flags specified.");
	else
	{
		OpenLock();
		data->miniDumpPath = path;
		data->miniDumpType = flags;
		CloseLock();
	}
}
