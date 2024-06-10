#include "Common.hpp"
#include "Edif.hpp"
#include "DarkEdif.hpp"
#include "ObjectSelection.hpp"
#ifdef __APPLE__
#include "MMF2Lib/CQualToOiList.h"
#include "MMF2Lib/CRunApp.h"
#endif

struct EdifGlobal
{
	TCHAR name[256];
	void * Value;

	EdifGlobal * Next;
};

void objInfoList::SelectNone(RunHeader* rhPtr) {
	set_NumOfSelected(0);
	set_ListSelected(-1);
	set_EventCount(rhPtr->GetRH2EventCount());
}

void objInfoList::SelectAll(RunHeader* rhPtr, bool explicitAll /* = false */) {
	if (!explicitAll)
	{
		set_NumOfSelected(0);
		set_ListSelected(-1);
		set_EventCount(-5); // does not match rh2EventCount, so implictly selects all
		return;
	}

	set_NumOfSelected(get_NObjects());
	set_ListSelected(get_Object());
	set_EventCount(rhPtr->GetRH2EventCount());
	if (get_NObjects() == 0)
		return;
	const short ourOiList = rhPtr->GetObjectListOblOffsetByIndex(get_Object())->get_rHo()->get_Number();
	for (auto ho : DarkEdif::ObjectIterator(rhPtr, ourOiList, DarkEdif::Selection::All, false))
		ho->get_rHo()->set_NextSelected(ho->get_rHo()->get_NumNext());
}

std::vector<short> qualToOi::HalfVector(std::size_t first)
{
	std::vector<short> list;
#ifdef _WIN32
	for (std::size_t i = first; OiAndOiList[i] != -1; i += 2)
		list.push_back(OiAndOiList[i]);
#elif defined(__ANDROID__)
	if (!OiAndOiList)
		get_OiList(0);
	for (std::size_t i = first; i < OiAndOiListLength; i += 2)
		list.push_back(OiAndOiList[i]);
#else // apple
	const short* const qoiList = ((CQualToOiList*)this)->qoiList;
	for (std::size_t i = first; qoiList[i] != -1; i += 2)
		list.push_back(qoiList[i]);
#endif
	return list;
}
std::vector<short> qualToOi::GetAllOiList() {
	return HalfVector(1);
}
std::vector<short> qualToOi::GetAllOi() {
	return HalfVector(0);
}

CRunAppMultiPlat* CRunAppMultiPlat::get_ParentApp() {
#ifdef _WIN32
	return ParentApp;
#elif defined(__APPLE__)
	return (CRunAppMultiPlat*)((CRunApp*)this)->parentApp;
#else // Android
	if (!parentApp && !parentAppIsNull)
	{
		// Application/CRunApp parentApp
		jfieldID fieldID = threadEnv->GetFieldID(meClass, "parentApp", "LApplication/CRunApp;");
		JNIExceptionCheck();
		jobject appJava = threadEnv->GetObjectField(me, fieldID);
		JNIExceptionCheck();
		if (appJava)
			parentApp = std::make_unique<CRunAppMultiPlat>(appJava, runtime);
		else
			parentAppIsNull = true;
	}
	return parentApp.get();
#endif
}

std::size_t CRunAppMultiPlat::GetNumFusionFrames() {
#ifdef _WIN32
	return hdr.NbFrames;
#elif defined(__APPLE__)
	return (std::size_t)((CRunApp*)this)->gaNbFrames;
#else // Android
	if (numTotalFrames == 0)
	{
		jfieldID fieldID = threadEnv->GetFieldID(meClass, "gaNbFrames", "I");
		JNIExceptionCheck();
		jint totalFrames = threadEnv->GetIntField(me, fieldID);
		JNIExceptionCheck();
		numTotalFrames = (std::size_t)totalFrames;
	}
	return numTotalFrames;
#endif
}

#ifdef _WIN32
Edif::Runtime::Runtime(Extension * ext) : hoPtr(ext->rdPtr->get_rHo()),
	ObjectSelection(hoPtr->get_AdRunHeader())
{
}

Edif::Runtime::~Runtime()
{
}

void Edif::Runtime::Rehandle()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::REHANDLE, 0, 0);
}

static int steadilyIncreasing = 0;
void Edif::Runtime::GenerateEvent(int EventID)
{
	auto rhPtr = hoPtr->AdRunHeader;
	// If there is a fastloop in progress, generating an event inside an action will alter
	// Action Count, making it smaller. Thus Action Count constantly decrementing will create
	// an infinite loop, as Action Count never reaches the maximum needed to end the loop.
	// This bug usually shows when creating 2+ of an object inside a fastloop when the fastloop runs 3+ times.
	const int oldActionCount = rhPtr->GetRH2ActionCount();
	const int oldActionLoopCount = rhPtr->GetRH2ActionLoopCount();

	// This action count won't be reset, so to allow multiple of the same event with different object selection,
	// we change the count every time, and in an increasing manner.
	rhPtr->SetRH2ActionCount(oldActionCount + (++steadilyIncreasing));
	rhPtr->SetRH2ActionLoopCount(0);

	// Saving tokens allows events to be run from inside expressions
	// https://community.clickteam.com/forum/thread/108993-application-crashed-in-some-cases-when-calling-an-event-via-expression/?postID=769763#post769763
	// As of CF2.5 build 293.9, this is done by runtime anyway, but if you want to
	// support Fusion 2.0, and you're generating events from expressions, you should include this
	expression* const saveExpToken = rhPtr->rh4.ExpToken;

	// Fix event group being incorrect after event finishes.
	// This being incorrect doesn't have any major effects, as the event parsing part of
	// runtime sets rhEventGroup based on a local variable evgPtr, which it relies on instead
	eventGroup* const evg = rhPtr->EventGroup;

	CallRunTimeFunction2(hoPtr, RFUNCTION::GENERATE_EVENT, EventID, 0);

	rhPtr->EventGroup = evg;
	rhPtr->rh4.ExpToken = saveExpToken;
	rhPtr->SetRH2ActionCount(oldActionCount);
	rhPtr->SetRH2ActionLoopCount(oldActionLoopCount);
}

void Edif::Runtime::PushEvent(int EventID)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::PUSH_EVENT, EventID, 0);
}

void * Edif::Runtime::Allocate(size_t size)
{
	return (void *) CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, size * sizeof(TCHAR));
}

TCHAR * Edif::Runtime::CopyString(const TCHAR * String)
{
	TCHAR * New = NULL;
	New = (TCHAR *) Allocate(_tcslen(String) + 1);
	_tcscpy(New, String);

	return New;
}

char * Edif::Runtime::CopyStringEx(const char * String)
{
	char * New = NULL;
	New = (char *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, (strlen(String) + 1) * sizeof(char));
	strcpy(New, String);

	return New;
}


wchar_t * Edif::Runtime::CopyStringEx(const wchar_t * String)
{
	wchar_t * New = NULL;
	New = (wchar_t *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, (wcslen(String) + 1) * sizeof(wchar_t));
	wcscpy(New, String);

	return New;
}


void Edif::Runtime::Pause()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::PAUSE, 0, 0);
}

void Edif::Runtime::Resume()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::CONTINUE, 0, 0);
}

void Edif::Runtime::Redisplay()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::REDISPLAY, 0, 0);
}

void Edif::Runtime::GetApplicationDrive(TCHAR * Buffer)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DRIVE, (long) Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DIR, (long) Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::PATH, (long) Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::APP_NAME, (long) Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::TEMP_PATH, (long) Buffer);
}

void Edif::Runtime::Redraw()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::REDRAW, 0, 0);
}

void Edif::Runtime::Destroy()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::DESTROY, 0, 0);
}

void Edif::Runtime::ExecuteProgram(ParamProgram * Program)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::EXECUTE_PROGRAM, 0, (long) Program);
}

long Edif::Runtime::EditInteger(EditDebugInfo * EDI)
{
	return CallRunTimeFunction2(hoPtr, RFUNCTION::EDIT_INT, 0, (long) EDI);
}

long Edif::Runtime::EditText(EditDebugInfo * EDI)
{
	return CallRunTimeFunction2(hoPtr, RFUNCTION::EDIT_TEXT, 0, (long) EDI);
}

void Edif::Runtime::CallMovement(int ID, long Parameter)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::CALL_MOVEMENT, ID, Parameter);
}

void Edif::Runtime::SetPosition(int X, int Y)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::SET_POSITION, X, Y);
}

CallTables * Edif::Runtime::GetCallTables()
{
	return (CallTables *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_CALL_TABLES, 0, 0);
}

bool Edif::Runtime::IsHWACapableRuntime()
{
	// The old IsHWA() function. This returns true if it's a HWA-capable runtime, even if
	// the app is using Standard display mode. This function is necessary as HWA runtimes still allocate
	// extra RAM for the unused HWA features, moving the offsets and sizes of graphics-related classes
	//
	// Consider using GetAppDisplayMode() >= SurfaceDriver::Direct3D9 for "HWA with shaders" detection.
	// Note Direct3D8 is greater than Direct3D9.

	return hoPtr->AdRunHeader->rh4.rh4Mv->CallFunction(NULL, CallFunctionIDs::ISHWA, 0, 0, 0) == 1;
}

SurfaceDriver Edif::Runtime::GetAppDisplayMode()
{
	static SurfaceDriver sd = SurfaceDriver::Max;
	if (sd == SurfaceDriver::Max)
	{
		// Using GAOF_XX flags
		//constexpr int GAOF_DDRAW = 0x0002;
		//constexpr int GAOF_DDRAWVRAM = 0x0004;
		//constexpr int GAOF_D3D9 = 0x4000;
		//constexpr int GAOF_D3D8 = 0x8000;
		constexpr int GAOF_DDRAWEITHER = GAOF_DDRAW | GAOF_DDRAWVRAM;
		constexpr int GAOF_D3D11 = GAOF_D3D9 | GAOF_D3D8;

		// No need to find parent-most app; subapps have same OtherFlags
		const int i = hoPtr->AdRunHeader->App->hdr.OtherFlags;

		if ((i & GAOF_DDRAWEITHER) != 0)
			sd = SurfaceDriver::DirectDraw;
		else if ((i & GAOF_D3D11) == GAOF_D3D11)
			sd = SurfaceDriver::Direct3D11;
		else if ((i & GAOF_D3D8) != 0)
			sd = SurfaceDriver::Direct3D8;
		else if ((i & GAOF_D3D9) != 0)
			sd = SurfaceDriver::Direct3D9;
		else if ((i & (GAOF_D3D11 | GAOF_DDRAWEITHER)) == 0)
			sd = SurfaceDriver::Bitmap;
		else
			throw std::runtime_error("Unrecognised display mode");
	}
	return sd;
}
#endif // _WIN32

bool Edif::Runtime::IsUnicode()
{
#ifdef _WIN32
	return hoPtr->AdRunHeader->rh4.rh4Mv->CallFunction(NULL, CallFunctionIDs::ISUNICODE, 0, 0, 0) == 1;
#else
	// At this point in DarkEdif dev, Unicode is assumed in non-Windows runtime, as it's assumed to be CF2.5.
	// This is because the Android exporter in Fusion 2.0 only went to Android OS v4.3, API 18,
	// which ended in 2013 and was before 64-bit phones were even introduced in API 21.
	return true;
#endif
}

#ifdef _WIN32
event2 &Edif::Runtime::CurrentEvent()
{
	return *(event2 *) (((char *) param1) - CND_SIZE);
}
#endif // WIN32

std::size_t AltVals::GetAltValueCount() const {
#ifdef _WIN32
	return DarkEdif::IsFusion25 ? CF25.NumAltValues : 26;
#else
throw std::runtime_error("not implemented");
#endif
}
std::size_t AltVals::GetAltStringCount() const {
#ifdef _WIN32
	return DarkEdif::IsFusion25 ? CF25.NumAltStrings : 26;
#else
	throw std::runtime_error("not implemented");
#endif
}
const TCHAR* AltVals::GetAltStringAtIndex(const std::size_t i) const {
	if (i >= GetAltStringCount())
		return nullptr;
#ifdef _WIN32
	const TCHAR* c = DarkEdif::IsFusion25 ? CF25.Strings[i] : MMF2.rvStrings[i];
	if (c && c[0] == _T('\0')) // Fusion uses null and blank the same, we merge
		c = nullptr;
#else
	const TCHAR* c = nullptr;
	throw std::runtime_error("not implemented");
#endif
	return c;
}
const CValueMultiPlat* AltVals::GetAltValueAtIndex(const std::size_t i) const {
#ifdef _WIN32
	return DarkEdif::IsFusion25 ? &CF25.Values[i] : &MMF2.rvpValues[i];
#else
	throw std::runtime_error("Not implemented");
#endif
}

void AltVals::SetAltStringAtIndex(const std::size_t i, const std::tstring_view& str) {
	if (i >= GetAltStringCount())
	{
		if (!DarkEdif::IsFusion25)
			return LOGE(_T("Cannot set alt string at index %zu, invalid index."), i);

		// TODO: if this does not work, just throw
#ifdef _WIN32
		void * v = mvReAlloc(Edif::SDK->mV, (void *)CF25.Strings, (int)i * sizeof(TCHAR *));
#else
		void* v = malloc(i * sizeof(TCHAR*));
#endif
		if (!v)
			return LOGF(_T("Failed to expand alt strings to %zu entries."), i);
#ifdef _WIN32
		*(void **)&CF25.Strings = v; // TODO: Simplify
		CF25.NumAltStrings = (int)i;
#else
		throw std::runtime_error("Not implemented");
#endif
	}
#ifdef _WIN32
	// TODO: is ** needed
	const TCHAR **c = DarkEdif::IsFusion25 ? &CF25.Strings[i] : &MMF2.rvStrings[i];
	if (*c)
		mvFree(Edif::SDK->mV, (void *)*c);
	if (str.empty())
		*c = nullptr;
	else
	{
		TCHAR * t = (TCHAR *) mvMalloc(Edif::SDK->mV, (str.size() + 1) * sizeof(TCHAR));
		memcpy(t, str.data(), str.size() * sizeof(TCHAR));
		t[str.size()] = _T('\0');
		*c = t;
	}
#else
	throw std::runtime_error("Not implemented");
#endif
}
void AltVals::SetAltValueAtIndex(const std::size_t, const double)
{
	throw std::runtime_error("Not implemented");
}
void AltVals::SetAltValueAtIndex(const std::size_t, const int)
{
	throw std::runtime_error("Not implemented");
}
std::uint32_t AltVals::GetInternalFlags() const {
#ifdef _WIN32
	return DarkEdif::IsFusion25 ? CF25.InternalFlags : MMF2.rvValueFlags;
#else
	throw std::runtime_error("not implemented");
#endif
}
void AltVals::SetInternalFlags(const std::uint32_t nf) {
#ifdef _WIN32
	if (DarkEdif::IsFusion25)
		CF25.InternalFlags = nf;
	else
		MMF2.rvValueFlags = nf;
#else
	throw std::runtime_error("not implemented");
#endif
}

RunObjectMultiPlatPtr Edif::Runtime::RunObjPtrFromFixed(int fixedvalue)
{
	int index = fixedvalue & 0x0000FFFF;

	if (index < 0 || (size_t)index >= hoPtr->get_AdRunHeader()->get_MaxObjects())
		return NULL;

	auto theObject = hoPtr->get_AdRunHeader()->GetObjectListOblOffsetByIndex(index);
	if (theObject == NULL || FixedFromRunObjPtr(theObject) != fixedvalue)
		return NULL;
	return theObject;
}

int Edif::Runtime::FixedFromRunObjPtr(RunObjectMultiPlatPtr object)
{
	if (object != NULL)
		return object->get_rHo()->GetFixedValue();
	return 0;
}

void Edif::Runtime::CancelRepeatingObjectAction()
{
	// The RunObject * passed to a Object action parameter points to the first instance of the selected objects
	// that this action has.
	// Normal behaviour is the runtime will call the action repeatedly, once for each object instance.
	// So you just run as if the RunObject * is the only instance it's being run on, and the Fusion runtime
	// will cycle through each RunObject * that is the object instances passed by the event.
	// Otherwise, you disable ACTFLAGS_REPEAT to prevent the internal loop.
	//
	// For conditions, Fusion passes Object directly as ParamObject * instead, which DarkEdif reads the oiList from
	// and passes that to the Ext::Condition function; any looping you want to do is to be done using that oiList,
	// e.g. by using a DarkEdif::ObjectIterator() or rhPtr->AdRunHeader->GetOIListByIndex()
	auto as = hoPtr->get_AdRunHeader()->GetRH4ActionStart();
	as->set_evtFlags(as->get_evtFlags() & ~ACTFLAGS_REPEAT);
}

int HeaderObject::GetFixedValue() {
	return (get_CreationId() << 16) | get_Number();
}

#if defined(_WIN32)
short Edif::Runtime::GetOIFromObjectParam(std::size_t paramIndex)
{
	const EventParam* curParam = ParamZero;
	for (std::size_t i = 0; i < paramIndex; ++i)
		curParam = (const EventParam*)((const std::uint8_t*)curParam + curParam->size);
	if ((Params)curParam->Code != Params::Object)
		LOGE(_T("GetOIFromExtParam: Returning a OI for a non-Object parameter.\n"));
	LOGV(_T("GetOIFromExtParam: Returning OiList %hi, oi is %hi.\n"), curParam->evp.W[0], curParam->evp.W[1]);
	return curParam->evp.W[0];
}
int Edif::Runtime::GetCurrentFusionFrameNumber()
{
	// First Fusion frame is 0, so make 1-based
	return 1 + hoPtr->AdRunHeader->App->nCurrentFrame;
}

// Gets the RH2 event count, used in object selection
int RunHeader::GetRH2EventCount()
{
	return this->rh2.EventCount;
}
// Gets the RH4 event count for OR, used in object selection in OR-related events.
int RunHeader::GetRH4EventCountOR()
{
	return this->rh4.EventCountOR;
}

// Reads the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
int RunHeader::GetRH2ActionCount()
{
	return this->rh2.ActionCount;
}
// Reads the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
int RunHeader::GetRH2ActionLoopCount()
{
	return this->rh2.ActionLoopCount;
}

// Sets the rh2.rh2ActionCount variable, used in an action with multiple instances selected, to repeat one action.
void RunHeader::SetRH2ActionCount(int newActionCount)
{
	rh2.ActionCount = newActionCount;
}
// Sets the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions in an event.
void RunHeader::SetRH2ActionLoopCount(int newActLoopCount)
{
	rh2.ActionLoopCount = newActLoopCount;
}

objectsList* RunHeader::get_ObjectList() {
	return ObjectList;
}
objInfoList* RunHeader::GetOIListByIndex(std::size_t index)
{
	assert(DarkEdif::ObjectSelection::oiListItemSize != 0 || index == 0); // first time is index 0
	assert((std::size_t)NumberOi > index); // invalid OI

	return (objInfoList*)(((std::uint8_t*)OiList) + DarkEdif::ObjectSelection::oiListItemSize * index);
}
event2* RunHeader::GetRH4ActionStart() {
	return rh4.ActionStart;
}

bool RunHeader::GetRH2ActionOn() {
	return rh2.ActionOn != 0;
}
eventGroup* RunHeader::get_EventGroup() {
	return EventGroup;
}
std::size_t RunHeader::GetNumberOi() {
	return NumberOi - 1; // Windows runtime includes an invalid Oi at the end of list as part of count
}
qualToOi* RunHeader::GetQualToOiListByOffset(std::size_t byteOffset) {
	return (qualToOi*)(((std::uint8_t*)QualToOiList) + (byteOffset & 0x7FFF));
}
RunObjectMultiPlatPtr RunHeader::GetObjectListOblOffsetByIndex(std::size_t index) {
	return ObjectList[index].oblOffset;
}

EventGroupFlags RunHeader::GetEVGFlags() {
	return this->EventGroup->evgFlags;
}
std::size_t RunHeader::get_MaxObjects() {
	return this->MaxObjects;
}
std::size_t RunHeader::get_NObjects() {
	return this->NObjects;
}
CRunAppMultiPlat* RunHeader::get_App() {
	return (CRunAppMultiPlat*)App;
}

short HeaderObject::get_NextSelected() {
	return this->NextSelected;
}
unsigned short HeaderObject::get_CreationId() {
	return this->CreationId;
}
short HeaderObject::get_Number() {
	return this->Number;
}
short HeaderObject::get_NumNext() {
	return this->NumNext;
}
short HeaderObject::get_Oi() {
	return this->Oi;
}
objInfoList* HeaderObject::get_OiList() {
	return this->OiList;
}
bool HeaderObject::get_SelectedInOR() {
	return this->SelectedInOR;
}
HeaderObjectFlags HeaderObject::get_Flags() {
	return this->Flags;
}
RunHeader* HeaderObject::get_AdRunHeader() {
	return AdRunHeader;
}

void HeaderObject::set_NextSelected(short ns) {
	NextSelected = ns;
}
void HeaderObject::set_SelectedInOR(bool b) {
	SelectedInOR = b;
}
short event2::get_evtNum() {
	return evtNum;
}
OINUM event2::get_evtOi() {
	return evtOi;
}
/*short event2::get_evtSize() {
	return evtSize;
}*/
std::int8_t event2::get_evtFlags() {
	return evtFlags;
}
void event2::set_evtFlags(std::int8_t evtF) {
	evtFlags = evtF;
}
event2* event2::Next() {
	return (event2*)(((char*)this) + evtSize);
}
int event2::GetIndex() {
	//if (DarkEdif::IsFusion25)
	//	return this->evtInhibit;
	return this->evtIdentifier;
}

HeaderObject* RunObject::get_rHo() {
	return (HeaderObject *)&rHo;
}
rCom* RunObject::get_roc() {
	if (!this || (rHo.OEFlags & (OEFLAGS::MOVEMENTS | OEFLAGS::ANIMATIONS | OEFLAGS::SPRITES)) == OEFLAGS::NONE)
		return nullptr;
	return &roc;
}
rMvt* RunObject::get_rom() {
	if (!this || (rHo.OEFlags & OEFLAGS::MOVEMENTS) == OEFLAGS::NONE)
		return nullptr;
	return &rom;
}
rAni* RunObject::get_roa() {
	if (!this || (rHo.OEFlags & OEFLAGS::ANIMATIONS) == OEFLAGS::NONE)
		return nullptr;

	return (rAni *)((char*)this + roc.rcOffsetAnimation);
}
Sprite* RunObject::get_ros() {
	if (!this || (rHo.OEFlags & OEFLAGS::SPRITES) == OEFLAGS::NONE)
		return nullptr;
	char* c = ((char*)&roc) + sizeof(rCom);
	if ((rHo.OEFlags & OEFLAGS::MOVEMENTS) == OEFLAGS::MOVEMENTS)
		c += sizeof(rMvt);
	if ((rHo.OEFlags & OEFLAGS::ANIMATIONS) == OEFLAGS::ANIMATIONS)
		c += sizeof(rAni);
	return (Sprite*)c;
	//return (Sprite*)((char*)rHo + roc.rcOffsetSprite);
}
AltVals* RunObject::get_rov() {
	if (!this || (rHo.OEFlags & OEFLAGS::VALUES) != OEFLAGS::VALUES)
		return nullptr;
	return (AltVals*)(((char*)this) + rHo.OffsetValue);
}
RunObjectMultiPlatPtr objectsList::GetOblOffsetByIndex(std::size_t index) {
	return this[index].oblOffset;
}

int objInfoList::get_EventCount() {
	return EventCount;
}
short objInfoList::get_ListSelected() {
	return ListSelected;
}
int objInfoList::get_NumOfSelected() {
	return NumOfSelected;
}
short objInfoList::get_Oi() {
	return Oi;
}
int objInfoList::get_NObjects() {
	return NObjects;
}
short objInfoList::get_Object() {
	return Object;
}
const TCHAR* objInfoList::get_name() {
	return name;
}
int objInfoList::get_oilNext() {
	return Next;
}
bool objInfoList::get_oilNextFlag() {
	return NextFlag;
}
decltype(objInfoList::CurrentRoutine) objInfoList::get_oilCurrentRoutine() {
	return CurrentRoutine;
}
int objInfoList::get_oilCurrentOi() {
	return CurrentOi;
}
int objInfoList::get_oilActionCount() {
	return ActionCount;
}
int objInfoList::get_oilActionLoopCount() {
	return ActionLoopCount;
}
void objInfoList::set_NumOfSelected(int ns) {
	NumOfSelected = ns;
}
void objInfoList::set_ListSelected(short sh) {
	ListSelected = sh;
}
void objInfoList::set_EventCount(int ec) {
	EventCount = ec;
}
void objInfoList::set_EventCountOR(int ec) {
	EventCountOR = ec;
}
short objInfoList::get_QualifierByIndex(std::size_t index) {
	return Qualifiers[index];
}
short qualToOi::get_Oi(std::size_t idx) {
	return OiAndOiList[idx * 2];
}
short qualToOi::get_OiList(std::size_t idx) {
	return OiAndOiList[idx * 2 + 1];
}

std::uint8_t eventGroup::get_evgNCond() {
	return evgNCond;
}
std::uint8_t eventGroup::get_evgNAct() {
	return evgNAct;
}
std::uint16_t eventGroup::get_evgIdentifier() {
	return evgIdentifier;
}
std::uint16_t eventGroup::get_evgInhibit() {
	return evgInhibit;
}

event2 * eventGroup::GetCAByIndex(size_t index)
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (index >= (size_t)(evgNCond + evgNAct))
		return nullptr;

	event2 * ret = (event2*)(&this[1]);
	for (size_t i = 0; i < index && ret; i++) {
		ret = ret->Next();
	}
	return ret;
}


extern HINSTANCE hInstLib;

void Edif::Runtime::WriteGlobal(const TCHAR * name, void * Value)
{
	RunHeader * rhPtr = hoPtr->AdRunHeader;

	while (rhPtr->App->ParentApp)
		rhPtr = rhPtr->App->ParentApp->Frame->rhPtr;

	EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->GetExtUserData(rhPtr->App, hInstLib);

	if (!Global)
	{
		Global = new EdifGlobal;

		_tcscpy(Global->name, name);
		Global->Value = Value;

		Global->Next = 0;

		rhPtr->rh4.rh4Mv->SetExtUserData(rhPtr->App, hInstLib, Global);

		return;
	}

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
		{
			Global->Value = Value;
			return;
		}

		if (!Global->Next)
			break;

		Global = Global->Next;
	}

	Global->Next = new EdifGlobal;
	Global = Global->Next;

	_tcscpy(Global->name, name);

	Global->Value = Value;
	Global->Next = 0;
}

void * Edif::Runtime::ReadGlobal(const TCHAR * name)
{
	RunHeader * rhPtr = hoPtr->AdRunHeader;

	while (rhPtr->App->ParentApp)
		rhPtr = rhPtr->App->ParentApp->Frame->rhPtr;

	EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->GetExtUserData(rhPtr->App, hInstLib);

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
			return Global->Value;

		Global = Global->Next;
	}

	return NULL;
}

#else  // !_WIN32

// We use this in both Android and iOS. It uses stack memory for text up to 1kb,
// and heap memory for anything else, reusing the memory for every string expression.
//
// This would be considered unsafe in nested A/C/E scenarios,
// i.e. expression("3rd party object", expression1("this obj"), expression2("this obj"))
// but the Obj-C/Java wrapper has to copy out the UTF-8 text anyway, so we don't have
// to worry about expression1 becoming 2 or memory freed too early.

static char * lastHeapRet;
static char stackRet[1024];
static char zero[4];
TCHAR * Edif::Runtime::CopyString(const TCHAR * String) {
	if (!String[0])
		return zero;

	const size_t len = strlen(String) + 1;
	if (len < sizeof(stackRet))
	{
		strcpy(stackRet, String);
		LOGV("Returning text on stack: %p moved to stack %p \"%s\".\n", String, stackRet, stackRet);
		return stackRet;
	}
	char * temp = (char *)realloc(lastHeapRet, len);
	if (temp == NULL)
	{
		free(lastHeapRet);
		lastHeapRet = NULL;
		LOGE("Ran out of memory allocating %zu bytes for string returning \"%.20s...\"!\n", len, String);
		strcpy(stackRet, "Out of memory! See logcat.");
		return stackRet;
	}
	LOGV("Returning text on heap: %p moved to heap %p \"%s\".\n", String, temp, temp);
	strcpy(temp, String);
	return (lastHeapRet = temp);
}

#if defined(__ANDROID__)

Edif::Runtime::Runtime(Extension* ext, jobject javaExtPtr2) :
	javaExtPtr(javaExtPtr2, "Edif::Runtime::javaExtPtr from Edif::Runtime ctor"), ObjectSelection(NULL)
{
	std::string exc;
	javaExtPtrClass = global(mainThreadJNIEnv->GetObjectClass(javaExtPtr), "Extension::javaExtPtrClass from Extension ctor");
	if (javaExtPtrClass.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaExtPtrClass, got exception %s.\n", exc.c_str());
	}

	jfieldID javaHoField = mainThreadJNIEnv->GetFieldID(javaExtPtrClass, "ho", "LObjects/CExtension;");
	if (javaHoField == NULL) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaHoField, got exception %s.\n", exc.c_str());
	}

	javaHoObject = global(mainThreadJNIEnv->GetObjectField(javaExtPtr, javaHoField), "Extension::javaHoObject from Extension ctor");
	if (javaHoObject.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaHoObject, got exception %s.\n", exc.c_str());
	}

	javaHoClass = global(mainThreadJNIEnv->GetObjectClass(javaHoObject), "Extension::javaHoClass from Extension ctor");
	if (javaHoClass.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not find javaHoClass method, got exception %s.\n", exc.c_str());
	}

	javaCEventClass = global(mainThreadJNIEnv->FindClass("Events/CEvent"), "CEvent class");
	if (javaCEventClass.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not find javaCEventClass method, got exception %s.\n", exc.c_str());
	}

	ext->rdPtr = std::make_shared<RunObject>(javaHoObject.ref, javaHoClass.ref, this);
	this->hoPtr = ext->rdPtr->get_rHo();
	ext->rhPtr = hoPtr->get_AdRunHeader();
	this->ObjectSelection.rhPtr = ext->rhPtr;
}

void Edif::Runtime::InvalidateByNewACE()
{
	ObjectSelection.rhPtr->InvalidatedByNewGeneratedEvent();
	hoPtr->InvalidatedByNewGeneratedEvent();
}


#define GenEdifFunction(x) \
	static jmethodID javaMethodID = threadEnv->GetMethodID(javaHoClass, x, "()V"); \
	if (javaMethodID == NULL) {\
		std::string exc = GetJavaExceptionStr(); \
		LOGE("Could not find %s method, got exception %s.\n", x, exc.c_str()); \
	} \
	else \
		threadEnv->CallVoidMethod(javaHoObject, javaMethodID);

Edif::Runtime::~Runtime()
{
}

extern thread_local JNIEnv * threadEnv;
void Edif::Runtime::Rehandle()
{
	static jmethodID javaMethodID = threadEnv->GetMethodID(javaHoClass.ref, "reHandle", "()V");
	threadEnv->CallVoidMethod(javaHoObject.ref, javaMethodID);
	// GenEdifFunction reHandle
}

static int steadilyIncreasing = 0;
void Edif::Runtime::GenerateEvent(int EventID)
{
	const auto& rhPtr = this->ObjectSelection.pExtension->rhPtr;

	// If there is a fastloop in progress, generating an event inside an action will alter
	// Action Count, making it smaller. Thus Action Count constantly decrementing will create
	// an infinite loop, as Action Count is kept lower than its maximum.
	// This bug usually shows when creating 2+ of an object inside a fastloop when the fastloop runs 3+ times.
	const int oldActionCount = rhPtr->GetRH2ActionCount();
	const int oldActionLoopCount = rhPtr->GetRH2ActionLoopCount();

	// This action count won't be reset, so to allow multiple of the same event with different object selection,
	// we change the count every time, and in an increasing manner.
	rhPtr->SetRH2ActionCount(oldActionCount + (++steadilyIncreasing));
	rhPtr->SetRH2ActionLoopCount(0);

	// In older Fusion builds, the expression token became invalidated and that was the only problem.
	// In Windows GenerateEvent, you can see the workaround of saving and restoring the pointer to old expression.
	// Android also swaps out the underlying array when it changes expression, so we have to restore the array too.
	// Due to Android's JVM use of references, this is efficient as we don't have to do a full array clone.

	const int rh4CurToken = rhPtr->GetRH4CurToken();
	jobjectArray rh4Tokens = rhPtr->GetRH4Tokens();
	if (rh4Tokens)
		rh4Tokens = (jobjectArray)threadEnv->NewGlobalRef(rh4Tokens);

	// Fix event group being incorrect after event finishes.
	// This being incorrect doesn't have any major effects, as the event parsing part of
	// runtime sets this rhEventGroup based on a local variable evgPtr, which it relies on instead
	// We won't be using this while we're off running this event, so we can swap the reference out to a local
	std::unique_ptr<eventGroup> evg = rhPtr->eventProgram ? std::move(rhPtr->eventProgram->eventGrp) : nullptr;

	// Cached variables to do with object selection will be invalidated by this new event
	rhPtr->InvalidatedByNewGeneratedEvent();
	hoPtr->InvalidatedByNewGeneratedEvent();

	static jmethodID javaMethodID = threadEnv->GetMethodID(javaHoClass.ref, "generateEvent", "(II)V");
	threadEnv->CallVoidMethod(javaHoObject, javaMethodID, EventID, 0);

	rhPtr->SetRH2ActionCount(oldActionCount);
	rhPtr->SetRH2ActionLoopCount(oldActionLoopCount);
	rhPtr->SetRH4CurToken(rh4CurToken);
	rhPtr->SetRH4Tokens(rh4Tokens);
	if (rh4Tokens)
		threadEnv->DeleteGlobalRef(rh4Tokens);
	if (evg)
		rhPtr->eventProgram->eventGrp = std::move(evg); // and swap it back in
}

void Edif::Runtime::PushEvent(int EventID)
{
	// Cached variables to do with object selection will be invalidated by this new event,
	// but this event won't run until later, so we don't need to immediately invalidate them.

	jmethodID javaMethodID = threadEnv->GetMethodID(javaHoClass, "pushEvent", "(II)V");
	threadEnv->CallVoidMethod(javaHoObject, javaMethodID, EventID, 0);
}

void * Edif::Runtime::Allocate(size_t size)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: Allocate missing.
	// return (void *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, size * sizeof(TCHAR));
	return NULL;
}

// Dummy functions. The conversion to Modified-UTF8 happens in JStrToCStr, inside Edif::Expression's return.
char * Edif::Runtime::CopyStringEx(const char * String) {
	return CopyString(String);
}
wchar_t * Edif::Runtime::CopyStringEx(const wchar_t * String) {
	throw std::runtime_error("Do not use wchar_t in Android!");
	return (wchar_t *)String;
}

JNIEnv * Edif::Runtime::AttachJVMAccessForThisThread(const char * threadName, bool asDaemon)
{
	const auto thisThreadID = std::this_thread::get_id();
	if (threadEnv != nullptr)
	{
		LOGF("Thread ID %s already has JNI access.\n", ThreadIDToStr(thisThreadID).c_str());
		return nullptr;
	}

	pthread_setname_np(pthread_self(), threadName);

	JavaVMAttachArgs args = {
		.name = threadName,
		.group = NULL,
		.version = JNI_VERSION_1_6
	};

	// Daemon means the JVM won't keep the app running if this thread is still alive.
	// Do you want main thread exiting to choose whether the app is running or not?
	jint error;
	if (asDaemon)
		error = global_vm->AttachCurrentThreadAsDaemon(&threadEnv, &args);
	else
		error = global_vm->AttachCurrentThread(&threadEnv, &args);
	if (error != JNI_OK)
	{
		LOGF("Couldn't attach thread %s (ID %s) to JNI, AttachCurrentThread%s error %i.\n",
			threadName, ThreadIDToStr(thisThreadID).c_str(), asDaemon ? "AsDaemon" : "", error);
		return nullptr;
	}
	LOGV("Attached thread %s (ID %s) to JNI.\n", threadName, ThreadIDToStr(thisThreadID).c_str());
	JNIExceptionCheck();
	return threadEnv;
}
void Edif::Runtime::DetachJVMAccessForThisThread()
{
	const auto thisThreadID = std::this_thread::get_id();
	if (threadEnv == nullptr)
	{
		LOGF("Can't detach JVM access, thread ID %s already doesn't have JNI access.\n", ThreadIDToStr(thisThreadID).c_str());
		return;
	}
	JNIExceptionCheck();
	const jint error = global_vm->DetachCurrentThread();
	if (error != JNI_OK)
	{
		LOGF("Couldn't detach thread ID %s from JNI, DetachCurrentThread error %i.\n", ThreadIDToStr(thisThreadID).c_str(), error);
		return;
	}
	LOGV("Detached thread ID %s from JNI OK.\n", ThreadIDToStr(thisThreadID).c_str());

	threadEnv = nullptr;
}
JNIEnv * Edif::Runtime::GetJNIEnvForThisThread()
{
	return threadEnv;
}


void Edif::Runtime::Pause()
{
	GenEdifFunction("pause");
}

void Edif::Runtime::Resume()
{
	GenEdifFunction("resume");
}

void Edif::Runtime::Redisplay()
{
	GenEdifFunction("redisplay");
}

void Edif::Runtime::GetApplicationDrive(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DRIVE, (long)Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DIR, (long)Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::PATH, (long)Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::APP_NAME, (long)Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
}

void Edif::Runtime::Redraw()
{
	GenEdifFunction("redraw");
}

void Edif::Runtime::Destroy()
{
	GenEdifFunction("destroy");
}

void Edif::Runtime::CallMovement(int ID, long Parameter)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// See ~line 592, CExtension.java
	// GenEdifFunction("redisplay");
	//CallRunTimeFunction2(hoPtr, RFUNCTION::CALL_MOVEMENT, ID, Parameter);
}

void Edif::Runtime::SetPosition(int X, int Y)
{
	jmethodID javaMethodID = threadEnv->GetMethodID(javaExtPtrClass, "setPosition", "(II)V");
	threadEnv->CallVoidMethod(javaExtPtr, javaMethodID, X, Y);
}

static EdifGlobal * staticEdifGlobal; // LB says static/global values are functionally equivalent to getUserExtData, so... yay.

void Edif::Runtime::WriteGlobal(const TCHAR * name, void * Value)
{
	EdifGlobal * Global = (EdifGlobal *)staticEdifGlobal;

	if (!Global)
	{
		Global = new EdifGlobal;

		_tcscpy(Global->name, name);
		Global->Value = Value;
		Global->Next = NULL;

		staticEdifGlobal = Global;

		return;
	}

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
		{
			Global->Value = Value;
			return;
		}

		if (!Global->Next)
			break;

		Global = Global->Next;
	}

	Global->Next = new EdifGlobal;
	Global = Global->Next;

	_tcscpy(Global->name, name);

	Global->Value = Value;
	Global->Next = 0;
}

/*
// This code may be used by globals, if we ever re-integrate JNI method with getStorage and whatnot
static char * GetObjectNameWithPackage(const char * name)
{
	static bool gotten = false;
	static std::string packageName = "";
	if (!gotten)
	{
		jclass javaExtPtrClass = threadEnv->GetObjectClass(javaExtPtr);
		jmethodID getClassNameMethod = threadEnv->GetMethodID(javaExtPtrClass, "getName", "()Ljava/lang/String;");
		jstring className = (jstring)threadEnv->CallObjectMethod(javaExtPtrClass, getClassNameMethod);
		const char * classNameCPtr = threadEnv->GetStringUTFChars(className, NULL);
		packageName = classNameCPtr;
		threadEnv->ReleaseStringUTFChars(className, classNameCPtr);
		gotten = true;
	}

	static char newName[256];
	sprintf(newName, "L%s/%s;", packageName.c_str(), name);
	return newName;
}*/

void * Edif::Runtime::ReadGlobal(const TCHAR * name)
{
	/*
	// Access CRun from CRunExtension::rh
	// Access CRunApp from CRun::rhApp
	// Access CRunApp from CRunApp::parentApp
	// Access CRun again from CRunApp::run

	jclass javaExtPtrClass = threadEnv->GetObjectClass(javaExtPtr);
	jclass CRunClass = threadEnv->FindClass(GetObjectNameWithPackage("CRun"));
	jclass CRunAppClass = threadEnv->FindClass(GetObjectNameWithPackage("CRunApp"));

	jfieldID CRunExtension_rh_fieldID = threadEnv->GetFieldID(javaExtPtrClass, "rh", GetObjectNameWithPackage("CRun"));
	jfieldID CRun_rhApp_fieldID = threadEnv->GetFieldID(CRunClass, "rhApp", GetObjectNameWithPackage("CRunApp"));
	jfieldID CRunApp_parentApp_fieldID = threadEnv->GetFieldID(CRunAppClass, "parentApp", GetObjectNameWithPackage("CRunApp"));
	jfieldID CRunApp_run_fieldID = threadEnv->GetFieldID(CRunAppClass, "run", GetObjectNameWithPackage("CRunApp"));

	jobject cRun = threadEnv->GetObjectField(javaExtPtr, CRunExtension_rh_fieldID);
	jobject cRunApp = threadEnv->GetObjectField(javaExtPtr, CRun_rhApp_fieldID);

	for (jobject cRunAppTemp = cRunApp; cRunAppTemp != NULL; )
	{
		cRunApp = cRunAppTemp;
		cRun = threadEnv->GetObjectField(cRunApp, CRunApp_run_fieldID);

		cRunAppTemp = threadEnv->GetObjectField(cRunApp, CRunApp_parentApp_fieldID);
	}
	char methodParams[256];

	sprintf(methodParams, "(%sI)V", GetObjectNameWithPackage("EdifGlobal"));
	jmethodID CRun_addStorage_methodID = threadEnv->GetMethodID(CRunClass, "addStorage", methodParams);

	sprintf(methodParams, "(I)%s", GetObjectNameWithPackage("EdifGlobal"));
	jmethodID CRun_getStorage_methodID = threadEnv->GetMethodID(CRunClass, "getStorage", methodParams);

	jmethodID CRun_delStorage_methodID = threadEnv->GetMethodID(CRunClass, "delStorage", "(I)V");

	int id = 'EDIF';

	jobject edifData = threadEnv->CallObjectMethod(cRun, CRun_getStorage_methodID, id);
	*/
	EdifGlobal * Global = staticEdifGlobal;

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
			return Global->Value;

		Global = Global->Next;
	}

	return NULL;
}

/**
 * JNI/C++: Get Class Name
 * @param env [in] JNI context
 * @param myCls [in] Class object, which the name is asked of
 * @param fullpath [in] true for full class path, else name without package context
 * @return Name of class myCls, encoding UTF-8
 */
const char* getClassName(jclass myCls, bool fullpath)
{
	static std::string res;

	jclass ccls = threadEnv->FindClass("java/lang/Class");
	jmethodID mid_getName = threadEnv->GetMethodID(ccls, "getName", "()Ljava/lang/String;");
	jstring strObj = (jstring)threadEnv->CallObjectMethod(myCls, mid_getName);
	const char* localName = threadEnv->GetStringUTFChars(strObj, 0);
	res = localName;
	threadEnv->ReleaseStringUTFChars(strObj, localName);
	if (!fullpath)
	{
		std::size_t pos = res.find_last_of('.');
		if (pos != std::string::npos)
			res = res.substr(pos + 1);
	}
	return res.c_str();
}

short Edif::Runtime::GetOIFromObjectParam(std::size_t paramIndex)
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (*(long *)&paramIndex < 0)
		raise(SIGINT);
	// read evtPtr.evtParams[paramIndex] as PARAM_OBJECT
	static jfieldID evtParamsFieldID = mainThreadJNIEnv->GetFieldID(this->javaCEventClass.ref, "evtParams", "[LParams/CParam;");
	JNIExceptionCheck();
	jobjectArray evtParams = (jobjectArray)mainThreadJNIEnv->GetObjectField(curCEvent, evtParamsFieldID);
	JNIExceptionCheck();
	jobject thisParam = mainThreadJNIEnv->GetObjectArrayElement(evtParams, paramIndex);
	JNIExceptionCheck();
#ifdef _DEBUG
	jint pParamCode = mainThreadJNIEnv->GetShortField(thisParam, mainThreadJNIEnv->GetFieldID(mainThreadJNIEnv->GetObjectClass(thisParam), "code", "S"));
	JNIExceptionCheck();
	if ((Params)pParamCode != Params::Object)
		LOGE(_T("GetOIFromExtParam: Returning a OI for a non-Object parameter.\n"));
#endif
	// Read the equivalent of evp.W[0]
	jshort oiList = mainThreadJNIEnv->GetShortField(thisParam, mainThreadJNIEnv->GetFieldID(mainThreadJNIEnv->GetObjectClass(thisParam), "oiList", "S"));
	JNIExceptionCheck();
#if defined(_DEBUG) && (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_DEBUG)
	jshort oi = mainThreadJNIEnv->GetShortField(thisParam, mainThreadJNIEnv->GetFieldID(mainThreadJNIEnv->GetObjectClass(thisParam), "oi", "S"));
	JNIExceptionCheck();
	LOGD(_T("GetOIFromExtParam: Returning OiList %hi (%#04hx; non-qual: %hi, %#04hx), oi is %hi (%#04hx; non-qual: %hi, %#04hx).\n"),
		oiList, oiList, (short)(oiList & 0x7FFF), (short)(oiList & 0x7FFF), oi, oi, (short)(oi & 0x7FFF), (short)(oi & 0x7FFF));
#endif
	return oiList;
}

int Edif::Runtime::GetCurrentFusionFrameNumber()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	return 1 + hoPtr->get_AdRunHeader()->get_App()->get_nCurrentFrame();
}

// Edit this to monitor specific jobject/jclass references. End with null.
const char * globalToMonitor[1] = { NULL };

// Gets the RH2 event count, used in object selection
int RunHeader::GetRH2EventCount()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	// CEventProgram rh2EventCount
	// CRun stores this as rhEvtProg

	return get_EventProgram()->get_rh2EventCount();
}
// Gets the RH2 event count, used in object selection
void RunHeader::SetRH2EventCount(int newEventCount)
{
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), newEventCount);
	// CEventProgram rh2EventCount
	// CRun stores this as rhEvtProg

	return get_EventProgram()->set_rh2EventCount(newEventCount);
}
// Gets the RH4 event count for OR, used in object selection in OR-related events.
int RunHeader::GetRH4EventCountOR()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	// CEventProgram rh4EventCountOR
	return get_EventProgram()->get_rh4EventCountOR();
}

// Reads the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
int RunHeader::GetRH2ActionCount()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	// CEventProgram rh2ActionCount
	return get_EventProgram()->get_rh2ActionCount();
}
// Sets the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
void RunHeader::SetRH2ActionCount(int newActionCount)
{
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), newActionCount);
	// CEventProgram rh2ActionCount
	return get_EventProgram()->set_rh2ActionCount(newActionCount);
}

// Reads the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
int RunHeader::GetRH2ActionLoopCount()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	// CEventProgram rh2ActionLoopCount
	return get_EventProgram()->get_rh2ActionLoopCount();
}
// Sets the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
void RunHeader::SetRH2ActionLoopCount(int newActLoopCount)
{
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), newActLoopCount);
	// CEventProgram rh2ActionLoopCount
	return get_EventProgram()->set_rh2ActionLoopCount(newActLoopCount);
}

// Gets the current expression token index
int RunHeader::GetRH4CurToken()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	int rh4CurToken = threadEnv->GetIntField(crun, rh4CurTokenFieldID);
	JNIExceptionCheck();
	return rh4CurToken;
}
// Sets the current expression token index
void RunHeader::SetRH4CurToken(int newCurToken)
{
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), newCurToken);
	threadEnv->SetIntField(crun, rh4CurTokenFieldID, newCurToken);
	JNIExceptionCheck();
}

// Gets the current expression token array; relevant in Android only.
jobjectArray RunHeader::GetRH4Tokens()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	jobjectArray ptr = (jobjectArray)threadEnv->GetObjectField(crun, rh4TokensFieldID);
	JNIExceptionCheck();
	return ptr;
}
// Sets the current expression token array; relevant in Android only.
void RunHeader::SetRH4Tokens(jobjectArray newTokensArray)
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	threadEnv->SetObjectField(crun, rh4TokensFieldID, newTokensArray);
	JNIExceptionCheck();
}

objInfoList * RunHeader::GetOIListByIndex(std::size_t index)
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (OiList.invalid())
		GetOiList(); // ignore return
	if (index >= OiListLength)
		return nullptr;

	if ((long)index < 0)
		raise(SIGINT);

	return &OiListArray[index];
}
jobjectArray RunHeader::GetOiList()
{
	if (OiList.valid())
		return OiList.ref;
	// jObjectArray
	OiList = global((jobjectArray)threadEnv->GetObjectField(this->crun, oiListFieldID), "RunHeader::GetOIListByIndex's rhOiList");
	JNIExceptionCheck();

	OiListLength = threadEnv->GetArrayLength(OiList.ref);
	JNIExceptionCheck();
	for (int i = 0; i < OiListLength; ++i)
	{
		jobject item = mainThreadJNIEnv->GetObjectArrayElement(OiList, i);
		JNIExceptionCheck();
		OiListArray.emplace_back(objInfoList{ i, this, item, runtime });
	}

	return OiList.ref;
}

event2 * RunHeader::GetRH4ActionStart() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	// During A/C/E curCEvent should be copied out, A/C/E func code run, then copied back after.
	// Failure to do this will result in curCEvent inconsistency which may affect any expression-function
	// objects, fastloops, etc., and makes debugging events harder.
	if (!get_EventProgram()->GetRH2ActionOn())
	{
		LOGV(_T("GetRH4ActionStart(): curCEvent is invalid, was rh4ActStart read during a non-action?\n"));
		return nullptr;
	}

	// rh4ActionStart is not present in the Android runtime, but it is in iOS/Windows.
	// It points to the currently running action in an event, set in call_Actions().
	// That's just the current CEvent, if it's running action, which rh2ActionOn == true indicates.
	// However this wasn't patched for a lot of builds; so we allow it to be missing.
	if (CEventProgram::rh4ActStartFieldID != NULL)
	{
		if (!rh4ActStart)
		{
			// Assume EventGroup is out of date if curCEvent is, and reset it too
			if (EventGroup.has_value())
			{
				EventGroup.reset();
				// We load eventgroup from eventProgram, which is tied to the frame so program should be valid
				get_EventProgram()->eventGrp.reset();
				threadEnv->DeleteGlobalRef(runtime->curRH4ActStart.ref);
				runtime->curRH4ActStart.ref = nullptr;
			}

			if (runtime->curRH4ActStart.invalid())
			{
				// rh4ActStart will hold a global ref if needed
				jobject rh4AS = threadEnv->GetObjectField(get_EventProgram()->me, CEventProgram::rh4ActStartFieldID);
				JNIExceptionCheck();
				if (!rh4AS)
					LOGE(_T("Can't read rh4ActStart, returned null, but rh2ActionOn is true, so it should be a valid event.\n"));

				rh4ActStart = std::make_unique<event2>(get_EventGroup(), event2::FindIndexMagicNum, rh4AS, runtime);
			}
		}

		return rh4ActStart.get();
	}

	// If we don't have rh4ActionStart patch, work around it by returning the curCEvent.
	// This only works if this ext's action is being run, which sets curCEvent.
	// We can't return null because rh2ActionOn is true, so an action is running, so this cannot be null.
	if (runtime->curCEvent.invalid())
		LOGF(_T("GetRH4ActionStart(): curCEvent is invalid, and the runtime implementation of rh4ActStart is not available.\n"));

	// CEvent exists and is out of date, and we're running an action
	if (!rh4ActStart || rh4ActStart->me.ref != runtime->curCEvent.ref)
	{
		// Assume EventGroup is out of date if curCEvent is, and reset it too
		if (EventGroup.has_value())
		{
			EventGroup.reset();
			// We load eventgroup from eventProgram, which is tied to the frame so program should be valid
			get_EventProgram()->eventGrp.reset();
		}

		rh4ActStart = std::make_unique<event2>(get_EventGroup(), event2::FindIndexMagicNum, runtime->curRH4ActBasedOnCEventOnly, runtime);
	}
	return rh4ActStart.get();
}

CEventProgram* RunHeader::get_EventProgram() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!eventProgram)
	{
		jobject eventProgramJava = mainThreadJNIEnv->GetObjectField(crun, eventProgramFieldID);
		JNIExceptionCheck();

		eventProgram = std::make_unique<CEventProgram>(eventProgramJava, runtime);
	}
	return eventProgram.get();
}
RunHeader::RunHeader(jobject me, jclass meClass, Edif::Runtime* runtime) :
	crun(me, "RunHeader's CRun"), crunClass(meClass, "RunHeader's CRun class"), runtime(runtime)
{
	if (rh4TokensFieldID == nullptr)
	{
		rh4TokensFieldID = threadEnv->GetFieldID(crunClass, "rh4Tokens", "[LExpressions/CExp;");
		JNIExceptionCheck();

		// CEventProgram rh4CurToken
		rh4CurTokenFieldID = threadEnv->GetFieldID(crunClass, "rh4CurToken", "I");
		JNIExceptionCheck();
		eventProgramFieldID = mainThreadJNIEnv->GetFieldID(crunClass, "rhEvtProg", "LEvents/CEventProgram;");
		JNIExceptionCheck();

		oiListFieldID = threadEnv->GetFieldID(crunClass, "rhOiList", "[LRunLoop/CObjInfo;");
		JNIExceptionCheck();
		oiListClass = global(threadEnv->FindClass("RunLoop/CObjInfo"), "RunHeader::GetOIListByIndex's rhOiList class");
		JNIExceptionCheck();
	}
}
// Static definitions - default inited to zero
jfieldID RunHeader::rh4TokensFieldID, RunHeader::rh4CurTokenFieldID,
	RunHeader::eventProgramFieldID, RunHeader::oiListFieldID;

void RunHeader::InvalidatedByNewGeneratedEvent()
{
	if (EventGroup)
		EventGroup.reset();
	if (eventProgram)
		eventProgram->InvalidatedByNewGeneratedEvent();
	if (OiList.valid())
	{
		for (auto& o : OiListArray)
			o.InvalidatedByNewGeneratedEvent();
	}
	if (ObjectList)
	{
		for (auto h = ObjectList->activeObjs.begin(); h != ObjectList->activeObjs.end();)
		{
			auto h2 = h->lock();
			if (!h2)
			{
				h = ObjectList->activeObjs.erase(h);
				continue;
			}
			h2->get_rHo()->InvalidatedByNewGeneratedEvent();
			++h;
		}
	}
	if (NObjects)
		NObjects.reset();
	if (!QualToOiListArray.empty())
	{
		// If QualToOiList changes size during runtime, to properly refresh qual with length included,
		// it's necessary to null the ref of QualToOiList and set QualToOiListLength to 0.
		// It doesn't seemed like it does change QualToOiList, just its entries' OiAndOiList within, though.
		for (std::size_t i = 0; i < QualToOiListLength; ++i)
			QualToOiListArray[i].InvalidatedByNewCondition();
	}
}

// Static definitions - default inited to zero
jfieldID CEventProgram::rh4ActStartFieldID;

eventGroup * CEventProgram::get_eventGroup() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!eventGrp)
	{
		jfieldID rhEventProgFieldID = mainThreadJNIEnv->GetFieldID(meClass, "rhEventGroup", "LEvents/CEventGroup;");
		JNIExceptionCheck();
		jobject eventGroupJava = mainThreadJNIEnv->GetObjectField(me, rhEventProgFieldID);
		JNIExceptionCheck();
		// This can be null, if running events from Handle tick.
		if (eventGroupJava != nullptr)
		{
			eventGrp = std::make_unique<eventGroup>(eventGroupJava, runtime);
			LOGV(_T("Running %s() - got a new eventGroup of %p, going to store it in eventGroup struct at %p."), _T(__FUNCTION__), eventGroupJava, eventGrp.get());
		}
	}
	return eventGrp.get();
}
CEventProgram::CEventProgram(jobject me, Edif::Runtime* runtime) :
	me(me, "CEventProgram"), runtime(runtime)
{
	meClass = global(threadEnv->GetObjectClass(me), "CEventProgram class");
	JNIExceptionCheck();

	// rh4ActionStart is missing in earlier Runtime versions (<= 295.10)
	if (RunHeader::eventProgramFieldID != NULL)
	{
		rh4ActStartFieldID = threadEnv->GetFieldID(meClass, "rh4ActionStart", "LActions/CAct;");
		// Missing rh4ActionStart patch, which we allow
		if (!rh4ActStartFieldID)
		{
			threadEnv->ExceptionClear();
			LOGW(_T("Missing the rh4ActionStart field. Reading rh4ActionStart can fail and kill the application!\n"));
		}
	}
}
void CEventProgram::InvalidatedByNewGeneratedEvent()
{
	rh2EventCount.reset();
	rh4EventCountOR.reset();
	// rh2ActionOn.reset(); // currently not cached
	if (eventGrp)
		eventGrp.reset();
}
void CEventProgram::SetEventGroup(jobject grp)
{
	jfieldID rhEventProgFieldID = mainThreadJNIEnv->GetFieldID(meClass, "rhEventGroup", "LEvents/CEventGroup;");
	JNIExceptionCheck();
	mainThreadJNIEnv->SetObjectField(me, rhEventProgFieldID, grp);
	JNIExceptionCheck();
	eventGrp = grp ? std::make_unique<eventGroup>(grp, runtime) : nullptr;
	runtime->ObjectSelection.pExtension->rhPtr->EventGroup = grp ? eventGrp.get() : nullptr;
}

int CEventProgram::get_rh2EventCount() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!rh2EventCount.has_value())
	{
		static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2EventCount", "I");
		JNIExceptionCheck();
		rh2EventCount = threadEnv->GetIntField(me, fieldID);
		JNIExceptionCheck();
	}
#if _DEBUG
	else
	{
		jfieldID fieldID2 = threadEnv->GetFieldID(meClass, "rh2EventCount", "I");
		JNIExceptionCheck();
		int newVal = threadEnv->GetIntField(me, fieldID2);
		JNIExceptionCheck();
		if (rh2EventCount.value() != newVal)
			LOGE(_T("EVENT COUNT CACHE WAS OUT OF DATE: %i returned, compared to real rh2EventCount %i.\n"), rh2EventCount.value(), newVal);
		rh2EventCount = newVal;
	}
#endif
	return rh2EventCount.value();
}
void CEventProgram::set_rh2EventCount(int newEventCount)
{
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), newEventCount);
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2EventCount", "I");
	JNIExceptionCheck();
	rh2EventCount = newEventCount;
	threadEnv->SetIntField(me, fieldID, newEventCount);
	JNIExceptionCheck();
}
int CEventProgram::get_rh4EventCountOR()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!rh4EventCountOR.has_value())
	{
		static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh4EventCountOR", "I");
		JNIExceptionCheck();
		rh4EventCountOR = threadEnv->GetIntField(me, fieldID);
		JNIExceptionCheck();
	}
#if _DEBUG
	else
	{
		jfieldID fieldID2 = threadEnv->GetFieldID(meClass, "rh4EventCountOR", "I");
		JNIExceptionCheck();
		int newVal = threadEnv->GetIntField(me, fieldID2);
		JNIExceptionCheck();
		if (rh2EventCount.value() != newVal)
			LOGE(_T("EVENT OR COUNT CACHE WAS OUT OF DATE: %i returned, compared to real rh4EventCountOR %i.\n"), rh2EventCount.value(), newVal);
	}
#endif
	return rh4EventCountOR.value();
}
void CEventProgram::set_rh4EventCountOR(int newEventCount)
{
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), newEventCount);
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh4EventCountOR", "I");
	JNIExceptionCheck();
	rh4EventCountOR = newEventCount;
	threadEnv->SetIntField(me, fieldID, newEventCount);
	JNIExceptionCheck();
}

// Reads the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
int CEventProgram::get_rh2ActionCount()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2ActionCount", "I");
	JNIExceptionCheck();
	int rh2ActionCount = threadEnv->GetIntField(me, fieldID);
	JNIExceptionCheck();
	return rh2ActionCount;
}
// Sets the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
void CEventProgram::set_rh2ActionCount(int newActionCount)
{
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), newActionCount);
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2ActionCount", "I");
	JNIExceptionCheck();
	rh2ActionCount = newActionCount;
	threadEnv->SetIntField(me, fieldID, newActionCount);
	JNIExceptionCheck();
}

// Reads the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
int CEventProgram::get_rh2ActionLoopCount()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2ActionLoopCount", "I");
	JNIExceptionCheck();
	int rh2ActionLoopCount = threadEnv->GetIntField(me, fieldID);
	JNIExceptionCheck();
	return rh2ActionLoopCount;
}
// Sets the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
void CEventProgram::set_rh2ActionLoopCount(int newActLoopCount)
{
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), newActLoopCount);
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2ActionLoopCount", "I");
	JNIExceptionCheck();
	rh2ActionLoopCount = newActLoopCount;
	threadEnv->SetIntField(me, fieldID, newActLoopCount);
	JNIExceptionCheck();
}

bool CEventProgram::GetRH2ActionOn() {
	// TODO: We can possibly optimize this by storing true/false Action/Condition jump funcs
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2ActionOn", "Z");
	JNIExceptionCheck();
	bool yes = threadEnv->GetBooleanField(me, fieldID);
	JNIExceptionCheck();
	return yes;
}

eventGroup * RunHeader::get_EventGroup() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));

	if (!EventGroup.has_value() || !EventGroup.value())
	{
		LOGV(_T("Running %s() - eventgroup out of date, updating it."), _T(__FUNCTION__));
		EventGroup = get_EventProgram()->get_eventGroup();
	}

	return EventGroup.value();
}

std::size_t RunHeader::GetNumberOi() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!NumberOi.has_value())
	{
		// NumberOi is actually rhMaxOI
		// Note: EventProgram has short maxOi, but CRun has int maxOi
		JNIExceptionCheck();
		jfieldID fieldID = mainThreadJNIEnv->GetFieldID(crunClass, "rhMaxOI", "I");
		JNIExceptionCheck();
		NumberOi = (std::size_t)mainThreadJNIEnv->GetIntField(crun, fieldID);
		JNIExceptionCheck();
	}

	return NumberOi.value();
}
qualToOi *RunHeader::GetQualToOiListByOffset(std::size_t index) {
	if ((index & 0x8000) == 0)
	{
		LOGE("GetQualToOiListByOffset: Invalid qualifier oi %zu looked up.", index);
		return nullptr;
	}

	index &= 0x7FFF;
	LOGV(_T("Running %s(%zu)."), _T(__FUNCTION__), index);

	// CQualToOiList[] CEventProgram.qualToOiList
	if (QualToOiList.invalid())
	{
		auto evp = this->get_EventProgram();
		jfieldID fieldID = threadEnv->GetFieldID(evp->meClass, "qualToOiList", "[LEvents/CQualToOiList;");
		JNIExceptionCheck();
		QualToOiList = global((jobjectArray)threadEnv->GetObjectField(evp->me, fieldID),
			"RunHeader::GetQualToOiListByIndex's QualToOiList");
		JNIExceptionCheck();
		// can't look up from OiList, that's the class of array, not array element
		QualToOiClass = global(threadEnv->FindClass("Events/CQualToOiList"), "RunHeader::GetQualToOiListByIndex's QualToOiList class");
		JNIExceptionCheck();
		QualToOiListLength = threadEnv->GetArrayLength(QualToOiList.ref);
		JNIExceptionCheck();

		for (std::size_t i = 0; i < QualToOiListLength; ++i)
		{
			jobject thisParam = threadEnv->GetObjectArrayElement(QualToOiList, index);
			JNIExceptionCheck();
			QualToOiListArray.emplace_back(qualToOi(this, i, thisParam, runtime));
		}
	}

#ifdef _DEBUG
	if (*(long *)&index < 0)
		raise(SIGINT);
#endif

	if (index >= QualToOiListLength)
		return nullptr;

	return &QualToOiListArray[index];
}
RunObjectMultiPlatPtr RunHeader::GetObjectListOblOffsetByIndex(std::size_t index) {
	LOGV(_T("Running %s(%zu)."), _T(__FUNCTION__), index);
	if (!ObjectList)
	{
		// CObject[] CRun.rhObjectsList
		static jfieldID fieldID = mainThreadJNIEnv->GetFieldID(crunClass, "rhObjectList", "[LObjects/CObject;");
		JNIExceptionCheck();
		jobjectArray jobArr = (jobjectArray)mainThreadJNIEnv->GetObjectField(crun, fieldID);
		JNIExceptionCheck();
		ObjectList = std::make_unique<objectsList>(jobArr, runtime);
	}

	return ObjectList->GetOblOffsetByIndex(index);
}

EventGroupFlags RunHeader::GetEVGFlags() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	return get_EventGroup()->get_evgFlags();
}
CRunApp* RunHeader::get_App() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!App)
	{
		jfieldID fieldID = mainThreadJNIEnv->GetFieldID(crunClass, "rhApp", "LApplication/CRunApp;");
		JNIExceptionCheck();
		jobject appJava = mainThreadJNIEnv->GetObjectField(crun, fieldID);
		JNIExceptionCheck();
		App = std::make_unique<CRunAppMultiPlat>(appJava, runtime);
	}
	return App.get();
}
size_t RunHeader::get_MaxObjects() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!MaxObjects.has_value())
	{
		// Note: CRun has int rhMaxObjects
		static jfieldID maxObjectsFieldID = mainThreadJNIEnv->GetFieldID(crunClass, "rhMaxObjects", "I");
		JNIExceptionCheck();
		MaxObjects = mainThreadJNIEnv->GetIntField(crun, maxObjectsFieldID);
		JNIExceptionCheck();
	}
	return MaxObjects.value();
}
// Gets number of valid object instances currently in frame
size_t RunHeader::get_NObjects() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!NObjects.has_value())
	{
		// Note: CRun has int rhMaxObjects
		static jfieldID nObjectsFieldID = mainThreadJNIEnv->GetFieldID(crunClass, "rhNObjects", "I");
		JNIExceptionCheck();
		NObjects = mainThreadJNIEnv->GetIntField(crun, nObjectsFieldID);
		JNIExceptionCheck();
	}
	return NObjects.value();
}

short HeaderObject::get_NextSelected() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!NextSelected.has_value())
	{
		// Part of CObject
		jfieldID nextSelectedFieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoNextSelected", "S");
		JNIExceptionCheck();
		NextSelected = mainThreadJNIEnv->GetShortField(me, nextSelectedFieldID);
		JNIExceptionCheck();
	}
	return NextSelected.value();
}
unsigned short HeaderObject::get_CreationId() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!CreationId.has_value())
	{
		// Part of CObject
		jfieldID CreationIdFieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoCreationId", "S");
		JNIExceptionCheck();
		CreationId = mainThreadJNIEnv->GetShortField(me, CreationIdFieldID);
		JNIExceptionCheck();
	}
	return CreationId.value();
}
short HeaderObject::get_Number() {
	LOGV(_T("Running %s() on %p."), _T(__FUNCTION__), this);
	if (!Number.has_value())
	{
		// Part of CObject
		jfieldID NumberFieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoNumber", "S");
		JNIExceptionCheck();
		Number = mainThreadJNIEnv->GetShortField(me, NumberFieldID);
		JNIExceptionCheck();
	}
	return Number.value();
}
short HeaderObject::get_NumNext() {
	LOGV(_T("Running %s() on %p."), _T(__FUNCTION__), this);
	if (!NumNext.has_value())
	{
		// Part of CObject
		static jfieldID NumNextFieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoNumNext", "S");
		JNIExceptionCheck();
		NumNext = mainThreadJNIEnv->GetShortField(me, NumNextFieldID);
		JNIExceptionCheck();
	}
	return NumNext.value();
}
short HeaderObject::get_Oi() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!Oi.has_value())
	{
		// Part of CObject
		static jfieldID OiFieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoOi", "S");
		JNIExceptionCheck();
		Oi = mainThreadJNIEnv->GetShortField(me, OiFieldID);
		JNIExceptionCheck();
	}
	return Oi.value();
}
objInfoList * HeaderObject::get_OiList() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!OiList)
	{
		if (get_AdRunHeader()->OiListArray.empty())
			get_AdRunHeader()->GetOiList();
		for (int i = 0, j = get_AdRunHeader()->OiListLength; i < j; ++i)
		{
			objInfoList* oiList = get_AdRunHeader()->GetOIListByIndex(i);
			if (get_Oi() == oiList->get_Oi())
			{
				OiList = oiList;
				oiListIndex = i;
				return OiList;
			}
		}

		// can't find the OiList in the list, something went awry
		LOGE(_T("Missing OiList of object Java class \"%s\", Oi %hi in the app OiList."), getClassName(meClass, false), get_Oi());
		raise(SIGINT);
	}

	return OiList;
}
bool HeaderObject::get_SelectedInOR() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!SelectedInOR.has_value())
	{
		// Part of CObject
		static jfieldID fieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoSelectedInOR", "B");
		JNIExceptionCheck();
		SelectedInOR = mainThreadJNIEnv->GetByteField(me, fieldID) != 0;
		JNIExceptionCheck();
	}
	return SelectedInOR.value();
}
HeaderObjectFlags HeaderObject::get_Flags() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!Flags.has_value())
	{
		// Part of CObject
		static jfieldID fieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoFlags", "S");
		JNIExceptionCheck();
		Flags = (HeaderObjectFlags)mainThreadJNIEnv->GetShortField(me, fieldID);
		JNIExceptionCheck();
	}
	return Flags.value();
}
RunHeader* HeaderObject::get_AdRunHeader() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!AdRunHeader)
	{
		// Part of CObject
		LOGV("Class name of meClass: \"%s\".", getClassName(meClass, true));
		static jfieldID fieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoAdRunHeader", "LRunLoop/CRun;");
		JNIExceptionCheck();
		jobject rhObject = mainThreadJNIEnv->GetObjectField(me, fieldID);
		JNIExceptionCheck();
		jclass rhObjectClass = threadEnv->GetObjectClass(rhObject);
		AdRunHeader = std::make_unique<RunHeader>(rhObject, rhObjectClass, runtime);
		JNIExceptionCheck();
	}
	return AdRunHeader.get();
}

void HeaderObject::set_NextSelected(short ns) {
	LOGV(_T("Running %s(%hi)."), _T(__FUNCTION__), ns);
	NextSelected = ns;
	// Part of CObject
	static jfieldID nextSelectedFieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoNextSelected", "S");
	JNIExceptionCheck();
	mainThreadJNIEnv->SetShortField(me, nextSelectedFieldID, ns);
	JNIExceptionCheck();
}
void HeaderObject::set_SelectedInOR(bool b) {

	// despite being a Java byte field, it's used as bool
	LOGV(_T("Running %s(bool %i)."), _T(__FUNCTION__), b ? 1 : 0);
	SelectedInOR = b;
	// Part of CObject
	static jfieldID selectedInORFieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoSelectedInOR", "B");
	JNIExceptionCheck();
	mainThreadJNIEnv->SetByteField(me, selectedInORFieldID, b);
	JNIExceptionCheck();
}

// static definition
jfieldID HeaderObject::numberFieldID = nullptr;

HeaderObject::HeaderObject(RunObject * ro, jobject me, jclass meClass, Edif::Runtime* runtime) :
	me(me), meClass(meClass), runtime(runtime), runObj(ro)
{
	// May be looked up by accident
	if (numberFieldID == nullptr)
	{
		// Part of CObject
		numberFieldID = mainThreadJNIEnv->GetFieldID(meClass, "hoNumber", "S");
		JNIExceptionCheck();
	}
}
void HeaderObject::InvalidatedByNewGeneratedEvent()
{
	// invalidated by event change: eventnumber, nextselected, numprev, numnext, selectedinor, Flags
	EventNumber.reset();
	NextSelected.reset();
	NumPrev.reset();
	NumNext.reset();
	SelectedInOR.reset();
	Flags.reset();
}
//

short HeaderObject::GetObjectParamNumber(jobject obj) {
	LOGV(_T("Running %s() on %p."), _T(__FUNCTION__), obj);
	if (numberFieldID == nullptr)
	{
		// Part of CObject
		numberFieldID = mainThreadJNIEnv->GetFieldID(mainThreadJNIEnv->GetObjectClass(obj), "hoNumber", "S");
		JNIExceptionCheck();
	}
	const short num = mainThreadJNIEnv->GetShortField(obj, numberFieldID);
	JNIExceptionCheck();
	return num;
}

int CRunAppMultiPlat::get_nCurrentFrame()
{
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!nCurrentFrame.has_value())
	{
		static jfieldID fieldID = mainThreadJNIEnv->GetFieldID(meClass, "currentFrame", "I");
		JNIExceptionCheck();
		nCurrentFrame = mainThreadJNIEnv->GetIntField(me, fieldID);
		JNIExceptionCheck();
	}
	return nCurrentFrame.value();
}
CRunAppMultiPlat::CRunAppMultiPlat(jobject me, Edif::Runtime* runtime) :
	me(me), runtime(runtime)
{
	meClass = threadEnv->GetObjectClass(me);
	JNIExceptionCheck();
}

// static definition - zero inited
jfieldID event2::evtFlagsFieldID, event2::evtSizeFieldID, event2::evtCodeFieldID, event2::evtOiFieldID;

event2::event2(eventGroup * owner, int index, jobject evt, Edif::Runtime * run) :
	me(evt, "event2 global"), runtime(run), owner(owner), index(index)
{
	meClass = global(threadEnv->GetObjectClass(me), "event2 class");
	JNIExceptionCheck();
	if (evtFlagsFieldID == nullptr)
	{
		evtFlagsFieldID = threadEnv->GetFieldID(meClass, "evtFlags", "B");
		JNIExceptionCheck();
		// Size is read into local var during Java event ctor and discarded
		//evtSizeFieldID = threadEnv->GetFieldID(meClass, "evtSize", "S");
		//JNIExceptionCheck();
		evtCodeFieldID = threadEnv->GetFieldID(meClass, "evtCode", "I");
		JNIExceptionCheck();
		evtOiFieldID = threadEnv->GetFieldID(meClass, "evtOi", "S");
		JNIExceptionCheck();
	}

	// Java arrays don't feature an index search, so we call Java equivalent of arr.asList().indexOf(obj)
	if (index == FindIndexMagicNum)
	{
		jclass arraysClass = threadEnv->FindClass("java/util/Arrays");
		JNIExceptionCheck();
		jmethodID listMakeFunc = threadEnv->GetStaticMethodID(arraysClass, "asList", "([Ljava/lang/Object;)Ljava/util/List;");
		JNIExceptionCheck();
		jobjectArray evgEvents = owner->GetEventList();
		jobject arrAsList = threadEnv->CallStaticObjectMethod(arraysClass, listMakeFunc, evgEvents);
		JNIExceptionCheck();
		jclass listClass = threadEnv->GetObjectClass(arrAsList);
		JNIExceptionCheck();
		jmethodID indexOfFunc = threadEnv->GetMethodID(listClass, "indexOf", "(Ljava/lang/Object;)I");
		JNIExceptionCheck();
		jint indexResult = threadEnv->CallIntMethod(arrAsList, indexOfFunc, evt);
		JNIExceptionCheck();
		index = indexResult;
		if (indexResult == -1)
			LOGE("Couldn't find action/condition within event's A/C list.");

		threadEnv->DeleteLocalRef(arrAsList);
	}
}
short event2::get_evtNum() {
	// In Windows, evtNum is high WORD of evtCode, in a union.
	// In Android, there is no union; just evtCode, so pull it out and shift it.

	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	const jint evtCode = threadEnv->GetIntField(me, evtCodeFieldID);
	JNIExceptionCheck();
	return (jshort)(evtCode >> 16);
}
OINUM event2::get_evtOi() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	const jshort evtOi = threadEnv->GetShortField(me, evtOiFieldID);
	JNIExceptionCheck();
	return evtOi;
}
/*short event2::get_evtSize() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	const jshort evtSize = threadEnv->GetShortField(me, evtSizeFieldID);
	JNIExceptionCheck();
	return evtSize;
}*/
std::int8_t event2::get_evtFlags() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	const jbyte evtFlags = threadEnv->GetByteField(me, evtFlagsFieldID);
	JNIExceptionCheck();
	return evtFlags;
}
void event2::set_evtFlags(std::int8_t evtF) {
	LOGV(_T("Running %s(%hhi)."), _T(__FUNCTION__), evtF);
	threadEnv->SetByteField(me, evtFlagsFieldID, evtF);
	JNIExceptionCheck();
}
std::unique_ptr<event2> event2::Next() {
	return owner->GetCAByIndex(index + 1);
}
int event2::GetIndex() {
	return index;
}

RunObject::RunObject(jobject meP, jclass meClassP, Edif::Runtime * runtime)
	: me(meP, "RunObject ctor"), meClass(meClassP, "RunObject class ctor")
{
	rHo = std::make_unique<HeaderObject>(this, me.ref, meClass.ref, runtime);
}


HeaderObject * RunObject::get_rHo() {
	return rHo.get();
}
rCom* RunObject::get_roc() {
	// read from Java: Objects.CRCom roc
	LOGF("%s not implemented\n", __PRETTY_FUNCTION__);
	return nullptr;
}
rMvt* RunObject::get_rom() {
	// read from Java: Movements.CRMvt rom
	LOGF("%s not implemented\n", __PRETTY_FUNCTION__);
	return nullptr;
}
rAni* RunObject::get_roa() {
	// read from Java: Animations.CRAni roa
	LOGF("%s not implemented\n", __PRETTY_FUNCTION__);
	return nullptr;
}
Sprite* RunObject::get_ros() {
	// read from Java: Sprites.CRSpr ros
	LOGF("%s not implemented\n", __PRETTY_FUNCTION__);
	return nullptr;
}
AltVals* RunObject::get_rov() {
	// read from Java: Values.CRVal rov
	LOGF("%s not implemented\n", __PRETTY_FUNCTION__);
	return nullptr;
}

objectsList::objectsList(jobjectArray me, Edif::Runtime* runtime) :
	me(me, "objectList ctor from rhPtr"), runtime(runtime)
{
	meClass = global(threadEnv->GetObjectClass(me), "objectList array inside rhPtr's class");
	JNIExceptionCheck();
	length = threadEnv->GetArrayLength(me);
	JNIExceptionCheck();
}

RunObjectMultiPlatPtr objectsList::GetOblOffsetByIndex(std::size_t index) {
#ifdef _DEBUG
	if (*(long*)&index < 0)
		raise(SIGINT);
#endif

	if (index >= length)
		return nullptr;

	//CObject cObj = run.rhObjectList[index];
	// The CObject is a CRunExtension as well, so we'll just return it directly.
	jobject rhObjEntry = threadEnv->GetObjectArrayElement(me, index);
	JNIExceptionCheck();
	jclass rhObjEntryClass = threadEnv->GetObjectClass(rhObjEntry);
	JNIExceptionCheck();
	auto ro = std::make_shared<RunObject>(rhObjEntry, rhObjEntryClass, runtime);
	activeObjs.push_back(ro);
#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_VERBOSE)
	auto ho = ro->get_rHo();
	ho->get_Number(); // load for log line
	LOGV(_T("Running %s(%zu), returning jobject %p; ho = %p; num = %hi [index in ObjList], oi = %hi [OI index], oilist = %s."), _T(__FUNCTION__),
		index, rhObjEntry, ho, ho->get_Number(), ho->get_Oi(), ho->get_OiList()->get_name());
#endif
	return ro;
}

int objInfoList::get_EventCount() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!EventCount.has_value())
	{
		EventCount = threadEnv->GetIntField(me, eventCountFieldID);
		JNIExceptionCheck();
	}
#ifdef _DEBUG
	else
	{
		int newOilEventCount = mainThreadJNIEnv->GetIntField(me, eventCountFieldID);
		JNIExceptionCheck();
		if (EventCount != newOilEventCount)
		{
			LOGE(_T("OIL EVENT COUNT CACHE WAS OUT OF DATE: %i returned, compared to real oilEventCount %i.\n"), EventCount.value(), newOilEventCount);
			EventCount = newOilEventCount;
		}
	}
#endif

	return EventCount.value();
}
int objInfoList::get_EventCountOR() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!EventCountOR.has_value())
	{
		EventCountOR = mainThreadJNIEnv->GetIntField(me, eventCountORFieldID);
		JNIExceptionCheck();
	}
#ifdef _DEBUG
	else
	{
		int newOilEventCount = mainThreadJNIEnv->GetIntField(me, eventCountORFieldID);
		JNIExceptionCheck();
		if (EventCountOR != newOilEventCount)
		{
			LOGE(_T("OIL EVENT COUNT OR CACHE WAS OUT OF DATE: %i returned, compared to real oilEventCountOR %i.\n"), EventCountOR.value(), newOilEventCount);
			EventCountOR = newOilEventCount;
		}
	}
#endif

	return EventCountOR.value();
}
short objInfoList::get_ListSelected() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!ListSelected.has_value())
	{
		ListSelected = mainThreadJNIEnv->GetShortField(me, listSelectedFieldID);
		JNIExceptionCheck();
	}
#ifdef _DEBUG
	else
	{
		short oldValue = ListSelected.value();
		short curValue = mainThreadJNIEnv->GetShortField(me, listSelectedFieldID);
		JNIExceptionCheck();
		if (oldValue != curValue)
		{
			LOGE(_T("!!!! ListSelected was wrong!! Was %hi, now %hi."), oldValue, curValue);
			//		ListSelected = curValue;
		}
	}
#endif
	return ListSelected.value();
}
int objInfoList::get_NumOfSelected() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!NumOfSelected.has_value())
	{
		NumOfSelected = mainThreadJNIEnv->GetIntField(me, numOfSelectedFieldID);
		JNIExceptionCheck();
	}
	return NumOfSelected.value();
}
short objInfoList::get_Oi() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!Oi.has_value())
	{
		Oi = mainThreadJNIEnv->GetShortField(me, oiFieldID);
		JNIExceptionCheck();
	}
	return Oi.value();
}
int objInfoList::get_NObjects() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!NObjects.has_value())
	{
		NObjects = mainThreadJNIEnv->GetIntField(me, nObjectsFieldID);
		JNIExceptionCheck();
	}
	return NObjects.value();
}
short objInfoList::get_Object() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!Object.has_value())
	{
		Object = mainThreadJNIEnv->GetShortField(me, objectFieldID);
		JNIExceptionCheck();
	}
	return Object.value();
}
const TCHAR* objInfoList::get_name() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!name.has_value())
	{
		JavaAndCString str;
		str.ctx = (jstring)mainThreadJNIEnv->GetObjectField(me, nameFieldID);
		JNIExceptionCheck();
		str.ptr = mainThreadJNIEnv->GetStringUTFChars((jstring)str.ctx, NULL);
		name = str.ptr ? std::string(str.ptr) : std::string();
		JNIExceptionCheck();

		threadEnv->DeleteLocalRef((jobject)str.ctx);
		JNIExceptionCheck();
	}
	return name.value().c_str();
}
void objInfoList::set_NumOfSelected(int ns) {
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), ns);
	NumOfSelected = ns;
	mainThreadJNIEnv->SetIntField(me, numOfSelectedFieldID, ns);
	JNIExceptionCheck();
}
void objInfoList::set_ListSelected(short sh) {
	LOGV(_T("Running %s(%hi); %p; oilObject %hi, oilOi %hi, name %s."), _T(__FUNCTION__), sh, this,
		Object.value_or(-2), Oi.value_or(-2), name.value_or("??"s).c_str());
	ListSelected = sh;
	mainThreadJNIEnv->SetShortField(me, listSelectedFieldID, sh);
	JNIExceptionCheck();
}
void objInfoList::set_EventCount(int ec) {
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), ec);
	EventCount = ec;
	mainThreadJNIEnv->SetIntField(me, eventCountFieldID, ec);
	JNIExceptionCheck();
}
void objInfoList::set_EventCountOR(int ec) {
	LOGV(_T("Running %s(%i)."), _T(__FUNCTION__), ec);
	EventCountOR = ec;
	mainThreadJNIEnv->SetIntField(me, eventCountORFieldID, ec);
	JNIExceptionCheck();
}
short objInfoList::get_QualifierByIndex(std::size_t index) {
	if (!QualifiersLoaded)
	{
		jshortArray list = (jshortArray)threadEnv->GetObjectField(me, qualifiersFieldID);
		JNIExceptionCheck();
		jshort * js = threadEnv->GetShortArrayElements(list, NULL);
		// MAX_QUALIFIERS == 8
		memcpy(Qualifiers, js, MAX_QUALIFIERS * sizeof(short));
		JNIExceptionCheck();
		threadEnv->ReleaseShortArrayElements(list, js, JNI_ABORT); // JNI_ABORT does not copy back changes
		QualifiersLoaded = true;
	}
	return Qualifiers[index];
}

// When an ActionLoop is active, this is the next object number in the loop.
int objInfoList::get_oilNext() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	const int oilNext = mainThreadJNIEnv->GetIntField(me, nextFieldID);
	JNIExceptionCheck();
	return oilNext;
}
// When an ActionLoop is active, this is whether to iterate further or not.
bool objInfoList::get_oilNextFlag() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	const bool oilNextFlag = mainThreadJNIEnv->GetBooleanField(me, nextFlagFieldID);
	JNIExceptionCheck();
	return oilNextFlag;
}
int objInfoList::get_oilCurrentRoutine() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	const int oilCurrentRoutine = mainThreadJNIEnv->GetIntField(me, currentRoutineFieldID);
	JNIExceptionCheck();
	return oilCurrentRoutine;
}
// When an Action is active, this specifies which object is currently being iterated. -1 if invalid.
int objInfoList::get_oilCurrentOi() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	const int oilCurrentOi = mainThreadJNIEnv->GetIntField(me, currentOiFieldID);
	JNIExceptionCheck();
	return oilCurrentOi;
}
// When an Action is active, this applies oilCurrentRountine
int objInfoList::get_oilActionCount() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	const int oilActionCount = mainThreadJNIEnv->GetIntField(me, actionCountFieldID);
	JNIExceptionCheck();
	return oilActionCount;
}
// When an ActionLoop is active (Action repeating in a fastloop), this applies oilCurrentRountine
int objInfoList::get_oilActionLoopCount() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	const int oilActionLoopCount = mainThreadJNIEnv->GetIntField(me, actionLoopCountFieldID);
	JNIExceptionCheck();
	return oilActionLoopCount;
}

// static definition
jfieldID objInfoList::eventCountFieldID, objInfoList::actionLoopCountFieldID, objInfoList::objectFieldID,
	objInfoList::nameFieldID, objInfoList::numOfSelectedFieldID, objInfoList::listSelectedFieldID,
	objInfoList::eventCountORFieldID, objInfoList::qualifiersFieldID, objInfoList::nextFieldID,
	objInfoList::nextFlagFieldID, objInfoList::currentRoutineFieldID, objInfoList::currentOiFieldID,
	objInfoList::actionCountFieldID, objInfoList::typeFieldID, objInfoList::nObjectsFieldID,
	objInfoList::oiFieldID;

objInfoList::objInfoList(int index, RunHeader* containerRH, jobject me2, Edif::Runtime* runtime)
	: index(index), me(global(me2, "objInfoList ctor")), containerRH(containerRH), runtime(runtime)
{
	meClass = global(threadEnv->GetObjectClass(me2), "objInfoList ctor, class");

	if (eventCountFieldID == nullptr)
	{
		eventCountFieldID = threadEnv->GetFieldID(meClass, "oilEventCount", "I");
		JNIExceptionCheck();
		actionLoopCountFieldID = threadEnv->GetFieldID(meClass, "oilActionLoopCount", "I");
		JNIExceptionCheck();
		objectFieldID = threadEnv->GetFieldID(meClass, "oilObject", "S");
		JNIExceptionCheck();
		nameFieldID = threadEnv->GetFieldID(meClass, "oilName", "Ljava/lang/String;");
		JNIExceptionCheck();
		numOfSelectedFieldID = threadEnv->GetFieldID(meClass, "oilNumOfSelected", "I");
		JNIExceptionCheck();
		listSelectedFieldID = threadEnv->GetFieldID(meClass, "oilListSelected", "S");
		JNIExceptionCheck();
		eventCountORFieldID = threadEnv->GetFieldID(meClass, "oilEventCountOR", "I");
		JNIExceptionCheck();
		qualifiersFieldID = threadEnv->GetFieldID(meClass, "oilQualifiers", "[S");
		JNIExceptionCheck();
		nextFieldID = threadEnv->GetFieldID(meClass, "oilNext", "I");
		JNIExceptionCheck();
		nextFlagFieldID = threadEnv->GetFieldID(meClass, "oilNextFlag", "Z");
		JNIExceptionCheck();
		currentRoutineFieldID = threadEnv->GetFieldID(meClass, "oilCurrentRoutine", "I");
		JNIExceptionCheck();
		currentOiFieldID = threadEnv->GetFieldID(meClass, "oilCurrentOi", "I");
		JNIExceptionCheck();
		actionCountFieldID = threadEnv->GetFieldID(meClass, "oilActionCount", "I");
		JNIExceptionCheck();
		typeFieldID = threadEnv->GetFieldID(meClass, "oilType", "S");
		JNIExceptionCheck();
		oiFieldID = threadEnv->GetFieldID(meClass, "oilOi", "S");
		JNIExceptionCheck();
		nObjectsFieldID = threadEnv->GetFieldID(meClass, "oilNObjects", "I");
		JNIExceptionCheck();
	}

#ifdef _DEBUG
	get_Object();
	get_name();
	JNIExceptionCheck();
	LOGV(_T("objInfoList made at index %i, ptr %p."), index, this);
#endif
}
objInfoList::objInfoList(objInfoList&&o)
{
	ActionCount = o.ActionCount;
	ActionLoopCount = o.ActionLoopCount;
	EventCount = o.EventCount;
	index = o.index;
	ListSelected = o.ListSelected;
	name = o.name;
	NObjects = o.NObjects;
	NumOfSelected = o.NumOfSelected;
	Object = o.Object;
	Oi = o.Oi;
	QualifiersLoaded = o.QualifiersLoaded;
	if (QualifiersLoaded)
		memcpy(Qualifiers, o.Qualifiers, sizeof(Qualifiers));
	runtime = o.runtime;
	containerRH = o.containerRH;
	Type = o.Type;
	me = std::move(o.me);
	meClass = std::move(o.meClass);
}
objInfoList::~objInfoList()
{
	LOGV(_T("objInfoList destroyed, index %i, ptr %p."), index, this);
}

void objInfoList::InvalidatedByNewGeneratedEvent()
{
	// invalidated by new event: listselected, object, eventcount, numofselected, nobjects, actioncount, actionloopcount
	ListSelected.reset();
	Object.reset();
	EventCount.reset();
	NumOfSelected.reset();
	NObjects.reset();
	ActionCount.reset();
	ActionLoopCount.reset();
}


short qualToOi::get_Oi(std::size_t i) {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	// Update internal list
	if (OiAndOiListLength == SIZE_MAX)
		get_OiList(0);

	if (i * 2 >= OiAndOiListLength)
		return -1;

	return OiAndOiList[i * 2];
}
short qualToOi::get_OiList(std::size_t i) {
	LOGV(_T("Running qualToOi::%s()."), _T(__FUNCTION__));
	if (OiAndOiListLength == SIZE_MAX)
	{
		jshort* js = threadEnv->GetShortArrayElements(oiAndOiListJava, NULL);
		JNIExceptionCheck();
		OiAndOiListLength = threadEnv->GetArrayLength(oiAndOiListJava);
		JNIExceptionCheck();
		OiAndOiList = std::make_unique<short[]>(OiAndOiListLength);
		memcpy(OiAndOiList.get(), js, OiAndOiListLength * sizeof(short));
		threadEnv->ReleaseShortArrayElements(oiAndOiListJava, js, JNI_ABORT); // JNI_ABORT does not copy back changes
		JNIExceptionCheck();
		LOGV(_T("qualToOi::%s() - OiAndOiList for qual %i was populated OK with %zu entries."), _T(__FUNCTION__), offsetInQualToOiList, OiAndOiListLength);
	}
	else
		LOGV(_T("qualToOi::%s() -  OiAndOiList for qual %i was already populated with %zu entries, returning index %zu."), _T(__FUNCTION__), offsetInQualToOiList, OiAndOiListLength, i * 2 + 1);

	if (i * 2 + 1 >= OiAndOiListLength)
		return -1;

	return OiAndOiList[i * 2 + 1];
}

qualToOi::qualToOi(RunHeader* rh, int offset, jobject me, Edif::Runtime* runtime) :
	rh(rh), offsetInQualToOiList(offset), me(me, "qualToOi ctor"), runtime(runtime)
{
	LOGV(_T("qualToOi::%s() - OiAndOiList not populated, doing so."), _T(__FUNCTION__));
	jfieldID fieldID = mainThreadJNIEnv->GetFieldID(rh->QualToOiClass, "qoiList", "[S");
	JNIExceptionCheck();
	oiAndOiListJava = global((jshortArray)threadEnv->GetObjectField(me, fieldID), "qoiList grab");
	JNIExceptionCheck();
}
void qualToOi::InvalidatedByNewCondition()
{
	OiAndOiList.reset();
	OiAndOiListLength = SIZE_MAX;

	// The object inside main CRun QualToOiList is out of date, not just its qoiList variable
	this->me = global(threadEnv->GetObjectArrayElement(rh->QualToOiList, offsetInQualToOiList), "qualToOi refresh");
	JNIExceptionCheck();
	jfieldID fieldID = mainThreadJNIEnv->GetFieldID(rh->QualToOiClass, "qoiList", "[S");
	JNIExceptionCheck();
	oiAndOiListJava = global((jshortArray)threadEnv->GetObjectField(me, fieldID), "qoiList grab");
	JNIExceptionCheck();
}
qualToOi::qualToOi(qualToOi&& q) {
	me = std::move(q.me);
	offsetInQualToOiList = q.offsetInQualToOiList;
	OiAndOiList = std::move(q.OiAndOiList);
	OiAndOiListLength = q.OiAndOiListLength;
	rh = q.rh;
	runtime = q.runtime;
	oiAndOiListJava = std::move(q.oiAndOiListJava);
}

std::uint8_t eventGroup::get_evgNCond() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!evgNCond.has_value())
	{
		evgNCond = (std::uint8_t)threadEnv->GetShortField(me, evgNCondFieldID);
		JNIExceptionCheck();
	}
	return evgNCond.value();
}
std::uint8_t eventGroup::get_evgNAct() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!evgNAct.has_value())
	{
		evgNAct = (std::uint8_t)threadEnv->GetShortField(me, evgNActFieldID);
		JNIExceptionCheck();
	}
	return evgNAct.value();
}
std::int16_t eventGroup::get_evgIdentifier() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!evgIdentifier.has_value())
	{
		// We read identifier from evgLine
		evgIdentifier = threadEnv->GetShortField(me, evgLineFieldID);
		JNIExceptionCheck();
	}
	return evgIdentifier.value();
}

EventGroupFlags eventGroup::get_evgFlags() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (!evgFlags.has_value())
	{
		evgFlags = (EventGroupFlags)threadEnv->GetShortField(me, evgFlagsFieldID);
		JNIExceptionCheck();
	}
	return evgFlags.value();
}

std::unique_ptr<event2> eventGroup::GetCAByIndex(std::size_t index) {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (evgEvents.invalid())
		GetEventList(); // ignore return

	if (index >= evgEventsLength)
		return nullptr;

	jobject item = threadEnv->GetObjectArrayElement(evgEvents, index);
	JNIExceptionCheck();
	return std::make_unique<event2>(this, index, item, runtime);
}

jobjectArray eventGroup::GetEventList() {
	LOGV(_T("Running %s()."), _T(__FUNCTION__));
	if (evgEvents.invalid())
	{
		evgEvents = global((jobjectArray)threadEnv->GetObjectField(me, evgEventsFieldID), "evgEvents inside eventGroup");
		JNIExceptionCheck();
		evgEventsLength = threadEnv->GetArrayLength(evgEvents);
		JNIExceptionCheck();
		LOGV(_T("Running %s(), line %d, was invalid, new one is %p, size %d, evgID %hi."), _T(__FUNCTION__), __LINE__, evgEvents.ref, evgEventsLength, get_evgIdentifier());
	}
	else
		LOGV(_T("Running %s(), line %d. Was already valid."), _T(__FUNCTION__), __LINE__);
	return evgEvents;
}


// Static definition - default inited to zero, i.e. nullptr
jfieldID eventGroup::evgNCondFieldID, eventGroup::evgNActFieldID,
	eventGroup::evgFlagsFieldID, eventGroup::evgLineFieldID,
	eventGroup::evgEventsFieldID;
eventGroup::eventGroup(jobject me, Edif::Runtime * runtime) :
	me(me, "eventGroup ctor"), runtime(runtime)
{
	meClass = global(threadEnv->GetObjectClass(me), "eventGroup class");
	JNIExceptionCheck();
	if (evgNCondFieldID == nullptr)
	{
		evgNCondFieldID = threadEnv->GetFieldID(meClass, "evgNCond", "S");
		JNIExceptionCheck();
		evgNActFieldID = threadEnv->GetFieldID(meClass, "evgNAct", "S");
		JNIExceptionCheck();
		evgLineFieldID = threadEnv->GetFieldID(meClass, "evgLine", "S");
		JNIExceptionCheck();
		evgFlagsFieldID = threadEnv->GetFieldID(meClass, "evgFlags", "S");
		JNIExceptionCheck();
		evgEventsFieldID = threadEnv->GetFieldID(meClass, "evgEvents", "[LEvents/CEvent;");
		JNIExceptionCheck();
	}
}

#else // iOS

#include "MMF2Lib/CRunExtension.h"
#include "MMF2Lib/CRun.h"
#include "MMF2Lib/CObject.h"
#include "MMF2Lib/CEventProgram.h"
#include "MMF2Lib/CObjInfo.h"
#include "MMF2Lib/CQualToOiList.h"
#include "MMF2Lib/CEvents.h"
#include "MMF2Lib/CActExtension.h"

Edif::Runtime::Runtime(Extension * ext, void * const objCExtPtr) :
	objCExtPtr(objCExtPtr), ObjectSelection((RunHeader *)((CRunExtension *)objCExtPtr)->rh)
{
	this->hoPtr = (HeaderObject *)((CRunExtension *)objCExtPtr)->ho;

	ext->rdPtr = (RunObject*)hoPtr;
	ext->rhPtr = hoPtr->get_AdRunHeader();
	this->ObjectSelection.rhPtr = ext->rhPtr;
}


Edif::Runtime::~Runtime()
{
}

extern "C" void DarkEdifObjCFunc(PROJECT_NAME_RAW, generateEvent)(void * ext, int code, int param);
extern "C" void DarkEdifObjCFunc(PROJECT_NAME_RAW, reHandle)(void * ext);

void Edif::Runtime::Rehandle()
{
	DarkEdifObjCFunc(PROJECT_NAME_RAW, reHandle)(this->objCExtPtr);
}

void Edif::Runtime::GenerateEvent(int EventID)
{
	// Fix event group being incorrect after event finishes.
	// This being incorrect doesn't have any major effects, as the event parsing part of
	// runtime sets rhEventGroup based on a local variable evgPtr, which it relies on instead
	auto evg = ((CRun*)ObjectSelection.rhPtr)->rhEvtProg->rhEventGroup;
	DarkEdifObjCFunc(PROJECT_NAME_RAW, generateEvent)(this->objCExtPtr, EventID, 0);
	((CRun*)ObjectSelection.rhPtr)->rhEvtProg->rhEventGroup = evg;
}

void Edif::Runtime::PushEvent(int EventID)
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void * Edif::Runtime::Allocate(size_t size)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: Allocate missing.
	// return (void *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, size * sizeof(TCHAR));
	return NULL;
}

// Dummy functions.
char * Edif::Runtime::CopyStringEx(const char * String) {
	return CopyString(String);
}
wchar_t * Edif::Runtime::CopyStringEx(const wchar_t * String) {
	throw std::runtime_error("Do not use wchar_t in iOS!");
	return (wchar_t *)String;
}

void Edif::Runtime::Pause()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void Edif::Runtime::Resume()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void Edif::Runtime::Redisplay()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void Edif::Runtime::GetApplicationDrive(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DRIVE, (long)Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DIR, (long)Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::PATH, (long)Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::APP_NAME, (long)Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
}

void Edif::Runtime::Redraw()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void Edif::Runtime::Destroy()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void Edif::Runtime::CallMovement(int ID, long Parameter)
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
	// See ~line 592, CExtension.java
	// GenEdifFunction("redisplay");
	//CallRunTimeFunction2(hoPtr, RFUNCTION::CALL_MOVEMENT, ID, Parameter);
}

void Edif::Runtime::SetPosition(int X, int Y)
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

static EdifGlobal * staticEdifGlobal; // LB says static/global values are functionally equivalent to getUserExtData, so... yay.

void Edif::Runtime::WriteGlobal(const TCHAR * name, void * Value)
{
	EdifGlobal * Global = (EdifGlobal *)staticEdifGlobal;

	if (!Global)
	{
		Global = new EdifGlobal;

		_tcscpy(Global->name, name);
		Global->Value = Value;
		Global->Next = NULL;

		staticEdifGlobal = Global;

		return;
	}

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
		{
			Global->Value = Value;
			return;
		}

		if (!Global->Next)
			break;

		Global = Global->Next;
	}

	Global->Next = new EdifGlobal;
	Global = Global->Next;

	_tcscpy(Global->name, name);

	Global->Value = Value;
	Global->Next = 0;
}

void * Edif::Runtime::ReadGlobal(const TCHAR * name)
{
	EdifGlobal * Global = staticEdifGlobal;

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
			return Global->Value;

		Global = Global->Next;
	}

	return NULL;
}

short Edif::Runtime::GetOIFromObjectParam(std::size_t paramIndex)
{
	// This still works for conditions, as they store params in same offset.
	const eventParam* curParam = ((CActExtension*)curCEvent)->pParams[0];
	for (int i = 0; i < paramIndex; ++i)
		curParam = (const eventParam*)((const std::uint8_t*)curParam + curParam->evpSize);
	if ((Params)curParam->evpCode != Params::Object)
		LOGE(_T("GetOIFromExtParam: Returning a OI for a non-Object parameter."));
	LOGD(_T("GetOIFromExtParam: Returning OiList %hi, oi is %hi.\n"), curParam->evp.evpW.evpW0, curParam->evp.evpW.evpW1);
	return curParam->evp.evpW.evpW0;
}
int Edif::Runtime::GetCurrentFusionFrameNumber()
{
	auto a = hoPtr->get_AdRunHeader();
	auto b = a->get_App();
	if (!b)
		return -1;
	auto c = ((CRunApp *)b)->currentFrame;
	return 1 + c;
}
CRunAppMultiPlat* RunHeader::get_App() {
	return (CRunAppMultiPlat*)((CRun*)this)->rhApp;
}

// Gets the RH2 event count, used in object selection
int RunHeader::GetRH2EventCount()
{
	// no idea why it's called rh4 event count in iOS
	return ((CRun*)this)->rhEvtProg->rh2EventCount;
}
// Gets the RH4 event count for OR, used in object selection in OR-related events.
int RunHeader::GetRH4EventCountOR()
{
	return ((CRun*)this)->rhEvtProg->rh4EventCountOR;
}

objectsList* RunHeader::get_ObjectList() {
	return (objectsList*)((CRun*)this)->rhObjectList;
}

objInfoList* RunHeader::GetOIListByIndex(std::size_t index)
{
#if _DEBUG
	int maxOi = ((CRun*)this)->rhMaxOI;
	assert(maxOi > index || (long)index < 0); // invalid OI
#endif
	return (objInfoList*)((((CRun*)this)->rhOiList)[index]);
}
event2* RunHeader::GetRH4ActionStart() {
	return (event2*)((CRun*)this)->rhEvtProg->rh4ActionStart;
}
eventGroup* RunHeader::get_EventGroup() {
	return (eventGroup*)((CRun*)this)->rhEvtProg->rhEventGroup;
}

std::size_t RunHeader::GetNumberOi() {
	// rhMaxOi on non-Windows
	return ((CRun*)this)->rhMaxOI;
}
qualToOi* RunHeader::GetQualToOiListByOffset(std::size_t byteOffset) {
	// Not initialized during start of frame's CreateRunObject; the qualToOiList is null here (not just its [0])
	return (qualToOi*)((((CRun*)this)->rhEvtProg->qualToOiList)[byteOffset & 0x7FFF]);
}
RunObjectMultiPlatPtr RunHeader::GetObjectListOblOffsetByIndex(std::size_t index) {
	return (RunObjectMultiPlatPtr)((CRun*)this)->rhObjectList[index];
}

EventGroupFlags RunHeader::GetEVGFlags() {
	return (EventGroupFlags)((CRun*)this)->rhEvtProg->rhEventGroup->evgFlags;
}
std::size_t RunHeader::get_MaxObjects() {
	return ((CRun*)this)->rhMaxObjects;
}
std::size_t RunHeader::get_NObjects() {
	return ((CRun*)this)->rhNObjects;
}


short HeaderObject::get_NextSelected() {
	return ((CObject*)this)->hoNextSelected;
}
unsigned short HeaderObject::get_CreationId() {
	return ((CObject*)this)->hoCreationId;
}
short HeaderObject::get_Number() {
	return ((CObject*)this)->hoNumber;
}
short HeaderObject::get_NumNext() {
	return ((CObject*)this)->hoNumNext;
}
short HeaderObject::get_Oi() {
	return ((CObject*)this)->hoOi;
}
objInfoList* HeaderObject::get_OiList() {
	return (objInfoList*)((CObject*)this)->hoOiList;
}
bool HeaderObject::get_SelectedInOR() {
	return ((CObject*)this)->hoSelectedInOR;
}
HeaderObjectFlags HeaderObject::get_Flags() {
	return (HeaderObjectFlags)((CObject*)this)->hoFlags;
}
RunHeader* HeaderObject::get_AdRunHeader() {
	return (RunHeader*)((CObject*)this)->hoAdRunHeader;
}

void HeaderObject::set_NextSelected(short ns) {
	((CObject*)this)->hoNextSelected = ns;
}
void HeaderObject::set_SelectedInOR(bool b) {
	((CObject*)this)->hoSelectedInOR = b;
}
short event2::get_evtNum() {
	return ((tagEVT*)this)->evtCode.evtSCode.evtNum;
}
OINUM event2::get_evtOi() {
	return ((tagEVT*)this)->evtOi;
}
short event2::get_evtSize() {
	return ((tagEVT*)this)->evtSize;
}
std::int8_t event2::get_evtFlags() {
	return ((tagEVT*)this)->evtFlags;
}
void event2::set_evtFlags(std::int8_t evtF) {
	((tagEVT*)this)->evtFlags = evtF;
}
event2* event2::Next() {
	return (event2*)(((char*)this) + ((tagEVT*)this)->evtSize);
}
int event2::GetIndex() {
	//if (DarkEdif::IsFusion25)
	//	return this->evtInhibit;
	return ((tagEVT*)this)->evtIdentifier;
}

HeaderObject* RunObject::get_rHo() {
	return (HeaderObject*)this;
}
rCom* RunObject::get_roc() {
	return (rCom*)((CObject*)this)->roc;
}
rMvt* RunObject::get_rom() {
	return (rMvt*)((CObject*)this)->rom;
}
rAni* RunObject::get_roa() {
	return (rAni*)((CObject*)this)->roa;
}
Sprite* RunObject::get_ros() {
	return (Sprite*)((CObject*)this)->ros;
}
AltVals* RunObject::get_rov() {
	return (AltVals*)((CObject*)this)->rov;
}
RunObjectMultiPlatPtr objectsList::GetOblOffsetByIndex(std::size_t index) {
	return (RunObjectMultiPlatPtr)((CObjInfo**)this)[index];
}

int objInfoList::get_EventCount() const {
	return ((CObjInfo*)this)->oilEventCount;
}
int objInfoList::get_EventCountOR() const {
	return ((CObjInfo*)this)->oilEventCountOR;
}
short objInfoList::get_ListSelected() const {
	return ((CObjInfo*)this)->oilListSelected;
}
int objInfoList::get_NumOfSelected() const {
	return ((CObjInfo*)this)->oilNumOfSelected;
}
short objInfoList::get_Oi() const {
	if ((long)this == 0xFFFF0000)
		return -1;
	return ((CObjInfo*)this)->oilOi;
}
int objInfoList::get_NObjects() const {
	return ((CObjInfo*)this)->oilNObjects;
}
short objInfoList::get_Object() const {
	return ((CObjInfo*)this)->oilObject;
}
const TCHAR* objInfoList::get_name() {
	return [((CObjInfo*)this)->oilName UTF8String];
}
void objInfoList::set_NumOfSelected(int ns) {
	((CObjInfo*)this)->oilNumOfSelected = ns;
}
void objInfoList::set_ListSelected(short sh) {
	((CObjInfo*)this)->oilListSelected = sh;
}
void objInfoList::set_EventCount(int ec) {
	((CObjInfo*)this)->oilEventCount = ec;
}
void objInfoList::set_EventCountOR(int ec) {
	((CObjInfo*)this)->oilEventCountOR = ec;
}
short objInfoList::get_QualifierByIndex(std::size_t index) const {
	return ((CObjInfo*)this)->oilQualifiers[index];
}

std::uint8_t eventGroup::get_evgNCond() const {
	return ((eventGroup2*)this)->evgNCond;
}
std::uint8_t eventGroup::get_evgNAct() const {
	return ((eventGroup2*)this)->evgNAct;
}
std::uint16_t eventGroup::get_evgIdentifier() const {
	return ((eventGroup2*)this)->evgFree;
}
std::uint16_t eventGroup::get_evgInhibit() const {
	return *(std::uint16_t*)&((eventGroup2*)this)->evgInhibit;
}

event2* eventGroup::GetCAByIndex(size_t index)
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (index >= (size_t)(((eventGroup2*)this)->evgNCond) + ((eventGroup2*)this)->evgNAct)
		return nullptr;

	event2* ret = (event2*)(&((event2*)this)[1]);
	for (size_t i = 0; i < index && ret; i++) {
		ret = ret->Next();
	}
	return ret;
}

#endif // APPLE

#endif // Apple or Android
