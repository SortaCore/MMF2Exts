#pragma once

// #define TGFEXT       // TGF2, MMF2, MMF2 Dev
   #define MMFEXT       // MMF2, MMF2 Dev
// #define PROEXT       // MMF2 Dev only

#ifdef RUN_ONLY
	#define CurLang (*::SDK->json.u.object.values[::SDK->json.u.object.length - 1].value)
#else
	const extern struct _json_value & CurrentLanguage();
	#define CurLang CurrentLanguage()
#endif

#define DLLExport   __stdcall
#pragma comment(lib, "..\\Lib\\mmfs2.lib")

#define EnterCriticalSectionDerpy(x) \
	EnterCriticalSection(x); \
	sprintf_s(::Buffer, "Thread %u : Entered on %s, line %i.\r\n", GetCurrentThreadId(), __FILE__, __LINE__); \
	::CriticalSection = ::Buffer + ::CriticalSection
	

#define LeaveCriticalSectionDerpy(x) \
	sprintf_s(::Buffer, "Thread %u : Left on %s, line %i.\r\n", GetCurrentThreadId(), __FILE__, __LINE__); \
	::CriticalSection = ::Buffer + ::CriticalSection; \
	LeaveCriticalSection(x)


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

extern char Buffer [200];
extern std::string CriticalSection;
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

	// Keep as last or risk overwriting by functions accessing this address
	size_t DarkEDIF_Prop_Size;
	char DarkEDIF_Props[0];
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

#define COMMON_H
#include "Extension.h"
#undef COMMON_H
