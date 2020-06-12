#pragma once

// #define TGFEXT	// TGF2, MMF2, MMF2 Dev
#define MMFEXT		// MMF2, MMF2 Dev
// #define PROEXT	// MMF2 Dev only

#ifdef RUN_ONLY
	#define CurLang (*::SDK->json.u.object.values[::SDK->json.u.object.length - 1].value)
#else
	const extern struct _json_value & CurrentLanguage();
	#define CurLang CurrentLanguage()
#endif

#define JSON_COMMENT_MACRO Extension::Version

#include <sstream>
#include "Edif.h"
#include "Resource.h"
#include "DarkEdif.h"

#include "..\Inc\Windows\ksnd.h"
// Static linking of KSND
#pragma comment(lib, "..\\Lib\\Windows\\ksndstatic.lib")
#pragma comment(lib, "winmm.lib")

#define CheckForPlayer(makeError, funcName, ...) \
	if (curPlayer == nullptr) { \
		if (makeError) \
			CreateError(funcName##": No player selected!"); \
		return __VA_ARGS__; \
	}

#ifdef _UNICODE
	#error KSND library not available in Unicode; all functions are ANSI.
	// In case it ever is available, TCHAR variants are used in this ext.
#endif


extern Edif::SDK * SDK;
// edPtr : Used at edittime and saved in the MFA/CCN/EXE files

struct EDITDATA
{
	// Header - required
	extHeader		eHeader;

	// Object's data

//	short			swidth;
//	short			sheight;

	// Keep DarkEdif variables as last. Undefined behaviour otherwise.
	int				DarkEdif_Prop_Size;
	char			DarkEdif_Props[];

	// DarkEdif functions, use within Extension ctor.
	bool IsPropChecked(int propID);
	std::tstring GetPropertyStr(const char * propName);
	std::tstring GetPropertyStr(int propID);
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
