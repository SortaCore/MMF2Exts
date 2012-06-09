// ============================================================================
//
// This file are where the Conditions/Actions/Expressions are defined.
// You can manually enter these, or use CICK (recommended)
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
{0};

// Definitions of parameters for each action
short actionsInfos[]=
{0};

// Definitions of parameters for each expression
short expressionsInfos[]=
	{
		IDMN_EXPRESSION2, M_EXPRESSION2, EXP_EXPRESSION2, EXPFLAG_STRING, 1, EXPPARAM_STRING, 0,
	};



// ============================================================================
//
// CONDITION ROUTINES
// 
// ============================================================================




// ============================================================================
//
// EXPRESSIONS ROUTINES
// 
// ============================================================================

long WINAPI DLLExport GetVol(LPRDATA rdPtr,long param1)
{
	long p1 = CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_STRING);
	//Reversing the string.
	LPSTR buf = (LPSTR)callRunTimeFunction(rdPtr, RFUNCTION_GETSTRINGSPACE, MAX_PATH , 0);
	buf[0] = '\0';
	DWORD w1,w2,w3;
	char ignore[10];
	//None of the info is null because sometimes that crashes Win9x machines.
	GetVolumeInformation( (LPSTR)p1 , buf , MAX_PATH , &w1 , &w2 , &w3 , ignore , 9 );
	
	rdPtr->rHo.hoFlags |= HOF_STRING;
	return (long)buf;
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
			0
			};
	
short (WINAPI * ActionJumps[])(LPRDATA rdPtr, long param1, long param2) =
			{
			0
			};

long (WINAPI * ExpressionJumps[])(LPRDATA rdPtr, long param) = 
			{     
			GetVol,
			0
			};