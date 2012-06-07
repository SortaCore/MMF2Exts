#ifndef COLLECTION_OF_2DPOINTS_DATA
#define COLLECTION_OF_2DPOINTS_DATA
struct Linked2DPoint{

	int x;
	int y;

	Linked2DPoint* nextPoint;

};

struct CollectionOf2DPointsData
{
	//#include "MagicRDATA.h"

	headerObject rHo;
	rCom rc;
			rSpr rs;
	LPRRDATA rRd;
	Linked2DPoint* firstPoint;
	Linked2DPoint* lastPoint;
	Linked2DPoint* currentPointOnLoop;
	int count;
	
};
#endif