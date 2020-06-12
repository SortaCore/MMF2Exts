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

// Must be before windows.h, or WinSock v1 is added by windows.h and WinSock v2 conflicts with it
#include <winsock2.h>
#ifndef _WINDOWS_
	#include <windows.h>
#endif
#include <ws2tcpip.h>
#include <iphlpapi.h>
//#include <af_irda.h>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

#include "Edif.h"
#include "json.h"
#include "Resource.h"
#include "DarkEdif.h"

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

	// =====
	// DarkEdif functions, use within Extension ctor.
	// =====

	// Returns property checked or unchecked.
	bool IsPropChecked(int propID);
	// Returns std::tstring property setting from property name.
	std::tstring GetPropertyStr(const char * propName);
	// Returns std::tstring property string from property ID.
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
	AltVals			rv;				// Alterable values

	// Required
	Extension * pExtension;

	/*
		You shouldn't add any variables or anything here. Add them as members
		of the Extension class (Extension.h) instead.
	*/
};

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

// Include the structs/enums/etc used later on in the object
#include "Structures.h"

#include "Extension.h"
