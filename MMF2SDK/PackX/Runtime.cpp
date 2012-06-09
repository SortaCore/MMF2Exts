// ============================================================================
//
// This file contains routines that are handled during the Runtime.
//
// Including creating, display, and handling your object.
// 
// ============================================================================

// Common Include
#include	"common.h"

// --------------------
// GetRunObjectDataSize
// --------------------
// Returns the size of the runtime datazone of the object
// 
ushort WINAPI DLLExport GetRunObjectDataSize(fprh rhPtr, LPEDATA edPtr)
{
	return(sizeof(RUNDATA));
}


// ---------------
// CreateRunObject
// ---------------
// The routine where the object is actually created
// 
short WINAPI DLLExport CreateRunObject(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr)
{
/*
   This routine runs when your object is created, as you might have guessed.
   It is here that you must transfer any data you need in rdPtr from edPtr,
   because after this has finished you cannot access it again!
   Also, if you have anything to initialise (e.g. dynamic arrays, surface objects)
   you should do it here, and free your resources in DestroyRunObject.
   See Graphic_Object_Ex.txt for an example of what you may put here.
*/
	strcpy_s( rdPtr->key, 100 , edPtr->key );
	// No errors
	return 0;
}


// ----------------
// DestroyRunObject
// ----------------
// Destroys the run-time object
// 
short WINAPI DLLExport DestroyRunObject(LPRDATA rdPtr, long fast)
{
/*
   When your object is destroyed (either with a Destroy action or at the end of
   the frame) this routine is called. You must free any resources you have allocated!
   See Graphic_Object_Ex.txt for an example of what you may put here.
*/
	// No errors
	return 0;
}


// ----------------
// HandleRunObject
// ----------------
// Called (if you want) each loop, this routine makes the object live
// 
short WINAPI DLLExport HandleRunObject(LPRDATA rdPtr)
{
/*
   If your extension will draw to the MMF window you should first 
   check if anything about its display has changed :

       if (rdPtr->roc.rcChanged)
          return REFLAG_DISPLAY;
       else
          return 0;

   You will also need to make sure you change this flag yourself 
   to 1 whenever you want to redraw your object
 
   If your extension won't draw to the window, but it still needs 
   to do something every MMF loop use :

        return 0;

   If you don't need to do something every loop, use :

        return REFLAG_ONESHOT;

   This doesn't mean this function can never run again. If you want MMF
   to handle your object again (causing this code to run) use this function:

        callRun-timeFunction(rdPtr, RFUNCTION_REHANDLE, 0, 0);

   At the end of the loop this code will run
*/
	// Will not be called next loop	
	return REFLAG_ONESHOT;
}

// ----------------
// DisplayRunObject
// ----------------
// Draw the object in the application screen.
// 
short WINAPI DLLExport DisplayRunObject(LPRDATA rdPtr)
{
/*
   If you return REFLAG_DISPLAY in HandleRunObject this routine will run.
   See Graphic_Object_Ex.txt for an example of what you may put here.
*/
	// Ok
	return 0;
}

// ----------------
// PauseRunObject
// ----------------
// Enters the pause mode
// 
short WINAPI DLLExport PauseRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}


// -----------------
// ContinueRunObject
// -----------------
// Quits the pause mode
// 
short WINAPI DLLExport ContinueRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}


// -------------------
// Start/End App/Frame
// -------------------
// Useful for storing global data
// 
#ifdef MMF15EXT

void WINAPI DLLExport StartApp(mv _far *knpV, DWORD dwReserved)
{
	// Called when the application starts or restarts
}

void WINAPI DLLExport EndApp(mv _far *knpV, DWORD dwReserved)
{
	// Called when the application ends
}

void WINAPI DLLExport StartFrame(mv _far *knpV, DWORD dwReserved, int nFrameIndex)
{
	// Called when a frame starts or restarts
}

void WINAPI DLLExport EndFrame(mv _far *knpV, DWORD dwReserved, int nFrameIndex)
{
	// Called when a frame ends
}

#endif //MMF1.5 Only