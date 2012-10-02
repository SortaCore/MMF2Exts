#pragma once

// #define TGFEXT       // TGF2, MMF2, MMF2 Dev
   #define MMFEXT       // MMF2, MMF2 Dev
// #define PROEXT       // MMF2 Dev only

#include    "Edif.h"
#include	"Resource.h"

// Suppress the deprecated warnings for VC2005
//#define _CRT_SECURE_NO_WARNINGS

//Stream & String
#include <string>
#include <vector>
using namespace std;



#ifdef UNICODE
#define tstringstream wstringstream
#define tstring wstring
#define tchar wchar_t
#define ts sizeof(wchar_t)
#pragma warning (disable:4172)
inline wchar_t * Uni(const char * var)
{
	wchar_t ret [255] (L"\0");
	MultiByteToWideChar(CP_ACP, NULL, var, -1, ret, 255);
	return ret;
}
#pragma warning (default:4172)
#else
#define tstringstream stringstream
#define tstring string
#define tchar char
#define ts sizeof(char)
#define Uni(var) var
#endif

#include    "Edif.h"
#include	"Resource.h"

// Unicode thing
#include	<tchar.h>
#define TCHAR tchar
// rTemplate include
#define NOTOLD 1

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
	rVal			rv;				// Alterable values

    // Required
	Extension * pExtension;

    /*
        You shouldn't add any variables or anything here. Add them as members
        of the Extension class (Extension.h) instead.
    */
};

#include "Extension.h"
