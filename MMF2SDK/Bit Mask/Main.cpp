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
		{
		IDMN_CONDITION, M_CONDITION, CND_CONDITION, EVFLAGS_ALWAYS, 3, PARAM_EXPRESSION, PARAM_EXPRESSION, PARAM_EXPRESSION, M_CND_P1, M_CND_P2, M_CND_P3,
		};

// Definitions of parameters for each action
short actionsInfos[]=
		{
		IDMN_ACTION, M_ACTION,	ACT_ACTION,	0, 0,
		};

// Definitions of parameters for each expression
short expressionsInfos[]=
		{
		IDMN_EXPRESSION, M_EXPRESSION, 0, 0, 3, EXPPARAM_LONG, EXPPARAM_LONG, EXPPARAM_STRING, 0, 0, 0,
		
		//Note in the following.  If you are returning a string, you set the EXPFLAG_STRING.	
		IDMN_EXPRESSION2, M_EXPRESSION2, 1, 0 ,2, EXPPARAM_LONG, EXPPARAM_STRING, 0,0,

		ID__GETNTHBIT, IDS_STRING7002, 2, 0, 2, EXPPARAM_LONG,EXPPARAM_LONG,0,0,
		

		};



// ============================================================================
//
// The main code
// 
// ============================================================================

unsigned int BitMask( unsigned int first , unsigned int second , char* mask )
{
    int out = 0;
    unsigned int bitMask = 1;
    
    int len = strlen(mask);
    if ( len > 32 ) len = 32;
    
    for ( int i = 0 ; i < len ; i++ )
    {
        bool bit = 0;
        bool a_bit = !!(first & bitMask);
        bool b_bit = !!(second & bitMask);
        switch (mask[i])
        {
               case '0':
                    bit = 0;
                    break;
               case '1':
                    bit = 1;
                    break;
               case '&':
                    bit = a_bit && b_bit;
                    break;
               case '|':
               case '-':
                    bit = a_bit || b_bit;
                    break;
               case '^':
                    bit = a_bit ^ b_bit;
                    break;
               case 'f':
			   case 'F':
                    bit = a_bit;
                    break;
               case 'l':
			   case 'L':
                    bit = b_bit;
                    break;
               case 't':
			   case 'T':
                   bit = !a_bit;
                    break;
                    default:
                            bit = 0;
                            break;
        }
        out |= bitMask*bit;
        bitMask *= 2;
    }
    return out;
}

unsigned int SingleMask( unsigned int num , char* mask )
{
    return BitMask(num,0,mask);  
}

bool GetNthBit( unsigned int num , int bit )
{
    return !!(num & (1 << (bit)));
}


// ============================================================================
//
// EXPRESSIONS ROUTINES
// 
// ============================================================================

long WINAPI DLLExport Expression(LPRDATA rdPtr,long param1)
{

	long p1 = CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	long p2 = CNC_GetNextExpressionParameter(rdPtr, param1, TYPE_INT);
	char* p3 = (char*)CNC_GetNextExpressionParameter(rdPtr, param1, TYPE_STRING);

	// Performs the wonderfull calculation
	return BitMask(p1,p2,p3);
}



long WINAPI DLLExport Expression2(LPRDATA rdPtr,long param1)
{

	long p1 = CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	char* p3 = (char*)CNC_GetNextExpressionParameter(rdPtr, param1, TYPE_STRING);

	return SingleMask(p1,p3);
}


long WINAPI DLLExport Expression3(LPRDATA rdPtr,long param1)
{
	long p1 = CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	long p2 = CNC_GetNextExpressionParameter(rdPtr, param1, TYPE_INT);

	return GetNthBit(p1,p2);

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
			Expression,
			Expression2,
			Expression3,
			0
			};