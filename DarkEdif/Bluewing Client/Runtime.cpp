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

extern std::atomic<bool> AppWasClosed; // Event type; other threads can wait for this to be triggered

// Called when the application starts or restarts. Also called for subapps.
void FusionAPI StartApp(mv *mV, CRunApp* pApp)
{
	#pragma DllExportHint
}

// Called when the application ends or restarts. Also called for subapps.
#ifdef _WIN32
void FusionAPI EndApp(mv *mV, CRunApp* pApp)
{
	#pragma DllExportHint
	if (pApp->ParentApp)
	{
		OutputDebugStringA(PROJECT_NAME " - EndApp called, but it's subapp. Ignoring.\n");
		return;
	}
#else
// Called when JavaVM shuts down. Not called for subapps.
ProjectFunc void EndApp(JNIEnv *, jclass)
{
#endif
	AppWasClosed = true;

	OutputDebugStringA(PROJECT_NAME " - EndApp called.\n");
}
