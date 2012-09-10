
// By Anders Riggelsen (Andos)
// http://www.clickteam.com/epicenter/ubbthreads.php?ubb=showflat&Number=214148&gonew=1

// These files do not match up with the originals - modified for Edif (callbacks inside the extension class, etc..)

#include "Common.h"

Riggs::ObjectSelection::ObjectSelection(RunHeader * rhPtr)
{
	this->rhPtr = rhPtr;
	this->ObjectList = rhPtr->ObjectList;		//get a pointer to the mmf object list
	this->OiList = rhPtr->OiList;				//get a pointer to the mmf object info list
	this->QualToOiList = rhPtr->QualToOiList;	//get a pointer to the mmf qualifier to Oi list
	oiListItemSize = sizeof(objInfoList);

//Only add the sizes to the runtime structures if they weren't compiled directly for those runtimes
	#ifndef UNICODE
		if ( rhPtr->rh4.rh4Mv->CallFunction(NULL, EF_ISUNICODE, 0, 0, 0) )
			oiListItemSize += 24;
	#endif
	#ifndef HWABETA
		if ( rhPtr->rh4.rh4Mv->CallFunction(NULL, EF_ISHWA, 0, 0, 0) )
			oiListItemSize += sizeof(LPVOID);
	#endif
}

//Selects *all* objects of the given object-type
void Riggs::ObjectSelection::SelectAll(short Oi)
{
	objInfoList * const ObjectInfo = GetOILFromOI(Oi);

	ObjectInfo->NumOfSelected = ObjectInfo->NObjects;
	ObjectInfo->ListSelected = ObjectInfo->Object;
	ObjectInfo->EventCount = rhPtr->rh2.EventCount;

	int i = ObjectInfo->Object;
	while(i >= 0)
	{
		HeaderObject * Object = ObjectList[i].oblOffset;
		Object->NextSelected = Object->NumNext;
		i = Object->NumNext;
	}
}

//Resets all objects of the given object-type
void Riggs::ObjectSelection::SelectNone(short Oi)
{
	objInfoList * ObjectInfo = GetOILFromOI(Oi);

	ObjectInfo->NumOfSelected = 0;
	ObjectInfo->ListSelected = -1;
	ObjectInfo->EventCount = rhPtr->rh2.EventCount;
}

//Resets the SOL and inserts only one given object
void Riggs::ObjectSelection::SelectOneObject(RunObject * object)
{
	objInfoList * ObjectInfo = GetOILFromOI(object->roHo.Oi);

	ObjectInfo->NumOfSelected = 1;
	ObjectInfo->EventCount = rhPtr->rh2.EventCount;
	ObjectInfo->ListSelected = object->roHo.Number;
	ObjectList[object->roHo.Number].oblOffset->NextSelected = -1;
}

//Resets the SOL and inserts the given list of objects
void Riggs::ObjectSelection::SelectObjects(short Oi, RunObject ** objects, int count)
{
	if (count <= 0)
		return;

	objInfoList * const ObjectInfo = GetOILFromOI(Oi);

	ObjectInfo->NumOfSelected = count;
	ObjectInfo->EventCount = rhPtr->rh2.EventCount;
	
	short prevNumber = objects[0]->roHo.Number;
	ObjectInfo->ListSelected = prevNumber;
	
	for(int i=1; i<count; i++)
	{
		short currentNumber = objects[i]->roHo.Number;
		ObjectList[prevNumber].oblOffset->NextSelected = currentNumber;
		prevNumber = currentNumber;
	}
	ObjectList[prevNumber].oblOffset->NextSelected = -1;
}

//Return the number of selected objects for the given object-type
int Riggs::ObjectSelection::GetNumberOfSelected(short Oi)
{
	if (Oi & 0x8000)
	{
		Oi &= 0x7FFF;	//Mask out the qualifier part
		int numberSelected = 0;

		qualToOi * CurrentQualToOiStart = (qualToOi *)((char*)QualToOiList + Oi);
		qualToOi * CurrentQualToOi = CurrentQualToOiStart;

		while(CurrentQualToOi->OiList >= 0)
		{
			objInfoList * CurrentOi = GetOILFromOI(CurrentQualToOi->OiList);
			numberSelected += CurrentOi->NumOfSelected;
			CurrentQualToOi = (qualToOi *)((char*)CurrentQualToOi + 4);
		}
		return numberSelected;
	}
	else
	{
		return GetOILFromOI(Oi)->NumOfSelected;
	}
}

bool Riggs::ObjectSelection::ObjectIsOfType(RunObject * object, short Oi)
{
	if (Oi & 0x8000)
	{
		Oi &= 0x7FFF;	//Mask out the qualifier part
		qualToOi * CurrentQualToOiStart = (qualToOi *)((char*)QualToOiList + Oi);
		qualToOi * CurrentQualToOi = CurrentQualToOiStart;

		while(CurrentQualToOi->OiList >= 0)
		{
			objInfoList * CurrentOi = GetOILFromOI(CurrentQualToOi->OiList);
			if (CurrentOi->Oi == object->roHo.Oi)
				return true;
			CurrentQualToOi = (qualToOi *)((char*)CurrentQualToOi + 4);
		}
		return false;
	}
	else
		return (object->roHo.Oi == Oi);
}


//Returns the object-info structure from a given object-type
objInfoList * Riggs::ObjectSelection::GetOILFromOI(short Oi)
{
	for(int i = 0; i < rhPtr->NumberOi; ++i)
	{
		objInfoList * oil = (objInfoList *)(((char*)OiList) + oiListItemSize*i);
		if (oil->Oi == Oi)
			return oil;
	}
	return NULL;
}

