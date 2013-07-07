#include "Common.h"

LONG WINAPI UnhandledExceptionCatcher(PEXCEPTION_POINTERS pExceptionPtrs)
{
	if (!pExceptionPtrs || !pExceptionPtrs->ExceptionRecord)
	{
		GlobalExt->OutputNow(5, -1, "Failed to catch crash, invalid pointers supplied.");
		return EXCEPTION_CONTINUE_SEARCH;
	}
	

	// Run On Unhandled Exception event.
	// It may encounter issues if the MMF2 runtime has crashed. In which case do the best we can.
	try
	{
		GlobalExt->Runtime.GenerateEvent(2);
	}
	catch (...)
	{
		GlobalExt->OutputNow(5, -1, "Error generating event, this new exception was ignored.");
	}

	std::stringstream str;

	// Create minidump
	if (GlobalExt->Data->MiniDumpPath.size() > 0)
	{
		HANDLE dumpFile = CreateFileA(GlobalExt->Data->MiniDumpPath.c_str(), GENERIC_READ | GENERIC_WRITE, 
			NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
		
		// Opened the minidump file
		if (dumpFile != NULL && dumpFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION mdei; 
			mdei.ClientPointers     = FALSE; 
			mdei.ExceptionPointers  = pExceptionPtrs; 
			mdei.ThreadId           = GetCurrentThreadId(); 
			
			if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), NULL, (MINIDUMP_TYPE)GlobalExt->Data->MiniDumpType, (pExceptionPtrs != 0) ? &mdei : 0, NULL, NULL))
			{
				str << "MiniDumpWriteDump() function failed. Error [" << GetLastError() << "] occured.";
				GlobalExt->OutputNow(5, -1, str.str().c_str());
				GlobalExt->OutputNow(5, -1, "This error can be located from http://msdn.microsoft.com/en-us/library/ms681381(v=vs.85).aspx.");
				GlobalExt->OutputNow(5, -1, "Please confirm your settings of the minidump.");
			}
			else // Minidump write successful
			{
				str << "Minidump created at path [" << GlobalExt->Data->MiniDumpPath << "].";
				GlobalExt->OutputNow(5, -1, str.str().c_str());
				str.str("");
			}

			CloseHandle(dumpFile);
		}
		else // Minidump file couldn't be opened
		{
			str << "CreateFile failed. Error [" << GetLastError() << "] occured.";
			GlobalExt->OutputNow(5, -1, str.str().c_str());
			str.str("");
		}
	}

	// Starting statment
	str << "*** Unhandled exception occured in the MMF2 program at address 0x"
			   << (void *)pExceptionPtrs->ExceptionRecord->ExceptionAddress
			   << " ***";
	GlobalExt->OutputNow(5, -1, str.str().c_str());
	if (GlobalExt->Data->FileHandle)
		str.str("");
	else
		str << "\r\n";
	
	bool NoHandling = false;
	switch (pExceptionPtrs->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			str << "The thread tried to read from or write to a virtual address for which "
						  "it does not have the appropriate access.";
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			str << "The thread tried to access an array element that is out of bounds (and "
						  "the underlying hardware supports bounds checking)."; 
			break;
		case EXCEPTION_BREAKPOINT:
			str << "A breakpoint exception was encountered - the object will pass the "
						  "exception on.";
			NoHandling = true;
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			str << "The thread tried to read or write data that is misaligned on hardware "
						  "that does not provide alignment. For example, 16-bit values must be "
						  "aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and "
						  "so on.";
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			str << "One of the operands in a floating-point operation is denormal. A "
						  "denormal value is one that is too small to represent as a standard "
						  "floating-point value.";
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			str << "The thread tried to divide a floating-point value by a floating-point "
						  "divisor of zero.";
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			str << "The result of a floating-point operation cannot be represented exactly "
						  "as a decimal fraction.";
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			str << "This exception is an uncommon floating-point exception.";
			break;
		case EXCEPTION_FLT_OVERFLOW:
			str << "The exponent of a floating-point operation is greater than the magnitude "
						  "allowed by the corresponding type.";
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			str << "The stack overflowed or underflowed as the result of a floating-point "
						  "operation.";
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			str << "The exponent of a floating-point operation is less than the magnitude "
						  "allowed by the corresponding type.";
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			str << "The thread tried to execute an invalid instruction.";
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			str << "The thread tried to access a page that was not present, and the system "
						  "was unable to load the page. For example, this exception might occur if "
						  "a network connection is lost while running a program over the network.";
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			str << "The thread tried to divide an integer value by an integer divisor of zero.";
			break;
		case EXCEPTION_INT_OVERFLOW:
			str << "The result of an integer operation caused a carry out of the most significant "
						  "bit of the result.";
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			str << "An exception handler returned an invalid disposition to the exception dispatcher. "
						  "This is incredibly rare for C/C++ programs."; 
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			str << "The thread tried to continue after an exception, but this exception type makes it "
						  "impossible."/* " DebugObject will not attempt to continue after this error."*/;
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			str << "The thread tried to execute an instruction whose operation is not allowed in the "
						  "current machine mode.";
			break;
		case EXCEPTION_SINGLE_STEP:
			str << "A trace trap or other single-instruction mechanism signaled that one instruction "
						  "has been executed.";
			NoHandling = true;
			break;
		case EXCEPTION_STACK_OVERFLOW:
			str << "The thread used up its allocated stack memory.";
			break;
		default:
			str << "Unrecognised exception code [" << pExceptionPtrs->ExceptionRecord->ExceptionCode << "].";
			break;
	}

	if (!GlobalExt->Data->FileHandle)
	{
		if (GlobalExt->Data->DoMsgBoxIfPathNotSet)
			MessageBoxA(NULL, str.str().c_str(), "DebugObject - caught exception.", MB_OK | MB_ICONERROR);
	}
	else
		GlobalExt->OutputNow(5, -2, str.str().c_str());

	// Not set to ignore
	if (NoHandling || !GlobalExt->Data)
		return EXCEPTION_CONTINUE_SEARCH;

	switch (GlobalExt->Data->HandleExceptionVia)
	{
		case GlobalData::HANDLE_VIA_QUIT:
			GlobalExt->OutputNow(5, -1, "Handling via quit.");
			ExitProcess(-1);
			return EXCEPTION_CONTINUE_SEARCH;

		case GlobalData::HANDLE_VIA_INFINITE_WAIT:
			GlobalExt->OutputNow(5, -1, "Handling via infinite sleep.");
			Sleep(INFINITE);
			GlobalExt->OutputNow(5, -1, "Passed infinite sleep.");
			return EXCEPTION_CONTINUE_SEARCH;

		case GlobalData::HANDLE_VIA_IGNORE:
			GlobalExt->OutputNow(5, -1, "Handling via passing on.");
			return EXCEPTION_CONTINUE_SEARCH;

		case GlobalData::HANDLE_VIA_EMAIL:
			GlobalExt->OutputNow(5, -1, "Handling via email.");
			GlobalExt->OutputNow(5, -1, "Not programmed; ignoring exception.");
			return EXCEPTION_CONTINUE_SEARCH;

		case GlobalData::HANDLE_VIA_BOOT_PROCESS:
			GlobalExt->OutputNow(5, -1, "Booting crash process...");
			GlobalExt->OutputNow(5, -1, "Not programmed; ignoring exception.");
			return EXCEPTION_CONTINUE_SEARCH;
			
		case GlobalData::HANDLE_VIA_FORCE_DEBUG_BREAK:
			__asm int 3;
			return EXCEPTION_CONTINUE_SEARCH;

		case GlobalData::HANDLE_VIA_CONTINUE:
			if (GlobalExt->Data->ContinuesCount > 0)
			{
				--(GlobalExt->Data->ContinuesCount);
			}
			else
			{
				if (GlobalExt->Data->ContinuesCount == 0)
				{
					GlobalExt->OutputNow(5, -1, "Max continues expired; passing exception to debugger.");
					GlobalExt->Data->ContinuesCount = GlobalExt->Data->ContinuesMax;
					return EXCEPTION_CONTINUE_SEARCH;
				}
			}
			// No return, pass onto default

		default:
			GlobalExt->OutputNow(5, -1, "Handling via continuing execution.");
			return EXCEPTION_CONTINUE_EXECUTION;
	}
}

// Handles Ctrl+C, Ctrl+Break, and console's close button
BOOL WINAPI HandlerRoutine(DWORD ControlType)
{
	// Note: always return true, to indicate this function has handled it.
	if (!GlobalExt)
		return TRUE;
	
	// Normally, we would handle everything here. But the system will terminate the MMF2
	// program if we don't return quickly enough, so we delegate it to Extension::Handle().

	switch (ControlType)
	{
		case CTRL_C_EVENT:
			GlobalExt->OutputNow(5, -1, "Ctrl+C hit. Triggering event...");
			break;

		case CTRL_BREAK_EVENT:
			GlobalExt->OutputNow(5, -1, "Ctrl+Break hit. Triggering event...");
			break;

		case CTRL_CLOSE_EVENT:
			GlobalExt->OutputNow(5, -1, "Close console hit. Triggering event...");
			break;

		default:
			GlobalExt->OutputNow(5, -1, "Unknown control type sent to console. No response.");
			return TRUE;
	}

	GlobalExt->Data->ConsoleBreakType = (ControlType & 0xFF) + 3; // Event 3+ for the correct condition events
	GlobalExt->Runtime.Rehandle();
	return TRUE;
}

// Receives input text from console
DWORD WINAPI ReceiveConsoleInput(void *)
{
	char InputText[256];
	
	// Continue until shutdown is enabled
	while (GlobalExt && GlobalExt->Data && GlobalExt->Data->ConsoleEnabled)
	{
		// memset() infinite-loops if > 255 characters
		for (unsigned char c = 0; c < 255; ++c)
			InputText[c] = 0;
		
		// Read console
		if (std::cin.good()) 
			std::cin.getline(InputText, 255);
		else // Jump to next loop
		{
			std::cin.clear();
			std::cin.getline(InputText, 255); // Remove current buffer of ctrl command
			continue;
		}

		// On restart of app or other weirdness, std::cin.getline()
		if (!GlobalExt || !GlobalExt->Data || !GlobalExt->Data->ConsoleEnabled)
			break;

		// Open lock
		while (GlobalExt->Data->ReadingThis)
			Sleep(0);

		GlobalExt->Data->ReadingThis = true; 
		#ifdef _DEBUG
			GlobalExt->Data->LastLockFile = __FILE__;
			GlobalExt->Data->LastLockLine = __LINE__;
		#endif
		GlobalExt->Data->ReleaseConsoleInput = false;
		GlobalExt->Data->ConsoleReceived = InputText;

		// Close lock
		GlobalExt->Data->ReadingThis = false;

		// Boot events (since this is not the usual thread, multi-threadify it)
		GlobalExt->Runtime.Rehandle();
		
		while (!GlobalExt->Data->ReleaseConsoleInput)
			Sleep(10);
	}

	return 0;
}