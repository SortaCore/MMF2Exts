// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================

#include "common.h"

// ============================================================================
//
// CONDITIONS
// 
// ============================================================================

CONDITION(
	/* ID */			0,
	/* Name */			"%o: %0 == %1",
	/* Flags */			EVFLAGS_ALWAYS|EVFLAGS_NOTABLE,
	/* Params */		(2,PARAM_NUMBER,"First number",PARAM_NUMBER,"Second number")
) {
	int p1=Param(TYPE_INT);
	int p2=Param(TYPE_INT);
	
	return p1==p2;
}


CONDITION(
	/* ID */			1,
	/* Name */			"%o : On trigger",
	/* Flags */			0,
	/* Params */		(0)
) {
	return true;
}

// ============================================================================
//
// ACTIONS
// 
// ============================================================================

ACTION(
	/* ID */			0,
	/* Name */			"Trigger condition",
	/* Flags */			0,
	/* Params */		(0)
) {
	rdPtr->rRd->GenerateEvent(1);
}

ACTION(
	/* ID */			1,
	/* Name */			"Dummy action",
	/* Flags */			0,
	/* Params */		(0)
) {

}

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================

EXPRESSION(
	/* ID */			0,
	/* Name */			"Sum(",
	/* Flags */			0,
	/* Params */		(2,EXPPARAM_NUMBER,"First number",EXPPARAM_NUMBER,"Second number")
) {
	int p1=ExParam(TYPE_INT);
	int p2=ExParam(TYPE_INT);
	
	return p1+p2;
}

EXPRESSION(
	/* ID */			1,
	/* Name */			"StringReturn$(",
	/* Flags */			EXPFLAG_STRING,
	/* Params */		(1,EXPPARAM_STRING,"String to return")
) {
	char * p1=(char *)ExParam(TYPE_STRING);

	ReturnString(p1);
}