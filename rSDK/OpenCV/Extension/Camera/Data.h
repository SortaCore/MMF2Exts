//#include "DataTypes.h"

#include <cv.h>
#include <highgui.h>

class rRundata;
typedef rRundata * LPRRDATA;

// --------------------------------
// RUNNING OBJECT DATA STRUCTURE
// --------------------------------
// If you want to store anything between actions/conditions/expressions
// you should store it here

//typedef struct tagRDATA
//{
//	//#include "MagicRDATA.h"
//
//headerObject rHo;
//rCom rc;
//rSpr rs;
//LPRRDATA rRd;
//
//CvCapture * myCapture;
//	
//} RUNDATA;


#include "Camera-Data.h"

typedef CameraData tagRDATA;

typedef tagRDATA RUNDATA;

typedef	RUNDATA	* LPRDATA;

// --------------------------------
// EDITION OF OBJECT DATA STRUCTURE
// --------------------------------
// These values let you store data in your extension that will be saved in the MFA
// You should use these with properties

typedef struct tagEDATA_V1
{
	extHeader		eHeader;
	short			swidth;
	short			sheight;

	int				xAngle;	
	int				yAngle;	
	int				zAngle;	

} EDITDATA;

typedef EDITDATA * LPEDATA;
