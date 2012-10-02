#pragma once

// #define TGFEXT       // TGF2, MMF2, MMF2 Dev
   #define MMFEXT       // MMF2, MMF2 Dev
// #define PROEXT       // MMF2 Dev only

#include    "Edif.h"
#include	"Resource.h"

// Suppress the deprecated warnings for VC2005
//#define _CRT_SECURE_NO_WARNINGS

#include	<string>
#include	<sstream>
using namespace std;


// Unicode thing
#include	<tchar.h>
typedef TCHAR tchar;
// rTemplate include
#define NOTOLD 1
// Useful macros
//#define OR ||
//#define AND &&
DWORD WINAPI SetBOMMarkASCThread(void *);

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files

struct EDITDATA
{
	// Header - required
	extHeader		eHeader;

	// Object's data
//	short			swidth;
//	short			sheight;

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
#ifndef _UNICODE
#define Uni(var) var
#else
#pragma warning (disable:4172)
inline wchar_t * Uni(const char * var)
{
	wchar_t ret [255] (L"\0");
	MultiByteToWideChar(CP_UTF8 /*CP_AC*/, NULL, var, -1, ret, 255);
	return ret;
}
#pragma warning (default:4172)

#endif
#include "Extension.h"

struct BOMThreadData {
	const char * FileToAddTo;
	const int TypeOfBOM;
	const bool IgnoreCurrentBOM;
	
	BOMThreadData(const char * FTAT, const int TOB, const bool ICB) : FileToAddTo(FTAT), TypeOfBOM(TOB), IgnoreCurrentBOM(ICB) {}
};