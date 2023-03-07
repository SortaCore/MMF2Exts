// ============================================================================
// This file contains routines that are handled during runtime; during both
// Run Application and in built EXEs.
// It includes various Fusion-triggered events, such as StartApp and EndApp,
// window message processing, and runtime display of your object.
//
// If your object does not use any of those, a simple non-displaying object,
// you can safely exclude this file.
// ============================================================================

#include "Common.h"

// ============================================================================
// START APP / END APP / START FRAME / END FRAME routines
// ============================================================================

#ifdef _WIN32
// Called when the application starts or restarts. Also called for subapps.
void FusionAPI StartApp(mv *mV, CRunApp* pApp)
{
	#pragma DllExportHint
	// We don't need this function, just use EndApp, but it's good practice to do both of a pair...
	// starts and ends, loads and unloads, etc.
}

// Called when the application ends or restarts. Also called for subapps.
void FusionAPI EndApp(mv * mV, CRunApp * pApp)
{
	#pragma DllExportHint
	if (pApp->ParentApp)
	{
		OutputDebugStringA(PROJECT_NAME " - EndApp called, but it's subapp. Ignoring.\n");
		return;
	}
	Extension::AppWasClosed = true;
}
#elif defined (__ANDROID__)
// Called when JavaVM shuts down. Not called for subapps.
ProjectFunc void EndApp(JNIEnv *, jclass)
{
	// Note: on Java side, is declared as darkedif_EndApp, due to how RegisterNatives() works.
	Extension::AppWasClosed = true;
	OutputDebugStringA(PROJECT_NAME " - EndApp called.\n");
}
#else // No iOS auto-quit yet!
ProjectFunc void EndApp()
{
	Extension::AppWasClosed = true;
	OutputDebugStringA(PROJECT_NAME " - EndApp called.\n");
}
#endif
