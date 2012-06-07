// ============================================================================
//
// This file are where the Conditions/Actions/Expressions are defined.
// You can manually enter these, or use CICK
// See the Extension FAQ in this SDK for more info and where to download it
//
// ============================================================================

// Common Include
#include	"common.h"

// Quick memo: content of the eventInformations arrays
// ---------------------------------------------------
// Menu ID
// String ID
// Code
// Flags
// Number_of_parameters
// Parameter_type [Number_of_parameters]
// Parameter_TitleString [Number_of_parameters]

// Definitions of parameters for each condition
short conditionsInfos[]=
		{
		IDMN_CND_ISEMPTY, M_CND_ISEMPTY, CND_ISEMPTY, EVFLAGS_ALWAYS | EVFLAGS_NOTABLE, 0,	// normal condition
		IDMN_CND_CHANGED, M_CND_CHANGED, CND_CHANGED, 0, 0,									// event, triggered only when the event occurs
		0, 0, 0, 0, 0
		};

// Definitions of parameters for each action
short actionsInfos[]=
		{
		IDMN_ACT_SETTEXT,	M_ACT_SETTEXT,	ACT_SETTEXT,	0, 1, PARAM_EXPSTRING, IDS_SETTEXT_TITLE,
		IDMN_ACT_SETINT,	M_ACT_SETINT,	ACT_SETINT,		0, 1, PARAM_EXPRESSION, IDS_SETINT_TITLE,
		IDMN_ACT_SETFLOAT,	M_ACT_SETFLOAT,	ACT_SETFLOAT,	0, 1, PARAM_EXPRESSION, IDS_SETFLOAT_TITLE,
		};

// Definitions of parameters for each expression
short expressionsInfos[]=
		{
		IDMN_EXP_GETTEXT, M_EXP_GETTEXT, EXP_GETTEXT, EXPFLAG_STRING, 0,
		IDMN_EXP_GETINT, M_EXP_GETINT, EXP_GETINT, 0, 0,
		IDMN_EXP_GETFLOAT, M_EXP_GETFLOAT, EXP_GETFLOAT, 0, 0,
		};



// ============================================================================
//
// CONDITION ROUTINES
// 
// ============================================================================


// Example of normal condition
long WINAPI DLLExport CndIsEmpty(LPRDATA rdPtr, long param1, long param2)
{
	if ( rdPtr->hWnd != NULL )
		return (GetWindowTextLength(rdPtr->hWnd) == 0);
	return FALSE;
}

// Example of "event" condition
long WINAPI DLLExport CndChanged(LPRDATA rdPtr, long param1, long param2)
{
	// Always TRUE if first condition
	if (rdPtr->rHo.hoFlags & HOF_TRUEEVENT)
		return TRUE;

	// Otherwise true only if same loop number
    LPRH rhPtr = rdPtr->rHo.hoAdRunHeader;
	if ( rhPtr->rh4.rh4EventCount == rdPtr->dwLastChangedLoopNumber )
		return TRUE;

	return FALSE;
}



// ============================================================================
//
// ACTIONS ROUTINES
// 
// ============================================================================

// -----------------
// Set Text
// -----------------
// 
short WINAPI DLLExport ActSetText(LPRDATA rdPtr, long param1, long param2)
{
	// Parameter
	LPTSTR pParamStr = (LPTSTR)param1;

	if ( rdPtr->hWnd != NULL && pParamStr != NULL )
		SetWindowText(rdPtr->hWnd, pParamStr);

	return 0;
}

// -----------------
// Set Integer Value
// -----------------
// 
short WINAPI DLLExport ActSetInt(LPRDATA rdPtr, long param1, long param2)
{
	if ( rdPtr->hWnd != NULL )
	{
		TCHAR str[16];
		wsprintf(str, _T("%d"), param1);
		SetWindowText(rdPtr->hWnd, str);
	}

	return 0;
}

// -----------------
// Set Float Value
// -----------------
// 
short WINAPI DLLExport ActSetFloat(LPRDATA rdPtr, long param1, long param2)
{
	if ( rdPtr->hWnd != NULL )
	{
		long tmpf = CNC_GetFloatValue(rdPtr,0);
		float fValue = *((float *)&tmpf);

		TCHAR str[64];
		_stprintf_s(str, 64, _T("%g"), (double)fValue);

		SetWindowText(rdPtr->hWnd, str);
	}

	return 0;
}


// ============================================================================
//
// EXPRESSIONS ROUTINES
// 
// ============================================================================

// -----------------
// Get Text
// -----------------

long WINAPI DLLExport ExpGetText(LPRDATA rdPtr,long param1)
{
	LPTSTR pText = _T("");		// Default = empty string (note: string expressions must never return 0)

	if ( rdPtr->hWnd != NULL )
	{
		int nTextLength = GetWindowTextLength(rdPtr->hWnd);
		if ( nTextLength != 0 )
		{
			// Tells MMF to allocate a temporary buffer to store the string
			// This temporary buffer will be freed as soon as MMF no longer needs the returned string
			pText = (LPTSTR)callRunTimeFunction(rdPtr, RFUNCTION_GETSTRINGSPACE_EX, 0, (nTextLength+1) * sizeof(TCHAR));
			if ( pText != NULL )
			{
				// Get text from control
				GetWindowText(rdPtr->hWnd, pText, nTextLength+1);
			}
		}
	}

	// Setting the HOF_STRING flag lets MMF know that you return a string.
	rdPtr->rHo.hoFlags |= HOF_STRING;

	return (long)pText;
}

// -----------------
// Get Int
// -----------------

long WINAPI DLLExport ExpGetInt(LPRDATA rdPtr,long param1)
{
	long lValue = 0;
	if ( rdPtr->hWnd != NULL )
	{
		int nTextLength = GetWindowTextLength(rdPtr->hWnd);
		if ( nTextLength != 0 )
		{
			LPTSTR pText = (LPTSTR)calloc(nTextLength+1, sizeof(TCHAR));
			if ( pText != NULL )
			{
				GetWindowText(rdPtr->hWnd, pText, nTextLength+1);
				lValue = _ttol(pText);
				free(pText);
			}
		}
	}

	return lValue;
}

// -----------------
// Get Float
// -----------------

long WINAPI DLLExport ExpGetFloat(LPRDATA rdPtr,long param1)
{
	float fValue = 0;
	if ( rdPtr->hWnd != NULL )
	{
		int nTextLength = GetWindowTextLength(rdPtr->hWnd);
		if ( nTextLength != 0 )
		{
			LPTSTR pText = (LPTSTR)calloc(nTextLength+1, sizeof(TCHAR));
			if ( pText != NULL )
			{
				GetWindowText(rdPtr->hWnd, pText, nTextLength+1);

				LPTSTR endptr;
				fValue = (float)_tcstod(pText, &endptr);
				free(pText);
			}
		}
	}

	// Setting the HOF_FLOAT flag lets MMF know that you return a float.
	rdPtr->rHo.hoFlags |= HOF_FLOAT; 

	// Return float as integer to avoid C automatic conversion from float to long
	long FloatAsInt;
	*((float*)&FloatAsInt) = fValue;
	return FloatAsInt;	
}


// ----------------------------------------------------------
// Condition / Action / Expression jump table
// ----------------------------------------------------------
// Contains the address inside the extension of the different
// routines that handle the action, conditions and expressions.
// Located at the end of the source for convinience
// Must finish with a 0
//
long (WINAPI * ConditionJumps[])(LPRDATA rdPtr, long param1, long param2) = 
			{ 
			CndIsEmpty,
			CndChanged,
			};
	
short (WINAPI * ActionJumps[])(LPRDATA rdPtr, long param1, long param2) =
			{
			ActSetText,
			ActSetInt,
			ActSetFloat,
			0
			};

long (WINAPI * ExpressionJumps[])(LPRDATA rdPtr, long param) = 
			{     
			ExpGetText,
			ExpGetInt,
			ExpGetFloat,
			0
		};
