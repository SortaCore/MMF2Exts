#pragma once

#if defined(_WIN32) || defined (__ANDROID__)
	#error Included the wrong header for this OS.
#endif

// Cover up clang warnings about unused features we make available
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wcomment"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-variable"

#include "AllPlatformDefines.hpp"
#include "NonWindowsDefines.hpp"

#define ProjectFunc extern "C" __attribute__((visibility ("default")))
#define FusionAPI /* no declarator */
#define DLLExport __attribute__ ((stdcall))
#define SuppressZeroArraySizeWarning /* no op */
#define PhiDLLImport

// Surface.h pre-declaration
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



// ------------------------------------------------------------
// EXTENSION OBJECT DATA ZONE
// ------------------------------------------------------------

///////////////////////////////////////////////////////////////////////
//
// DEFINITION OF THE DIFFERENT PARAMETERS
//
///////////////////////////////////////////////////////////////////////

struct ParamObject {
	unsigned short	OffsetListOI,	//
					Number,			//
					Type;			// Version > FVERSION_NEWOBJECTS
};
struct RuntimeFunctions
{
	void* ext;

	struct string
	{
		void* ctx;
		const char* ptr;
	};
};

Params ReadActionOrConditionParameterType(const char *, bool &);
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

struct RunHeader {
	unsigned short GetEventCount();
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

// Defined in DarkEdif.cpp with ASM instructions to embed the binary.
//extern char darkExtJSON[];
//extern unsigned int darkExtJSONSize;
#define PROJ_FUNC_GEN2(x,y) x##y
#define PROJ_FUNC_GEN(x,y) PROJ_FUNC_GEN2(x,y)

ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _init());
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _dealloc());
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

// Undo warnings
#pragma clang diagnostic pop
