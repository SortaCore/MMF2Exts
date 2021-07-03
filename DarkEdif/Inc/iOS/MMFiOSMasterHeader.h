#pragma once

#if defined(_WIN32) || defined (__ANDROID__)
#error Included the wrong header for this OS.
#endif
#include <stdio.h>
#define memcpy_s(a,b,c,d) memcpy(a, c, d)
#define _strdup(a) strdup(a)
//wchar_t* wcsdup(const wchar_t*);
//wchar_t* wcscpy(wchar_t*, const wchar_t*);
//int wcscasecmp(const wchar_t*, const wchar_t*);
#define _wcsdup(a) wcsdup(a)
#define wcscpy_s(a,b,c) wcscpy(a,c)
#define strcat_s(a,b) strcat(a,b)
#define _stricmp(a,b) strcasecmp(a,b)
#define _strnicmp(a,b,c) strncasecmp(a,b,c)
#define sprintf_s(a,...) sprintf(a, __VA_ARGS__)
#define SUBSTRIFY(X) #X
#define STRIFY(X) #X

#define IDOK 0
#define IDCANCEL 1
#define MB_OK 0
#define MB_YESNO 0
#define MB_YESNOCANCEL 0
#define MB_DEFBUTTON1 0
#define MB_ICONERROR 1
#define MB_ICONWARNING 2
#define MB_ICONINFORMATION 3
#define MessageBox(a,b,c,d) MessageBoxA(a,b,c,d)
#define _msize(a) malloc_usable_size(a)
//#include <Foundation/Foundation.h>
#define OutputDebugStringA(a) { /* URGENT: NO OP */  }
#include <wchar.h>
#include <string>
#include <signal.h>
#include <stdarg.h>
#include <string_view>
using namespace std::string_literals;
using namespace std::string_view_literals;

#define ProjectFunc extern "C" __attribute__((visibility ("default"))) 
#define FusionAPI /* no declarator */
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#ifndef _UNICODE
#define TCHAR char
#define _tcsdup(a) strdup(a)
#define _tcslen(a) strlen(a)
#define _tcscpy(a,b) strcpy(a,b)
#define _tcscpy_s(a,b,c) strcpy(a,c)
#define _tcscat(a,b) strcat(a,b)
#define _tcscmp(a,b) strcmp(a,b)
#define _tcsicmp(a,b) strcasecmp(a,b)
#define memmove_s(a,b,c,d) memmove(a,c,d)
#define _tcsnicmp(a,b,c) strncasecmp(a,b,c)
#define _T(x) x
#define _totlower tolower
#define _totupper toupper
#define _ttof(f) atof(f)
#define _ttoi(i) atoi(i)
#define _vstprintf_s(a,b,c,d) sprintf_s(a,c,d)
#define _tcserror(a) strerror(a)
#if __cplusplus < 201703L
#error Not running in C++17 mode
#endif
namespace std {
	typedef std::string tstring;
	typedef std::stringstream tstringstream;
	typedef std::string_view tstring_view;
}

// You shouldn't be using Unicode during iOS builds, even for TCHAR.
// The NSString can use any internal encoding, so there's little point in converting pre-emptively.

#else
#error Not defined
#endif

#define LPCTSTR const TCHAR *
#define LPTSTR TCHAR *

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef float FLOAT;
typedef DWORD * LPDWORD;
typedef WORD * LPWORD;
typedef FLOAT * PFLOAT;
typedef void * HANDLE;
#define CALLBACK __stdcall
#define WINAPI __stdcall
typedef unsigned long DWORD;
typedef DWORD   COLORREF;
typedef DWORD   *LPCOLORREF;
typedef int BOOL;
typedef void * LPVOID;
typedef unsigned int UINT;
typedef long LONG;
typedef long * LPLONG;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned char * LPBYTE;
#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HFILE);
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HFONT);
DECLARE_HANDLE(HPALETTE);
DECLARE_HANDLE(HRGN);
DECLARE_HANDLE(HINSTANCE);
DECLARE_HANDLE(HCURSOR);
DECLARE_HANDLE(HGLOBAL);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HHOOK);

#define FAR
#define NEAR
typedef const char * LPCSTR;
typedef const wchar_t * LPCWSTR;
typedef	const void * LPCSPALETTE;
#define TRUE 1
#define FALSE 0
typedef int INT_PTR, *PINT_PTR;
typedef unsigned int UINT_PTR, *PUINT_PTR;

typedef long * LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef UINT_PTR            WPARAM;
typedef LONG_PTR            LPARAM;
typedef LONG_PTR            LRESULT;
typedef void * LOGFONTV1;
typedef void * ParamExtension;
typedef void * CRunApp;
#define MAX_PATH 255
#define __stdcall __attribute__ ((stdcall))
#define LOGFONT LOGFONTV1
#define LOGFONTA LOGFONTV1
#define LOGFONTW LOGFONTV1

#define DLLExport __attribute__ ((stdcall))
#define PhiDLLImport

#define _stprintf_s(a,b,c,...) sprintf(a, c, __VA_ARGS__)
#define _tcscpy_s(a,b,c) strcpy(a,c)
#define strcpy_s(a,b,c) strcpy(a,c)
// Make it quiet
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wcomment"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-variable"
int MessageBoxA(HWND hwnd, const TCHAR * caption, const TCHAR * text, int iconAndButtons);

// Note: doesn't use underlying_type due to incompatibility with one of the Android C++ STL libraries (stlport_static).
#define fancyenumop(enumType) \
enumType constexpr static operator|(enumType lhs, enumType rhs) { \
	return static_cast<enumType>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)); \
} \
enumType constexpr static operator&(enumType lhs, enumType rhs) { \
	return static_cast<enumType>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs)); \
} \
enumType static operator|=(enumType &lhs, enumType rhs) { \
	lhs = static_cast<enumType>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)); \
	return lhs; \
} \
enumType static operator&=(enumType &lhs, enumType rhs) { \
	lhs = static_cast<enumType>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs)); \
	return lhs; \
}
typedef unsigned short ushort;
typedef unsigned int uint;

//#include "Surface.h"

void LOGF(const char * x, ...);

#define LOGV(x,...) printf(x, __VA_ARGS__)
#define LOGI(x,...) printf(x, __VA_ARGS__)
#define LOGE(x,...) printf(x, __VA_ARGS__)

struct eventGroup {
	// Dummy group
	std::int16_t	evgInhibit;
	std::uint16_t	evgInhibitCpt;
	std::uint16_t	evgIdentifier;
	eventGroup() = delete;
};

struct CTransition;
struct CDebugger;
enum class KGI {
	VERSION,			// Version (required)
	NOTUSED,			// Not used
	PLUGIN,				// Version for plug-in (required)
	MULTIPLESUBTYPE,	// Allow sub-types
	NOTUSED2,			// Reserved
	ATX_CONTROL,		// Not used
	PRODUCT,			// Minimum product the extension is compatible with
	BUILD,				// Minimum build the extension is compatible with
	UNICODE_,			// Returns TRUE if the extension is in Unicode (UNICODE and _UNICODE are #defines)
};

// Surface.h pre-declaration
class cSurface;
struct cSurfaceImplementation;
struct CFillData;
class CInputFile;
struct sMask;
typedef void* jobject;

#define bit1  0x00000001
#define bit2  0x00000002
#define bit3  0x00000004
#define bit4  0x00000008
#define bit5  0x00000010
#define bit6  0x00000020
#define bit7  0x00000040
#define bit8  0x00000080
#define bit9  0x00000100
#define bit10 0x00000200
#define bit11 0x00000400
#define bit12 0x00000800
#define bit13 0x00001000
#define bit14 0x00002000
#define bit15 0x00004000
#define bit16 0x00008000
#define bit17 0x00010000
#define bit18 0x00020000
#define bit19 0x00040000
#define bit20 0x00080000
#define bit21 0x00100000
#define bit22 0x00200000
#define bit23 0x00400000
#define bit24 0x00800000
#define bit25 0x01000000
#define bit26 0x02000000
#define bit27 0x04000000
#define bit28 0x08000000
#define bit29 0x10000000
#define bit30 0x20000000
#define bit31 0x40000000
#define bit32 0x80000000

struct EDITDATA;
struct LevelObject;

typedef short OINUM;
typedef short HFII;

// Definitions for extensions
#define TYPE_LONG	0x0000
#define TYPE_INT	TYPE_LONG
#define TYPE_STRING	bit1
#define TYPE_FLOAT	bit2				// Pour les extensions
#define TYPE_DOUBLE bit2

// ------------------------------------------------------
// EXTENSION EDITION HEADER STRUCTURE
// ------------------------------------------------------
struct extHeader_v1
{
	short extSize,
		  extMaxSize,
		  extOldFlags,		// For conversion purpose
		  extVersion;		// Version number
};
//typedef extHeader_v1*	extHeader *V1;

struct extHeader
{
	unsigned long extSize,
				  extMaxSize,
				  extVersion;			// Version number
	void *		  extID;				// object's identifier
	void *		  extPrivateData;		// private data
};

// Callback function identifiers for CallFunction
enum class CallFunctionIDs {
	// Editor only
	INSERTPROPS = 1,		// Insert properties into Property window
	REMOVEPROP,				// Remove property
	REMOVEPROPS,			// Remove properties
	REFRESHPROP,			// Refresh propery
	REALLOCEDITDATA,		// Reallocate edPtr
	GETPROPVALUE,			// Get object's property value
	GETAPPPROPVALUE,		// Get application's property value
	GETFRAMEPROPVALUE,		// Get frame's property value
	SETPROPVALUE,			// Set object's property value
	SETAPPPROPVALUE,		// Set application's property value
	SETFRAMEPROPVALUE,		// Set frame's property value
	GETPROPCHECK,			// Get object's property check state
	GETAPPPROPCHECK,		// Get application's property check state
	GETFRAMEPROPCHECK,		// Get frame's property check state
	SETPROPCHECK,			// Set object's property check state
	SETAPPPROPCHECK,		// Set application's property check state
	SETFRAMEPROPCHECK,		// Set frame's property check state
	INVALIDATEOBJECT,		// Refresh object in frame editor
	RECALCLAYOUT,			// Recalc runtime layout (docking)
	GETNITEMS,				// Get number of items - not yet implemented
	GETNEXTITEM,			// Get next item - not yet implemented
	GETNINSTANCES,			// Get number of item instances - not yet implemented
	GETNEXTINSTANCE,		// Get next item instance - not yet implemented

	// Editor & runtime
	MALLOC = 100,			// Allocate memory
	CALLOC,					// Allocate memory & set it to 0
	REALLOC,				// Re-allocate memory
	FREE,					// Free memory
	GETSOUNDMGR,			// Get sound manager
	CLOSESOUNDMGR,			// Close sound manager
	ENTERMODALLOOP,			// Reserved
	EXITMODALLOOP,			// Reserved
	CREATEEFFECT,			// Create effect (runtime only)
	DELETEEFFECT,			// Delete effect (runtime only)
	CREATEIMAGEFROMFILEA,	// Create image from file (runtime only)
	NEEDBACKGROUNDACCESS,	// HWA : tell the frame the frame surface can be read (runtime only)
	ISHWA,					// Returns TRUE if HWA version (editor and runtime)
	ISUNICODE,				// Returns TRUE if the editor or runtime is in Unicode mode
	ISUNICODEAPP,			// Returns TRUE if the application being loaded is a Unicode application
	GETAPPCODEPAGE,			// Returns the code page of the application
	CREATEIMAGEFROMFILEW,	// Create image from file (runtime only)
};



// ------------------------------------------------------------
// EXTENSION OBJECT DATA ZONE
// ------------------------------------------------------------

// Flags 
typedef unsigned int uint;
enum class OEFLAGS : uint {
	NONE = 0,
	DISPLAY_IN_FRONT = bit1,		// Active object/window control
	BACKGROUND = bit2,		// Background
	BACK_SAVE = bit3,		// No effect in HWA
	RUN_BEFORE_FADE_IN = bit4,
	MOVEMENTS = bit5,
	ANIMATIONS = bit6,
	TAB_STOP = bit7,
	WINDOW_PROC = bit8,		// Needs to receive window process messages (i.e. app was minimized)
	VALUES = bit9,		// Has alterable values/strings (will automatically create the associated a/c/e/p)
	SPRITES = bit10,
	INTERNAL_BACK_SAVE = bit11,	// No effect in HWA
	SCROLLING_INDEPENDENT = bit12,
	QUICK_DISPLAY = bit13,	// No effect in HWA
	NEVER_KILL = bit14,	// Never destroy object if too far from frame
	NEVER_SLEEP = bit15,
	MANUAL_SLEEP = bit16,
	TEXT = 0x10000,
	DONT_CREATE_AT_START = 0x20000,
};
fancyenumop(OEFLAGS);

// Flags modifiable by the program
enum class OEPREFS : short {
	NONE = 0,
	BACK_SAVE = bit1,		// No effect in HWA
	SCROLLING_INDEPENDENT = bit2,
	QUICK_DISPLAY = bit3,		// No effect in HWA
	SLEEP = bit4,
	LOAD_ON_CALL = bit5,
	GLOBAL = bit6,
	BACK_EFFECTS = bit7,
	KILL = bit8,
	INK_EFFECTS = bit9,
	TRANSITIONS = bit10,
	FINE_COLLISIONS = bit11,
	APPLET_PROBLEMS = bit12,
};
fancyenumop(OEPREFS);

// Running flags
enum class REFLAG : short {
	NONE = 0,
	ONE_SHOT = bit1,
	DISPLAY = bit2,
	MSG_HANDLED = bit3,
	MSG_CATCHED = bit4,
	MSG_DEF_PROC = bit5,
	// ?
	MSGRETURNVALUE = bit7,
};

#define IsRunTimeFunctionPresent(num)	(num < KPX_MAXFUNCTIONS && hoPtr->AdRunHeader->rh4.rh4KpxFunctions[(int)num].routine != NULL)
#define	CallRunTimeFunction(hoPtr,num,wParam,lParam)	0L/*(hoPtr->AdRunHeader->rh4.rh4KpxFunctions[(int)num].routine(hoPtr, wParam, lParam) )*/
#define	CallRunTimeFunction2(hoPtr,num,wParam,lParam)	0L/*(hoPtr->AdRunHeader->rh4.rh4KpxFunctions[(int)num].routine(hoPtr, wParam, lParam) )*/
#define	CallRunTimeFunction3(rh4_,num,wParam,lParam)	0L/*(rh4_.rh4KpxFunctions[num].routine(hoPtr, wParam, lParam) )*/

enum class RFUNCTION {
	//
	REHANDLE,				// Re-enable the call to Handle() every frame
	GENERATE_EVENT,			// Immediately create a triggered condition (do not call in functions that MMF hasn't began the call to, i.e. threads)
	PUSH_EVENT,				// Generates the event after the next event loop, safer for calling from threads and such.
	GET_STRING_SPACE_EX,	// Allocates memory from MMF for a char * or wchar_t *, letting you store a string.
	GET_PARAM_1,			// Retrieves the value of the first parameter of an a/c/e.
	GET_PARAM_2,			// Retrieves the value of 2nd+ parameter (first call with this is 2nd, next 3rd, etc).
	PUSH_EVENT_STOP,		// Removes event created by Push Event. Cannot do this with Generate Event as it happens immediately.
	PAUSE,
	CONTINUE,
	REDISPLAY,
	GET_FILE_INFOS,			// Retrieve information about the current app (see FILEINFOS namespace and Edif.Runtime.cpp)
	SUB_CLASS_WINDOW,
	REDRAW,					// Causes the object to redraw [a certain part or 100%?].
	DESTROY,
	GET_STRING_SPACE,		// Deprecated GET_STRING_SPACE_EX
	EXECUTE_PROGRAM,
	GET_OBJECT_ADDRESS,
	GET_PARAM,
	GET_EXP_PARAM,
	GET_PARAM_FLOAT,
	EDIT_INT,
	EDIT_TEXT,
	CALL_MOVEMENT,
	SET_POSITION,
	GET_CALL_TABLES
};
// For GetFileInfos
enum class FILEINFO {
	DRIVE = 1,
	DIR,
	PATH,
	APP_NAME,
	TEMP_PATH
};
class CImageFilterMgr;
struct ACEParamReader {
	virtual float GetFloat(int i) = 0;
	virtual const TCHAR * GetString(int i) = 0;
	virtual std::int32_t GetInteger(int i) = 0;
};

#define CNC_GetParameter(hoPtr)							{ }
#define CNC_GetIntParameter(hoPtr)						CallRunTimeFunction(hoPtr, RFUNCTION::GET_PARAM, 0, 0)
#define CNC_GetStringParameter(hoPtr)					CallRunTimeFunction(hoPtr, RFUNCTION::GET_PARAM, 0xFFFFFFFF, 0)
#define CNC_GetFloatValue(hoPtr, par)					CallRunTimeFunction(hoPtr, RFUNCTION::GET_PARAM_FLOAT, par, 0)

///////////////////////////////////////////////////////////////////////
//
// DEFINITION OF THE DIFFERENT PARAMETERS
//
///////////////////////////////////////////////////////////////////////

enum class Params : short {
	Object = 1,						// ParamObject
	Time,							// ParamTime
	Border,							// ParamBorder
	Direction,						// ParamDir
	Integer,						// ParamInt
	Sample,							// ParamSound
	Music,							// ParamSound also
	Position,						// ParamPosition
	Create,							// ParamCreate
	Animation,						// ParamAnimation
	NoP,							// No struct (no data)
	Player,							// ParamPlayer
	Every,							// ParamEvery
	Key,							// ParamKey
	Speed,							// ParamSpeed
	NewPosition,					// ParamNewPosition
	Joystick_Direction, 			// ParamNewPosition also
	Shoot,							// ParamShoot
	Playfield_Zone,					// ParamZone
	System_Create,					// ParamCreate
	Expression = 22,				// ParamExpression
	Comparison,						// ParamComparison
	Colour,							// ParamColour
	Buffer,							// ParamBuffer
	Frame,							// ParamFrame - Storyboard frame number 
	Sample_Loop,					// ParamSoundLoop
	Music_Loop,						// ParamSoundLoop also
	New_Direction,					// ParamNewDir
	Text_Number,					// ParamTextNum
	Click,							// ParamClick
	Program = 33,					// ParamProgram
	Old_GlobalVariable, 			// DEPRECATED - DO NOT USE
	Condition_Sample,				// ParamCondSound
	Condition_Music,				// ParamCondSound also
	Editor_Comment,					// ParamEditorComment - Event editor comment
	Group,							// ParamGroup
	Group_Pointer,					// ParamGroupPointer
	Filename,						// ParamFilename
	String,							// ParamString
	Compare_Time,					// ParamCmpTime
	Paste_Sprite,					// ParamPasteSprite
	Virtual_Key_Code,				// ParamVKCode
	String_Expression,				// ParamStringExp
	String_Comparison,				// ParamStringExp also
	Ink_Effect,						// ParamInkEffect
	Menu,							// ParamMenu
	Global_Variable,				// ParamVariable
	Alterable_Value,				// ParamVariable also
	Flag,							// ParamVariable also also!
	Global_Variable_Expression, 	// ParamExpression
	Alterable_Value_Expression,		// ParamExpression also
	Flag_Expression,				// ParamExpression also also!
	Extension,						// ParamExtension
	_8Dirs,							// Param8Dirs
	Movement,						// ParamMvt
	Global_String,					// ParamVariable
	Global_String_Expression,		// ParamExpression also also also!!
	Program_2,						// ParamProgram2
	Alterable_String,				// ParamVariable
	Alterable_String_Expression,	// ParamExpression
	Filename_2,						// ParamFilename2 - allows filters of extensions or something.
	Effect,							// ParamEffect - HWA effect?
	Custom_Base = 1000,				// Base number for custom returns
};
struct ParamObject {
	unsigned short	OffsetListOI,	//
					Number,			//
					Type;			// Version > FVERSION_NEWOBJECTS
};

enum class ExpParams : short {
	Long = 1,
	Integer = Long,
	Float = Integer,
	UnsignedInteger = Integer,
	GlobalVariable,
	String,
	AlterableValue,
	Flag,
};
enum class ExpReturns {
	Long = 0,
	Integer = Long,
	Float,
	String,
};

#define	CND_SIZE					sizeof(event2)
#define	ACT_SIZE					(sizeof(event2)-2) // Ignore Identifier

// Definition of conditions / actions flags
enum class EVFLAGS : short {
	NONE = 0,
	REPEAT = bit1,
	DONE = bit2,
	DEFAULT = bit3,
	DONEBEFOREFADEIN = bit4,
	NOTDONEINSTART = bit5,
	ALWAYS = bit6,
	BAD = bit7,
	BADOBJECT = bit8,
	DEFAULTMASK = (ALWAYS + REPEAT + DEFAULT + DONEBEFOREFADEIN + NOTDONEINSTART),
	// Originally EVFLAGS_NOTABLE
	NOTABLE = bit10
};
fancyenumop(EVFLAGS);

// Expression return type. Originally EXPFLAG_*, expression flags.
enum class ExpReturnType : short {
	Integer = 0,
	String = bit1,
	// enum item originally named DOUBLE, but this is misleading; a float is returned.
	Float = bit2,
	UnsignedInteger = Integer
};

Params ReadParameterType(const char *, bool &);
ExpParams ReadExpressionParameterType(const char *, bool &);
ExpReturnType ReadExpressionReturnType(const char * text);

struct runHeader2 {
	short EventCount;
};
struct runHeader3 {
};
struct runHeader4;
struct HeaderObject;
struct objectsList {
	HeaderObject  *	oblOffset;
};
struct objInfoList {
	short			Oi,  			 // THE ObjectInfo number
					ListSelected,	 // First selection
					Type,			 // Type of the object
					Object;			 // First objects in the game
	short			EventCount,
					NumOfSelected;
	objectsList *	ObjectList;			// Object list address

	int				NObjects,		 // Current number
					ActionCount,	 // Action loop counter
					ActionLoopCount; // Action loop counter
};
struct qualToOi {
	short OiList;
};

enum class HeaderObjectFlags : ushort {
	Destroyed = bit1,
	TrueEvent = bit2,
	RealSprite = bit3,
	FadeIn = bit4,
	FadeOut = bit5,
	OwnerDraw = bit6,
	NoCollision = bit14,
	Float = bit15,
	String = bit16
};
fancyenumop(HeaderObjectFlags);
struct RunHeader {
	unsigned short GetEventCount();
};

struct CreateObjectInfo {

};
typedef void * CCndExtension;
typedef void * CActExtension;
typedef void * CNativeExpInstance;

struct HeaderObject {

	RunHeader *			AdRunHeader;	// Run-header address
	short  				Number,			// Number of the object
						NextSelected;	// Selected object list. Do not move from &NextSelected == (this+2).
	short				EventNumber;
	HeaderObjectFlags	Flags;
	short				HFII,			// Number of LevObj
						Oi,				// Number of ObjInfo
						NumPrev,		// Same ObjInfo previous object
						NumNext,		// ... next
						Type;			// Type of the object
	unsigned short		CreationId;		// Number of creation
	objInfoList *		OiList;			// Pointer to OILIST information

	qualToOi *			QualToOiList;
};
struct RunObject {
	HeaderObject  	roHo;		  		// Common structure 
};

// Versions
#define MMFVERSION_MASK		0xFFFF0000
#define MMFBUILD_MASK		0x00000FFF		// MMF build
#define MMFVERFLAG_MASK		0x0000F000
#define MMFVERFLAG_HOME		bit16		// TGF
#define MMFVERFLAG_PRO		bit15		// MMF Pro
#define MMFVERFLAG_DEMO		bit14		// Demo
#define MMFVERFLAG_PLUGIN	bit13		// Plugin
#define MMFVERSION_15		0x01050000		// MMF 1.5
#define MMFVERSION_20		0x02000000		// MMF 2.0
#define	MMF_CURRENTVERSION	MMFVERSION_20

// WARNING: Android has a complete mismatch with actual SDK mV.
struct mv {
	//void * ReAllocEditData(EDITDATA * edPTr, unsigned int dwNewSize);
	//void InvalidateObject();
};
/*
__inline void * mvReAllocEditData(mv * mV, EDITDATA * edPtr, unsigned int dwNewSize) {
	return mV->ReAllocEditData(edPtr, dwNewSize);
}*/

struct RuntimeFunctions
{
	void * ext;

	struct string
	{
		void * ctx;
		const char * ptr;
	};
};


const int REFLAG_DISPLAY = 1;
const int REFLAG_ONESHOT = 2;


void LOGF(const char * x, ...);


// Defined in DarkEdif.cpp with ASM instructions to embed the binary.
//extern char darkExtJSON[];
//extern unsigned int darkExtJSONSize;
#define PROJ_FUNC_GEN2(x,y) x##y
#define PROJ_FUNC_GEN(x,y) PROJ_FUNC_GEN2(x,y)

ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _init());
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _close());
ProjectFunc int PROJ_FUNC_GEN(PROJECT_NAME_RAW,_getNumberOfConditions());
ProjectFunc long PROJ_FUNC_GEN(PROJECT_NAME_RAW,_condition(void * cppExtPtr, int ID, void * paramReader));
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW,_action(void * cppExtPtr, int ID, void * paramReader));
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW,_expression(void * cppExtPtr, int ID));
ProjectFunc void * PROJ_FUNC_GEN(PROJECT_NAME_RAW,_createRunObject(void * file, int cob, int version, void * objCExtPtr));
ProjectFunc short PROJ_FUNC_GEN(PROJECT_NAME_RAW,_handleRunObject(void * cppExtPtr));
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW,_destroyRunObject(void * cppExtPtr, bool bFast));

// Stolen from IncBin GitHub: https://github.com/graphitemaster/incbin
// Edited to remove g prefix.
#ifndef INCBIN_HDR
#define INCBIN_HDR
#include <limits.h>

#if defined(__SSE__) || defined(__neon__)
# define INCBIN_ALIGNMENT 16
#else
# if ULONG_MAX == 0xffffffffu
#  define INCBIN_ALIGNMENT 4
# else
#  define INCBIN_ALIGNMENT 8
# endif
#endif

#define INCBIN_ALIGN __attribute__((aligned(INCBIN_ALIGNMENT)))

#ifdef __cplusplus
#  define INCBIN_EXTERNAL extern "C"
#else
#  define INCBIN_EXTERNAL extern
#endif

#define INCBIN_STR(X) #X
#define INCBIN_STRINGIZE(X) INCBIN_STR(X)

#define INCBIN_EXTERN(NAME) \
    INCBIN_EXTERNAL const INCBIN_ALIGN unsigned char PROJECT_NAME_RAW ## NAME[]; \
    INCBIN_EXTERNAL const unsigned int PROJECT_NAME_RAW ## NAME ## Size

#define PROJECT_NAME_RAW_STR #PROJECT_NAME_RAW 
#define INCBIN(NAME, FILENAME) INCBIN2(PROJECT_NAME_RAW ## NAME, FILENAME)

#define INCBIN2(NAME, FILENAME) \
    __asm__(".const_data\n" \
            ".globl _" #NAME "\n"      \
            ".align " INCBIN_STRINGIZE(INCBIN_ALIGNMENT) "\n" \
            "_" #NAME ":\n" \
                ".incbin \"" FILENAME "\"\n" \
            ".globl _" #NAME "Size\n"      \
            ".align " INCBIN_STRINGIZE(INCBIN_ALIGNMENT) "\n" \
            "_" #NAME "Size:\n" \
            ".long _" #NAME "Size - _" #NAME "\n" \
    ); \
    INCBIN_EXTERN(NAME)
INCBIN_EXTERN(darkExtJSON);
#endif

#pragma clang diagnostic pop
