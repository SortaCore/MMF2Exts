#pragma once

// #define TGFEXT       // TGF2, MMF2, MMF2 Dev
   #define MMFEXT       // MMF2, MMF2 Dev
// #define PROEXT       // MMF2 Dev only

int CurrentLanguage();
#ifdef RUN_ONLY
	#define CurLang 2
#else
	#define CurLang CurrentLanguage()
	extern int StoredCurrentLanguage;
#endif
#define DLLExport   __stdcall

#pragma comment(lib, "..\\Lib\\mmfs2.lib")
#pragma comment(lib, "..\\Lib\\zlib.lib")
#include <sstream>
#include "zlib.h"

// Blank LacewingFunction to stop Lacewing header thinking we want a .lib file
#define LacewingFunction
#include "Lacewing.h"
#include "LacewingFunctions.h"

#include "Edif.h"
#include "Resource.h"

// Enable DarkEDIF's utility
#define MULTI_THREADING
#include "MultiThreading.h"

#include "DarkEDIF.h"

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	// Header - required
	extHeader			eHeader;

	// Any random edittime stuff? Place here.
	char pad0[5];				// For matching old extension
	bool AutomaticClear,
		 MultiThreading;
	bool Global;
	char edGlobalID[255];

	// Keep last or risk overwriting by functions accessing this address
	/*size_t DarkEDIF_Prop_Size;
	char DarkEDIF_Props[0];
	
	//	Properties.reserve(::SDK->json[CurLang]["Properties"].u.object.length);
	//	PropCheckboxes.reserve(::SDK->json[CurLang]["Properties"].u.object.length);
	*/
};

class Extension;

struct RUNDATA
{
	// Main header - required
	HeaderObject rHo;

	// Optional headers - depend on the OEFLAGS value, see documentation and examples for more info
//	rCom			rc;				// Common structure for movements & animations
//	rMvt			rm;				// Movements
//	Sprite			rs;				// Sprite (displayable objects)
//	AltVals			rv;				// Alterable values

    // Required
	Extension * pExtension;

    /*
        You shouldn't add any variables or anything here. Add them as members
        of the Extension class (Extension.h) instead.
    */
};
DWORD WINAPI LacewingLoopThread(void * ThisExt);


#include "Extension.h"
