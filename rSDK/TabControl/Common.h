// Include guard
#ifndef COMMON_H
#define COMMON_H

// Suppress the deprecated warnings for VC2005
#define _CRT_SECURE_NO_WARNINGS

#define __ID_TIMES_TEN__

//This defines tchar functions, which will evaluate
//to wchar in Unicode, and char in ASCII.
#include	<tchar.h>

// General includes
#include	"TemplateInc.h"
#include	<commctrl.h>

// Specific to this extension
#include	"Resource.h"
#include	"FlagsPrefs.h"
#include	"Information.h"
#include	"Data.h"

// rTemplate include
#include	"rTemplate.h"
#include	"ImageFlt.h"
#include	"ImgFlt.h"
#include	"CfcFile.h"

// Globals and prototypes
extern HINSTANCE hInstLib;
extern short * conditionsInfos;
extern short * actionsInfos;
extern short * expressionsInfos;
extern long (WINAPI ** ConditionJumps)(LPRDATA rdPtr, long param1, long param2);
extern short (WINAPI ** ActionJumps)(LPRDATA rdPtr, long param1, long param2);
extern long (WINAPI ** ExpressionJumps)(LPRDATA rdPtr, long param);
extern PropData Properties[];
extern WORD DebugTree[];

// End include guard
#endif

