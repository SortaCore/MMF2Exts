// Include guard
#ifndef COMMON_H
#define COMMON_H

// General includes
#include "..\Inc\TemplateInc.h"

// Other includes
#define MAX_SLOTS 32
#include	<string>
#include	<tchar.h>

#include <mmsystem.h> // For time functions
#pragma comment(lib, "winmm.lib")
#include	<wininet.h>

// Specific to this extension
#include "Resource.h"
#include "FlagsPrefs.h"
#include "Information.h"
#include "Data.h"

// rTemplate include
#include "..\Inc\rTemplate.h"

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
