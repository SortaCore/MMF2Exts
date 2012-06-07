#ifndef CAMERA_DATA
#define CAMERA_DATA
struct CameraData
{
	//#include "MagicRDATA.h"

	headerObject rHo;
	rCom rc;
//	rSpr rs;
	LPRRDATA rRd;
	CvCapture * myCapture;
	
};
#endif