
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
			  << "Returned error: [" << errno << "].\r\n"
			  << "Look for \"errno errors\" on Google for the number provided."; 
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
					rdPtr->rHo.hoAdRunHeader->rhApp->m_name,
					rdPtr->rHo.hoAdRunHeader->rhApp->m_appFileName,
					rdPtr->rHo.hoAdRunHeader->rhApp->m_editorFileName,
					rdPtr->rHo.hoAdRunHeader->rhApp->m_nCurrentFrame, rdPtr->rHo.hoAdRunHeader->rhFrame->m_name);

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

	// Can't output if handle is invalid
	if (!Data->FileHandle || !Data->DebugEnabled)
		return;

	// Get lock
	OpenLock();
	
	// Get time (if blank, remove tab for time also)
	if (Data->TimeFormat[0] != '\0')
	{
		time(&Data->rawtime);
		Data->timeinfo = localtime(&Data->rawtime);
		strftime(Data->RealTime, 255, Data->TimeFormat, Data->timeinfo);
		// Output
		fprintf_s(Data->FileHandle, "%i\t%i\t%s\t%s\r\n", Intensity, Line, Data->RealTime, TextToOutput);
	}
	else
		fprintf_s(Data->FileHandle, "%i\t%i\t%s\r\n", Intensity, Line, TextToOutput);
	
	// Cleanup
	fflush(Data->FileHandle);
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
	// For some reason, this warning still occurs even with #pragma disabling
	// This warning is correct; a crash will occur since i[2] is not 
	// initialised - in fact, i[2] doesn't even exxit.
	#pragma warning (disable:4700)
		int i[2];
		int j = i[2];
	#pragma warning (default:4700)
}

void Extension::CauseCrash_ArrayOutOfBoundsWrite(void)
{
	int i[2];
	i[2] = 0;
}