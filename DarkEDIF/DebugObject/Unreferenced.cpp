#include "Common.h"

LONG WINAPI UnhandledExceptionCatcher(PEXCEPTION_POINTERS pExceptionPtrs)
{
	if (!pExceptionPtrs || !pExceptionPtrs->ExceptionRecord)
	{
		GlobalExt->OutputNow(5, -1, "Failed to catch crash, invalid pointers supplied.");
		return EXCEPTION_CONTINUE_SEARCH;
	}
	
	std::stringstream OutputThis;
	// Starting statment
	OutputThis << "*** Unhandled exception occured in the MMF2 program at address 0x"
			   << (void *)pExceptionPtrs->ExceptionRecord->ExceptionAddress
			   << " ***";
	GlobalExt->OutputNow(5, -1, OutputThis.str().c_str());
	if (GlobalExt->Data->FileHandle)
		OutputThis.str("");
	else
		OutputThis << "\r\n";
	
	bool NoHandling = false;
	switch (pExceptionPtrs->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			OutputThis << "The thread tried to read from or write to a virtual address for which "
						  "it does not have the appropriate access.";
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			OutputThis << "The thread tried to access an array element that is out of bounds (and "
						  "the underlying hardware supports bounds checking)."; 
			break;
		case EXCEPTION_BREAKPOINT:
			OutputThis << "A breakpoint exception was encountered - the object will pass the "
						  "exception on.";
			NoHandling = true;
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			OutputThis << "The thread tried to read or write data that is misaligned on hardware "
						  "that does not provide alignment. For example, 16-bit values must be "
						  "aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and "
						  "so on.";
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			OutputThis << "One of the operands in a floating-point operation is denormal. A "
						  "denormal value is one that is too small to represent as a standard "
						  "floating-point value.";
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			OutputThis << "The thread tried to divide a floating-point value by a floating-point "
						  "divisor of zero.";
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			OutputThis << "The result of a floating-point operation cannot be represented exactly "
						  "as a decimal fraction.";
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			OutputThis << "This exception is an uncommon floating-point exception.";
			break;
		case EXCEPTION_FLT_OVERFLOW:
			OutputThis << "The exponent of a floating-point operation is greater than the magnitude "
						  "allowed by the corresponding type.";
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			OutputThis << "The stack overflowed or underflowed as the result of a floating-point "
						  "operation.";
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			OutputThis << "The exponent of a floating-point operation is less than the magnitude "
						  "allowed by the corresponding type.";
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			OutputThis << "The thread tried to execute an invalid instruction.";
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			OutputThis << "The thread tried to access a page that was not present, and the system "
						  "was unable to load the page. For example, this exception might occur if "
						  "a network connection is lost while running a program over the network.";
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			OutputThis << "The thread tried to divide an integer value by an integer divisor of zero.";
			break;
		case EXCEPTION_INT_OVERFLOW:
			OutputThis << "The result of an integer operation caused a carry out of the most significant "
						  "bit of the result.";
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			OutputThis << "An exception handler returned an invalid disposition to the exception dispatcher. "
						  "This is incredibly rare for C/C++ programs."; 
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			OutputThis << "The thread tried to continue after an exception, but this exception type makes it "
						  "impossible."/* " DebugObject will not attempt to continue after this error."*/;
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			OutputThis << "The thread tried to execute an instruction whose operation is not allowed in the "
						  "current machine mode.";
			break;
		case EXCEPTION_SINGLE_STEP:
			OutputThis << "A trace trap or other single-instruction mechanism signaled that one instruction "
						  "has been executed.";
			NoHandling = true;
			break;
		case EXCEPTION_STACK_OVERFLOW:
			OutputThis << "The thread used up its allocated stack memory.";
			break;
		default:
			OutputThis << "Unrecognised exception code [" << pExceptionPtrs->ExceptionRecord->ExceptionCode << "].";
			break;
	}
	if (!GlobalExt->Data->FileHandle)
	{
		if (GlobalExt->Data->DoMsgBoxIfPathNotSet)
			MessageBoxA(NULL, OutputThis.str().c_str(), "DebugObject - caught exception.", MB_OK | MB_ICONERROR);
	}
	else
		GlobalExt->OutputNow(5, -2, OutputThis.str().c_str());

	if (NoHandling || !GlobalExt->Data)
		return EXCEPTION_CONTINUE_SEARCH;
	else
	{
		DWORD d = 0;
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
/*
	// Otherwise
	DWORD rv;

    EXCEPTION_RECORD* per = pep->ExceptionRecord;

    if ( ( per->ExceptionCode == EXCEPTION_ACCESS_VIOLATION ) &&
         ( per->ExceptionInformation[0] != 0 ) )
    {
        rv = BasepCheckForReadOnlyResource( per->ExceptionInformation[1] );

        if ( rv == EXCEPTION_CONTINUE_EXECUTION )
            return EXCEPTION_CONTINUE_EXECUTION;
    }

    DWORD DebugPort = 0;

    rv = NtQueryInformationProcess( GetCurrentProcess(), ProcessDebugPort,
                                    &DebugPort, sizeof( DebugPort ), 0 );

    if ( ( rv >= 0 ) && ( DebugPort != 0 ) )
    {
        // Yes, it is -> Pass exception to the debugger
		OutputNow(5, -1, "*** Unhandled exception. Detected that a debug mode is enabled; passing control of unhandled exception to debugger. ***");
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Is custom filter for unhandled exceptions registered ?

    if ( BasepCurrentTopLevelFilter != 0 )
    {
        // Yes, it is -> Call the custom filter

        rv = (BasepCurrentTopLevelFilter)(pep);

        if ( rv == EXCEPTION_EXECUTE_HANDLER )
		{
			OutputNow(5, -1, "*** Unhandled exception. Caught and reported. ***");
            return EXCEPTION_EXECUTE_HANDLER;
		}

        if ( rv == EXCEPTION_CONTINUE_EXECUTION )
		{
			OutputNow(5, -1, "*** Unhandled exception. Detected that a debug mode is enabled; passing control of unhandled exception to debugger. ***");
            return EXCEPTION_CONTINUE_EXECUTION;
		}
    }   */

}

// Handles Ctrl+C, Ctrl+Break, and console's close button
bool WINAPI HandlerRoutine(DWORD ControlType)
{
	if (!GlobalExt)
		return true;

	// Note: always return true, to indicate this function has handled it.
	switch (ControlType)
	{
		case CTRL_C_EVENT:
			GlobalExt->OutputNow(5, -1, "Ctrl+C hit. No response set up...");
			return true;
		case CTRL_BREAK_EVENT:
			GlobalExt->OutputNow(5, -1, "Ctrl+Break hit. No response set up...");
			return true;
		case CTRL_CLOSE_EVENT:
			GlobalExt->OutputNow(5, -1, "Close console hit. No response set up...");
			return true;
		default:
			GlobalExt->OutputNow(5, -1, "Unknown control type sent to console. No response.");
			return true;
	}
}

// Receives input text from console
void WINAPI ReceiveConsoleInput()
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
			continue;

		GlobalExt->Data->ReleaseConsoleInput = false;
		GlobalExt->Data->ConsoleReceived = InputText;
		GlobalExt->Runtime.PushEvent(12);

		while (!GlobalExt->Data->ReleaseConsoleInput)
			Sleep(10);
	}
}