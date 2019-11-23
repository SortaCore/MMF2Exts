// Include guard
#ifndef COMMON_H
#define COMMON_H

// General includes
#include <tchar.h>
#include "TemplateInc.h"

// Specific to this extension
#include "Resource.h"
#include "FlagsPrefs.h"
#include "Information.h"
#include "Data.h"

// rTemplate include
#include "rTemplate.h"

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
