
// ============================================================================
//
// The following routines are used internally by MMF, and should not need to
// be modified.
//
// 
// ============================================================================

// Common Include
#include	"common.h"

HINSTANCE hInstLib;

// ============================================================================
//
// LIBRARY ENTRY & QUIT POINTS
// 
// ============================================================================

// -----------------
// Entry points
// -----------------
// Usually you do not need to do any initialization here: you will prefer to 
// do them in "Initialize" found in Edittime.cpp
BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		// DLL is attaching to the address space of the current process.
		case DLL_PROCESS_ATTACH:
			
			hInstLib = hDLL; // Store HINSTANCE
			break;

		// A new thread is being created in the current process.
		case DLL_THREAD_ATTACH:
			break;

		// A thread is exiting cleanly.
		case DLL_THREAD_DETACH:
			break;

		// The calling process is detaching the DLL from its address space.
		case DLL_PROCESS_DETACH:
			break;
	}
	
	return TRUE;
}

// -----------------
// Initialize
// -----------------
// Where you want to do COLD-START initialization.
// Called when the extension is loaded into memory.
//
extern "C" int WINAPI DLLExport Initialize(mv _far *mV, int quiet)
{
	// No error
	return 0;
}

// -----------------
// Free
// -----------------
// Where you want to kill and initialized data opened in the above routine
// Called just before freeing the DLL.
// 
extern "C" int WINAPI DLLExport Free(mv _far *mV)
{
	// No error
	return 0;
}

// ============================================================================
//
// GENERAL INFO
// 
// ============================================================================

// -----------------
// Get Infos
// -----------------
// 
extern "C" 
{
	DWORD WINAPI DLLExport GetInfos(int info)
	{
		
		switch (info)
		{
			case KGI_VERSION:
				return EXT_VERSION2;				// Do not change
			case KGI_PLUGIN:
				return EXT_PLUGIN_VERSION1;			// Do not change
			case KGI_PRODUCT:
#if defined(PROEXT)
				return PRODUCT_VERSION_DEV;			// Works with MMF Developer 
#else
				return PRODUCT_VERSION_STANDARD;	// Works with MMF Standard or above
#endif
			case KGI_BUILD:
				return MINBUILD;
			case KGI_UNICODE:
				#ifdef _UNICODE
					return TRUE;
				#else
					return FALSE;
				#endif
			default:
				return 0;
		}
	}
}

// ----------------------------------------------------------
// GetRunObjectInfos
// ----------------------------------------------------------
// Fills an information structure that tells CC&C everything
// about the object, its actions, conditions and expressions
// 

short WINAPI DLLExport GetRunObjectInfos(mv _far *mV, fpKpxRunInfos infoPtr)
{
	infoPtr->conditions = (LPBYTE)ConditionJumps;
	infoPtr->actions = (LPBYTE)ActionJumps;
	infoPtr->expressions = (LPBYTE)ExpressionJumps;

	infoPtr->numOfConditions = CND_LAST;
	infoPtr->numOfActions = ACT_LAST;
	infoPtr->numOfExpressions = EXP_LAST;

	infoPtr->editDataSize = MAX_EDITSIZE;
	infoPtr->editFlags= OEFLAGS;

	infoPtr->windowProcPriority = WINDOWPROC_PRIORITY;

	// See doc
	infoPtr->editPrefs = OEPREFS;

	// Identifier, for run-time identification
	infoPtr->identifier = IDENTIFIER;
	
	// Current version
	infoPtr->version = KCX_CURRENT_VERSION;
	
	return TRUE;
}

// ----------------------------------------------------------
// GetDependencies
// ----------------------------------------------------------
// Returns the name of the external modules that you wish MMF to include
// with stand-alone applications (these modules must be in the MMF
// Data\Runtime folder).
// 

//LPCTSTR szDep[] = {
//	_T("MyDll.dll"),
//	NULL
//};

LPCTSTR* WINAPI DLLExport GetDependencies()
{
	return NULL;	// szDep;
}

// -----------------
// LoadObject
// -----------------
// Routine called for each object when the object is read from the MFA file (edit time)
// or from the CCN or EXE file (run time).
// You can load data here, reserve memory etc...
//
int	WINAPI DLLExport LoadObject(mv _far *mV, LPCSTR fileName, LPEDATA edPtr, int reserved)
{
	return 0;
}

// -----------------
// UnloadObject
// -----------------
// The counterpart of the above routine: called just before the object is
// deleted from the frame.
//
void WINAPI DLLExport UnloadObject(mv _far *mV, LPEDATA edPtr, int reserved)
{
}

// --------------------
// UpdateEditStructure
// --------------------
// For you to update your object structure to newer versions
// Called at both edit time and run time
// 
#ifdef _UNICODE
HGLOBAL UpdateEditStructureAToW(mv *pMV, LPEDATAA edPtrA)
{
	// Allocate new buffer
	int newSize = offsetof(EDITDATAW, sText) + (strlen(edPtrA->sText)+1) * sizeof(WCHAR);
	HGLOBAL hg = GlobalAlloc(GPTR, newSize);
	LPEDATAW edPtrW = (LPEDATAW)GlobalLock(hg);

	// Convert edPtrA (EDITDATAA*) to edPtrW (EDITDATAW*) :

		// Copy beginning of structure until textFont
		memcpy(&edPtrW->eHeader, &edPtrA->eHeader, offsetof(EDITDATAA, textFont));

		// Convert LOGFONT
		memcpy(&edPtrW->textFont, &edPtrA->textFont, offsetof(LOGFONTA, lfFaceName));
		MultiByteToWideChar(mvGetAppCodePage(pMV, pMV->mvEditApp), 0, edPtrA->textFont.lfFaceName, LF_FACESIZE, edPtrW->textFont.lfFaceName, LF_FACESIZE);

		// Convert text
		MultiByteToWideChar(mvGetAppCodePage(pMV, pMV->mvEditApp), 0, edPtrA->sText, -1, edPtrW->sText, newSize-offsetof(EDITDATAW, sText));

	// Update EDITDATA size
	edPtrW->eHeader.extSize = offsetof(EDITDATAW, sText) + (wcslen(edPtrW->sText)+1) * sizeof(WCHAR);

	// It's a good idea to verify the memory didn't get corrupt at this point ;)
	_ASSERTE(_CrtCheckMemory());

	GlobalUnlock(hg);
	return hg;
}
#else
HGLOBAL UpdateEditStructureWToA(mv *pMV, LPEDATAW edPtrW)
{
	// Allocate new buffer
	int newSize = offsetof(EDITDATAA, sText) + (wcslen(edPtrW->sText)+1) * sizeof(char);
	HGLOBAL hg = GlobalAlloc(GPTR, newSize);
	LPEDATAA edPtrA = (LPEDATAA)GlobalLock(hg);

	// Convert edPtrW (EDITDATAW*) to edPtrA (EDITDATAA*)

		// Copy beginning of structure until textFont
		memcpy(&edPtrA->eHeader, &edPtrW->eHeader, offsetof(EDITDATAW, textFont));

		// Convert LOGFONT
		memcpy(&edPtrA->textFont, &edPtrW->textFont, offsetof(LOGFONTW, lfFaceName));
		WideCharToMultiByte(mvGetAppCodePage(pMV, pMV->mvEditApp), 0, edPtrW->textFont.lfFaceName, LF_FACESIZE, edPtrA->textFont.lfFaceName, LF_FACESIZE, NULL, NULL);

		// Convert text
		WideCharToMultiByte(mvGetAppCodePage(pMV, pMV->mvEditApp), 0, edPtrW->sText, -1, edPtrA->sText, newSize-offsetof(EDITDATAA, sText), NULL, NULL);

	// Update EDITDATA size
	edPtrA->eHeader.extSize = offsetof(EDITDATAA, sText) + (strlen(edPtrA->sText)+1) * sizeof(char);

	// It's a good idea to verify the memory didn't get corrupt at this point ;)
	_ASSERTE(_CrtCheckMemory());

	GlobalUnlock(hg);
	return hg;
}
#endif

HGLOBAL WINAPI DLLExport UpdateEditStructure(mv __far *pMV, void __far * oldEdPtr)
{
	HGLOBAL hg = NULL;

#ifdef _UNICODE
	// ANSI to Unicode
	if ( !mvIsUnicodeApp(pMV, pMV->mvEditApp) )
		hg = UpdateEditStructureAToW(pMV, (LPEDATAA)oldEdPtr);
#else
	// Unicode to ANSI
	if ( mvIsUnicodeApp(pMV, pMV->mvEditApp) )
		hg = UpdateEditStructureWToA(pMV, (LPEDATAW)oldEdPtr);
#endif // _UNICODE

	return hg;
}

// --------------------
// UpdateFileNames
// --------------------
// If you store file names in your datazone, they have to be relocated when the
// application is moved to a different directory: this routine does it.
// Called at edit time and run time.
//
// Call lpfnUpdate to update your file pathname (refer to the documentation)
// 
void WINAPI DLLExport UpdateFileNames(mv _far *mV, LPTSTR appName, LPEDATA edPtr, void (WINAPI * lpfnUpdate)(LPTSTR, LPTSTR))
{
}

// ---------------------
// EnumElts
// ---------------------
//
// Uncomment this function if you need to store an image in the image bank.
//
// Note: do not forget to enable the function in the .def file 
// if you remove the comments below.
//
/*
int WINAPI DLLExport EnumElts (mv __far *mV, LPEDATA edPtr, ENUMELTPROC enumProc, ENUMELTPROC undoProc, LPARAM lp1, LPARAM lp2)
{  
	int error = 0;

	// Replace wImgIdx with the name of the WORD variable you create within the edit structure
  
	// Enum images  
	if ( (error = enumProc(&edPtr->wImgIdx, IMG_TAB, lp1, lp2)) != 0 )
	{
		// Undo enum images	  
		undoProc (&edPtr->wImgIdx, IMG_TAB, lp1, lp2);	
	}  

	return error;
}
*/

