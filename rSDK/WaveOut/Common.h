// Include guard
#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#pragma comment(lib, "winmm.lib")

#include <tchar.h>

// Suppress the deprecated warnings for VC2005
#define _CRT_SECURE_NO_WARNINGS

/* 3 buffers should be enough: One current buffer, one queued buffer, and a back-up queue. */
#define NUM_BUFFERS 3

// General includes
#include	"TemplateInc.h"

// Specific to this extension
#include	"Resource.h"
#include	"FlagsPrefs.h"
#include	"Information.h"
#include	"Data.h"

// rTemplate include
#include	"rTemplate.h"

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
