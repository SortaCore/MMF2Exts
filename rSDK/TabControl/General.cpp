/// =====================================================================================
//
// The following routines are used internally by MMF, and should not need to be modified
// 
// =====================================================================================

#include "common.h"
__inline int mvGetAppCodePage(LPMV mV, LPVOID pApp) \
	{ return mV->mvCallFunction(NULL, 115, (LPARAM)pApp, (LPARAM)0, (LPARAM)0); }

HINSTANCE hInstLib;
EXT_INIT()

// ============================================================================
//
// LIBRARY ENTRY & QUIT POINTS
// 
// ============================================================================

// -----------------
// Entry points
// -----------------
// Usually you do not need to do any initialization here: it is preferable to
// do it in "Initialize" found in Edittime.cpp

BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{

	conditionsInfos = getConditionInfos();
	actionsInfos = getActionInfos();
	expressionsInfos = getExpressionInfos();
	
	ConditionJumps = getConditions();
	ActionJumps = getActions();
	ExpressionJumps = getExpressions();
	
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
				return EXT_VERSION2;
			case KGI_PLUGIN:
				return EXT_PLUGIN_VERSION1;
			case KGI_PRODUCT:
				return ForVersion;
			case KGI_BUILD:
				return MinimumBuild;
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
// Fills an information structure that tells MMF2 everything
// about the object, its actions, conditions and expressions
// 

short WINAPI DLLExport GetRunObjectInfos(mv _far *mV, fpKpxRunInfos infoPtr)
{
	infoPtr->conditions = (LPBYTE)ConditionJumps;
	infoPtr->actions = (LPBYTE)ActionJumps;
	infoPtr->expressions = (LPBYTE)ExpressionJumps;

	infoPtr->numOfConditions = Conditions.size();
	infoPtr->numOfActions = Actions.size();
	infoPtr->numOfExpressions = Expressions.size();

	infoPtr->editDataSize = sizeof(EDITDATA);
	
	MagicFlags(infoPtr->editFlags);

	infoPtr->windowProcPriority = WINDOWPROC_PRIORITY;

	MagicPrefs(infoPtr->editPrefs);

	infoPtr->identifier = IDENTIFIER;
	#ifndef UNICODE //This is used so conversion between ASCII and Unicode EDITDATA is registered
		infoPtr->version = 3;
	#else //ASCII
		infoPtr->version = 4;
	#endif
	return TRUE;
}

// ----------------------------------------------------------
// GetDependencies
// ----------------------------------------------------------
// Returns the name of the external modules that you wish MMF to include
// with stand-alone applications (these modules must be in the MMF
// Data\Runtime folder).
//

LPCSTR szDep[] = {
	"MyDll.dll",
	NULL
};
LPCSTR* WINAPI DLLExport GetDependencies()
{
	// Do some rSDK stuff
	#include "rGetDependencies.h"

	return NULL; // szDep;
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
#define p(foo) NewEdPtr->##foo = OldEdPtr->##foo
HGLOBAL WINAPI DLLExport UpdateEditStructure(mv __far *mV, OLDEDITDATA * AnyEdPtr)
{
		//Version 1 -> 3
		if (AnyEdPtr->eHeader.extVersion < 2)
		{
			LPEDATA NewEdPtr=0;
			OLDEDITDATA* OldEdPtr = AnyEdPtr;
			NewEdPtr = (EDITDATA*)GlobalAlloc(GPTR,sizeof(EDITDATA));
			memcpy(&NewEdPtr->eHeader, &OldEdPtr->eHeader, sizeof(extHeader));
			NewEdPtr->eHeader.extSize = sizeof(EDITDATA);    // Update the EDITDATA structure size
			NewEdPtr->eHeader.extVersion = 3;
			p(nWidth); p(nHeight);
			p(textColor); p(backColor);
			p(dwAlignFlags); p(dwFlags);
			memcpy(&NewEdPtr->textFont, &OldEdPtr->textFont, sizeof(LOGFONT));
			memcpy(&NewEdPtr->opts, &OldEdPtr->opts, sizeof(bool)*4);
			NewEdPtr->iconH = 16;
			NewEdPtr->iconW = 16;
			NewEdPtr->maxIcon = 256;
			NewEdPtr->buttonStyle = 0;
			NewEdPtr->tabDir = 0;
			NewEdPtr->lineMode = 0;
			NewEdPtr->tabMode = 0;
			NewEdPtr->sText[0] = 0;

			for (int i=0;i<16;i++)
				NewEdPtr->wImages[i] = 0;
			NewEdPtr->nImages = 0;

			return (HGLOBAL)NewEdPtr;
		}
		//Version 2 -> 3
		else if (AnyEdPtr->eHeader.extVersion == 2)
		{
			LPEDATA NewEdPtr=0;
			OLDEDITDATA2* OldEdPtr = (OLDEDITDATA2*)AnyEdPtr;
			NewEdPtr = (EDITDATA*)GlobalAlloc(GPTR,sizeof(EDITDATA));
			memcpy(&NewEdPtr->eHeader, &OldEdPtr->eHeader, sizeof(extHeader));
			NewEdPtr->eHeader.extSize = sizeof(EDITDATA);    // Update the EDITDATA structure size
			NewEdPtr->eHeader.extVersion = 3;
			p(nWidth); p(nHeight);
			p(textColor); p(backColor);
			p(dwAlignFlags); p(dwFlags);
			memcpy(&NewEdPtr->textFont, &OldEdPtr->textFont, sizeof(LOGFONT));
			memcpy(&NewEdPtr->opts, &OldEdPtr->opts, sizeof(bool)*4);
			p(iconH); p(iconW);
			p(maxIcon);
			p(buttonStyle);
			p(tabDir);
			p(lineMode);
			p(tabMode);
			NewEdPtr->sText[0] = 0;

			for (int i=0;i<16;i++)
				NewEdPtr->wImages[i] = 0;
			NewEdPtr->nImages = 0;

			return (HGLOBAL)NewEdPtr;
		}
#if defined(UNICODE) && !defined(RUN_ONLY) //If data is in ASCII format
		//Version 3 -> 4 (ASCII -> Unicode)
		else if (AnyEdPtr->eHeader.extVersion == 3)
		{
			//Initialise EDITDATAs
			EDITDATAA* OldEdPtr = (EDITDATAA*)AnyEdPtr;
			EDITDATAW* NewEdPtr = (EDITDATAW*)GlobalAlloc(GPTR,sizeof(EDITDATAW));
			
			// Copy eHeader
			memcpy(&NewEdPtr->eHeader, &OldEdPtr->eHeader, sizeof(extHeader));
			NewEdPtr->eHeader.extSize = sizeof(EDITDATAW);    // Update the EDITDATA structure size
			NewEdPtr->eHeader.extVersion = 4;

			// Copy until the LogFont structure
			p(nWidth); p(nHeight);
			p(textColor); p(backColor);
			p(dwAlignFlags); p(dwFlags);

			// Copy all similarities between LOGFONTA and LOGFONTW
			p(textFont.lfHeight); p(textFont.lfWidth);
			p(textFont.lfEscapement); p(textFont.lfOrientation);
			p(textFont.lfWeight); p(textFont.lfItalic);
			p(textFont.lfUnderline); p(textFont.lfStrikeOut);
			p(textFont.lfCharSet); p(textFont.lfOutPrecision);
			p(textFont.lfClipPrecision); p(textFont.lfQuality);
			p(textFont.lfPitchAndFamily);
			
			// LOGFONTA has char [] so
			MultiByteToWideChar(mvGetAppCodePage(mV, mV->mvEditApp), 0, OldEdPtr->textFont.lfFaceName, LF_FACESIZE, NewEdPtr->textFont.lfFaceName, LF_FACESIZE);
					
			//Continue with other properties
			memcpy(&NewEdPtr->opts, &OldEdPtr->opts, sizeof(bool)*4);
			p(iconH); p(iconW);
			p(maxIcon);
			p(buttonStyle);
			p(tabDir);
			p(lineMode);
			p(tabMode);
			for (int i=0;i<16;i++)
				p(wImages[i]);
			p(nImages);
			
			// Copy final char [], sText
			MultiByteToWideChar(mvGetAppCodePage(mV, mV->mvEditApp), 0, OldEdPtr->sText, sizeof(OldEdPtr->sText), NewEdPtr->sText, sizeof(NewEdPtr->sText));
			
			// Done
			return (HGLOBAL)NewEdPtr;
		}
#elif !defined(UNICODE) && !defined(RUN_ONLY) //If old data is Unicode, but running ASCII MMF2...
		//Version 4 -> 3 (Unicode -> ASCII)
		else if (AnyEdPtr->eHeader.extVersion == 4)
		{
			//None of that long copying thing. Let's do memory copying.
			//First declare the two pointers.
			EDITDATAW* OldEdPtr = (EDITDATAW*)AnyEdPtr;
			EDITDATAA* NewEdPtr = (EDITDATAA*)GlobalAlloc(GPTR,sizeof(EDITDATAA));
			
			// Copy eHeader
			memcpy(&NewEdPtr->eHeader, &OldEdPtr->eHeader, sizeof(extHeader));
			NewEdPtr->eHeader.extSize = sizeof(EDITDATAA);    // Update the EDITDATA structure size
			NewEdPtr->eHeader.extVersion = 3;

			// Copy until the LogFont structure
			p(nWidth); p(nHeight);
			p(textColor); p(backColor);
			p(dwAlignFlags); p(dwFlags);

			// Copy all similarities between LOGFONTA and LOGFONTW
			p(textFont.lfHeight); p(textFont.lfWidth);
			p(textFont.lfEscapement); p(textFont.lfOrientation);
			p(textFont.lfWeight); p(textFont.lfItalic);
			p(textFont.lfUnderline); p(textFont.lfStrikeOut);
			p(textFont.lfCharSet); p(textFont.lfOutPrecision);
			p(textFont.lfClipPrecision); p(textFont.lfQuality);
			p(textFont.lfPitchAndFamily);
			
			// LOGFONTA has char [] so
			WideCharToMultiByte(mvGetAppCodePage(mV, mV->mvEditApp), 0, OldEdPtr->textFont.lfFaceName, LF_FACESIZE, NewEdPtr->textFont.lfFaceName, LF_FACESIZE,NULL,NULL);
					
			//Continue with other properties
			memcpy(&NewEdPtr->opts, &OldEdPtr->opts, sizeof(bool)*4);
			p(iconH); p(iconW);
			p(maxIcon);
			p(buttonStyle);
			p(tabDir);
			p(lineMode);
			p(tabMode);
			for (int i=0;i<16;i++)
				p(wImages[i]);
			p(nImages);
			
			// Copy final char [], sText
			WideCharToMultiByte(mvGetAppCodePage(mV, mV->mvEditApp), 0, OldEdPtr->textFont.lfFaceName, LF_FACESIZE, NewEdPtr->textFont.lfFaceName, LF_FACESIZE,NULL,NULL);
			
			return (HGLOBAL)NewEdPtr;
		}
#endif
	return NULL;
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
void WINAPI DLLExport UpdateFileNames(mv _far *mV, LPSTR appName, LPEDATA edPtr, void (WINAPI * lpfnUpdate)(LPSTR, LPSTR))
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

int WINAPI DLLExport EnumElts (mv __far *mV, LPEDATA edPtr, ENUMELTPROC enumProc, ENUMELTPROC undoProc, LPARAM lp1, LPARAM lp2)
{  
	int error = FALSE;

	// Enum images
	for (int i=0;i<edPtr->nImages;i++) {
		error = enumProc(&edPtr->wImages[i],IMG_TAB,lp1,lp2);
		if (error) {
			// Undo enum images
			for (int j=i-1;j>=0;j--)
				undoProc(&edPtr->wImages[j],IMG_TAB,lp1,lp2);
			break;
		}
	}
	return error;
}
