// ============================================================================
// The following routines are used internally by MMF, and should not need to
// be modified.
// ============================================================================

#include "Common.h"


// ============================================================================
// LIBRARY ENTRY & QUIT POINTS
// ============================================================================

HINSTANCE hInstLib = NULL;

// Entry point for DLL, when DLL is loaded into memory.
// Usually you do not need to do any initialization here: you will prefer to
// do them in "Initialize" found in Edittime.cpp
BOOL WINAPI DllMain(HINSTANCE hDLL, std::uint32_t dwReason, LPVOID lpReserved)
{
	// DLL is attaching to the address space of the current process.
	if (dwReason == DLL_PROCESS_ATTACH && hInstLib == NULL)
	{
		hInstLib = hDLL; // Store HINSTANCE
		// no thread attach/detach for dynamic CRT
		#ifdef _DLL
			DisableThreadLibraryCalls(hDLL);
		#endif
	}

	return TRUE;
}

// Where you want to do COLD-START initialization.
// Called when the extension is loaded into memory.
int FusionAPI Initialize(mv *mV, int quiet)
{
#pragma DllExportHint
	return Edif::Init(mV);
}

// Where you want to kill and initialized data opened in the above routine
// Called just before freeing the DLL.
int FusionAPI Free(mv *mV)
{
#pragma DllExportHint
	// Edif is singleton, no clearup
	return 0; // No error
}


// ============================================================================
// GENERAL INFO
// ============================================================================

// Routine called for each object when the object is read from the MFA file (edit time)
// or from the CCN or EXE file (run time).
// You can load data here, reserve memory etc...
int FusionAPI LoadObject(mv * mV, const char * fileName, EDITDATA * edPtr, int reserved)
{
#pragma DllExportHint
	Edif::Init(mV, edPtr);
	return 0;
}

// The counterpart of LoadObject(): called just before the object is
// deleted from the frame.
void FusionAPI UnloadObject(mv * mV, EDITDATA * edPtr, int reserved)
{
	#pragma DllExportHint
}
