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
#include <sstream>
#include <vector>
using namespace std;

#include <tchar.h>

#ifdef UNICODE
#define tstringstream wstringstream
#define tstring wstring
#define tchar wchar_t
#define ts sizeof(wchar_t)
#pragma warning (disable:4172)
inline wchar_t * Uni(const char * var)
{
	wchar_t ret [255] (L"\0");
	MultiByteToWideChar(CP_UTF8 /*CP_AC*/, NULL, var, -1, ret, 255);
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
//#define TCHAR tchar
// rTemplate include
#define NOTOLD 1

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files

// PortAudio stuff: Define player
#define PA_USE_DS

// PortAudio stuff: Define sample
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

#include	"portaudio.h"
#pragma comment(lib, "..\\Lib\\portaudio.lib")
#pragma comment(lib, "C:\\Program Files\\Microsoft SDKs\\Windows\\v7.0A\\Lib\\dsound.lib")

// rTemplate include
//#include	"rTemplate.h"

/* #define SAMPLE_RATE  (17932) //Fails with this value, deliberately given.*/
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (1024)
#define NUM_SECONDS     (5)
#define NUM_CHANNELS    (2)
/* #define DITHER_FLAG     (paDitherOff)  */
#define DITHER_FLAG     (0) 

#ifndef NoExt // FOR NORMAL FUNCTIONS
//Macros for Thread Safety
#define ThreadSafe_Start() while (threadsafe){Sleep(0);} LastLockFile = ##__FILE__; LastLockLine = __LINE__; threadsafe=true
#define ThreadSafe_End() threadsafe=false
#define CallEvent(var) while (nogevent) {Sleep(0);} nogevent=true; Runtime.GenerateEvent(var); nogevent=false

#define Explode(foo) Unreferenced_Error(_T(foo), -1)	// Simplifying
#define Report(foo) Unreferenced_Report(_T(foo), -1)	// Ditto
#define ReturnToMMF(a,b,c) Unreferenced_ReturnToMMF(a,b,c)	// Ditto

#else // FOR THREADS
//Macros for Thread Safety
#define ThreadSafe_Start() while (Extension->threadsafe){Sleep(0);} Extension->LastLockFile = ##__FILE__; Extension->LastLockLine = __LINE__; Extension->threadsafe=true
#define ThreadSafe_End() Extension->threadsafe=false
#define CallEvent(var) while (Extension->nogevent) {Sleep(0);} Extension->nogevent=true; Extension->Runtime.GenerateEvent(var); Extension->nogevent=false

#define Explode(foo) Extension->Unreferenced_Error(_T(foo), ThreadID)	// Simplifying
#define Report(foo) Extension->Unreferenced_Report(_T(foo), ThreadID)	// Ditto

#define ReturnToMMF(a,b,c) Extension->Unreferenced_ReturnToMMF(a,b,c)	// Ditto
#endif


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

#include "Structures.h"
#include "Extension.h"
