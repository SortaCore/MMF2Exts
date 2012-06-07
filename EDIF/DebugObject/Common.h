#pragma once

// #define TGFEXT       // TGF2, MMF2, MMF2 Dev
   #define MMFEXT       // MMF2, MMF2 Dev
// #define PROEXT       // MMF2 Dev only
#include    "Edif.h"
#include	"Resource.h"
#include	<time.h>
#include	<sstream>
// edPtr : Used at edittime and saved in the MFA/CCN/EXE files


struct EDITDATA
{
	// Header - required
	extHeader		eHeader;

	// Object's data
//	short			swidth;
//	short			sheight;
	bool EnableAtStart;
	bool DoMsgBoxIfPathNotSet;
	char InitialPath [MAX_PATH+1];

};

class Extension;

struct RUNDATA
{
	// Main header - required
	headerObject rHo;

	// Optional headers - depend on the OEFLAGS value, see documentation and examples for more info
//	rCom			rc;				// Common structure for movements & animations
//	rMvt			rm;				// Movements
//	rSpr			rs;				// Sprite (displayable objects)
//	rVal			rv;				// Alterable values

    // Required
	Extension * pExtension;

    /*
        You shouldn't add any variables or anything here. Add them as members
        of the Extension class (Extension.h) instead.
    */
};
struct GlobalData {
	FILE * FileHandle;
	volatile bool FileHandleOpen;
	bool DebugEnabled, DoMsgBoxIfPathNotSet;
	time_t rawtime;
	struct tm * timeinfo;
	char * TimeFormat;
	char * RealTime;
	unsigned char NumUsages;

	// Allows debugging of what had the last lock
	#ifdef _DEBUG
		std::string LastLockFile;
		int LastLockLine;
		#define OpenLock() \
			while (Data->FileHandleOpen) \
				Sleep(0); \
			Data->FileHandleOpen = true; \
			Data->LastLockFile = __FILE__; \
			Data->LastLockLine = __LINE__
		#define CloseLock() Data->FileHandleOpen = false
	#else
		#define OpenLock() \
			while (Data->FileHandleOpen) \
				Sleep(0); \
			Data->FileHandleOpen = true
		#define CloseLock() Data->FileHandleOpen = false
	#endif

	// Handle exceptions
	char HandleExceptionVia, ContinuesCount, ContinuesMax;
	const enum {
		HANDLE_VIA_QUIT,
		HANDLE_VIA_INFINITE_WAIT,
		HANDLE_VIA_IGNORE,
		HANDLE_VIA_CONTINUE,

		HANDLE_VIA_MAX
	};
};

extern GlobalData * Data;
extern class Extension * GlobalExt;

#include "Extension.h"


