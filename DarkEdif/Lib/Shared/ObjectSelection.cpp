// DarkEdif Object Selection by Darkwire Software.
// Based on Edif edit by James McLaughlin, C++ wrapper by Anders Riggelsen (Andos), based on Select Object original by Dynasoft.
// https://www.andersriggelsen.dk/mmf2_objectselection.php
// https://community.clickteam.com/forum/thread/61672-for-developers-objectselection-framework/?postID=460665#post460665

#include "Common.hpp"
#include "DarkEdif.hpp"
#include "ObjectSelection.hpp"

namespace DarkEdif
{

// Static definition, the size of objInfoList struct.
// The DarkEdif library is Fusion 2.0 and 2.5 compatible; which is only relevant in Windows,
// and adjusts the struct for objInfoList.
// Based on which Fusion runtime is used (2.0, HWA, 2.5) runtime, and the #defines for the SDK,
// e.g. whether the ext itself is HWA-happy, the size is calculated.
// In non-Windows, it is assumed you are using 2.5, since 2.0 Android/iOS exporter was discontinued over a decade ago.
#if defined(_WIN32)
int ObjectSelection::oiListItemSize = -1;
#endif

ObjectSelection::ObjectSelection(RunHeader * rhPtr)
{
	// These are frame-wide, so can be shared among Extension,
	// but note there is multiple CRun if there are sub-apps
	this->rhPtr = rhPtr;

	// For iOS, this isn't fully initialised when objects are created on start of frame, e.g. CRun->CEventProgram->QualToOiList is null

#ifdef _WIN32
	// Fusion object list - one for each object instance, rhObjectList
	// This is pre-reserved to rhPtr->MaxObjects size, but rhPtr->NObjects reflects how many are in use;
	// this is non-contiguous if objects are destroyed, and uses nulls
	ObjectList = rhPtr->get_ObjectList();

	// Fusion object info list - static, one per object, rhOiList.
	// You can loop all object instances with oiList->oilObject and curHo->hoNumNext,
	// and loop selected with oiList->oilListSelected and curHo->hoNextSelected;
	// those variables reflect the index in rhObjectList.
	OiList = rhPtr->GetOIListByIndex(0);

	// Fusion qualifer array; static
	// Qualifier -> OiList[] - static, one per qualifier, but any number of OiList
	// CRun's rhQualToOiList, or EventProgram's qualToOiList, depending on runtime
	QualToOiList = rhPtr->GetQualToOiListByOffset(0);

	// Singleton init - don't bother setting if already inited
	if (ObjectSelection::oiListItemSize != -1)
		return;
	ObjectSelection::oiListItemSize = sizeof(objInfoList);

	// Only add the sizes to the runtime structures if they weren't compiled directly for those runtimes
	#ifndef _UNICODE
		if ( rhPtr->rh4.rh4Mv->CallFunction(NULL, CallFunctionIDs::ISUNICODE, 0, 0, 0) )
			ObjectSelection::oiListItemSize += 24; // objInfoList::name is built in ext as char[24], but app is using wchar_t[24]
	#endif
	// SDK is set up to support HWA, but runtime is not using HWA, so oi structs are smaller than expected
	#ifdef HWABETA
		if (! rhPtr->rh4.rh4Mv->CallFunction(NULL, CallFunctionIDs::ISHWA, 0, 0, 0) )
			ObjectSelection::oiListItemSize -= sizeof(LPVOID);
	#else
		// SDK is not set up to support HWA, but runtime is using HWA, so oi structs are bigger than expected
		if (rhPtr->rh4.rh4Mv->CallFunction(NULL, CallFunctionIDs::ISHWA, 0, 0, 0))
			ObjectSelection::oiListItemSize += sizeof(LPVOID);
	#endif
#endif
}

//Selects *all* objects of the given object-type
void ObjectSelection::SelectAll(short Oi) const
{
	objInfoList * ObjectInfo = GetOILFromOI(Oi);

	// set object listing to all-selected for this event
	ObjectInfo->set_NumOfSelected(ObjectInfo->get_NObjects());
	ObjectInfo->set_ListSelected(ObjectInfo->get_Object());
	ObjectInfo->set_EventCount(rhPtr->GetRH2EventCount());

	// Set object link chain to all selected
	for (short i = ObjectInfo->get_Object(); i >= 0;)
	{
		auto Object = rhPtr->GetObjectListOblOffsetByIndex(i)->get_rHo();
		Object->set_NextSelected(Object->get_NumNext());
		i = Object->get_NumNext();
	}
}

//Resets all objects of the given object-type
void ObjectSelection::SelectNone(short oiList) const
{
	for (auto oil : DarkEdif::QualifierOIListIterator(rhPtr, oiList, DarkEdif::Selection::Implicit))
		oil->SelectNone(rhPtr);
}

//Resets all objects of the given object-type
void ObjectSelection::SelectNone(RunObject& object) const
{
	HeaderObject* const ho = object.get_rHo();
	objInfoList* const objInfo = ho->get_OiList();

	// Store that 0 are selected, the first is invalid, and selection is for this event
	objInfo->set_NumOfSelected(0);
	objInfo->set_ListSelected(-1);
	objInfo->set_EventCount(rhPtr->GetRH2EventCount());

	// Go to first object instance of this type, and make sure its next selected is invalid too
	// There must be at least one obj instance, because we're passing it
	
	const RunObjectMultiPlatPtr firstObj = rhPtr->GetObjectListOblOffsetByIndex(ho->get_Number());
	HeaderObject * const firstObjHo = firstObj->get_rHo();
	firstObjHo->set_NextSelected(-1);
	// TODO: I think this last part is necessary for qualifiers, or OR events, but confirm it
	// possibly with a hardware breakpoint?
}

//Resets the SOL and inserts only one given object
void ObjectSelection::SelectOneObject(RunObject& object) const
{
	HeaderObject * const ho = object.get_rHo();
	objInfoList * const objInfo = ho->get_OiList();

	// Set only one selected, that it is this obj instance, and selection is valid for this event
	objInfo->set_NumOfSelected(1);
	objInfo->set_ListSelected(ho->get_Number());
	objInfo->set_EventCount(rhPtr->GetRH2EventCount());
	
	// Set next object in selection chain to none
	ho->set_NextSelected(-1);
}

// Resets the SOL and inserts the given list of objects
void ObjectSelection::SelectObjects(short Oi, RunObjectMultiPlatPtr * objects, std::size_t count) const
{
	if ((long)count <= 0)
		return;

	objInfoList* ObjectInfo = GetOILFromOI(Oi);

	// Set count of selected
	ObjectInfo->set_NumOfSelected((int)count);
	// Set selection list as valid for this event
	ObjectInfo->set_EventCount(rhPtr->GetRH2EventCount());

	// Set first selected instance to first passed
	short prevNumber = objects[0]->get_rHo()->get_Number();
	ObjectInfo->set_ListSelected(prevNumber);

	// For each remaining object instance, store them as next in selection list
	for (std::size_t i = 1; i < count; ++i)
	{
		short currentNumber = objects[i]->get_rHo()->get_Number();
		rhPtr->GetObjectListOblOffsetByIndex(prevNumber)->get_rHo()->set_NextSelected(currentNumber);
		prevNumber = currentNumber;
	}
	// For the last selected object, set next instance to be none
	rhPtr->GetObjectListOblOffsetByIndex(prevNumber)->get_rHo()->set_NextSelected(-1);
}

// Return the number of explicitly selected objects for the given object-type
std::size_t ObjectSelection::GetNumberOfSelected(short oiList) const
{
	if (oiList == -1)
		return 0;

	std::size_t numberSelected = 0;
	for (auto oiL : DarkEdif::QualifierOIListIterator(rhPtr, oiList, DarkEdif::Selection::Explicit))
		numberSelected += oiL->get_NumOfSelected();
	return numberSelected;
}

bool ObjectSelection::ObjectIsOfType(RunObject &object, short Oi) const
{
	if (Oi & 0x8000)
	{
		const std::vector<short> ois = rhPtr->GetQualToOiListByOffset(Oi & 0x7FFF)->GetAllOi();
		return std::find(ois.cbegin(), ois.cend(), object.get_rHo()->get_Oi()) != ois.cend();
	}

	return (object.get_rHo()->get_Oi() == Oi);
}


// Returns the object-info structure from a given object-type
objInfoList * ObjectSelection::GetOILFromOI(short Oi) const
{
	LOGV(_T("GetOILFromOI: moving an Oi %hi to index.\n"), Oi);
	for (auto oil : DarkEdif::AllOIListIterator(rhPtr))
	{
		if (oil->get_Oi() == Oi)
			return oil;
	}
	return nullptr;
}

template<class Ext, class T> bool ObjectSelection::FilterQualifierObjects(short oiList, bool negate, T(Ext::* filterFunction))
{
	bool hasSelected = false;
	for (auto oil : QualifierOIListIterator(rhPtr, oiList, Selection::All)) {
		hasSelected |= FilterNonQualifierObjects(oil->get_Oi(), negate, filterFunction);
		if (!hasSelected)
			return true;
	}
	return hasSelected;
}

template<class Ext, class T> bool ObjectSelection::FilterNonQualifierObjects(short oiList, bool negate, T(Ext::* filterFunction))
{
	auto&& pObjectInfo = rhPtr->GetOIListByIndex(oiList);
	bool hasSelected = false;

	if (pObjectInfo->get_EventCount() != rhPtr->GetRH2EventCount())
		SelectAll(oiList);	//The SOL is invalid, must reset.

	//If SOL is empty
	if (pObjectInfo->get_NumOfSelected() <= 0)
		return false;

	int firstSelected = -1;
	int count = 0;
	int current = pObjectInfo->get_ListSelected();

	RunObjectMultiPlatPtr previous = nullptr;

	while (current >= 0)
	{
		auto pObject = rhPtr->GetObjectListOblOffsetByIndex(current);
		bool useObject = DoCallback((void*)pExtension, filterFunction, (RunObject*)&*pObject);

		if (negate)
			useObject = !useObject;

		hasSelected |= useObject;

		if (useObject)
		{
			if (firstSelected == -1)
				firstSelected = current;

			if (previous != nullptr)
				previous->get_rHo()->set_NextSelected(current);

			previous = std::move(pObject);
			count++;
		}
		current = pObject->get_rHo()->get_NextSelected();
	}
	if (previous != nullptr)
		previous->get_rHo()->set_NextSelected(-1);

	pObjectInfo->set_ListSelected(firstSelected);
	pObjectInfo->set_NumOfSelected(count);

	return hasSelected;
}

// Iterates the object instances in qualifier OI or singular OI
ObjectIterator::ObjectIterator(RunHeader* rhPtr, short oiList, Selection selection, bool includeDestroyed /* = false */)
		: rhPtr(rhPtr), oiList(oiList), select(selection), includeDestroyed(includeDestroyed), curOiList(oiList)
{
	LOGV(_T("Object Iterator created on oilist index %hi.\n"), oiList);
	if (curOiList != -1)
	{
		if ((curOiList & 0x8000) == 0)
			qualOiList.push_back(oiList);
		else
		{
			auto qToOiList = rhPtr->GetQualToOiListByOffset(curOiList);
			if (qToOiList)
				qualOiList = qToOiList->GetAllOiList();
		}
		GetNext();
	}
}
ObjectIterator& ObjectIterator::operator++() {
	if (oil == nullptr || curHo == nullptr)
		return *this;
	GetNext();
	return *this;
}
// x++, instead of ++x
ObjectIterator ObjectIterator::operator++(int) {
	auto retval = *this; ++(*this); return retval;
}
bool ObjectIterator::operator==(ObjectIterator other) const {
	return curHo == other.curHo && oiList == other.oiList;
}
bool ObjectIterator::operator!=(ObjectIterator other) const {
	return !(*this == other);
}
ObjectIterator::reference ObjectIterator::operator*() {
	return curRo;
}
void ObjectIterator::GetNext()
{
	while (true)
	{
		++numNextRun;
		short nextObjNum = -1;
		if (curHo)
		{
			// if all: rely on object/numnext
			// if implicit: rely on list/nextselected; if no entries (ecmatch false), go to object/numnext
			// if explicit: rely on list/nextselected

			const bool ecMatch2 = oil->get_EventCount() == rhPtr->GetRH2EventCount();
			nextObjNum = select == Selection::All ? curHo->get_NumNext() : curHo->get_NextSelected();

			// If implicit selection and no condition-selection, select first OI in general
			if ((!ecMatch2 || (nextObjNum & 0x8000) != 0) && select == Selection::Implicit)
				nextObjNum = curHo->get_NumNext();
		}
		// Invalid Oi; jump to next object
		if ((nextObjNum & 0x8000) != 0)
		{
			if (qualOiList.size() > qualOiListAt) {
				curOiList = qualOiList[qualOiListAt++];
				oil = rhPtr->GetOIListByIndex(curOiList);

				const bool ecMatch = oil->get_EventCount() == rhPtr->GetRH2EventCount();
				nextObjNum = select == Selection::All ? oil->get_Object() : oil->get_ListSelected();

				// If implicit selection and no condition-selection, select first OI in general
				if ((!ecMatch || (nextObjNum & 0x8000) != 0) && select == Selection::Implicit)
					nextObjNum = oil->get_Object();

				// Skip to next frame: it'll either fall through to end of list, or grab next in qualifier
				if ((nextObjNum & 0x8000) != 0)
					continue;
			}
			// hit end of list
			else
			{
				curHo = nullptr;
				curRo = nullptr;
				oil = nullptr;
				curOiList = -1;
				numNextRun = SIZE_MAX;
				return;
			}
		}
		curRo = rhPtr->GetObjectListOblOffsetByIndex(nextObjNum);
		curHo = curRo ? curRo->get_rHo() : nullptr;
		if (curRo && (includeDestroyed || (curHo->get_Flags() & HeaderObjectFlags::Destroyed) == HeaderObjectFlags::None))
			return; // we got a valid one
	}
}
ObjectIterator::ObjectIterator(RunHeader* rhPtr, short oiList, Selection select, bool destroy, bool) :
	rhPtr(rhPtr), oiList(oiList), select(select), includeDestroyed(destroy), numNextRun(SIZE_MAX)
{
	// curOiList and oil already inited to empty, numNextRun is set to end already
}
ObjectIterator ObjectIterator::begin() const {
	return ObjectIterator(rhPtr, oiList, select, includeDestroyed);
}
ObjectIterator ObjectIterator::end() const {
	return ObjectIterator(rhPtr, oiList, select, includeDestroyed, false);
}

QualifierOIListIterator::QualifierOIListIterator(RunHeader* rhPtr, short oiList, Selection select)
	: rhPtr(rhPtr), oiList(oiList), select(select), curOiList(oiList)
{
	LOGV(_T("QualifierOIListIterator created on oilist index %hi.\n"), oiList);
	if (curOiList == -1)
		return;
	// Not a qualifier OI, make an iterator that only returns it
	if ((curOiList & 0x8000) == 0)
		qualOiList.push_back(curOiList);
	else
	{
		auto qToOiList = rhPtr->GetQualToOiListByOffset(curOiList);
		if (qToOiList)
			qualOiList = qToOiList->GetAllOiList();

		LOGI(_T("QualifierOIListIterator created on oilist index %hi.\n"), oiList);
	}

	GetNext();
}
QualifierOIListIterator& QualifierOIListIterator::operator++()
{
	if (oil == nullptr)
		return *this;
	GetNext();
	return *this;
}
// x++, instead of ++x
QualifierOIListIterator QualifierOIListIterator::operator++(int) {
	auto retval = *this; ++(*this); return retval;
}
bool QualifierOIListIterator::operator==(const QualifierOIListIterator& other) const {
	return curOiList == other.curOiList && oiList == other.oiList;
}
bool QualifierOIListIterator::operator!=(const QualifierOIListIterator& other) const { return !(*this == other); }
QualifierOIListIterator::reference QualifierOIListIterator::operator*() const { return oil; }
void QualifierOIListIterator::GetNext()
{
	for (short firstOi; qualOiListAt < qualOiList.size();)
	{
		curOiList = qualOiList[qualOiListAt++]; // always increment; end iterator is indicated by == length
		oil = rhPtr->GetOIListByIndex(curOiList);
		const bool ecMatch = oil->get_EventCount() == rhPtr->GetRH2EventCount();
		firstOi = select == Selection::All ? oil->get_Object() : oil->get_ListSelected();

		// If implicit selection and no condition-selection, select first OI in general
		if ((!ecMatch || firstOi == -1) && select == Selection::Implicit)
			firstOi = oil->get_Object();
		if ((firstOi & 0x8000) == 0)
			return; // invalid flag were not set, so this OiList has some valid instances; else continue loop and find next oilist
	}

	// End of list, clear up
	oil = nullptr;
	curOiList = -1;
}
QualifierOIListIterator::QualifierOIListIterator(RunHeader* rhPtr, short oiList, Selection select, bool) :
	rhPtr(rhPtr), oiList(oiList), select(select) {} // curOiList and oil already inited to empty
QualifierOIListIterator QualifierOIListIterator::begin() const { return QualifierOIListIterator(rhPtr, oiList, select); }
QualifierOIListIterator QualifierOIListIterator::end() const { return QualifierOIListIterator(rhPtr, oiList, select, false); }

AllObjectIterator::AllObjectIterator(RunHeader* rhPtr, bool) :
	rhPtr(rhPtr), numObjectsInFrame(rhPtr->get_NObjects()), objListAt(numObjectsInFrame)
{

}
AllObjectIterator::AllObjectIterator(RunHeader* rhPtr) :
	rhPtr(rhPtr), numObjectsInFrame(rhPtr->get_NObjects())
{
	// In theory there cannot be 0 objects, since this code can't be run without an object to run it
	if (numObjectsInFrame > 0)
		curRo = rhPtr->GetObjectListOblOffsetByIndex(0);
}

AllObjectIterator& AllObjectIterator::operator++() {
	if (curRo == nullptr)
		return *this;

	// haven't hit end of rhObjectList, but NObjects is how many valid entries are in rhObjectList,
	// so just abort early, rather than loop past all the reserved nulls in rhObjectList
	if (++objListAt == numObjectsInFrame)
	{
		curRo = nullptr;
		return *this;
	}

	while (true)
	{
		// This can be null, even before maxObjectsInFrame is reached, if object is removed
		curRo = rhPtr->GetObjectListOblOffsetByIndex(++objListTrueIndex);
		if (curRo != nullptr)
			break;
		// skip nulls in middle of rhObjectList
	}
	return *this;
}
// x++, instead of ++x
AllObjectIterator AllObjectIterator::operator++(int) { auto retval = *this; ++(*this); return retval; }
bool AllObjectIterator::operator==(AllObjectIterator other) const { return curRo == other.curRo; }
bool AllObjectIterator::operator!=(AllObjectIterator other) const { return !(*this == other); }
AllObjectIterator::reference AllObjectIterator::operator*() const { return curRo; }

AllObjectIterator AllObjectIterator::begin() const { return AllObjectIterator(rhPtr); }
AllObjectIterator AllObjectIterator::end() const { return AllObjectIterator(rhPtr, false); }

AllOIListIterator::AllOIListIterator(RunHeader* rhPtr, bool) :
	rhPtr(rhPtr), numOI(rhPtr->GetNumberOi()), oiListAt(numOI)
{

}
// Iterator for all the OI List in entire frame
AllOIListIterator::AllOIListIterator(RunHeader* rhPtr) :
	rhPtr(rhPtr), numOI(rhPtr->GetNumberOi())
{
	if (numOI > 0)
		oil = rhPtr->GetOIListByIndex(0);
}

AllOIListIterator& AllOIListIterator::operator++() {
	if (oil == nullptr)
		return *this;
	if (++oiListAt == numOI)
		oil = nullptr;
	else
		oil = rhPtr->GetOIListByIndex(oiListAt);
	return *this;
}
// x++, instead of ++x
AllOIListIterator AllOIListIterator::operator++(int) { auto retval = *this; ++(*this); return retval; }
bool AllOIListIterator::operator==(AllOIListIterator other) const { return oiListAt == other.oiListAt; }
bool AllOIListIterator::operator!=(AllOIListIterator other) const { return !(*this == other); }
AllOIListIterator::reference AllOIListIterator::operator*() const { return oil; }

AllOIListIterator AllOIListIterator::begin() const { return AllOIListIterator(rhPtr); }
AllOIListIterator AllOIListIterator::end() const { return AllOIListIterator(rhPtr, false); }

} // namespace DarkEdif
