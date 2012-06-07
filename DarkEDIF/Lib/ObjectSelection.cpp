
// By Anders Riggelsen (Andos)
// http://www.clickteam.com/epicenter/ubbthreads.php?ubb=showflat&Number=214148&gonew=1

// These files do not match up with the originals - modified for Edif (callbacks inside the extension class, etc..)

#include "Common.h"

Riggs::ObjectSelection::ObjectSelection(LPRH rhPtr)
{
	this->rhPtr = rhPtr;
	this->ObjectList = rhPtr->rhObjectList;		//get a pointer to the mmf object list
	this->OiList = rhPtr->rhOiList;				//get a pointer to the mmf object info list
	this->QualToOiList = rhPtr->rhQualToOiList;	//get a pointer to the mmf qualifier to Oi list
	oiListItemSize = sizeof(objInfoList);

//Only add the sizes to the runtime structures if they weren't compiled directly for those runtimes
#ifndef UNICODE
	if ( rhPtr->rh4.rh4Mv->mvCallFunction(NULL, EF_ISUNICODE, 0, 0, 0) )
		oiListItemSize += 24;
#endif
#ifndef HWABETA
	if ( rhPtr->rh4.rh4Mv->mvCallFunction(NULL, EF_ISHWA, 0, 0, 0) )
		oiListItemSize += sizeof(LPVOID);
#endif
}

//Selects *all* objects of the given object-type
void Riggs::ObjectSelection::SelectAll(short Oi)
{
	const LPOIL pObjectInfo = GetOILFromOI(Oi);

	pObjectInfo->oilNumOfSelected = pObjectInfo->oilNObjects;
	pObjectInfo->oilListSelected = pObjectInfo->oilObject;
	pObjectInfo->oilEventCount = rhPtr->rh2.rh2EventCount;

	int i = pObjectInfo->oilObject;
	while(i >= 0)
	{
		LPHO pObject = ObjectList[i].oblOffset;
		pObject->hoNextSelected = pObject->hoNumNext;
		i = pObject->hoNumNext;
	}
}

//Resets all objects of the given object-type
void Riggs::ObjectSelection::SelectNone(short Oi)
{
	LPOIL pObjectInfo = GetOILFromOI(Oi);

	pObjectInfo->oilNumOfSelected = 0;
	pObjectInfo->oilListSelected = -1;
	pObjectInfo->oilEventCount = rhPtr->rh2.rh2EventCount;
}

//Resets the SOL and inserts only one given object
void Riggs::ObjectSelection::SelectOneObject(LPRO object)
{
	LPOIL pObjectInfo = GetOILFromOI(object->roHo.hoOi);

	pObjectInfo->oilNumOfSelected = 1;
	pObjectInfo->oilEventCount = rhPtr->rh2.rh2EventCount;
	pObjectInfo->oilListSelected = object->roHo.hoNumber;
	ObjectList[object->roHo.hoNumber].oblOffset->hoNextSelected = -1;
}

//Resets the SOL and inserts the given list of objects
void Riggs::ObjectSelection::SelectObjects(short Oi, LPRO* objects, int count)
{
	if(count <= 0)
		return;

	LPOIL pObjectInfo = GetOILFromOI(Oi);

	pObjectInfo->oilNumOfSelected = count;
	pObjectInfo->oilEventCount = rhPtr->rh2.rh2EventCount;
	
	short prevNumber = objects[0]->roHo.hoNumber;
	pObjectInfo->oilListSelected = prevNumber;
	
	for(int i=1; i<count; i++)
	{
		short currentNumber = objects[i]->roHo.hoNumber;
		ObjectList[prevNumber].oblOffset->hoNextSelected = currentNumber;
		prevNumber = currentNumber;
	}
	ObjectList[prevNumber].oblOffset->hoNextSelected = -1;
}

//Return the number of selected objects for the given object-type
int Riggs::ObjectSelection::GetNumberOfSelected(short Oi)
{
	if(Oi & 0x8000)
	{
		Oi &= 0x7FFF;	//Mask out the qualifier part
		int numberSelected = 0;

		LPQOI CurrentQualToOiStart = (LPQOI)((char*)QualToOiList + Oi);
		LPQOI CurrentQualToOi = CurrentQualToOiStart;

		while(CurrentQualToOi->qoiOiList >= 0)
		{
			LPOIL CurrentOi = GetOILFromOI(CurrentQualToOi->qoiOiList);
			numberSelected += CurrentOi->oilNumOfSelected;
			CurrentQualToOi = (LPQOI)((char*)CurrentQualToOi + 4);
		}
		return numberSelected;
	}
	else
	{
		LPOIL pObjectInfo = GetOILFromOI(Oi);
		return pObjectInfo->oilNumOfSelected;
	}
}

bool Riggs::ObjectSelection::ObjectIsOfType(LPRO object, short Oi)
{
	if(Oi & 0x8000)
	{
		Oi &= 0x7FFF;	//Mask out the qualifier part
		LPQOI CurrentQualToOiStart = (LPQOI)((char*)QualToOiList + Oi);
		LPQOI CurrentQualToOi = CurrentQualToOiStart;

		while(CurrentQualToOi->qoiOiList >= 0)
		{
			LPOIL CurrentOi = GetOILFromOI(CurrentQualToOi->qoiOiList);
			if(CurrentOi->oilOi == object->roHo.hoOi)
				return true;
			CurrentQualToOi = (LPQOI)((char*)CurrentQualToOi + 4);
		}
		return false;
	}
	else
		return (object->roHo.hoOi == Oi);
}


//Returns the object-info structure from a given object-type
LPOIL Riggs::ObjectSelection::GetOILFromOI(short Oi)
{
	for(int i=0; i<rhPtr->rhNumberOi; ++i)
	{
		LPOIL oil = (LPOIL)(((char*)OiList) + oiListItemSize*i);
		if(oil->oilOi == Oi)
			return oil;
	}
	return NULL;
}

