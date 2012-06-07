// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================
#include "common.h"

// ============================================================================
//
// FUNCTIONS
// 
// ============================================================================

void RefreshList (LPRDATA rdPtr, bool GEvent = false)
{
	// Obtain object list
	LPRH	rhPtr  = rdPtr -> rHo.hoAdRunHeader;
	LPOBL   oblPtr = (LPOBL)rhPtr->rhObjectList;
	
	// Clear current list
	rdPtr -> FixedVector.clear();

	// Loop through object list and add to vector list
	for (int i = 0; i < rhPtr->rhNObjects; oblPtr++, i++)
	{
		LPRO roPtr = (LPRO)(oblPtr->oblOffset);
		if (roPtr == NULL) continue;
		rdPtr -> FixedVector.push_back((roPtr->roHo.hoCreationId << 16) + roPtr->roHo.hoNumber);
		if (GEvent) //If iterating action, generate an event.
		{
			rdPtr -> FValue = (roPtr->roHo.hoCreationId << 16) + roPtr->roHo.hoNumber;
			rdPtr -> rRd-> GenerateEvent(0);
		}
	}
}

// ============================================================================
//
// CONDITIONS
// 
// ============================================================================

CONDITION(
	/* ID */			0,
	/* Name */			_T("%o: On iterated value"),
	/* Flags */			0,
	/* Params */		(0)
) {
	// This is called by GEvent being true in RefreshList.
	// Just return true.
	return true;
}

// ============================================================================
//
// ACTIONS
// 
// ============================================================================

ACTION(
	/* ID */			0,
	/* Name */			_T("Iterate fixed value list"),
	/* Flags */			0,
	/* Params */		(0)
) {
	//Refresh the list, and generate an event for each fixed value
	RefreshList(rdPtr, true);
}

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================

EXPRESSION(
	/* ID */			0,
	/* Name */			_T("LastFixedValue("),
	/* Flags */			0,
	/* Params */		(0)
) {
	// Deprecated expression: Notify user
	MessageBox(NULL, _T("Sorry to interrupt your programming, but you have\nan old \"LastFixedValue\" expression in FixedValueList which needs replacing.\nClick the FixedValueList icon in your event editor to see all the events\nZlibStream is in."), _T("Darkwire Software - Old Expression Notice"), MB_OK);
	return 0;
}

EXPRESSION(
	/* ID */			1,
	/* Name */			_T("LastIteratedValue("),
	/* Flags */			0,
	/* Params */		(0)
) {
	return rdPtr -> FValue;
}

EXPRESSION(
	/* ID */			2,
	/* Name */			_T("GetFixedFromIndex("),
	/* Flags */			0,
	/* Params */		(1, EXPPARAM_NUMBER, _T("Index"))
) {
	int p1 = ExParam(TYPE_INT);
	RefreshList(rdPtr);
	if (p1 >= 0 && p1 < rdPtr -> FixedVector.size()) //Remember vectors are 0-based.
		return rdPtr -> FixedVector[p1];
	else
	{
		rdPtr -> LastError = _T("GetFixedFromIndex failed: index out of range.");
		return 0;
	}
	
}

EXPRESSION(
	/* ID */			3,
	/* Name */			_T("GetList$("),
	/* Flags */			EXPFLAG_STRING,
	/* Params */		(1, EXPPARAM_STRING, _T("Delimiter"))
) {
	tchar * p1 = (tchar *) ExParam(TYPE_STRING);
	if (p1 = _T(""))
	{
		rdPtr -> LastError = _T("Blank delimiter given.");
		ReturnStringSafe(_T("0"));
	}
	stringstream ss;
	string temp, temp2;
	RefreshList(rdPtr);
	for (int i = 0; i < rdPtr -> FixedVector.size(); i++)
	{
		//Add to stringstream
		ss << rdPtr -> FixedVector[i] << p1;
	}
	// Set temp to the stream
	temp = ss.str(); ss.flush();
	// This removes the ending delimiter. For example, "F1|F2|" goes to "F1|F2".
	// 0 is the offset of the first character to read: here it is 0, read from start.
	temp2 = temp.substr(0, temp.length()-(_tcslen(p1)*sizeof(tchar))); temp = "";
#ifndef UNICODE
	ReturnStringSafe(temp2.c_str());
#else
	//Cast to Unicode
	wchar_t * temp3;
	MultiByteToWideChar(mV->mvGetAppCodePage(mV, mV->mvEditApp), NULL, temp2.c_str(), NULL, temp3,NULL);
	ReturnStringSafe(temp3);
#endif
}

EXPRESSION(
	/* ID */			4,
	/* Name */			_T("LastError$("),
	/* Flags */			EXPFLAG_STRING,
	/* Params */		(0)
) {
	ReturnStringSafe(rdPtr -> LastError);
}
