#include "DataTypes.h"

class rRundata;
typedef rRundata * LPRRDATA;

// --------------------------------
// RUNNING OBJECT DATA STRUCTURE
// --------------------------------
// If you want to store anything between actions/conditions/expressions
// you should store it here

typedef struct tagRDATA
{
	#include "MagicRDATA.h"

	int				xAngle;	
	int				yAngle;	
	int				zAngle;

	//int				xLight;
	//int				yLight;	
	//int				zLight;

	float			specular;
	float			ambient;

	//char *				voxelPath;
	vxl_runtime_part **	voxelParts;
	char				n_parts;
	//char *			palettePath;

	//voxel_t			voxel;
	//int				frame;
	matrix_t		lMatrix;
	float			sinus[3];
	float			cosine[3];
	short *			surfaceDepthGrid;
	char *			screenDot;
	bool*			screenRender;
	bool			enableShading;
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
