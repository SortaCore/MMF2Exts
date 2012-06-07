// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================

#include "common.h"
//#include <cv.hpp>
//#include "highgui.h"
#include "Collection-of-2D-points-Data.h"


// ============================================================================
//
// CONDITIONS
// 
// ============================================================================

//CONDITION(
//	/* ID */			0,
//	/* Name */			"Get default diameter(\"%o\") %0",
//	/* Flags */			EVFLAGS_ALWAYS|EVFLAGS_NOTABLE,
//	/* Params */		(1,PARAM_COMPARISON,"Compare default diameter to a value")
//) {
//	return rdPtr->polygon_size;
//}

////CONDITION(
////	/* ID */			0,
////	/* Name */			"%o: %0 == %1",
////	/* Flags */			EVFLAGS_ALWAYS|EVFLAGS_NOTABLE,
////	/* Params */		(2,PARAM_NUMBER,"First number",PARAM_NUMBER,"Second number")
////) {
////	int p1=Param(TYPE_INT);
////	int p2=Param(TYPE_INT);
////	
////	return p1==p2;
////}
////

CONDITION(
	/* ID */			0,
	/* Name */			"%o : On looping through points",
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
	/* Name */			"Clear",
	/* Flags */			0,
	/* Params */		(0)
) {
	Linked2DPoint* currentPoint;

	do{
		currentPoint = rdPtr->firstPoint;

		if (currentPoint == NULL){
			break;
		}
		
		rdPtr->firstPoint = currentPoint->nextPoint;

		delete currentPoint;
	}while (TRUE);

	rdPtr->lastPoint = NULL;
	rdPtr->count = 0;
}

ACTION(
	/* ID */			1,
	/* Name */			"Add point (%0, %1) to collection",
	/* Flags */			0,
	/* Params */		(2,PARAM_EXPRESSION,"X position of point",PARAM_EXPRESSION,"Y position of point")
) {
	int p1 = Param(TYPE_INT);
	int p2 = Param(TYPE_INT);
	/*int p3 = Param(TYPE_INT);
	int p4 = Param(TYPE_INT);
	int p5 = Param(TYPE_INT);
	
	cvRectangle(rdPtr->myImage, cvPoint(p1,p2), cvPoint(p1 + p3,p2 + p4), CV_RGB(p5 & 0xFF,p5 & 0xFF00,p5 & 0xFF0000), CV_FILLED);
	*/

	if (rdPtr->firstPoint == NULL){
		rdPtr->firstPoint = rdPtr->lastPoint = new Linked2DPoint;

		rdPtr->firstPoint->x = p1;
		rdPtr->firstPoint->y = p2;
		rdPtr->firstPoint->nextPoint = NULL;
	} else {
		rdPtr->lastPoint->nextPoint = new Linked2DPoint;

		rdPtr->lastPoint = rdPtr->lastPoint->nextPoint;

		rdPtr->lastPoint->x = p1;
		rdPtr->lastPoint->y = p2;
		rdPtr->lastPoint->nextPoint = NULL;
	}

	rdPtr->count++;
}

ACTION(
	/* ID */			2,
	/* Name */			"Loop through all points",
	/* Flags */			0,
	/* Params */		(0)
) {
	Linked2DPoint* currentPoint = rdPtr->firstPoint;

	do{
		if (currentPoint == NULL){
			break;
		}
		
		rdPtr->currentPointOnLoop = currentPoint;

		rdPtr->rRd->GenerateEvent(0);

		currentPoint = currentPoint->nextPoint;
	}while (TRUE);

	rdPtr->currentPointOnLoop = NULL;

}

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================

EXPRESSION(
	/* ID */			0,
	/* Name */			"Get number of points(",
	/* Flags */			0,
	/* Params */		(0)
) {
	return rdPtr->count;
}

EXPRESSION(
	/* ID */			1,
	/* Name */			"Get X of current looped point(",
	/* Flags */			0,
	/* Params */		(0)
) {
	return rdPtr->currentPointOnLoop->x;
}

EXPRESSION(
	/* ID */			2,
	/* Name */			"Get Y of current looped point(",
	/* Flags */			0,
	/* Params */		(0)
) {
	return rdPtr->currentPointOnLoop->y;
}


//EXPRESSION(
//	/* ID */			1,
//	/* Name */			"StringReturn$(",
//	/* Flags */			EXPFLAG_STRING,
//	/* Params */		(1,EXPPARAM_STRING,"String to return")
//) {
//	char * p1=(char *)ExParam(TYPE_STRING);
//
//	ReturnString(p1);
//}