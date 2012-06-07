/*
 *		TigsExt.hpp
 *
 *		Functions for the Extension SDK (made for the MMFireSDK)
 *		Tigerworks, 2003  v1.1
 *		www.tigerworks.co.uk tigerworks@blueyonder.co.uk
 */

/* v1.1 update by FireMonkey (chris.branch@starfishgames.co.uk)
 *
 *	everything inline for extra efficiency
 *  everything also in single .h file (and changed to .hpp)
 *	GetStringSpace -> GetStringSpaceOld
 *	GetStringSpace updated to support large strings
 *	Added:
 *		DestroyLPRO, IsDestroyed
 *		todeg(a),torad(a),swap(a,b) macros
 *		IsMMFHOME/PRO/build and GetMMFbuild macros
*/


#ifndef TIGSEXT	// Escape multiple inclusion
#define TIGSEXT

#include <time.h>
// Common include
#include "common.h"

////////////////
// PROTOTYPES //
////////////////
_inline int ErrCheck(int,int,int);			// Error check an integer
_inline float ErrCheck(float,float,float);	// Error check a float
_inline bool IsInBounds(int,int,int);		// Integer is between two numbers
_inline bool IsInBounds(float,float,float);	// Float is between two numbers
_inline int GetIntAltVal(LPRO,int);			// Get integer alterable value
_inline float GetFloatAltVal(LPRO,int);		// Get float alterable value
_inline int FixedVal(LPRO);					// Get fixed value of an object
_inline float GetFloatParam(LPRDATA);		// Get float parameter
_inline void SetReturnString(LPRDATA);		// Set expression to return string
_inline void GenerateEvent(LPRDATA,int);	// Generate event
_inline void PushEvent(LPRDATA,int);		// Push event
_inline void DestroyObject(LPRDATA);		// Destroy the object
_inline void ReHandleObject(LPRDATA);		// Start calling HandleRunObject again
_inline char* GetStringSpace(LPRDATA,int);	// Get temporary string space from MMF
_inline char* GetStringSpaceOld(LPRDATA,int);	// Get temporary string space from MMF (pre-MMF1.5)
_inline int GetActionLoopCount(LPRDATA);	// Number of times action called by MMF for PARAM_OBJECT (starts at 0)
_inline bool IsLastActionLoop(LPRDATA);		// Is last time action is being called by MMF for PARAM_OBJECT
_inline long GetTimer();					// Timer count, 100 per second
_inline char UpperChar(char);				// Convert a char to uppercase
_inline char LowerChar(char);				// Convert a char to lowercase
_inline int GetDistance(int,int,int,int);	// Distance between two points
_inline int GetAngle(int,int,int,int);		// Angle between two points
_inline int RotateToward(int,int,int,int);	// Rotate towards direction/angle
_inline int AngToDir(LONG);					// Convert angle to direction
_inline LPRO LproFromFixed(LPRDATA rdPtr,long fixedvalue);

_inline void DestroyObject(LPRO Object);
_inline bool IsDestroyed(LPRO Object);

// Mathematical macros and definitions
#define RADtoDEG 180/3.14159		// Convert radians to degrees
//#define max(a,b)  (((a) > (b)) ? (a) : (b))	// Already defined
//#define min(a,b)  (((a) < (b)) ? (a) : (b))
#define dif(a,b)  (__max(a,b) - __min(a,b))
#define todeg(a)  ((a)*RADtoDEG)
#define torad(a)  ((a)/RADtoDEG)
#define swap(a,b) (a^=b^=a^=b)
#define IsTGF(v) (v->mvGetVersion != NULL && ((v->mvGetVersion() & MMFVERFLAG_MASK) & MMFVERFLAG_HOME) != 0)
#define IsMMFSTD(v) (v->mvGetVersion != NULL && ((v->mvGetVersion() & MMFVERFLAG_MASK) & (MMFVERFLAG_HOME|MMFVERFLAG_PRO)) == 0)
#define IsMMFPRO(v) (v->mvGetVersion != NULL && ((v->mvGetVersion() & MMFVERFLAG_MASK) & MMFVERFLAG_PRO) != 0)
#define IsMMFbuild(v,b) (v->mvGetVersion != NULL && (v->mvGetVersion() & MMFBUILD_MASK) == b)
#define GetMMFbuild(v) (v->mvGetVersion != NULL ? (v->mvGetVersion() & MMFBUILD_MASK) : 0)

// Make identifier
#define MAKEID(a,b,c,d) ((#@a << 24)|(#@b << 16)|(#@c << 8)|(#@d)) 



/////////////////////////////////////
// PARAM_ALTVALUE return structure //
/////////////////////////////////////

// PARAM_ALTVALUE returns a pointer to something (I don't know what)
// If you use it as a pointer to this structure, AltValIndex will hold the value you want
// (the index of the alterable value, 0 = value A, 1 = value B, up to 25)

typedef struct {
	long undefined;			// If you know what this data is for, contact me!
	BYTE AltValIndex;		// The alterable value index (the value you want)
} ParamAltValue;

/* example use
ParamAltValue* AltVal = (ParamAltValue*)param1;	// or CNC_GetParameter(rdPtr);
int AltValIndex = AltVal->AltValIndex;
*/




/*
 *		TigsExt functions
 *		(formerly TigsExt.cpp)
 */

///////////////////
// MISCELLANEOUS //
///////////////////

// ErrCheck:  Returns an integer limited by the given bounds
// int number:  number you wish to check
// int min: minimum value for this number
// int max: maximum value for this number
// Example:  int myvalue = ErrCheck(Value, 0, 25);
_inline int ErrCheck(int number, int min, int max)
{
	if (number > max) number = max;
	if (number < min) number = min;
	return number;
}

// Float version
_inline float ErrCheck(float number, float min, float max)
{
	if (number > max) number = max;
	if (number < min) number = min;
	return number;
}

// IsInBounds:  returns true if the given value is between the two given numbers
_inline bool IsInBounds(int number, int min, int max)
{
	if (number < min || number > max)
		return false;
	else return true;
}

// Float version
_inline bool IsInBounds(float number, float min, float max)
{
	if (number < min || number > max)
		return false;
	else return true;
}


// GetIntAltVal:  Get integer alterable value from object LPRO
// LPRO object:  the object LPRO that you want to retrieve the alterable value from
// int ValNum:  alterable value index to retrieve:  0 is value A, 1 is value B, up to 25
_inline int GetIntAltVal(LPRO object, int ValNum)
{
	if (!IsInBounds(ValNum,0,25))
		return 0;

	// Build 242 or below
//	if (object->rov.rvValuesType[ValNum] == 0)			// Is integer alterable value
//		return object->rov.rvValues[ValNum];			// Return that value
//	if (object->rov.rvValuesType[ValNum] == 2)			// Is float alterable value
//		return (int)*((float*)&object->rov.rvValues[ValNum]);	// Convert to int and return

	// Build 243 or above
	CValue* pValue = &object->rov.rvpValues[ValNum];
	if ( pValue->m_type == TYPE_INT )					// Is integer alterable value
		return pValue->m_long;
	if ( pValue->m_type == TYPE_DOUBLE )
		return (int)pValue->m_double;

	return 0; // should be impossible
}

// Float version
_inline float GetFloatAltVal(LPRO object, int ValNum)
{
	if (!IsInBounds(ValNum,0,25))
		return 0;

	// Build 242 or below
//	if (object->rov.rvValuesType[ValNum] == 0)			// Is integer alterable value
//		return (float)object->rov.rvValues[ValNum];	// Convert to float and return
//	if (object->rov.rvValuesType[ValNum] == 2)			// Is float alterable value
//		return *((float*)&object->rov.rvValues[ValNum]);	// Return that float

	// Build 243 or above
	CValue* pValue = &object->rov.rvpValues[ValNum];
	if ( pValue->m_type == TYPE_INT )					// Is integer alterable value
		return (float)pValue->m_long;
	if ( pValue->m_type == TYPE_DOUBLE )
		return (float)pValue->m_double;

	return 0; // should be impossible
}

// Double version
_inline double GetDoubleAltVal(LPRO object, int ValNum)
{
	if (!IsInBounds(ValNum,0,25))
		return 0;

	// Build 242 or below
//	if (object->rov.rvValuesType[ValNum] == 0)			// Is integer alterable value
//		return (float)object->rov.rvValues[ValNum];	// Convert to float and return
//	if (object->rov.rvValuesType[ValNum] == 2)			// Is float alterable value
//		return *((float*)&object->rov.rvValues[ValNum]);	// Return that float

	// Build 243 or above
	CValue* pValue = &object->rov.rvpValues[ValNum];
	if ( pValue->m_type == TYPE_INT )					// Is integer alterable value
		return (double)pValue->m_long;
	if ( pValue->m_type == TYPE_DOUBLE )
		return pValue->m_double;

	return 0; // should be impossible
}

// FixedVal(Object):  Returns the fixed value of the given object LPRO
_inline int FixedVal(LPRO object)
{ return ((object->roHo.hoCreationId << 16) + object->roHo.hoNumber); }

// LproFromFixed: Returns a pointer to RunObject structure from a fixed value
_inline LPRO LproFromFixed(LPRDATA rdPtr,long fixedvalue)
{
	LPOBL objList = rdPtr->rHo.hoAdRunHeader->rhObjectList;
	return (LPRO)(objList[0x0000FFFF & fixedvalue].oblOffset);
}



////////////////
// PARAMETERS //
////////////////

// Get float parameter for actions/conditions
_inline float GetFloatParam(LPRDATA rdPtr)
{
	long tmpf = CNC_GetFloatParameter(rdPtr);
	float param = *(float*)&tmpf;
	return param;
}

// SetReturnString:  Tells MMF that the current expression is going to return a string
_inline void SetReturnString(LPRDATA rdPtr)
{	rdPtr->rHo.hoFlags |= HOF_STRING;	}



///////////////////////
// RUNTIME FUNCTIONS //
///////////////////////

// GenerateEvent:  *instantly* generates an event
_inline void GenerateEvent(LPRDATA rdPtr, int EventID)
{	callRunTimeFunction(rdPtr,RFUNCTION_GENERATEEVENT,EventID,0); }

// PushEvent:  same as GenerateEvent, but the event is only generated at the end of MMF's event loop
_inline void PushEvent(LPRDATA rdPtr, int EventID)
{	callRunTimeFunction(rdPtr,RFUNCTION_PUSHEVENT,EventID,0); }

// DestroyObject:  destroys your object like the Destroy action (will call DestroyRunObject etc. etc.)
_inline void DestroyObject(LPRDATA rdPtr)
{	callRunTimeFunction(rdPtr,RFUNCTION_DESTROY,0,0);	}

// ReHandleObject:  start calling HandleRunObject() again
_inline void ReHandleObject(LPRDATA rdPtr)
{	callRunTimeFunction(rdPtr,RFUNCTION_REHANDLE,0,0);	}

// GetStringSpace:  gets some string space for an expression to return a string with.
// returns a pointer to your new memory.  MMF will automatically free this memory soon after your expression routine finishes
// NO LIMIT - BUILD 108 and up
_inline char* GetStringSpace(LPRDATA rdPtr, int size)
{	return (char *)callRunTimeFunction(rdPtr,RFUNCTION_GETSTRINGSPACE_EX,0,size); }

// GetStringSpaceOld:  gets some string space for an expression to return a string with.
// returns a pointer to your new memory.  MMF will automatically free this memory soon after your expression routine finishes
// LIMITED TO 32KB - but compatible with all builds
_inline char* GetStringSpaceOld(LPRDATA rdPtr, int size)
{	return (char *)callRunTimeFunction(rdPtr,RFUNCTION_GETSTRINGSPACE,size,0); }

// GetActionLoopCount(rdPtr): if you use PARAM_OBJECT as your first parameter, MMF will repeat your action
// for every one of those objects that met the event.  This will return the index of the action repetition
// e.g. 0 is the first run of the action, 1 is the next, etc.
_inline int GetActionLoopCount(LPRDATA rdPtr)
{	return rdPtr->rHo.hoAdRunHeader->rh2.rh2ActionLoopCount; }

// IsLastActionLoop:  As above, when MMF is looping your action for objects, this will return
// TRUE the LAST time MMF is running the action for you.
_inline bool IsLastActionLoop(LPRDATA rdPtr)
{
	if (rdPtr->rHo.hoAdRunHeader->rh2.rh2ActionLoop == 0)
		return TRUE;
	else return FALSE;
}


/////////////////////////
// OTHER MMF FUNCTIONS //
/////////////////////////

// DestroyLPRO:  Destroys another object as if using its Destroy action
_inline void DestroyObject(LPRO Object)
{	Object->roHo.hoFlags |= HOF_DESTROYED;	}

// IsDestroyed:  Test to see if an object has been destroyed, in HandleRunObject
_inline bool IsDestroyed(LPRO Object)
{	return (Object->roHo.hoFlags & HOF_DESTROYED ? true : false);	}



//////////////////////
// TIMING FUNCTIONS //
//////////////////////

// This value starts at 0 when the frame begins and counts up 100 every 1 second, constantly
_inline long GetTimer()
{	return (clock() / 10);	}

//////////////////////
// STRING FUNCTIONS //
//////////////////////


// Convert character to uppercase
_inline char UpperChar(char ch)
{
	// If in the range of a lowercase char
	if ((ch > 96) && (ch < 123))
		ch -= 32;	// Subtract down to range of an uppercase char

	return ch;
}

// Convert a character to lowercase
_inline char LowerChar(char ch)
{
	// If in the range of an uppercase char
	if ((ch > 64) && (ch < 91))
		ch += 32;	// Add up to lowercase char

	return ch;
}


// Angle between two points
_inline int GetAngle(int x1, int y1, int x2, int y2)
{
	double answer = (atan2(((double)y2 - (double)y1),((double)x2 - (double)x1)) * RADtoDEG) * -1 + 450;

	// Convert to integer
	return ((int)answer) % 360;
}

// Distance between 2 points
_inline int GetDistance(int xp, int yp, int xp2, int yp2)
{
	// Allow decimal calculation
	double x = xp;
	double y = yp;
	double x2 = xp2;
	double y2 = yp2;

	return (int)sqrt((dif(x,x2) * dif(x,x2))+(dif(y,y2) * dif(y,y2)));
}

// RotateToward():  Direction-calculator style rotate towards direction.  Works in 360 degrees too.
// int CurAngle:  Current angle/direction
// int TargetAngle:  Angle/direction you want to rotate towards
// int RotateAmt:  Amount of directions to rotate (e.g. 4 will rotate 4 degrees a time in 360 directions
// int Directions:  use 32 for 32 direction rotation, or 360 for 360 degree rotation, or even a custom amount (e.g. 8 will still work in 8-dirs)
_inline int RotateToward(int CurAngle, int TargetAngle, int RotateAmt, int Directions)
{
	LONG rightdist = (Directions + (CurAngle - TargetAngle)) % Directions;
	LONG leftdist  = (Directions + (TargetAngle - CurAngle)) % Directions;

	// Distance right is shorter, AND distance right is greater than rotate amount
	if ((rightdist <= leftdist) && (rightdist >= RotateAmt))
		return (Directions + (CurAngle - RotateAmt)) % Directions;		// Rotate by amount

	// Distance right is shorter AND distance right is less than rotate amount
	if ((rightdist <= leftdist) && (rightdist < RotateAmt))
		return (Directions + TargetAngle) % Directions;		// Target direction can be reached immediately

	// Left distance is shorter AND distance left is greater than rotate amount
	if ((rightdist > leftdist) && (leftdist >= RotateAmt))
		return (Directions + (CurAngle + RotateAmt)) % Directions;		// Rotate by amount

	if ((rightdist > leftdist) && (leftdist < RotateAmt))
		return (Directions + TargetAngle) % Directions;		// Target direction can be reached immediately

	// (Unreachable)
	return 0;

}

// Convert angle to direction
_inline int AngToDir(LONG Angle)
{
	// Allow decimal calculations
	Angle = (Angle - 90) % 360;
	double Ang = (double)Angle;

	double Dir = (Ang + 95.625) / 11.25;

	LONG NewDir = (LONG)(32 + Dir);
	
	return NewDir % 32;
}



#endif // TIGSEXT








