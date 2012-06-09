#ifndef COMMON_H
#define COMMON_H

#if defined(_DEBUG) && defined(WIN32)
	#define	_CRTDBG_MAP_ALLOC	1
#endif


#if (0 == DEBUG)
#define	NDEBUG				1
#endif

#define	IN_EXT_VERSION2		1
#define	RUN_TIME


// ------------------------------
// IS YOUR EXTENSION MMF1.5 ONLY?
// ------------------------------
// Uncomment the following line if your extension can work with MMF 1.0/1.2 also
// Remember to modify Extobj32.def by uncommenting the last four lines if not MMF1.5 only
//#define MMF15EXT


// General includes
#include	"..\inc\ccxhdr.h"
#include	"..\inc\ccx.h"
#include	"..\inc\surface.h"

// Specific to this kcx
#include	"resource.h"
#include	"main.h"


//Globals and Prototypes
extern HINSTANCE hInstLib;
extern short conditionsInfos[];
extern short actionsInfos[];
extern short expressionsInfos[];

long (WINAPI * ConditionJumps[])(LPRDATA rdPtr, long param1, long param2);
short (WINAPI * ActionJumps[])(LPRDATA rdPtr, long param1, long param2);
long (WINAPI * ExpressionJumps[])(LPRDATA rdPtr, long param);

/* Used to ensure the MMF version is 1.5, you can safely ignore this */
#ifdef MMF15EXT
#define	IS_COMPATIBLE(v) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFVERSION_MASK) >= MMFVERSION_15)
#else
#define IS_COMPATIBLE(v) true
#endif


#endif
