// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================

#include "common.h"
#include <cv.hpp>
#include "highgui.h"

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
////
////CONDITION(
////	/* ID */			1,
////	/* Name */			"%o : On trigger",
////	/* Flags */			0,
////	/* Params */		(0)
////) {
////	return true;
////}

// ============================================================================
//
// ACTIONS
// 
// ============================================================================

//rdPtr->rRd->GenerateEvent(1);

ACTION(
	/* ID */			0,
	/* Name */			"Initiate recording from device #%0",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPRESSION,"Device no #")
) {
	int p1 = Param(TYPE_INT);
	
	rdPtr->myCapture = cvCreateCameraCapture(p1);
	
	rdPtr->rc.rcChanged = true;
}

ACTION(
	/* ID */			1,
	/* Name */			"Initiate false recording from \"%0\"",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPSTRING,"Filename")
) {
	rdPtr->myCapture = cvCreateFileCapture((char *)Param(TYPE_STRING));

	/*int p1 = Param(TYPE_INT);
	int p2 = Param(TYPE_INT);
	int p3 = Param(TYPE_INT);
	int p4 = Param(TYPE_INT);
	int p5 = Param(TYPE_INT);
	
	cvRectangle(rdPtr->myImage, cvPoint(p1,p2), cvPoint(p1 + p3,p2 + p4), CV_RGB(p5 & 0xFF,p5 & 0xFF00,p5 & 0xFF0000), CV_FILLED);
	*/
	rdPtr->rc.rcChanged = true;
}

//ACTION(
//	/* ID */			1,
//	/* Name */			"Set X position of lamp to %0",
//	/* Flags */			0,
//	/* Params */		(1,PARAM_EXPRESSION,"X pos")
//) {
//	rdPtr->rHo.hoX = param1;
//	rdPtr->rc.rcChanged = true;
//}
//
//ACTION(
//	/* ID */			2,
//	/* Name */			"Set Y position of lamp to %0",
//	/* Flags */			0,
//	/* Params */		(1,PARAM_EXPRESSION,"Y pos")
//) {
//	rdPtr->rHo.hoY =  param1;
//	rdPtr->rc.rcChanged = true;
//}

//ACTION(
//	/* ID */			0,
//	/* Name */			"Set default diameter to %0 px",
//	/* Flags */			0,
//	/* Params */		(1,PARAM_EXPRESSION,"Diameter in pixels")
//) {
//	rdPtr->polygon_size = Param(TYPE_INT);
//	//rdPtr->rc.rcChanged = true;
//}

//ACTION(
//	/* ID */			4,
//	/* Name */			"Set specular level to %0",
//	/* Flags */			0,
//	/* Params */		(1,PARAM_EXPRESSION,"Enter a value between 0 and 100")
//) {
////rdPtr->specular = Param(TYPE_INT);
////rdPtr->specular /= 100;
//}
//
//ACTION(
//	/* ID */			5,
//	/* Name */			"Set ambient to %0",
//	/* Flags */			0,
//	/* Params */		(1,PARAM_EXPRESSION,"Enter a value between 0 and 100")
//) {
////rdPtr->ambient = Param(TYPE_INT);
////rdPtr->ambient /= 100;
//}
//
//ACTION(
//	/* ID */			6,
//	/* Name */			"Load %0 as palette",
//	/* Flags */			0,
//	/* Params */		(1,PARAM_EXPSTRING,"Enter the path to your .pal")
//) {
////	rdPtr->palettePath = (char * )Param(TYPE_STRING);
//}
//
//ACTION(
//	/* ID */			7,
//	/* Name */			"Set xLight to %0",
//	/* Flags */			0,
//	/* Params */		(1,PARAM_EXPRESSION,"Enter a value")
//) {
////rdPtr->xLight = Param(TYPE_INT);
//}
//
//ACTION(
//	/* ID */			8,
//	/* Name */			"Set yLight to %0",
//	/* Flags */			0,
//	/* Params */		(1,PARAM_EXPRESSION,"Enter a value")
//) {
////rdPtr->yLight = Param(TYPE_INT);
//}
//
//ACTION(
//	/* ID */			9,
//	/* Name */			"Set zLight to %0",
//	/* Flags */			0,
//	/* Params */		(1,PARAM_EXPRESSION,"Enter a value")
//) {
////rdPtr->zLight = Param(TYPE_INT);
//}

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================

//EXPRESSION(
//	/* ID */			0,
//	/* Name */			"Get width(",
//	/* Flags */			0,
//	/* Params */		(0)
//) {
//	return rdPtr->rHo.hoImgWidth;
//}
//EXPRESSION(
//	/* ID */			1,
//	/* Name */			"Get height(",
//	/* Flags */			0,
//	/* Params */		(0)
//) {
//	return rdPtr->rHo.hoImgWidth;
//}

//
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