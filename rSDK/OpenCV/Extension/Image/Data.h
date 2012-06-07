//#include "DataTypes.h"

#include "cv.h"

//#include "highgui.h"

class rRundata;
typedef rRundata * LPRRDATA;

// --------------------------------
// RUNNING OBJECT DATA STRUCTURE
// --------------------------------
// If you want to store anything between actions/conditions/expressions
// you should store it here

typedef struct tagRDATA
{
	//#include "MagicRDATA.h"

headerObject rHo;

	rCom rc;
			rSpr rs;
		//rSpr rs;
LPRRDATA rRd;


IplImage* myImage;


	//int				xAngle;	
	//int				yAngle;	
	//int				zAngle;

	////int				xLight;
	////int				yLight;	
	////int				zLight;

	////float			specular;
	////float			ambient;

	////char *				voxelPath;
	//vxl_runtime_part **	voxelParts;
	//char				n_parts;
	////char *			palettePath;

	////voxel_t			voxel;
	////int				frame;
	//matrix_t		lMatrix;
	//double			sinus[3];
	//double			cosine[3];
	//short *			surfaceDepthGrid;
	//char *			screenDot;
	//bool*			screenRender;

	//int polygon_size;

	//cSurface *		lamp;
	cSurface *		pSf;
	
} RUNDATA;

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
