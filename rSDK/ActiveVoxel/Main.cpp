// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================

#include "common.h"
#include "Voxel.h"

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


//rdPtr->rRd->GenerateEvent(1);

ACTION(
	/* ID */			0,
	/* Name */			"Load %0 as voxel/animation",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPSTRING,"Enter the path to your voxel and HVA file")
) {
	loadVoxel(rdPtr, (char *)Param(TYPE_STRING));
	rdPtr->rc.rcChanged = true;
}

ACTION(
	/* ID */			1,
	/* Name */			"Set X angle to %0°",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPRESSION,"Angle (Degrees)")
) {
	//(param1 % 360+360) % 360;
	rdPtr->xAngle = max(0,min(360,param1));
	rdPtr->rc.rcChanged = true;
}

ACTION(
	/* ID */			2,
	/* Name */			"Set Y angle to %0°",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPRESSION,"Angle (Degrees)")
) {
	rdPtr->yAngle =  max(0,min(360,param1));
	rdPtr->rc.rcChanged = true;
}

ACTION(
	/* ID */			3,
	/* Name */			"Set Z angle to %0°",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPRESSION,"Angle (Degrees)")
) {
	rdPtr->zAngle =  max(0,min(360,param1));
	rdPtr->rc.rcChanged = true;
}

ACTION(
	/* ID */			4,
	/* Name */			"Set specular level to %0",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPRESSION,"Enter a value between 0 and 100")
) {
//rdPtr->specular = Param(TYPE_INT);
//rdPtr->specular /= 100;
}

ACTION(
	/* ID */			5,
	/* Name */			"Set ambient to %0",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPRESSION,"Enter a value between 0 and 100")
) {
//rdPtr->ambient = Param(TYPE_INT);
//rdPtr->ambient /= 100;
}

ACTION(
	/* ID */			6,
	/* Name */			"Load %0 as palette",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPSTRING,"Enter the path to your .pal")
) {
//	rdPtr->palettePath = (char * )Param(TYPE_STRING);
}

ACTION(
	/* ID */			7,
	/* Name */			"Set xLight to %0",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPRESSION,"Enter a value")
) {
//rdPtr->xLight = Param(TYPE_INT);
}

ACTION(
	/* ID */			8,
	/* Name */			"Set yLight to %0",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPRESSION,"Enter a value")
) {
//rdPtr->yLight = Param(TYPE_INT);
}

ACTION(
	/* ID */			9,
	/* Name */			"Set zLight to %0",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPRESSION,"Enter a value")
) {
//rdPtr->zLight = Param(TYPE_INT);
}

ACTION(
	/* ID */			10,
	/* Name */			"Enable shading",
	/* Flags */			0,
	/* Params */		(0)
) {
rdPtr->enableShading = TRUE;
}

ACTION(
	/* ID */			11,
	/* Name */			"Disable shading",
	/* Flags */			0,
	/* Params */		(0)
) {
rdPtr->enableShading = FALSE;
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