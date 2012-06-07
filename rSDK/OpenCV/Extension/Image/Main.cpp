// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================

#include "common.h"
#include <cv.hpp>
#include <highgui.h>
#include "../Camera/Camera-Data.h"
#include "../Collection of 2D points/Collection-of-2D-points-Data.h"

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
	/* Name */			"Draw rectangle (%2, %3) at %0, %1",
	/* Flags */			0,
	/* Params */		(6,PARAM_EXPRESSION,"X pos",PARAM_EXPRESSION,"Y pos",PARAM_EXPRESSION,"Width",PARAM_EXPRESSION,"Height",PARAM_COLOUR,"Stroke color",PARAM_EXPRESSION,"Thickness")
) {
	int p1 = Param(TYPE_INT);
	int p2 = Param(TYPE_INT);
	int p3 = Param(TYPE_INT);
	int p4 = Param(TYPE_INT);
	int p5 = Param(TYPE_INT);
	int p6 = Param(TYPE_INT);
	
	if (p6 <= 1){
		p6 = 1;
	}
	if (p6 > 255){
		p6 = 255;
	}
	
	cvRectangle(rdPtr->myImage, cvPoint(p1,p2), cvPoint(p1 + p3,p2 + p4), CV_RGB(p5 & 0xFF,p5 & 0xFF00,p5 & 0xFF0000), p6);
	
	rdPtr->rc.rcChanged = true;
}


ACTION(
	/* ID */			1,
	/* Name */			"Draw line between (%0, %1) and (%2, %3)",
	/* Flags */			0,
	/* Params */		(7,PARAM_EXPRESSION,"First X pos",PARAM_EXPRESSION,"First Y pos",PARAM_EXPRESSION,"Second X pos",PARAM_EXPRESSION,"Second Y pos",PARAM_COLOUR,"Stroke color",PARAM_EXPRESSION,"Thickness",PARAM_EXPRESSION,"Antialiasing? 0=false, 1=true")
) {
	int p1 = Param(TYPE_INT);
	int p2 = Param(TYPE_INT);
	int p3 = Param(TYPE_INT);
	int p4 = Param(TYPE_INT);
	int p5 = Param(TYPE_INT);
	int p6 = Param(TYPE_INT);
	int p7 = Param(TYPE_INT);
	
	if (p6 <= 1){
		p6 = 1;
	}
	if (p6 > 255){
		p6 = 255;
	}
	
	cvLine(rdPtr->myImage, cvPoint(p1,p2), cvPoint(p3,p4), CV_RGB(p5 & 0xFF,p5 & 0xFF00,p5 & 0xFF0000), p6, p7 == 1 ? CV_AA : 0);
	
	rdPtr->rc.rcChanged = true;
}

ACTION(
	/* ID */			2,
	/* Name */			"Fill rectangle (%2, %3) at %0, %1",
	/* Flags */			0,
	/* Params */		(5,PARAM_EXPRESSION,"X pos",PARAM_EXPRESSION,"Y pos",PARAM_EXPRESSION,"Width",PARAM_EXPRESSION,"Height",PARAM_COLOUR,"Fill color")
) {
	int p1 = Param(TYPE_INT);
	int p2 = Param(TYPE_INT);
	int p3 = Param(TYPE_INT);
	int p4 = Param(TYPE_INT);
	int p5 = Param(TYPE_INT);
	
	cvRectangle(rdPtr->myImage, cvPoint(p1,p2), cvPoint(p1 + p3,p2 + p4), CV_RGB(p5 & 0xFF,p5 & 0xFF00,p5 & 0xFF0000), CV_FILLED);
	
	rdPtr->rc.rcChanged = true;
}

ACTION(
	/* ID */			3,
	/* Name */			"Draw text (%0) at %1, %2",
	/* Flags */			0,
	/* Params */		(4,PARAM_EXPSTRING,"Text to display",PARAM_EXPRESSION,"X pos",PARAM_EXPRESSION,"Y pos",PARAM_COLOUR,"Fill color")
) {

char* myString = (char *)Param(TYPE_STRING);

	int x = Param(TYPE_INT);
	int y = Param(TYPE_INT);
	int fontColorParameter = Param(TYPE_INT);
CvScalar fontColor = CV_RGB(fontColorParameter & 0xFF,fontColorParameter & 0xFF00,fontColorParameter & 0xFF0000);


CvFont font;
double hScale=0.5;
double vScale=0.5;
int    lineWidth=1;
cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX /*|CV_FONT_ITALIC*/, hScale,vScale,0,lineWidth);

cvPutText (rdPtr->myImage,myString,cvPoint(x,y), &font, fontColor);
//Other possible fonts:
//
//CV_FONT_HERSHEY_SIMPLEX, CV_FONT_HERSHEY_PLAIN,
//CV_FONT_HERSHEY_DUPLEX, CV_FONT_HERSHEY_COMPLEX,
//CV_FONT_HERSHEY_TRIPLEX, CV_FONT_HERSHEY_COMPLEX_SMALL,
//CV_FONT_HERSHEY_SCRIPT_SIMPLEX, CV_FONT_HERSHEY_SCRIPT_COMPLEX,


//
//	/* Params */		(1,PARAM_EXPSTRING,"Enter the path to your voxel model (.vxl)")
//) {
//	loadVoxel(rdPtr, );
	//int p3 = Param(TYPE_INT);
	//int p4 = Param(TYPE_INT);
	//int p5 = Param(TYPE_INT);
	//
	//cvRectangle(rdPtr->myImage, cvPoint(p1,p2), cvPoint(p1 + p3,p2 + p4), CV_RGB(p5 & 0xFF,p5 & 0xFF00,p5 & 0xFF0000), CV_FILLED);
	
	rdPtr->rc.rcChanged = true;
}




ACTION(
	/* ID */			4,
	/* Name */			"Get frame from camera %0",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPSTRING,"Text to display")
) {

char* myString = (char *)Param(TYPE_STRING);

	LPRH	rhPtr  = rdPtr->rHo.hoAdRunHeader;
	// List of object offsets
	LPOBL   oblPtr = (LPOBL)rhPtr->rhObjectList;
	for (int i=0; i<rhPtr->rhNObjects; oblPtr++) {
		if (!oblPtr->oblOffset) continue;

		CameraData* roPtr = (CameraData*)(oblPtr->oblOffset);

		if (roPtr->rHo.hoIdentifier == MAKEID(O,C,V,C)){ //BLMP MAKEID(B,L,M,P)
			if (roPtr->myCapture){
				IplImage * myImage = cvQueryFrame(roPtr->myCapture);

				if (rdPtr->myImage){
					if (rdPtr->myImage->width == myImage->width && rdPtr->myImage->height == myImage->height){
						cvCopy(myImage, rdPtr->myImage);
					} else {
						cvReleaseImage(&rdPtr->myImage);
					}
				}
				
				if (!rdPtr->myImage){
					rdPtr->myImage = cvCloneImage(myImage);
				}
			}
		}
		
		i++;
	}

	rdPtr->rc.rcChanged = true;
}

ACTION(
	/* ID */			5,
	/* Name */			"Load image from \"%0\"",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPSTRING,"Image to load")
) {
	char* p1 = (char *)Param(TYPE_STRING);

	cvReleaseImage(&rdPtr->myImage);

	rdPtr->myImage = cvLoadImage(p1, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
}

ACTION(
	/* ID */			6,
	/* Name */			"Save image to \"%0\"",
	/* Flags */			0,
	/* Params */		(1,PARAM_EXPSTRING,"Path for image to be saved")
) {
	char* p1 = (char *)Param(TYPE_STRING);

	cvSaveImage(p1, rdPtr->myImage);
}

ACTION(
	/* ID */			7,
	/* Name */			"Draw polygon from collection %0",
	/* Flags */			0,
	/* Params */		(2,PARAM_EXPSTRING,"ID of 2D point collection", PARAM_COLOUR,"Color", PARAM_EXPRESSION, "Thickness")
) {

	char* myString = (char *)Param(TYPE_STRING);
	int color = Param(TYPE_INT);
	int thickness = Param(TYPE_INT);

	if (thickness <= 1){
		thickness = 1;
	}
	if (thickness > 255){
		thickness = 255;
	}

	LPRH	rhPtr  = rdPtr->rHo.hoAdRunHeader;
	// List of object offsets
	LPOBL   oblPtr = (LPOBL)rhPtr->rhObjectList;
	for (int i=0; i<rhPtr->rhNObjects; oblPtr++) {
		if (!oblPtr->oblOffset) continue;

		CollectionOf2DPointsData* roPtr = (CollectionOf2DPointsData*)(oblPtr->oblOffset);

		if (roPtr->rHo.hoIdentifier == MAKEID(O,C,V,P)){ //BLMP MAKEID(B,L,M,P)
			if (roPtr->firstPoint){
				int ptsIndex = 0;

				int npts[1] = { roPtr->count};
				CvPoint **pts;

				pts = (CvPoint **) cvAlloc (sizeof (CvPoint *));
				pts[0] = (CvPoint *) cvAlloc (sizeof (CvPoint) * roPtr->count);

				Linked2DPoint* currentPoint = roPtr->firstPoint;

				for (int ptsIndex = 0; ptsIndex < roPtr->count; ptsIndex++){
					pts[0][ptsIndex].x = currentPoint->x;
					pts[0][ptsIndex].y = currentPoint->y;

					currentPoint = currentPoint->nextPoint;
				}

				//pts[0][0].x = 50;
				//pts[0][0].y = 210;
				//pts[0][1].x = 10;
				//pts[0][1].y = 280;
				//pts[0][2].x = 90;
				//pts[0][2].y = 285;
				//
				//cvFillPoly (rdPtr->myImage, pts, npts, 1, CV_RGB (255, 128, 0),8,0);
				cvPolyLine( rdPtr->myImage, pts, npts, 1, 1,CV_RGB(0,0,255), 1,  8, 0 );

				cvFree(&pts[0]);
				cvFree(&pts);





//cvPolyLine( void *img, CvPoint **pts, int *npts,
//            int contours, int closed, CvScalar color,
//            int thickness, int line_type, int shift )
//				cvDrawPolyLine(rdPtr->myImage, pts, npts, 0, 1, CV_RGB(color & 0xFF, color & 0xFF00, color & 0xFF0000), thickness);
			}
		}
		
		i++;
	}





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

EXPRESSION(
	/* ID */			0,
	/* Name */			"Get width(",
	/* Flags */			0,
	/* Params */		(0)
) {
	return rdPtr->rHo.hoImgWidth;
}
EXPRESSION(
	/* ID */			1,
	/* Name */			"Get height(",
	/* Flags */			0,
	/* Params */		(0)
) {
	return rdPtr->rHo.hoImgWidth;
}

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