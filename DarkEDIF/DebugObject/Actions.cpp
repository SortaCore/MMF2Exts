
#include "Common.h"

void Extension::Output(int Intensity, int Line, const char * TextP)
{
	// Duplicate variables
	char * Text = _strdup(TextP);

	// Output (OutputNow will acquire lock)
	OutputNow(Intensity, Line, Text);

	// Cleanup
	free(Text);
}

void Extension::SetOutputFile(char * FileP, int DescribeAppI = 0)
{
	// Can't continue if Data failed to initialise
	if (!Data)
		return;

	// Duplicate variables
	char * File = _strdup(FileP);

	// If file handle is valid, output closing message
	if (Data->FileHandle)
	{
		// No grab of file handle - OutputNow() will need it
		OutputNow(1, -1, "*** Log closed. ***");
		
		// Acquire lock
		OpenLock();

		// Close file
		fclose(Data->FileHandle);
		Data->FileHandle = NULL;
	}
	else // File handle invalid
	{
		// Just acquire lock
		OpenLock();
	}
	
	// Get handle to file
	unsigned char c = 0;
	while (true)
	{
		// Returns false on success
		if ((Data->FileHandle = _fsopen(File, "wb", _SH_DENYNO)) != NULL)
			break;
		else
		{
			std::stringstream s;
			s << "Failed to open log file: \r\n"
			  << File << "\r\n"
			  << "Returned error: [" << errno << "] :\r\n"
			  << "[" << strerror(errno) << "]\r\n"
			  << "Look for \"errno errors\" on Google for more information."; 
			if (MessageBoxA(NULL, s.str().c_str(), "DebugObject - Error", MB_RETRYCANCEL | MB_ICONERROR) == IDCANCEL)
			{
				CloseLock();
				return;
			}
		}
	}
	
	// If file already exists, add blank line
	fseek(Data->FileHandle, 0, SEEK_END);
	if (ftell(Data->FileHandle) > 0)
		fputs("\r\n", Data->FileHandle);
	
	fflush(Data->FileHandle);

	// Close lock
	CloseLock();
	
	// Report success
	OutputNow(1, -1, "*** Log opened. ***");
	bool DescribeApp = (DescribeAppI != 0);
	if (DescribeApp)
	{
		// Re-acquire lock
		OpenLock();

		// Print application description
		fprintf_s(Data->FileHandle,"\"%s\" executing from \"%s\"\r\n"
				  "Original MFA path: [%s]\r\n"
				  "File first opened from frame #%i [%s]\r\n",
					rdPtr->rHo.AdRunHeader->App->name,
					rdPtr->rHo.AdRunHeader->App->appFileName,
					rdPtr->rHo.AdRunHeader->App->editorFileName,
					rdPtr->rHo.AdRunHeader->App->nCurrentFrame, rdPtr->rHo.AdRunHeader->Frame->name);

		// Close lock
		CloseLock();
	}

	// Cleanup
	free(File);
}

void Extension::SetHandler(int Reaction, int Continues)
{
	// Check Reaction is a valid value (enum starts at 0, ends with HANDLE_VIA_MAX)
	if (Reaction < GlobalData::HANDLE_VIA_MAX && Reaction >= 0)
	{
		// If Reaction is Continue, check Continues is valid, and set Continues
		if (Reaction == GlobalData::HANDLE_VIA_CONTINUE && (Continues <= 100 && Continues != 0))
		{
			OpenLock();

			Data->HandleExceptionVia = GlobalData::HANDLE_VIA_CONTINUE;
			Data->ContinuesMax = Continues;
			Data->ContinuesCount = Continues;

			CloseLock();
		}
		else
		{
			OpenLock();

			Data->HandleExceptionVia = Reaction;
			Data->ContinuesMax = -1;
			Data->ContinuesCount = -1;

			CloseLock();
		}
	}
	
}
void Extension::OutputNow(int Intensity, int Line, const char * TextToOutput)
{
	// Can't output if Data failed to initialise
	if (!Data)
		return;

	// Get lock
	OpenLock();

	// Can't output if debug is off or no debug method is enabled
	if (!Data->DebugEnabled || (!Data->ConsoleEnabled && !Data->FileHandle))
	{
		CloseLock();
		return;
	}
	
	// Get time (if blank, remove tab for time also)
	if (Data->TimeFormat[0] != '\0')
	{
		time(&Data->rawtime);
		Data->timeinfo = localtime(&Data->rawtime);
		strftime(Data->RealTime, 255, Data->TimeFormat, Data->timeinfo);
		// Output
		if (Data->FileHandle)
			fprintf_s(Data->FileHandle, "%i\t%i\t%s\t%s\r\n", Intensity, Line, Data->RealTime, TextToOutput);
		
		// Console wants colourisin'
		if (Data->ConsoleEnabled)
		{
			SetConsoleTextAttribute(Data->ConsoleOut, 0x0A);
			printf_s("%i\t%i\t%s\t", Intensity, Line, Data->RealTime);
			SetConsoleTextAttribute(Data->ConsoleOut, 0x0B);
			printf_s("%s\r\n", TextToOutput);
			SetConsoleTextAttribute(Data->ConsoleOut, 0x07);
		}
	}
	else
	{
		if (Data->FileHandle)
			fprintf_s(Data->FileHandle, "%i\t%i\t%s\r\n", Intensity, Line, TextToOutput);
		
		// Console wants colourisin'
		if (Data->ConsoleEnabled)
		{
			SetConsoleTextAttribute(Data->ConsoleOut, 0x0A);
			printf_s("%i\t%i\t", Intensity, Line);
			SetConsoleTextAttribute(Data->ConsoleOut, 0x0B);
			printf_s("%s\r\n", TextToOutput);
			SetConsoleTextAttribute(Data->ConsoleOut, 0x07);
		}
	}
	
	// Cleanup
	if(Data->FileHandle)
		fflush(Data->FileHandle);
	if(Data->ConsoleEnabled)
		std::cout.flush();
	CloseLock();
}
void Extension::SetOutputTimeFormat(char * FormatP)
{
	// Duplicate variables
	char * Format = _strdup(FormatP);
	
	// Acquire lock, so we don't get fights over TimeFormat
	OpenLock();

	// Check size
	if (strlen(Format) < 100)
		strcpy_s(Data->TimeFormat, 255, Format);
	else
		MessageBoxA(NULL, "Could not change time format: too large (100 is the maximum).", "DebugObject - Error setting time format", MB_OK|MB_ICONERROR);
	
	// Close lock	
	CloseLock();

	// Cleanup
	free(Format);
}

void Extension::SetOutputOnOff(int OnOff)
{
	// Can't continue if Data failed to initialise
	if (!Data)
		return;

	// Acquire lock
	OpenLock();

	// Set debug to boolean equivalent
	Data->DebugEnabled = (OnOff != 0);

	// Close lock
	CloseLock();
}

void Extension::SetConsoleOnOff(int OnOff)
{
	// Can't continue if Data failed to initialise
	if (!Data)
		return;

	// Acquire lock
	OpenLock();

	// AllocConsole() will fail if called >1 times.
	if (Data->ConsoleEnabled == (OnOff != 0))
	{
		// Nothing to do!
		CloseLock();
		return;
	}
	// Set debug to boolean equivalent
	Data->ConsoleEnabled = (OnOff != 0);
	if (Data->DebugEnabled)
	{
		// Start up a console
		if (Data->ConsoleEnabled)
		{
			// success
			if (AllocConsole())
			{
				// Get console handle for log output
				Data->ConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
				Data->ConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
				// Default to shutdown off
				// Shutdown = false;
				std::setfill(' ');		// Set fill character to spaces
				std::right(std::cout);	// Set padding to right

				// Invoke the system to move the standard console streams
				// to the new allocated console
				freopen("CONOUT$", "w", stderr); 
				freopen("CONIN$",  "r", stdin); 
				freopen("CONOUT$", "w", stdout); 

				// Handle all console events (the X was clicked, Ctrl+C pressed, etc)
				SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandlerRoutine, TRUE);

				Data->ReleaseConsoleInput = false;
				Data->ConsoleReceived = "";
				CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ReceiveConsoleInput, NULL, NULL, NULL);

				CloseLock();
			}
			else
			{
				CloseLock();
				OutputNow(5, -1, "Console opening function AllocConsole() failed."
								 "This generally occurs when a console has already been allocated.");
			}
		}
		else // Close down a console
		{
			if (FreeConsole())
			{
				Data->ConsoleOut = NULL;
				Data->ConsoleIn = NULL;
				SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandlerRoutine, FALSE);
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
}

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
	// initialised - in fact, i[2] doesn't even exist.
	#pragma warning (push)

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