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
		DisableThreadLibraryCalls(hDLL); // no thread attach/detach
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

// For you to update your EDITDATA to newer versions.
// Called both at edittime and runtime
/* HGLOBAL FusionAPI UpdateEditStructure(mv *mV, void * OldEdPtr)
{
#pragma DllExportHint
	// Version is in EDITDATA::eHeader::extVersion, and in DarkEdif dumb properties, it's currently not set.
	// When smart properties are introduced, versioning and upgrades will be  managed for you, and you can provide your
	// own upgrade function if you want in case you used manual layout at any point.

	// If you decide you want to change the data, use GlobalAlloc(GPTR, size) to get HGLOBAL,
	// GlobalLock() to get the pointer from the HGLOBAL, edit it up, then use and GlobalUnlock() to release it ready for Fusion to use.
	// GlobalFree() if you want to undo the GlobalAlloc(), for whatever reason.
	// You must have the global unlocked if you return a HGLOBAL.

	// Note that mvReallocEditData() cannot be used on OldEdPtr! You have to create from scratch; you can read, but not resize.
	// Make sure you set eHeader properly in the new EDITDATA.
	// If you're moving from manual layout of memory to using either DarkEdif layout (e.g. NOPROPS is not defined), then you'll have
	// to poke through DarkEdif code to replicate the layout; InitialisePropertiesFromJSON() is a good starter point.

	return NULL; // No change
}*/
