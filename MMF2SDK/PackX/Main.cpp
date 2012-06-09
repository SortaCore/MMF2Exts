//Welcome to your auto-generated extension template!
//The first thing you should do is set the active build to debug in the projects menu
//Then build! If there are errors try and find out what caused them
//then check to see it is because of what you typed in in your .aces file

//Use the handy transfer.bat file in the debug directory of the SDK to transfer the cox files to MMF (Close down any projects using the extension first)
//You can get all sorts of crashes if your interface code is wrong so check everything thouroughly or pay later!
//Check each condition, action and expression can be placed and make sure each expression is valid by using the validify button in the expression editor

//If all goes well, happy making!
//What you still need to do :
//	Change the 3 extension build filenames (Project->Settings->V2Template32->Link)
//	Make your icons
//	Make your setup box
//	Fill out your extension info and author info in the string tables
//	Change your version info in the resource file
//	Change your IDENTIFIER, flags and prefrences in main.h
//	Change your runtime data and edittime data structures to suit your needs
//	Fill out all the required routines in Runtime.cpp and Edittime.cpp
//	Flesh out your actions, conditions and expressions
//	Write help files, examples and documentation!



// ============================================================================
//
// This file are where the Conditions/Actions/Expressions are defined.
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


short conditionsInfos[]=
	{ 
	CID_IsInRange,	CID_IsInRange,	0,	EVFLAGS_ALWAYS | EVFLAGS_NOTABLE,	1,	PARAM_EXPSTRING,	CP0ID_IsInRange,
	0 };

short actionsInfos[]=
	{ 
	AID_SetKey,	AID_SetKey,	0,	0,	1,	PARAM_EXPSTRING,	AP0ID_SetKey,
	0 };

short expressionsInfos[]=
	{ 
	EID_PackX,	EID_PackX,	0,	0,	1,	EXPPARAM_STRING,	0,
	EID_UnPackX,	EID_UnPackX,	1,	EXPFLAG_STRING,	1,	EXPPARAM_LONG,	0,
	EID_StringMaxLength,	EID_StringMaxLength,	2,	0,	0,
	EID_KeyCharCount,	EID_KeyCharCount,	3,	0,	0,
	0 };

int KeyCharCountInternal(char*);

//============================================================================
//
// CONDITION ROUTINES
// 
// ============================================================================


long WINAPI DLLExport IsInRange(LPRDATA rdPtr, long param1, long param2)
{
	char * string=(LPSTR)param1;
	char * key= rdPtr->key;

	//Find the max
	int chars = KeyCharCountInternal(key) + 1;
	long max = (long)(double)log( (double)pow(2,(double)(sizeof (long))*8) ) / log ((double) chars ) ;


	if ((strlen(string)-1) < max)
		return TRUE;

	return FALSE;
}



//============================================================================
//
// ACTIONS ROUTINES
// 
// ============================================================================



short WINAPI DLLExport SetKey(LPRDATA rdPtr, long param1, long param2)
{
	char * p1=(LPSTR)param1;
	strcpy_s(rdPtr->key, 100, p1);
	return 0;
}



//============================================================================
//
// EXPRESSIONS ROUTINES
// 
// ============================================================================


long WINAPI DLLExport PackX(LPRDATA rdPtr, long param1)
{
	char * string=(LPSTR)CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_STRING);
	char * key= rdPtr->key;
	
	//Find the max
	int chars = KeyCharCountInternal(key) + 1;
	long max = (long)(double)log( (double)pow(2,(double)(sizeof (long))*8) ) / log ((double) chars ) ;

	if (strnlen(string, max+1) > max)
	{
		string[max] = '\0';
	}


	//first, find the values of the keys
	int codes[256];
	for(unsigned int i = 0; i < 256; ++i)
		codes[i] = 0;

	//look though the string
		//States:
			//0		=		okay
			//1		=		enter [
			//2		=		hypthan -
			//3		=		dash \ [ignore next charecor as control charector
			//99	=		done
	int state = 0;
	int pos = 0;
	int num = 0;
	char from = '\0';
	char to = '\0';
	int len = 	strlen(key);
	while (pos <= len && state != 99)
	{
		if (key[pos] == '\0')
			state = 99;
		else if (key[pos] == '\\' && state != 3)
			state = 3;
		else if (state != 3)
		{
			// [ , ] , - , \;
			if (key[pos] == '[' )
				state = 1;
			//This tells the program to itemate on from-to charectors
			else if (key[pos] == ']' && state==2)
			{
				state = 0;
				if ((int)from > (int)to)
				{
					char store = from;
					from = to;
					to = store;
				}
				int todo = (int)to - (int)from;
				int on = 0;
				while (on <= todo)
				{
					codes[(int)from + on] = ++num;
					on++;
				}
			}
			else if (key[pos] == '-' && state == 1)
				state = 2;
			//getting them item letters
			else if (state == 1)
				from = key[pos];
			else if (state == 2)
				to = key[pos];
			else
				codes[(int)key[pos]] = ++num;
		}
		else
		{
			// \n , \q (quote)
			if			(key[pos] == 'n')
						codes[(int)'\n'] = num++;
			else	if (key[pos] == 'q')
						codes[(int)'"'] = num++;
			else	if (key[pos] == '[')
						codes[(int)'['] = num++;
			else	if (key[pos] == ']')
						codes[(int)']'] = num++;
			else	if (key[pos] == '\\')
						codes[(int)'\\'] = num++;
			state = 0;

		}
	pos++;
	}

	//================
	//The Conversion
	//================
	num++;
	
	long size =strlen(string)-1;
	int val = 0;
	int pown = 0;
	for (int n=0;n<=size;n++)
	{
		if (codes[string[n]] > 0 && codes[string[n]] <= num)
		{
				val +=   (int)(pow((double)num,pown))*(codes[string[n]]);
				pown++;
		}
	}
	return val;
}



//=========================================================
//UN PACK X
//=========================================================


long WINAPI DLLExport UnPackX(LPRDATA rdPtr, long param1)
{
	long code=CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	char* key=rdPtr->key;
	rdPtr->rHo.hoFlags |= HOF_STRING;

		char codes[256];
	codes[0] = '\0';
	memset(codes,'\0',256);
	int state = 0;
	int pos = 0;
	int num = 0;
	char from = '\0';
	char to = '\0';
	int len = 	strlen(key);
	while (pos <= len && state != 99)
	{
		if (key[pos] == '\0')
			state = 99;
		else if (key[pos] == '\\' && state != 3)
			state = 3;
		else if (state != 3)
		{
			// [ , ] , - , \;
			if (key[pos] == '[' )
				state = 1;
			//This tells the program to itemate on from-to charectors
			else if (key[pos] == ']' && state==2)
			{
				state = 0;
				//Swap them around if we need to
				if ((int)from > (int)to)
				{
					char store = from;
					from = to;
					to = store;
				}
				//How many charecters are there in this range?
				int todo = (int)to - (int)from;
				//..And which one am i currently on
				int on = 0;
				//For each charecter
				while (on <= todo)
				{
					//Codes is a Values-->Char lookup table.
					codes[++num] = (char)((int)from + (on++));
				}
			}
			else if (key[pos] == '-' && state == 1)
				state = 2;
			//getting them item letters
			else if (state == 1)
				from = key[pos];
			else if (state == 2)
				to = key[pos];
			else
			{
				codes[++num] = key[pos];
			}
		}
		else
		{
			// \n , \q (quote)
			if			(key[pos] == 'n')
						codes[num++] = '\n';
			else	if (key[pos] == 'q')
						codes[num++] = '"';
			else	if (key[pos] == '[')
						codes[num++] = '[';
			else	if (key[pos] == ']')
						codes[num++] = ']';
			else	if (key[pos] == '\\')
						codes[num++] = '\\';
			state = 0;

		}
	pos++;
	}
	num++;
	//==============
	//The Conversion
	//==============
	LPSTR string = (LPSTR)callRunTimeFunction(rdPtr, RFUNCTION_GETSTRINGSPACE, 33 , 0); 
	int n = 0;
	int dif = 0;
	//While the code is not 0.
	while (code != 0)
	{
		dif = code % (num);
		//Divide
		code /= num;
		//Set the string position n to the value. And increase n.
		string[n++] = codes[dif];
	}
	string[n] = '\0';

	return (long)string;
}

//=========================================================
//STRING MAX LENGTH
//=========================================================

long WINAPI DLLExport StringMaxLength(LPRDATA rdPtr, long param1)
{
	char * key=rdPtr->key;
		int chars = KeyCharCountInternal(key) + 1;
	return  (long)(double)log( (double)pow(2,(double)(sizeof (long))*8) ) / log ((double) chars ) ;
}
//=========================================================
//KEY CHAR COUNT (INTERFACE)
//=========================================================
long WINAPI DLLExport KeyCharCount(LPRDATA rdPtr, long param1)
{
	char * key = rdPtr->key;
	return KeyCharCountInternal( key );
}

//=========================================================
//INTERNAL USE: KEY CHAR COUNT
//=========================================================
int KeyCharCountInternal( char* key )
{
	int codes[256];
	//look though the string
		//States:
			//0		=		okay
			//1		=		enter [
			//2		=		hypthan -
			//3		=		dash \ [ignore next charecor as control charector
			//99	=		done
	int state = 0;
	int pos = 0;
	int num = 0;
	char from = '\0';
	char to = '\0';
	int len = 	strlen(key);
	while (pos <= len && state != 99)
	{
		if (key[pos] == '\0')
			state = 99;
		else if (key[pos] == '\\' && state != 3)
			state = 3;
		else if (state != 3)
		{
			// [ , ] , - , \;
			if (key[pos] == '[' )
				state = 1;
			//This tells the program to itemate on from-to charectors
			else if (key[pos] == ']' && state==2)
			{
				state = 0;
				if ((int)from > (int)to)
				{
					char store = from;
					from = to;
					to = store;
				}
				int todo = (int)to - (int)from;
				int on = 0;
				while (on <= todo)
				{
					codes[(int)from + (on++)] = num++;
				}
			}
			else if (key[pos] == '-' && state == 1)
				state = 2;
			//getting them item letters
			else if (state == 1)
				from = key[pos];
			else if (state == 2)
				to = key[pos];
			else
				num++;
		}
		else
		{
			// \n , \q (quote)
			if			(key[pos] == 'n')
						num++;
			else	if (key[pos] == 'q')
						num++;
			else	if (key[pos] == '[')
						num++;
			else	if (key[pos] == ']')
						num++;
			else	if (key[pos] == '\\')
						num++;
			state = 0;

		}
	pos++;
	}
	return num;
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
	IsInRange,
	0
	};

short (WINAPI * ActionJumps[])(LPRDATA rdPtr, long param1, long param2) =
	{
	SetKey,
	0
	};

long (WINAPI * ExpressionJumps[])(LPRDATA rdPtr, long param) =
	{
	PackX,
	UnPackX,
	StringMaxLength,
	KeyCharCount,
	0
	};


 