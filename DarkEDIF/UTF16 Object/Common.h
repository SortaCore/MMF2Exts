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

// If your extension will be using multithreading, remove the #if and #endif lines here.
#if 0
	#define MULTI_THREADING
	#include "MultiThreading.h"
#endif
#ifndef _UNICODE
	#error UTF-16 Object requires Unicode runtime, or the text expressions will be downgraded by Fusion to non-Unicode, which means they can never be displayed.
#endif

#include    "Edif.h"
#include	"Resource.h"
#include	"DarkEdif.h"

#include <cvt/wstring>
#include <codecvt>
extern Edif::SDK * SDK;
// edPtr : Used at edittime and saved in the MFA/CCN/EXE files

struct EDITDATA
{
	// Header - required
	extHeader		eHeader;

	// Object's data
	
//	short			swidth;
//	short			sheight;

	// Keep DarkEDIF variables as last. Undefined behaviour otherwise.
	int				DarkEDIF_Prop_Size;
	char			DarkEDIF_Props[0];

	// DarkEDIF functions, use within Extension ctor.
	bool IsPropChecked(int propID);
	const char * GetPropertyStr(const char * propName);
	const char * GetPropertyStr(int propID);
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



#include "Extension.h"
