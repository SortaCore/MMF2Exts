
#ifndef COMMON_H
#define COMMON_H

#if defined(_DEBUG) && defined(_WIN32)
	#define	_CRTDBG_MAP_ALLOC	1
#endif

// ----------------------------------------------
// VERSION AND BUILD YOUR EXTENSION CAN WORK WITH
// ----------------------------------------------
//#define TGFEXT	// TGF, MMF Standard and MMF Pro
#define	MMFEXT		// MMF Standard and MMF Pro
//#define PROEXT	// MMF Pro only

// Build number of the minimum required version of MMF
#define	MINBUILD	228

// General includes
#include "..\Inc\ccxhdr.h"
#include "..\Inc\Surface.h"

// Specific to this cox
#include "resource.h"
#include "main.h"

// Globals and Prototypes
extern HINSTANCE hInstLib;
extern short conditionsInfos[];
extern short actionsInfos[];
extern short expressionsInfos[];

long (WINAPI * ConditionJumps[])(LPRDATA rdPtr, long param1, long param2);
short (WINAPI * ActionJumps[])(LPRDATA rdPtr, long param1, long param2);
long (WINAPI * ExpressionJumps[])(LPRDATA rdPtr, long param);

#define MAKEID(a,b,c,d) ((#@a << 24)|(#@b << 16)|(#@c << 8)|(#@d))

// Used to ensure the MMF version is 1.5, you can safely ignore this
/*
#if defined(MMFEXT)
	#ifdef DEMO
		#define	IS_COMPATIBLE(v) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFBUILD_MASK) >= MINBUILD && (v->mvGetVersion() & MMFVERSION_MASK) >= MMFVERSION_20 && ((v->mvGetVersion() & MMFVERFLAG_MASK) & MMFVERFLAG_HOME) == 0 && (v->mvGetVersion() & MMFVERFLAG_DEMO) != 0)
	#else
		#define	IS_COMPATIBLE(v) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFBUILD_MASK) >= MINBUILD && (v->mvGetVersion() & MMFVERSION_MASK) >= MMFVERSION_20 && ((v->mvGetVersion() & MMFVERFLAG_MASK) & MMFVERFLAG_HOME) == 0)
	#endif
#elif defined(PROEXT)
	#ifdef DEMO
		#define IS_COMPATIBLE(v) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFBUILD_MASK) >= MINBUILD && (v->mvGetVersion() & MMFVERSION_MASK) >= MMFVERSION_20 && ((v->mvGetVersion() & MMFVERFLAG_MASK) & MMFVERFLAG_PRO) != 0 && (v->mvGetVersion() & MMFVERFLAG_DEMO) != 0)
	#else
		#define IS_COMPATIBLE(v) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFBUILD_MASK) >= MINBUILD && (v->mvGetVersion() & MMFVERSION_MASK) >= MMFVERSION_20 && ((v->mvGetVersion() & MMFVERFLAG_MASK) & MMFVERFLAG_PRO) != 0)
	#endif
#else
	#ifdef DEMO
		#define	IS_COMPATIBLE(v) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFBUILD_MASK) >= MINBUILD && (v->mvGetVersion() & MMFVERSION_MASK) >= MMFVERSION_20 && (v->mvGetVersion() & MMFVERFLAG_DEMO) != 0)
	#else
		#define	IS_COMPATIBLE(v) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFBUILD_MASK) >= MINBUILD && (v->mvGetVersion() & MMFVERSION_MASK) >= MMFVERSION_20)
	#endif
#endif
*/

#endif
