#pragma once
#define TGFEXT       // TGF2, MMF2, MMF2 Dev
// #define MMFEXT       // MMF2, MMF2 Dev
// #define PROEXT       // MMF2 Dev only
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

#include    "Edif.h"
#include	"Resource.h"

// Suppress the deprecated warnings for VC2005
//#define _CRT_SECURE_NO_WARNINGS

#include "conversions.h"
//Stream & String
#include <string>
#include <sstream>
#include <vector>
using namespace std;

#include <tchar.h>

#ifdef UNICODE
#define tstringstream wstringstream
#define tstring wstring
#define tchar wchar_t
#define ts sizeof(wchar_t)
#else
#define tstringstream stringstream
#define tstring string
#define tchar char
#define ts sizeof(char)
#endif

#ifndef WIN32_LEAN_AND_MEAN // Ensures windows.h doesn't include winsock v1
#error WIN32_LEAN_AND_MEAN must be in preprocessor definitions!
#endif						// Not using it in preprocessor causes a ton of errors with non-rSDK files

#ifndef _WINDOWS_
#include <windows.h>
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
//#include <Af_irda.h>

// This code allows threads to be terminated in DestroyRunObject or elsewhere.
// Terminating threads should not be used, as a memory leak of the variables is very probable.
#ifdef AllowTermination
#define TermPush(CreateT)	SocketThreadList.push_back(CreateT)
#define NullStoredHandle(id) ThreadSafe_Start(); \
							 Extension->SocketThreadList[id] = NULL; \
							 ThreadSafe_End()
//#define TerminateOnEnd // Define this to have DestroyRunObject run TerminateThread() on all the threads.
#else
#define TermPush(CreateT) CreateT
#define NullStoredHandle(id)
#endif

#pragma comment(lib, "Ws2_32.lib")

#ifndef NoExt // FOR NORMAL FUNCTIONS
//Macros for Thread Safety
#define ThreadSafe_Start() while (threadsafe){Sleep(0);} LastLockFile = ##__FILE__; LastLockLine = __LINE__; threadsafe=true
#define ThreadSafe_End() threadsafe=false
#define CallEvent(var) while (nogevent) {Sleep(0);} nogevent=true; Runtime.GenerateEvent(var); nogevent=false

#define Explode(foo) Unreferenced_Error(_T(foo), -1)	// Simplifying
#define Report(foo) Unreferenced_Report(_T(foo), -1)	// Ditto
#define ReturnToMMF(a,b,c,d) Unreferenced_ReturnToMMF(a,b,c,d)	// Ditto

#else // FOR THREADS
//Macros for Thread Safety
#define ThreadSafe_Start() while (Extension->threadsafe){Sleep(0);} Extension->LastLockFile = ##__FILE__; Extension->LastLockLine = __LINE__; Extension->threadsafe=true
#define ThreadSafe_End() Extension->threadsafe=false
#define CallEvent(var) while (Extension->nogevent) {Sleep(0);} Extension->nogevent=true; Extension->Runtime.GenerateEvent(var); Extension->nogevent=false

#define Explode(foo) Extension->Unreferenced_Error(_T(foo), SocketID)	// Simplifying
#define Report(foo) Extension->Unreferenced_Report(_T(foo), SocketID)	// Ditto
#define ReturnToMMF(a,b,c,d) Extension->Unreferenced_ReturnToMMF(a,b,c,d)	// Ditto
#endif
// Unicode thing
#include <tchar.h>
// rTemplate include
#define NOTOLD 1

// Include the structs/enums/etc used later on in the object
#include "Structures.h"


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