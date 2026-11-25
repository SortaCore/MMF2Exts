// Suppress warnings for popping up for DE's own internal usage
#define DARKEDIF_INTERNAL_INCLUDE
#include "Common.hpp"
#include "Edif.hpp"
#include "DarkEdif.hpp"
#include "ObjectSelection.hpp"
#include <functional>
#ifdef __APPLE__
#if MacBuild
#include "MMF2Lib/MMF2Runtime.h"
#endif // MacBuild
#include "MMF2Lib/CQualToOiList.h"
#include "MMF2Lib/CRunApp.h"
#include "MMF2Lib/CRCom.h"
#include "MMF2Lib/CRSpr.h"
#include "MMF2Lib/CRunFrame.h"
#include "MMF2Lib/CEventProgram.h"
#include "MMF2Lib/CObjInfo.h"
#include "MMF2Lib/CValue.h"
extern "C" void DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, generateEvent)(void*, int, int);
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
		//ActionCount = rhPtr->GetRH2ActionCount();
		//ActionLoopCount = rhPtr->GetRH2ActionLoopCount();
		set_NumOfSelected(-1);
		set_ListSelected(-1);
		set_EventCount(-5); // does not match rh2EventCount, so implictly selects all
		return;
	}

	set_NumOfSelected(get_NObjects());
	set_ListSelected(get_Object());
	set_EventCount(rhPtr->GetRH2EventCount());
	if (get_NObjects() == 0)
		return;

	// Get the oilist index for looping
	const short ourOiList = rhPtr->GetOIListIndexFromOi(get_Oi());

	// From there we can loop instances and re-select them
	for (auto ho : DarkEdif::ObjectIterator(rhPtr, ourOiList, DarkEdif::Selection::All, false))
		ho->get_rHo()->set_NextSelected(ho->get_rHo()->get_NumNext());
}

short RunHeader::GetOIListIndexFromOi(const short oi)
{
	if (oi == -1)
		LOGF(_T("OI -1 should not have been passed to GetOIListIndexFromOi().\n"));

	// Qualifier OI not expected
	if ((oi & 0x8000) != 0)
		LOGF(_T("OI %hi is a qualifier or invalid OI; it should not have been passed to GetOIListIndexFromOi().\n"), oi);

	// The OIList is a sparse array, but consistently increasing, as I understand it.
	// TODO: If this is not the case, the last > j, and j > oi checks need removing,
	// so that looping the entire iterator is permitted.
	// Also consider HeaderObject::get_OiList().
	short i = 0, j, last = -1;
	for (auto oiList : DarkEdif::AllOIListIterator(this))
	{
		j = oiList->get_Oi();

		if (j == oi)
			return i;

		// TODO: Until I know SOL is always consistently increasing, this is LOGF to abort the app
		if (last > j)
			LOGF(_T("Is SOL non-increasing? %hi > %hi. Contact DarkEdif developers.\n"), last, j);
		last = j;

		// Went past
		if (j > oi)
			LOGE(_T("Exceeded expected oi %hi with %hi.\n"), oi, j);
		++i;
	}

	// You're either looking up what is already a OIList index, as a OI,
	// or you're reading something else entirely. Either way, this is a ext dev issue.
	LOGF(_T("Could not find expected oi %hi.\n"), oi);
	return INT16_MIN;
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
#elif defined(__APPLE__)
	const short* const qoiList = ((CQualToOiList*)this)->qoiList;
	for (std::size_t i = first; qoiList[i] != -1; i += 2)
		list.push_back(qoiList[i]);
#else
	#error Unexpected platform
#endif
	return list;
}
std::vector<short> qualToOi::GetAllOiList() {
	return HalfVector(1);
}
std::vector<short> qualToOi::GetAllOi() {
	return HalfVector(0);
}

const TCHAR * CRunFrameMultiPlat::get_name()
{
#ifdef _WIN32
	return name;
#elif defined(__APPLE__)
	return [((CRunFrame*)this)->frameName UTF8String];
#elif defined(__ANDROID__)
	if (frameName.empty())
	{
		const jfieldID fieldID = threadEnv->GetFieldID(meClass, "frameName", "Ljava/lang/String;");
		JNIExceptionCheck();
		JavaAndCString javaFrameName((jstring)threadEnv->GetObjectField(me, fieldID));
		frameName = std::string(javaFrameName.str());
	}
	return frameName.c_str();
#else
#error Unexpected platform
#endif
}
RunHeader * CRunFrameMultiPlat::get_rhPtr()
{
#ifdef _WIN32
	return rhPtr;
#elif defined(__APPLE__)
	return (RunHeader *)((CRunFrame*)this)->app->run;
#elif defined(__ANDROID__)
	return this->runtime->ext->rhPtr;
#else
#error Unexpected platform
#endif
}

int CRunAppMultiPlat::get_nCurrentFrame()
{
#ifdef _WIN32
	return nCurrentFrame;
#elif defined(__APPLE__)
	return ((CRunApp*)this)->currentFrame;
#elif defined(__ANDROID__)
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!nCurrentFrame.has_value())
	{
		static jfieldID fieldID = threadEnv->GetFieldID(meClass, "currentFrame", "I");
		JNIExceptionCheck();
		nCurrentFrame = threadEnv->GetIntField(me, fieldID);
		JNIExceptionCheck();
	}
	return nCurrentFrame.value();
#else
#error Unexpected platform
#endif
}

CRunAppMultiPlat* CRunAppMultiPlat::get_ParentApp() {
#ifdef _WIN32
	return ParentApp;
#elif defined(__APPLE__)
	return (CRunAppMultiPlat*)((CRunApp*)this)->parentApp;
#elif defined(__ANDROID__)
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
#else
	#error Unexpected platform
#endif
}

// Gets current Fusion frame number (1+)
std::size_t CRunAppMultiPlat::GetNumFusionFrames() {
#ifdef _WIN32
	return hdr.gaNbFrames;
#elif defined(__APPLE__)
	return (std::size_t)((CRunApp*)this)->gaNbFrames;
#elif defined(__ANDROID__)
	if (numTotalFrames == 0)
	{
		jfieldID fieldID = threadEnv->GetFieldID(meClass, "gaNbFrames", "I");
		JNIExceptionCheck();
		jint totalFrames = threadEnv->GetIntField(me, fieldID);
		JNIExceptionCheck();
		numTotalFrames = (std::size_t)totalFrames;
	}
	return numTotalFrames;
#else
	#error Unexpected platform
#endif
}

// Gets frame pointer
CRunFrameMultiPlat* CRunAppMultiPlat::get_Frame()
{
#ifdef _WIN32
	return Frame;
#elif defined(__APPLE__)
	return (CRunFrameMultiPlat *)((CRunApp*)this)->frame;
#elif defined(__ANDROID__)
	// runtime is only unset if CRunAppMultiPlat wrapper was made in the EndApp callback,
	// which is run when frame is already null.
	if (!runtime)
	{
		LOGE(_T("Cannot read rhPtr during end of app.\n"));
		return nullptr;
	}

	if (!frame)
	{
		const jfieldID fieldID = threadEnv->GetFieldID(meClass, "frame", "LApplication/CRunFrame;");
		JNIExceptionCheck();
		const jobject javaCurrentFrame = threadEnv->GetObjectField(me, fieldID);
		JNIExceptionCheck();
		assert(javaCurrentFrame); // should never be null
		frame = std::make_unique<CRunFrameMultiPlat>(javaCurrentFrame, runtime);
	}
	return frame.get();
#else
#error Unexpected platform
#endif
}

// Gets application name
const TCHAR * CRunAppMultiPlat::get_name()
{
#ifdef _WIN32
	// TODO: Is this ANSI or Unicode?
	return name;
#elif defined(__APPLE__)
	// NSString to C++ string
	return [((CRunApp*)this)->appName UTF8String];
#elif defined(__ANDROID__)
	if (appName.empty())
	{
		const jfieldID fieldID = threadEnv->GetFieldID(meClass, "appName", "Ljava/lang/String;");
		JNIExceptionCheck();
		JavaAndCString javaAppName((jstring)threadEnv->GetObjectField(me, fieldID));
		JNIExceptionCheck();
		appName = std::tstring(javaAppName.str());
	}
	return appName.c_str();
#else
#error Unexpected platform
#endif
}

// Gets application filepath
const TCHAR * CRunAppMultiPlat::get_appFileName()
{
#ifdef _WIN32
	// TODO: Is this ANSI or Unicode?
	return appFileName;
#elif defined(__ANDROID__) || defined(__APPLE__)
	LOGV("Reading CRunAppMultiPlat::appFileName variable, not available in non-Windows\n");
	return "<Not available in non-Windows>";
#else
#error Unexpected platform
#endif
}
// Gets editor filepath
const TCHAR * CRunAppMultiPlat::get_editorFileName()
{
#ifdef _WIN32
	// TODO: Is this ANSI or Unicode?
	return editorFileName;
#elif defined(__APPLE__)
	// NSString to C++ string
	return [((CRunApp*)this)->appEditorPathname UTF8String];
#elif defined(__ANDROID__)
	// available in chunk CHUNK_APPEDITORFILENAME 0x222E, but high effort
	LOGV("Reading CRunAppMultiPlat::editorFileName variable, not available in Android\n");
	return "<Not available in Android>";
#else
#error Unexpected platform
#endif
}

// Gets original MFA filepath
const TCHAR * CRunAppMultiPlat::get_targetFileName()
{
#ifdef _WIN32
	// TODO: Is this ANSI or Unicode?
	return targetFileName;
#elif defined(__APPLE__)
	// NSString to C++ string
	return [((CRunApp*)this)->appTargetPathname UTF8String];
#elif defined(__ANDROID__)
	// available in chunk CHUNK_APPTARGETFILENAME 0x222F, but high effort
	LOGV("Reading CRunAppMultiPlat::targetFileName variable, not available in Android\n");
	return "<Not available in Android>";
#else
#error Unexpected platform
#endif
}


#if TEXT_OEFLAG_EXTENSION
std::uint32_t Edif::Runtime::GetRunObjectTextColor() const
{
	if (extFont == NULL)
		return LOGF(_T("Can't get object text color: font was not set.\n")), 0;
	return extFont->fontColor;
}
void Edif::Runtime::SetRunObjectTextColor(const std::uint32_t color)
{
	if (extFont == NULL)
		return LOGF(_T("Can't set object font: font was not set.\n"));
	extFont->fontColor = color;
}

void Edif::Runtime::SetRunObjectFont(const void* const pLf, const void* const pRc)
{
	if (extFont == NULL)
		return LOGF(_T("Can't set object font: font was not set.\n"));

#ifdef _WIN32
	extFont->SetFont((const LOGFONT *)pLf);
#elif defined(__ANDROID__)
	extFont->SetFont((const jobject)pLf);
#else
	extFont->SetFont(pLf);
#endif
	if (fontChangedFunc)
		(ext->*fontChangedFunc)(false, (DarkEdif::Rect *)pRc);
}
#endif

#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
void Edif::Runtime::SetSurfaceWithSize(int width, int height)
{
	if (surf || ext->surf)
		LOGF(_T("Don't double-setup the Extension display.\n"));
	surf = std::make_unique<DarkEdif::Surface>(ext->rhPtr, true, true, width, height, true);
	surf->SetAsExtensionDisplay(ext);
	ext->surf = surf.get();
}
#endif

#ifdef _WIN32
// Inited in DarkEdif ctor; true if 296.09
bool windowsRuntimeSavesActionContext;
// True if build 292.02+, where CF2.5+ was released. Whether that build has the child event variable is not proven yet.
bool windowsRuntimeHasChildEvents;
#endif

void Edif::Runtime::GenerateEvent(int EventID)
{
	DarkEdif::DieIfCallerIsNotMainThread("Runtime.GenerateEvent");

	RunHeader* const rh = ObjectSelection.rhPtr;
#ifdef _WIN32
	RunHeader2& rh2 = rh->rh2;
#elif defined(__APPLE__)
	CRun* const run = (CRun*)rh;
	CEventProgram* const evp = run->rhEvtProg;
#elif defined(__ANDROID__)
	CEventProgram* const evp = rh->get_EventProgram();
#else
	#error Unknown platform
#endif

	// I don't think exts should ever do it; the runtime barely expects generated events, and even with
	// this new event context and object-selection preserving, it's still not intended usage.
	// Handle is the best place to run something if Display or another thread needs to generate it.
#ifndef DARKEDIF_DISABLE_GENERATE_EVENT_CALL_CHECK
	if (curCall != (curCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression | RuntimeToExtCall::Handle)))
		return LOGF(_T("Running events in unexpected runtime call %d. Expecting event generating from A/C/E or Handle only.\n"), (int)curCall);
#endif // DARKEDIF_DISABLE_GENERATE_EVENT_CALL_CHECK

	// As an example of why you don't generate in CreateRunObject: the runtime hasn't finished loading all events and exts yet.
#ifdef __APPLE__
	if (!run->runtimeIsReady)
		return LOGE(_T("Can't generate events yet, runtime is not ready."));
#endif // Apple

	// While you can use EventID 0x8000 to store two longs in param0/1, that sort of hack isn't really necessary
	// when you can just save them in the ext and read them back in the condition function.
	if ((EventID & 0x7FFF) != EventID|| (std::size_t)EventID > SDK->ConditionFunctions.size())
		return LOGF(_T("Generating invalid event ID %i. Max condition ID is %zu.\n"), EventID, SDK->ConditionFunctions.size() - 1);

	const RuntimeToExtCall lastCall = curCall;

	// Re-entrant/recursion check
	if (++generatingEventsCounter > 125)
	{
		LOGE(_T("Generating events has recursed into stack overflow territory. %s is generating event ID %hu.\n"),
			hoPtr->get_OiList()->get_name(), (std::uint16_t)(EventID & 0x7FFF));
	}

	// The recorded A/C phase covers expressions nested within this instance's A/C.
	// An expression called by another object has no recorded parent phase here;
	// only that case needs the runtime's action/condition distinction.

	RuntimeToExtCall curCall = this->curCall;
	// If running an action in this ext, or running an expression and rh2ActionOn shows it is within an action,
	// then we must save action context; ditto for conditions
	if ((curCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) == RuntimeToExtCall::Expression)
		curCall |= rh->GetRH2ActionOn() ? RuntimeToExtCall::Action : RuntimeToExtCall::Condition;
	const bool preserveActions = (curCall & RuntimeToExtCall::Action) != RuntimeToExtCall::Unset;
	const bool preserveConditions = (curCall & RuntimeToExtCall::Condition) != RuntimeToExtCall::Unset;
#if defined(_WIN32) || defined(__APPLE__)
	EventGroupMP* const group = curCall != RuntimeToExtCall::Handle ? rh->get_EventGroup() : nullptr;
	const bool preserveOR = preserveConditions && group &&
		(rh->GetEVGFlags() & EventGroupFlags::OrInGroup) != EventGroupFlags::None;
	const int oldEventCount = rh->GetRH2EventCount(),
		oldORCount = rh->GetRH4EventCountOR(),
		oldActionCount = rh->GetRH2ActionCount();
#elif !defined(__ANDROID__)
	#error Unknown platform
#endif
	const long oldParam1 = param1, oldParam2 = param2;
	const int oldEventNumber = hoPtr->get_EventNumber();
	const HeaderObjectFlags oldTrueEvent = hoPtr->get_Flags() & HeaderObjectFlags::TrueEvent;
	const auto oldCurrentParam = hoPtr->get_CurrentParam();

#ifdef _DEBUG
	// Check the current flag for repeat isn't reset unexpectedly; a condition/action that has REPEAT on
	// shouldn't resume with it off or vice versa.
	event2* const callerAction = preserveActions ? rh->GetRH4ActionStart() : nullptr;
	const bool callerActionRepeat = callerAction && (callerAction->get_evtFlags() & ACTFLAGS_REPEAT) != 0;
#endif

#if defined(_WIN32) || defined(__APPLE__)
	struct Context {
		int code;
		short oi;
		bool enablePick;
#ifdef __APPLE__
		int param;
#endif
	};
	struct ActionState { bool loop; int pass; } actionState;
	struct ConditionState {
#ifdef _WIN32
		struct Cursors {
			qualToOi* qualifier;
			HeaderObject* object;
			objInfoList* oiList;
			objInfoList* previous;
		} cursors;
		short conditionsFalse;
		char stop;
#elif defined(__APPLE__)
		void* qualifier;
		int qualifierPos;
		CObject* object;
		int oiList;
		CObject* previous;
		CObjInfo* previousOi;
		BOOL conditionsFalse, stop;
		int qualifierIndices[4];
#endif
		short type;
		std::uint32_t picks[4];
	};
	struct Pending {
#ifdef _WIN32
		struct Shuffle {
			HeaderObject** buffer;
			HeaderObject** position;
			int count;
		} shuffle;
		void (*endRoutine)();
		void* forEach;
#elif defined(__APPLE__)
		CArrayList* shuffle;
		BOOL endForEach;
		LPFOREACH forEach;
		event* child;
#endif
	} pending;
#endif

#ifdef _WIN32
	// Fusion 296.9 GenerateEvent saves/restores some fields, namely
	// rh4ActionStart, rh4ExpToken and rh2ActionOn.
	// MMF2 and earlier runtimes don't save them.
	// We indicate with windowsRuntimeSavesActionContext.
	const bool actionOn = !windowsRuntimeSavesActionContext && rh->GetRH2ActionOn();
	EventGroupMP* const oldGroup = group;
	event2* const oldActionStart = preserveActions && !windowsRuntimeSavesActionContext ? rh->rh4.rh4ActionStart : nullptr;
	expression* const oldToken = ((lastCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) != RuntimeToExtCall::Unset) && !windowsRuntimeSavesActionContext ? rh->rh4.rh4ExpToken : nullptr;
	EventParam* const oldParamZero = ParamZero;
	const Context context = { rh->rhCurCode, rh->rhCurOi, rh2.rh2EnablePick };
	if (preserveActions)
		actionState = { rh2.rh2ActionLoop, rh2.rh2ActionLoopCount };
	ConditionState* const conditionState = preserveConditions ?
		static_cast<ConditionState*>(_alloca(sizeof(ConditionState))) : nullptr;
	static_assert(sizeof(ConditionState::Cursors) == offsetof(RunHeader2, rh2EventPrev) +
		sizeof(rh2.rh2EventPrev) - offsetof(RunHeader2, rh2EventQualPos), "Condition cursor layout changed");
	static_assert(offsetof(RunHeader4, rh4PickFlags3) + sizeof(std::uint32_t) -
		offsetof(RunHeader4, rh4PickFlags0) == sizeof(conditionState->picks), "Pick flag layout changed");
	if (preserveConditions) {
		// Shallow pointer values only; the objects and OIs they refer to are not copied.
		std::memcpy(&conditionState->cursors, &rh2.rh2EventQualPos, sizeof(conditionState->cursors));
		conditionState->type = rh2.rh2EventType;
		conditionState->conditionsFalse = rh->rh4.rh4ConditionsFalse;
		conditionState->stop = rh->rh3.rh3DoStop;
		std::memcpy(conditionState->picks, &rh->rh4.rh4PickFlags0, sizeof(conditionState->picks));
	}
	static_assert(sizeof(Pending::Shuffle) == offsetof(RunHeader2, rh2ShuffleNumber) +
		sizeof(rh2.rh2ShuffleNumber) - offsetof(RunHeader2, rh2ShuffleBuffer), "Shuffle layout changed");
	pending.endRoutine = rh2.rh2ActionEndRoutine;
	std::memcpy(&pending.shuffle, &rh2.rh2ShuffleBuffer, sizeof(pending.shuffle));
	pending.forEach = rh->rh4.rh4ForEachs;
	event2* const child = windowsRuntimeSavesActionContext && DarkEdif::IsFusion25Plus ? rh->rhChildEventToExecute : nullptr;
	const std::size_t valuesSize = 2 * sizeof(CValueMultiPlat);
	unsigned char* const values = !((lastCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) != RuntimeToExtCall::Unset) ? nullptr : static_cast<unsigned char*>(_alloca(valuesSize));
	static_assert(offsetof(RunHeader4, rh4ExpValue2) - offsetof(RunHeader4, rh4ExpValue1) ==
		sizeof(CValueMultiPlat), "Parameter cache layout changed");
	if (((lastCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) != RuntimeToExtCall::Unset))
		// The runtime's two adjacent caches contain non-owning forceValue records.
		std::memcpy(values, &rh->rh4.rh4ExpValue1, valuesSize);
#elif defined(__APPLE__)
	// At some point in CF2.5 before the checked 296.9 beta runtimes,
	// iOS/Mac added group/actionOn/expression-token preservation to generateEvent.
	// Both now stack those fields and param[0]; the early iOS snapshot only
	// stacked param[0]. Do not duplicate those saves for current Apple runtimes.
	// All pointer members below are shallow.
	const Context context = { evp->rhCurCode, evp->rhCurOi, evp->rh2EnablePick != NO,
		evp->rhCurParam[1] };
	const int oldTempBase = run->rhBaseTempValues, oldTempValue = run->rhCurTempValue;
	if (preserveActions)
		actionState = { evp->rh2ActionLoop != NO, evp->rh2ActionLoopCount };
	ConditionState* const conditionState = preserveConditions ?
		static_cast<ConditionState*>(__builtin_alloca(sizeof(ConditionState))) : nullptr;
	if (preserveConditions) {
		conditionState->qualifier = evp->rh2EventQualPos;
		conditionState->qualifierPos = evp->rh2EventQualPosNum;
		conditionState->object = evp->rh2EventPos;
		conditionState->oiList = evp->rh2EventPosOiList;
		conditionState->previous = evp->rh2EventPrev;
		conditionState->previousOi = evp->rh2EventPrevOiList;
		conditionState->type = evp->rh2EventType;
		conditionState->conditionsFalse = evp->rh4ConditionsFalse;
		conditionState->stop = evp->rh3DoStop;
		std::memcpy(conditionState->qualifierIndices, &evp->qualOilPtr, sizeof(conditionState->qualifierIndices));
		std::memcpy(conditionState->picks, evp->rh4PickFlags, sizeof(conditionState->picks));
	}
	pending = { evp->rh2ShuffleBuffer, evp->bEndForEach,
		run->rh4ForEachs, run->rhChildEventToExecute };
	const int oldTokenIndex = !((lastCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) != RuntimeToExtCall::Unset) ? 0 : run->rh4CurToken;
	CValue* value1 = !((lastCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) != RuntimeToExtCall::Unset) ? nil : [[CValue alloc] initWithValue:run->rh4ExpValue1];
	CValue* value2 = !((lastCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) != RuntimeToExtCall::Unset) ? nil : [[CValue alloc] initWithValue:run->rh4ExpValue2];
	void* const oldCEvent = curCEvent;
#elif defined(__ANDROID__)
	// These four condition cursors are package-private, so they require JNI access.
	// The Java guard also preserves OI/qualifier cursors and object selection,
	// avoiding JNI calls inside the object and OI loops.
	int qualifierIndices[4];
	if (preserveConditions) {
		qualifierIndices[0] = evp->get_qualOilPtr();
		qualifierIndices[1] = evp->get_qualOilPos();
		qualifierIndices[2] = evp->get_qualOilPtr2();
		qualifierIndices[3] = evp->get_qualOilPos2();
	}
	// Keep this wrapper alive while the Java group is natively stacked. Its action
	// wrappers refer to it; invalidation must not destroy it midway through the call.
	std::unique_ptr<EventGroupMP> groupWrapper = std::move(evp->eventGrp);
	std::unique_ptr<event2> actionWrapper = std::move(rh->rh4ActStart);
	global<jobject> oldCEvent = curCEvent.swap_out();
	global<jobject> oldActStart = curRH4ActStart.swap_out();
	jobject const oldActBasedOnEvent = curRH4ActBasedOnCEventOnly;
#else
	#error Unknown platform
#endif

#if defined(_WIN32) || defined(__APPLE__)
	struct SelectionState { objInfoList* oil; std::size_t first, count; };
	struct OrMark { int fixed; bool selected; };
	std::vector<SelectionState> selections;
	std::vector<int> selected;
	std::vector<objInfoList*> orOils;
	std::vector<OrMark> orMarks;
	struct OiCursor {
		objInfoList* oil;
		decltype(std::declval<objInfoList&>().get_oilCurrentRoutine()) routine;
		int current, next, loop;
		bool nextFlag;
	};
	struct QualCursor {
		qualToOi* qoi;
		decltype(std::declval<qualToOi&>().get_qoiCurrentRoutine()) routine;
		int current, next, pos, loop;
		bool nextFlag, selectedFlag;
	};
	std::vector<OiCursor> oiCursors;
	std::vector<QualCursor> qualCursors;
#ifdef _WIN32
	std::vector<objInfoList*> modifiedOils;
	if (preserveOR && windowsRuntimeSavesActionContext && rh->rhModifiedOiList)
		modifiedOils.assign(rh->rhModifiedOiList, rh->rhModifiedOiListEnd);
#endif // _WIN32

	// Preserve object selection only when returning to an A/C/E, not Handle
	if (((lastCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) != RuntimeToExtCall::Unset))
	{
		for (objInfoList* oil : DarkEdif::AllOIListIterator(rh))
		{
			const short oiList = rh->GetOIListIndexFromOi(oil->get_Oi());
			if (preserveActions)
			{
				if (oil->get_oilActionCount() == oldActionCount)
				{
					oiCursors.push_back({ oil, oil->get_oilCurrentRoutine(), oil->get_oilCurrentOi(),
						oil->get_oilNext(), oil->get_oilActionLoopCount(), oil->get_oilNextFlag() });
				}
				for (std::size_t i = 0; i < MAX_QUALIFIERS; ++i) {
					const short qualifier = oil->get_QualifierByIndex(i);
					if (qualifier == -1)
						break;
					qualToOi* qoi = rh->GetQualToOiListByOffset((std::uint16_t)qualifier);
					if (!qoi)
						continue;
					if (std::any_of(qualCursors.begin(), qualCursors.end(), [qoi](const QualCursor& c) { return c.qoi == qoi; })) continue;
					if (qoi->get_qoiActionCount() == oldActionCount)
						qualCursors.push_back({ qoi, qoi->get_qoiCurrentRoutine(), qoi->get_qoiCurrentOi(),
							qoi->get_qoiNext(), qoi->get_qoiActionPos(), qoi->get_qoiActionLoopCount(),
							qoi->get_qoiNextFlag(), qoi->get_qoiSelectedFlag() });
				}
			}

			// Save all selected in event, including all instances and those explicitly set to zero instances,
			// which may happen with qualifiers
			if (oil->get_EventCount() == oldEventCount) {
				selections.push_back({ oil, selected.size(), 0 });
				for (RunObjectMultiPlatPtr ro : DarkEdif::ObjectIterator(rh, oiList, DarkEdif::Selection::Explicit, true))
					selected.push_back(ro->get_rHo()->GetFixedValue());
				selections.back().count = selected.size() - selections.back().first;
			}
			if (preserveOR && oil->get_EventCountOR() == oldORCount) {
				orOils.push_back(oil);
				for (RunObjectMultiPlatPtr ro : DarkEdif::ObjectIterator(rh, oiList, DarkEdif::Selection::All, true))
					orMarks.push_back({ ro->get_rHo()->GetFixedValue(), ro->get_rHo()->get_SelectedInOR() });
			}
		}
	}

#endif // Windows/Apple selection snapshot

	// Nested callbacks must not inherit the caller's phase on this instance.
	curCall = RuntimeToExtCall::Unset;

	// Detach this event's queued actions before generating, so the generated event doesn't inherit them
	// and run them as it closes
#ifdef _WIN32
	rh2.rh2ActionEndRoutine = nullptr;
	std::memset(&rh2.rh2ShuffleBuffer, 0, sizeof(pending.shuffle));
	rh->rh4.rh4ForEachs = nullptr;
	if (windowsRuntimeHasChildEvents)
		rh->rhChildEventToExecute = nullptr;
	if (!windowsRuntimeSavesActionContext && actionOn)
		rh2.rh2ActionOn = false;
	CallRunTimeFunction2(hoPtr, RFUNCTION::GENERATE_EVENT, EventID, 0);
#elif defined(__APPLE__)
	evp->bEndForEach = NO;
	evp->rh2ShuffleBuffer = nil;
	run->rh4ForEachs = nullptr;
	run->rhChildEventToExecute = nullptr;
	// computeEventList resets the temporary-value cursor to this base. Keep
	// nested events above the caller's live CValues rather than overwriting them.
	run->rhBaseTempValues = oldTempValue;
	DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, generateEvent)(objCExtPtr, EventID, 0);
#elif defined(__ANDROID__)
	rh->InvalidatedByNewGeneratedEvent();
	hoPtr->InvalidatedByNewGeneratedEvent();
	threadEnv->CallVoidMethod(javaExtPtr, generateEventMethodID, EventID, (jboolean)preserveActions, (jboolean)preserveConditions);
	JNIExceptionCheck();
	// Invalidate again in case the generated event re-entered into this ext and set them
	rh->InvalidatedByNewGeneratedEvent();
	hoPtr->InvalidatedByNewGeneratedEvent();
#else
	#error Unknown platform
#endif
#if defined(_WIN32) || defined(__APPLE__)
	const int eventCount = std::max(oldEventCount, rh->GetRH2EventCount()) + 1;
	const int orCount = preserveOR ? std::max(oldORCount, rh->GetRH4EventCountOR()) + 1 : rh->GetRH4EventCountOR();
	const int actionCount = preserveActions ? std::max(oldActionCount, rh->GetRH2ActionCount()) + 1 : rh->GetRH2ActionCount();
#endif

#if defined(_WIN32)
	rh->rhCurCode = context.code;
	rh->rhCurOi = context.oi;
	rh2.rh2EnablePick = context.enablePick;
	rh2.rh2ActionEndRoutine = pending.endRoutine;
	std::memcpy(&rh2.rh2ShuffleBuffer, &pending.shuffle, sizeof(pending.shuffle));
	rh->rh4.rh4ForEachs = pending.forEach;
	if (preserveActions) {
		rh2.rh2ActionLoop = actionState.loop;
		rh2.rh2ActionLoopCount = actionState.pass;
	}
	if (preserveConditions) {
		std::memcpy(&rh2.rh2EventQualPos, &conditionState->cursors, sizeof(conditionState->cursors));
		rh2.rh2EventType = conditionState->type;
		rh->rh4.rh4ConditionsFalse = conditionState->conditionsFalse;
		rh->rh3.rh3DoStop = conditionState->stop;
		std::memcpy(&rh->rh4.rh4PickFlags0, conditionState->picks, sizeof(conditionState->picks));
	}
	if (windowsRuntimeHasChildEvents)
		rh->rhChildEventToExecute = child;
	rh->rhEventGroup = oldGroup;
	if (!windowsRuntimeSavesActionContext) {
		rh2.rh2ActionOn = actionOn;
		if (preserveActions)
			rh->rh4.rh4ActionStart = oldActionStart;
		if (((lastCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) != RuntimeToExtCall::Unset))
			rh->rh4.rh4ExpToken = oldToken;
	}
	if (((lastCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) != RuntimeToExtCall::Unset))
		std::memcpy(&rh->rh4.rh4ExpValue1, values, valuesSize);
	if (preserveOR && windowsRuntimeHasChildEvents && rh->rhModifiedOiList) {
		std::copy(modifiedOils.begin(), modifiedOils.end(), rh->rhModifiedOiList);
		rh->rhModifiedOiListEnd = rh->rhModifiedOiList + modifiedOils.size();
	}
#elif defined(__APPLE__)
	run->rhBaseTempValues = oldTempBase;
	run->rhCurTempValue = oldTempValue;
	evp->rhCurCode = context.code;
	evp->rhCurOi = context.oi;
	evp->rh2EnablePick = context.enablePick;
	evp->rhCurParam[1] = context.param;
	evp->bEndForEach = pending.endForEach;
	evp->rh2ShuffleBuffer = pending.shuffle;
	run->rh4ForEachs = pending.forEach;
	run->rhChildEventToExecute = pending.child;
	if (preserveActions) {
		evp->rh2ActionLoop = actionState.loop;
		evp->rh2ActionLoopCount = actionState.pass;
	}
	if (preserveConditions) {
		evp->rh2EventQualPos = (CQualToOiList *)conditionState->qualifier;
		evp->rh2EventQualPosNum = conditionState->qualifierPos;
		evp->rh2EventPos = conditionState->object;
		evp->rh2EventPosOiList = conditionState->oiList;
		evp->rh2EventPrev = conditionState->previous;
		evp->rh2EventPrevOiList = conditionState->previousOi;
		evp->rh2EventType = conditionState->type;
		evp->rh4ConditionsFalse = conditionState->conditionsFalse;
		evp->rh3DoStop = conditionState->stop;
		std::memcpy(&evp->qualOilPtr, conditionState->qualifierIndices, sizeof(conditionState->qualifierIndices));
		std::memcpy(evp->rh4PickFlags, conditionState->picks, sizeof(conditionState->picks));
	}
	if (((lastCall & (RuntimeToExtCall::Action | RuntimeToExtCall::Condition | RuntimeToExtCall::Expression)) != RuntimeToExtCall::Unset)) {
		run->rh4CurToken = oldTokenIndex;
		[run->rh4ExpValue1 forceValue:value1];
		[run->rh4ExpValue2 forceValue:value2];
	}
	[value1 release];
	[value2 release];
#elif defined(__ANDROID__)
	if (preserveConditions) {
		evp->set_qualOilPtr(qualifierIndices[0]);
		evp->set_qualOilPos(qualifierIndices[1]);
		evp->set_qualOilPtr2(qualifierIndices[2]);
		evp->set_qualOilPos2(qualifierIndices[3]);
	}
	// Refresh wrappers after nested execution and restoration, including action counts.
	rh->InvalidatedByNewGeneratedEvent();
	hoPtr->InvalidatedByNewGeneratedEvent();
	evp->eventGrp = std::move(groupWrapper);
	rh->rh4ActStart = std::move(actionWrapper);
	curCEvent = oldCEvent.swap_out();
	curRH4ActStart = oldActStart.swap_out();
	curRH4ActBasedOnCEventOnly = oldActBasedOnEvent;
#else
	#error Unknown platform
#endif
#if defined(_WIN32) || defined(__APPLE__)
	rh->SetRH2ActionCount(actionCount);
	rh->SetRH4EventCountOR(orCount);
	rh->SetRH2EventCount(eventCount);
	for (const OiCursor& c : oiCursors) {
		c.oil->set_oilCurrentRoutine(c.routine);
		c.oil->set_oilCurrentOi(c.current);
		c.oil->set_oilNext(c.next);
		c.oil->set_oilActionLoopCount(c.loop);
		c.oil->set_oilNextFlag(c.nextFlag);
		c.oil->set_oilActionCount(actionCount);
	}
	for (const QualCursor& c : qualCursors) {
		c.qoi->set_qoiCurrentRoutine(c.routine);
		c.qoi->set_qoiCurrentOi(c.current);
		c.qoi->set_qoiNext(c.next);
		c.qoi->set_qoiActionPos(c.pos);
		c.qoi->set_qoiActionLoopCount(c.loop);
		c.qoi->set_qoiNextFlag(c.nextFlag);
		c.qoi->set_qoiSelectedFlag(c.selectedFlag);
		c.qoi->set_qoiActionCount(actionCount);
	}
#endif // Windows/Apple action cursors
#ifdef _DEBUG
	// Re-entry may touch this action record, but its per-instance repeat policy
	// is expected to be stable. Diagnose a change rather than restoring it.
	if (callerAction)
		assert(((callerAction->get_evtFlags() & ACTFLAGS_REPEAT) != 0) == callerActionRepeat);
#endif
#if defined(_WIN32) || defined(__APPLE__)
	for (const SelectionState& s : selections) {
		s.oil->SelectNone(ext->rhPtr);
		RunObjectMultiPlatPtr previous = nullptr;
		int count = 0;
		for (std::size_t i = s.first; i < s.first + s.count; ++i) {
			RunObjectMultiPlatPtr ro = RunObjPtrFromFixed(selected[i]);
			if (!ro)
				continue;
			if (previous)
				previous->get_rHo()->set_NextSelected(selected[i] & 0xFFFF);
			else
				s.oil->set_ListSelected(selected[i] & 0xFFFF);
			previous = ro;
			++count;
		}
		if (previous)
			previous->get_rHo()->set_NextSelected(-1);
		s.oil->set_NumOfSelected(count);
	}
	for (objInfoList* oil : orOils) {
		for (RunObjectMultiPlatPtr ro : DarkEdif::ObjectIterator(rh, rh->GetOIListIndexFromOi(oil->get_Oi()), DarkEdif::Selection::All, true))
			ro->get_rHo()->set_SelectedInOR(false);
		oil->set_EventCountOR(orCount);
	}
	for (const OrMark& mark : orMarks) {
		RunObjectMultiPlatPtr ro = RunObjPtrFromFixed(mark.fixed);
		if (ro)
			ro->get_rHo()->set_SelectedInOR(mark.selected);
	}

#endif // Windows/Apple selection restore

	hoPtr->set_EventNumber(oldEventNumber);
	hoPtr->set_CurrentParam(oldCurrentParam);
	hoPtr->set_Flags((hoPtr->get_Flags() & ~HeaderObjectFlags::TrueEvent) | oldTrueEvent);

#ifdef _WIN32
	ParamZero = oldParamZero;
#elif defined(__APPLE__)
	curCEvent = oldCEvent;
#elif defined(__ANDROID__)
	// Java event wrappers were restored above.
#else
	#error Unknown platform
#endif

	param1 = oldParam1;
	param2 = oldParam2;
	curCall = lastCall;

	// Don't think this can ever happen, but it's worth checking
	if (--generatingEventsCounter < 0)
	{
		LOGF(_T("Generating events has lost track of a generated event. %s just finished generating event ID %hu.\n"),
			hoPtr->get_OiList()->get_name(), (std::uint16_t)(EventID & 0x7FFF));
	}
}

void Edif::Runtime::PreDestroy()
{
	// This function executes directly before DestroyRunObject.
	// Note that recovery if saved event state or other runtime is going to be murdered is not possible,
	// we'll have to remake this object instantly with all its saved settings after telling the runtime it destroyed it,
	// which becomes unintended lingering behaviour.
	// Incidentally, while DestroyRunObject is short return on Windows, the runtime ignores the return code.
	// In theory a Destroy action should not be instant, but instead queue for rh4DestroyList to run at end of tick,
	// marking HeaderObjectFlags::Destroyed.
	if (curCall != RuntimeToExtCall::Unset)
		LOGF(_T("Cannot destroy this \"%s\" object, currently within its own call.\n"), ext->rdPtr->get_rHo()->get_OiList()->get_name());

	ext->Runtime.curCall = Edif::RuntimeToExtCall::Destroy;

	if (generatingEventsCounter != 0)
		LOGF(_T("Cannot destroy this \"%s\" object when it is midway through generating events.\n"),
			ext->rdPtr->get_rHo()->get_OiList()->get_name());
}


void HeaderObject::Destroy()
{
	if ((get_Flags() & HeaderObjectFlags::Destroyed) != HeaderObjectFlags::None)
		return;

#ifdef _WIN32
	// Run our own Object > Destroy action, which will trigger disappearing animation,
	// movement, sprite collision changes and fade out transitions.

	// Cache the call table, as it is shared amongst extensions
	static CallTables* const tables =
		reinterpret_cast<CallTables*>(CallRunTimeFunction2(this, RFUNCTION::GET_CALL_TABLES, 0, 0));
	RunHeader* const rh = this->get_AdRunHeader();
	objInfoList* const oil = this->get_OiList();
	const int oldActionCount = oil->get_oilActionCount(),
		oldActionPass = oil->get_oilActionLoopCount(),
		oldCurrent = oil->get_oilCurrentOi();
	const bool oldNextFlag = oil->get_oilNextFlag(), oldEnablePick = rh->rh2.rh2EnablePick,
		oldActionLoop = rh->rh2.rh2ActionLoop;

	// Update to our current event's action selection, and set only this object instance in the action
	oil->set_oilActionCount(rh->GetRH2ActionCount());
	oil->set_oilActionLoopCount(rh->GetRH2ActionLoopCount());
	oil->set_oilCurrentOi(this->get_Number());
	oil->set_oilNextFlag(false);

	alignas(event2) unsigned char actionStorage[sizeof(event2)] = {};
	event2* const action = reinterpret_cast<event2*>(actionStorage);
	action->evtSize = ACT_SIZE;
	action->evtType = this->hoType;
	action->evtNum = 24; // Common Destroy (ACT_EXTDESTROY).
	action->evtOi = this->get_Oi();
	action->evtOiList = (short)rh->GetOIListIndexFromOi(action->evtOi);
	// Call table slot is object type + NUMBEROF_SYSTEMTYPES: Active = 2 + 7.
	// Its first 80 actions are the common actions, shared with extensions.
	// This queues destroy for end of next event tick, but immediately sets HeaderObjectFlag::Destroyed.
	// DestroyRunObject is triggered either at that tick, or after Disappearing animation finishes.
	tables->pActions[2 + 7](action);

	// Restore action object selection
	oil->set_oilActionCount(oldActionCount);
	oil->set_oilActionLoopCount(oldActionPass);
	oil->set_oilCurrentOi(oldCurrent);
	oil->set_oilNextFlag(oldNextFlag);
	rh->rh2.rh2EnablePick = oldEnablePick;
	rh->rh2.rh2ActionLoop = oldActionLoop;
#elif defined(__APPLE__)
	CObject* const object = (CObject*)this;
	CRun* const run = object->hoAdRunHeader;
	CEventProgram* const evp = run->rhEvtProg;
	CObjInfo* const oil = object->hoOiList;
	const int oldActionCount = oil->oilActionCount, oldActionPass = oil->oilActionLoopCount,
		oldCurrent = oil->oilCurrentOi;
	const BOOL oldNextFlag = oil->oilNextFlag, oldEnablePick = evp->rh2EnablePick,
		oldActionLoop = evp->rh2ActionLoop, oldRepeat = evp->repeatFlag;
	event action = {};
	action.evtSize = ACT_SIZE;
	action.evtCode.evtSCode.evtType = object->hoType;
	action.evtCode.evtSCode.evtNum = 24; // Common Destroy (ACT_EXTDESTROY).
	action.evtOi = object->hoOi;
	action.evtOiList = (short)get_AdRunHeader()->GetOIListIndexFromOi(object->hoOi);
	@try {
		// Set our selection
		oil->oilActionCount = evp->rh2ActionCount;
		oil->oilActionLoopCount = evp->rh2ActionLoopCount;
		oil->oilCurrentOi = object->hoNumber;
		oil->oilNextFlag = NO;

		actDisappear(&action, run);
	}
	@finally {
		// Restore selection
		oil->oilActionCount = oldActionCount;
		oil->oilActionLoopCount = oldActionPass;
		oil->oilCurrentOi = oldCurrent;
		oil->oilNextFlag = oldNextFlag;
		evp->rh2EnablePick = oldEnablePick;
		evp->rh2ActionLoop = oldActionLoop;
		evp->repeatFlag = oldRepeat;
	}
#elif defined(__ANDROID__)
	// The Java wrapper handles destroying, as saving event context is faster on Java side
	threadEnv->CallStaticVoidMethod(runtime->javaExtPtrClass, Edif::Runtime::destroyObjectMethodID, me);
	JNIExceptionCheck();
	Flags.reset(); // HeaderObjectFlags::Destroy is now set
#else
	#error Unrecognized platform
#endif
}

#ifdef _WIN32
Edif::Runtime::Runtime(Extension * ext) : hoPtr(ext->rdPtr->get_rHo()), ext(ext),
	ObjectSelection(hoPtr->get_AdRunHeader())
{
	SDKPointer = Edif::SDK;
	DarkEdif::LateInit(ext);
}

Edif::Runtime::~Runtime()
{
}

void Edif::Runtime::Rehandle()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::REHANDLE, 0, 0);
}

void Edif::Runtime::PushEvent(int EventID)
{
	DarkEdif::DieIfCallerIsNotMainThread("Runtime.PushEvent");

	CallRunTimeFunction2(hoPtr, RFUNCTION::PUSH_EVENT, EventID, 0);
}

void * Edif::Runtime::Allocate(size_t size)
{
	return (void *) CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, size * sizeof(TCHAR));
}

TCHAR * Edif::Runtime::CopyString(const TCHAR * String)
{
	// Allows exts to call into other exts to read the string returns,
	// without guessing if the returned memory was heap or not
	if (runtimeCopyHeapAlloc)
		return _tcsdup(String);

	TCHAR * New = NULL;
	New = (TCHAR *) Allocate(_tcslen(String) + 1);
#pragma warning(suppress:4996)
	_tcscpy(New, String);

	return New;
}

char * Edif::Runtime::CopyStringEx(const char * String)
{
	if (runtimeCopyHeapAlloc)
		return _strdup(String);

	char * New = NULL;
	New = (char *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, (strlen(String) + 1) * sizeof(char));
#pragma warning(suppress:4996)
	strcpy(New, String);

	return New;
}


wchar_t * Edif::Runtime::CopyStringEx(const wchar_t * String)
{
	if (runtimeCopyHeapAlloc)
		return _wcsdup(String);

	wchar_t * New = NULL;
	New = (wchar_t *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, (wcslen(String) + 1) * sizeof(wchar_t));
#pragma warning(suppress:4996)
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

	return hoPtr->hoAdRunHeader->rh4.rh4Mv->mvCallFunction(NULL, CallFunctionIDs::ISHWA, 0, 0, 0) == 1;
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
		const int i = hoPtr->hoAdRunHeader->rhApp->hdr.gaOtherFlags;

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

#if TEXT_OEFLAG_EXTENSION
// Return the font used by the object.
void FusionAPI GetRunObjectFont(RUNDATA* rdPtr, LOGFONT* pLf)
{
#pragma DllExportHint
	((RunObject*)rdPtr)->GetExtension()->Runtime.GetRunObjectFont(pLf);
}
void Edif::Runtime::GetRunObjectFont(LOGFONT* pLf) const
{
	if (extFont == NULL || !extFont->logFont)
		return LOGF(_T("Can't get object font: font was not set."));
	memcpy(pLf, extFont->logFont.get(), sizeof(LOGFONT));
}
// Return the text color of the object.
COLORREF FusionAPI GetRunObjectTextColor(RUNDATA* rdPtr)
{
#pragma DllExportHint
	return ((RunObject*)rdPtr)->GetExtension()->Runtime.GetRunObjectTextColor();
}
// Change the font used by the object.
void FusionAPI SetRunObjectFont(RUNDATA* rdPtr, LOGFONT* pLf, RECT* pRc)
{
#pragma DllExportHint
	((RunObject*)rdPtr)->GetExtension()->Runtime.SetRunObjectFont(pLf, pRc);
}
// Change the text color of the object.
void FusionAPI SetRunObjectTextColor(RUNDATA* rdPtr, COLORREF rgb)
{
#pragma DllExportHint
	((RunObject*)rdPtr)->GetExtension()->Runtime.SetRunObjectTextColor(rgb);
}
#endif // TEXT_OEFLAG_EXTENSION

#endif // _WIN32

bool Edif::Runtime::IsUnicode()
{
#ifdef _WIN32
	return hoPtr->hoAdRunHeader->rh4.rh4Mv->mvCallFunction(NULL, CallFunctionIDs::ISUNICODE, 0, 0, 0) == 1;
#else
	// At this point in DarkEdif dev, Unicode is assumed in non-Windows runtime, as it's assumed to be CF2.5.
	// This is because the Android exporter in Fusion 2.0 only went to Android OS v4.3, API 18,
	// which ended in 2013 and was before 64-bit phones were even introduced in API 21.
	return true;
#endif
}

std::uint8_t RunSprite::GetAlphaBlendCoefficient() const {
	return 255 - (get_EffectParam() >> 24);
}
std::uint32_t RunSprite::GetRGBCoefficient() const {
	return get_EffectParam() & 0xFFFFFFU;
}

#ifdef _WIN32
event2 &Edif::Runtime::CurrentEvent()
{
	return *(event2 *) (((char *) param1) - CND_SIZE);
}

RunSpriteFlag RunSprite::get_Flags() const {
	return rsFlags;
}
BlitOperation RunSprite::get_Effect() const {
	return rsEffect;
}
std::uint32_t RunSprite::get_layer() const {
	return rsLayer;
}
// Returns a mix of alpha + color blend coefficient
int RunSprite::get_EffectParam() const {
	return rsEffectParam;
}
int RunSprite::get_EffectShader() const {
	return -1;
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
	if (i >= GetAltValueCount())
		return nullptr;
	return DarkEdif::IsFusion25 ? &CF25.Values[i] : &MMF2.rvpValues[i];
#else
	throw std::runtime_error("Not implemented");
#endif
}

void AltVals::SetAltStringAtIndex(const std::size_t i, const std::tstring_view& str) {
	if (i >= GetAltStringCount())
	{
		if (!DarkEdif::IsFusion25)
			return LOGE(_T("Cannot set alt string at index %zu, invalid index.\n"), i);

		// TODO: if this does not work, just throw
#ifdef _WIN32
		void * v = mvReAlloc(Edif::SDK->mV, (void *)CF25.Strings, (int)i * sizeof(TCHAR *));
#else
		void* v = malloc(i * sizeof(TCHAR*)); // TODO: test this, and alt value equivalent
#endif
		if (!v)
			return LOGF(_T("Failed to expand alt strings to %zu entries.\n"), i);
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
void AltVals::SetAltValueAtIndex(const std::size_t i, const double d)
{
	if (i >= GetAltValueCount())
	{
		if (!DarkEdif::IsFusion25)
			return LOGE(_T("Cannot set alt value at index %zu, invalid index.\n"), i);

		// TODO: if this does not work, just throw
#ifdef _WIN32
		void * v = mvReAlloc(Edif::SDK->mV, (void *)CF25.Values, (int)i * sizeof(CValueMultiPlat));
#else
		void* v = NULL; // malloc(i * sizeof(CValueMultiPlat));
#endif
		if (!v)
			return LOGF(_T("Failed to expand alt strings to %zu entries.\n"), i);
#ifdef _WIN32
		*(void **)&CF25.Values = v; // TODO: Simplify
		CF25.NumAltValues = (int)i;
#else
		throw std::runtime_error("Not implemented");
#endif
	}

#ifdef _WIN32
	auto v = DarkEdif::IsFusion25 ? &CF25.Values[i] : &MMF2.rvpValues[i];
	v->m_type = TYPE_DOUBLE;
	v->m_double = d;
#else
	throw std::runtime_error("Not implemented");
#endif
}
void AltVals::SetAltValueAtIndex(const std::size_t i, const int l)
{
	if (i >= GetAltValueCount())
	{
		if (!DarkEdif::IsFusion25)
			return LOGE(_T("Cannot set alt value at index %zu, invalid index.\n"), i);

		// TODO: if this does not work, just throw
#ifdef _WIN32
		void * v = mvReAlloc(Edif::SDK->mV, (void *)CF25.Values, (int)i * sizeof(CValueMultiPlat));
#else
		void* v = NULL; // malloc(i * sizeof(CValueMultiPlat));
#endif
		if (!v)
			return LOGF(_T("Failed to expand alt strings to %zu entries.\n"), i);
#ifdef _WIN32
		*(void **)&CF25.Values = v; // TODO: Simplify
		CF25.NumAltValues = (int)i;
#else
		throw std::runtime_error("Not implemented");
#endif
	}

#ifdef _WIN32
	auto v = DarkEdif::IsFusion25 ? &CF25.Values[i] : &MMF2.rvpValues[i];
	v->m_type = TYPE_LONG;
	v->m_long = l;
#else
	throw std::runtime_error("Not implemented");
#endif
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

	if ((size_t)index >= hoPtr->get_AdRunHeader()->get_MaxObjects())
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
	if (as == nullptr)
		LOGF(_T("Invalid RH4ActionStart when trying to cancel repeating action.\n"));
	as->set_evtFlags(as->get_evtFlags() & ~ACTFLAGS_REPEAT);
}

int HeaderObject::GetFixedValue() {
	return (get_CreationId() << 16) | get_Number();
}

#if defined(_WIN32)
short Edif::Runtime::GetOIListIndexFromObjectParam(std::size_t paramIndex)
{
	const EventParam* curParam = ParamZero;
	for (std::size_t i = 0; i < paramIndex; ++i)
		curParam = (const EventParam*)((const std::uint8_t*)curParam + curParam->size);
	if ((Params)curParam->Code != Params::Object)
		LOGE(_T("GetOIListIndexFromObjectParam: Returning a OiList index for a non-Object parameter.\n"));
	LOGI(_T("GetOIListIndexFromObjectParam: Returning OiList %hi, oi is %hi.\n"), curParam->evp.W[0], curParam->evp.W[1]);
	return curParam->evp.W[0];
}
int Edif::Runtime::GetCurrentFusionFrameNumber()
{
	// First Fusion frame is 0, so make 1-based
	return 1 + hoPtr->hoAdRunHeader->rhApp->nCurrentFrame;
}

// Gets the RH2 event count, used in object selection
int RunHeader::GetRH2EventCount()
{
	return this->rh2.rh2EventCount;
}
// Gets the RH2 event count, used in object selection
void RunHeader::SetRH2EventCount(int newEventCount)
{
	rh2.rh2EventCount = newEventCount;
}
// Gets the RH4 event count for OR, used in object selection in OR-related events.
int RunHeader::GetRH4EventCountOR()
{
	return this->rh4.rh4EventCountOR;
}
void RunHeader::SetRH4EventCountOR(int value) {
	rh4.rh4EventCountOR = value;
}


// Reads the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
int RunHeader::GetRH2ActionCount()
{
	return this->rh2.rh2ActionCount;
}
// Reads the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
int RunHeader::GetRH2ActionLoopCount()
{
	return this->rh2.rh2ActionLoopCount;
}

// Sets the rh2.rh2ActionCount variable, used in an action with multiple instances selected, to repeat one action.
void RunHeader::SetRH2ActionCount(int newActionCount)
{
	rh2.rh2ActionCount = newActionCount;
}
// Sets the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions in an event.
void RunHeader::SetRH2ActionLoopCount(int newActLoopCount)
{
	rh2.rh2ActionLoopCount = newActLoopCount;
}

objectsList* RunHeader::get_ObjectList() {
	return rhObjectList;
}
objInfoList* RunHeader::GetOIListByIndex(std::size_t index)
{
	if (index >= GetNumberOi())
	{
		LOGE(_T("RunHeader::GetOIListByIndex() was passed invalid OIList index %zu, are you passing a OI instead?\n"), index);
		return nullptr;
	}

	assert(DarkEdif::ObjectSelection::oiListItemSize != 0 || index == 0); // first time is index 0

	// rhOiList is sparse, and AllOiListIterator may search by index, so finding a null oiList is allowed
	// Some rhOiList is -1 instead of null
	return (objInfoList*)(((std::uint8_t*)rhOiList) + DarkEdif::ObjectSelection::oiListItemSize * index);
}
event2* RunHeader::GetRH4ActionStart() {
	return rh4.rh4ActionStart;
}

bool RunHeader::GetRH2ActionOn() {
	return rh2.rh2ActionOn != 0;
}
void RunHeader::SetRH2ActionOn(bool newActOn) {
	rh2.rh2ActionOn = newActOn ? 1 : 0;
}

EventGroupMP * RunHeader::get_EventGroup() {
	return rhEventGroup;
}
std::size_t RunHeader::GetNumberOi() {
	// Windows runtime includes one invalid Oi at the end of list as part of count,
	// but may in fact include several invalid at the end!
	// Not sure how these happen or if they're cross-platform problems.
	// I've reproduced in the example EXE, but reproducing didn't work.
	// I inspected platform incompatible exts,
	// exts that have MFX but refuse to load, global events, global objects,
	// copy-pasted objects from another frame, etc.
	// I have yet to find invalids midway in the list though, just extra at end.
	std::size_t numOi = rhNumberOi - 1;

	// Don't use GetOIListByIndex(), it'd recurse back here
	while (numOi > 0 && ((objInfoList *) ((char *)rhOiList) + (DarkEdif::ObjectSelection::oiListItemSize * (numOi - 1)))->oilOi < 0)
		--numOi;
	return numOi;
}
qualToOi* RunHeader::GetQualToOiListByOffset(std::size_t offset) {
	// Windows edits the offset calculation if the app setting "Fix qualifier limit error" is on.
	// Since CF2.5 296.06, you can read the offset of 1 or 8 from rhQualToOiListSize;
	// prior to that, it's always 1-byte.
	if (!rhQualToOiList)
		return nullptr;
	return (qualToOi*)(((std::uint8_t*)rhQualToOiList) + ((offset & 0x7FFF) * std::max(1U, rhQualToOiListSize)));
}
RunObjectMultiPlatPtr RunHeader::GetObjectListOblOffsetByIndex(std::size_t index) {
	if (index >= get_MaxObjects())
		LOGF(_T("Invalid object index dereferenced: %zu is not a valid index in rhObjectList."), index);
	return rhObjectList[index].oblOffset;
}

EventGroupFlags RunHeader::GetEVGFlags() {
	return this->rhEventGroup->evgFlags;
}
std::size_t RunHeader::get_MaxObjects() {
	return this->rhMaxObjects;
}
std::size_t RunHeader::get_NObjects() {
	return this->rhNObjects;
}
CRunAppMultiPlat* RunHeader::get_App() {
	return (CRunAppMultiPlat*)rhApp;
}
int RunHeader::get_WindowX() const {
	return rhWindowX;
}
int RunHeader::get_WindowY() const {
	return rhWindowY;
}

short HeaderObject::get_NextSelected() {
	return this->hoNextSelected;
}
unsigned short HeaderObject::get_CreationId() {
	return this->hoCreationId;
}
short HeaderObject::get_Number() {
	return this->hoNumber;
}
short HeaderObject::get_NumNext() {
	return this->hoNumNext;
}
short HeaderObject::get_Oi() {
	return this->hoOi;
}
objInfoList* HeaderObject::get_OiList() {
	return this->hoOiList;
}
bool HeaderObject::get_SelectedInOR() {
	return this->hoSelectedInOR;
}
HeaderObjectFlags HeaderObject::get_Flags() {
	return this->hoFlags;
}
void HeaderObject::set_Flags(HeaderObjectFlags value) {
	hoFlags = value;
}
int HeaderObject::get_EventNumber() const {
	return hoEventNumber;
}
EventParam* HeaderObject::get_CurrentParam() const {
	return hoCurrentParam;
}
void HeaderObject::set_EventNumber(int value) {
	hoEventNumber = value;
}
void HeaderObject::set_CurrentParam(EventParam* value) {
	hoCurrentParam = value;
}
RunHeader* HeaderObject::get_AdRunHeader() {
	return hoAdRunHeader;
}

void HeaderObject::set_NextSelected(short ns) {
	hoNextSelected = ns;
}
void HeaderObject::set_SelectedInOR(bool b) {
	hoSelectedInOR = b;
}
int HeaderObject::get_X() const {
	return hoX;
}
void HeaderObject::SetX(int x) {
	SetPosition(x, hoY); // trigger fancy updates
}
int HeaderObject::get_Y() const {
	return hoY;
}
void HeaderObject::SetY(int y) {
	SetPosition(hoX, y); // trigger fancy updates
}
int HeaderObject::get_ImgWidth() const {
	return hoImgWidth;
}
void HeaderObject::SetImgWidth(int w) {
	hoImgWidth = w;
	// TODO: Should this set rcChanged, rcCheckCollides, rmMoveFlag
}
int HeaderObject::get_ImgHeight() const {
	return hoImgHeight;
}
int HeaderObject::get_ImgXSpot() const {
	return hoImgXSpot;
}
int HeaderObject::get_ImgYSpot() const {
	return hoImgYSpot;
}
void HeaderObject::SetImgHeight(int h) {
	hoImgHeight = h;
	// TODO: Should this set rcChanged, rcCheckCollides, rmMoveFlag
	// Check if Active does that? I guess it renders via runtime
	hoRect.bottom = hoRect.top + h;
}
void HeaderObject::SetPosition(int x, int y) {
	// TODO: Confirm what this sets in terms of rcChanged, rcCheckCollides, rmMoveFlag
	CallRunTimeFunction2(this, RFUNCTION::SET_POSITION, x, y);
}
void HeaderObject::SetSize(int width, int height)
{
	hoImgWidth = width;
	hoImgHeight = height;
	hoRect.bottom = hoRect.top + height;
	hoRect.right = hoRect.left + width;

	rCom* com = ((RunObject*)this)->get_roc();
	if (com)
	{
		com->set_changed(true);
		com->set_checkCollides(true);
	}
	rMvt* mvt = ((RunObject*)this)->get_rom();
	if (mvt)
		mvt->rmMoveFlag = true;
	// TODO: Does this require something else? Set position, redraw, rcChanged?
}
int HeaderObject::get_Identifier() const {
	return hoIdentifier;
}

rCom::MovementID rCom::get_nMovement() const { return (MovementID)rcNMovement; }
int rCom::get_anim() const { return rcAnim; }
int rCom::get_image() const { return rcImage; }
float rCom::get_scaleX() const { return rcScaleX; }
float rCom::get_scaleY() const { return rcScaleY; }
int rCom::get_dir() const { return rcDir; }
float rCom::GetAngle() const {
	return DarkEdif::IsHWAFloatAngles ? rcAngle : (float)(*(int*)&rcAngle);
}
int rCom::get_speed() const { return rcSpeed; }
int rCom::get_minSpeed() const { return rcMinSpeed; }
int rCom::get_maxSpeed() const { return rcMaxSpeed; }
bool rCom::get_changed() const { return rcChanged; }
bool rCom::get_checkCollides() const { return rcCheckCollides; }
// Sets current direction (0-31, 0 is right, incrementing ccw)
void rCom::set_dir(const int val) {
	if ((val & 31) != val)
		LOGE(_T("Direction set to %i, outside of range 0-31.\n"), val);
	rcDir = val;
}
void rCom::set_nMovement(MovementID mvt) { rcNMovement = (int)mvt; }
void rCom::set_anim(int val) { rcAnim = val; }
void rCom::set_image(int val) { rcImage = val; }
void rCom::set_scaleX(float val) { rcScaleX = val; }
void rCom::set_scaleY(float val) { rcScaleY = val; }
void rCom::SetAngle(float val) {
	if (DarkEdif::IsHWAFloatAngles)
		rcAngle = val;
	else
		*(int*)&rcAngle = (int)val;
}
void rCom::set_speed(int val) { rcSpeed = val; }
void rCom::set_minSpeed(int val) { rcMinSpeed = val; }
void rCom::set_maxSpeed(int val) { rcMaxSpeed = val; }
void rCom::set_changed(bool val) { rcChanged = val; }
void rCom::set_checkCollides(bool val) { rcCheckCollides = val; }

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
	if (!this || (rHo.hoOEFlags & (OEFLAGS::MOVEMENTS | OEFLAGS::ANIMATIONS | OEFLAGS::SPRITES)) == OEFLAGS::NONE)
		return nullptr;
	return &roc;
}
rMvt* RunObject::get_rom() {
	if (!this || (rHo.hoOEFlags & OEFLAGS::MOVEMENTS) == OEFLAGS::NONE)
		return nullptr;
	if (roc.get_nMovement() == rCom::MovementID::Launching)
	{
		LOGW(_T("Requested NMovement from a launched object.\n"));
		return nullptr;
	}
	return &rom;
}
rAni* RunObject::get_roa() {
	if (!this || (rHo.hoOEFlags & OEFLAGS::ANIMATIONS) == OEFLAGS::NONE)
		return nullptr;

	return (rAni *)(((char*)this) + roc.rcOffsetAnimation);
}
RunSprite* RunObject::get_ros() {
	if (!this || (rHo.hoOEFlags & OEFLAGS::SPRITES) == OEFLAGS::NONE)
		return nullptr;
	return (RunSprite*)(((char*)this) + roc.rcOffsetSprite);
}
AltVals* RunObject::get_rov() {
	if (!this || (rHo.hoOEFlags & OEFLAGS::VALUES) != OEFLAGS::VALUES)
		return nullptr;
	return (AltVals*)(((char*)this) + rHo.hoOffsetValue);
}
RunObjectMultiPlatPtr objectsList::GetOblOffsetByIndex(std::size_t index) {
	return this[index].oblOffset;
}

int objInfoList::get_EventCount() {
	return oilEventCount;
}
int objInfoList::get_EventCountOR() {
	return oilEventCountOR;
}
short objInfoList::get_ListSelected() {
	return oilListSelected;
}
int objInfoList::get_NumOfSelected() {
	return oilNumOfSelected;
}
short objInfoList::get_Oi() {
	return oilOi;
}
int objInfoList::get_NObjects() {
	return oilNObjects;
}
short objInfoList::get_Object() {
	return oilObject;
}
const TCHAR* objInfoList::get_name() {
	return oilName;
}
int objInfoList::get_oilNext() const {
	return oilNext;
}
void objInfoList::set_oilNext(int value) { oilNext = value; }

bool objInfoList::get_oilNextFlag() const {
	return oilNextFlag;
}
void objInfoList::set_oilNextFlag(bool value) { oilNextFlag = value; }

decltype(objInfoList::oilCurrentRoutine) objInfoList::get_oilCurrentRoutine() const {
	return oilCurrentRoutine;
}
void objInfoList::set_oilCurrentRoutine(decltype(objInfoList::oilCurrentRoutine) value) { oilCurrentRoutine = value; }

int objInfoList::get_oilCurrentOi() const {
	return oilCurrentOi;
}
void objInfoList::set_oilCurrentOi(int value) { oilCurrentOi = value; }

int objInfoList::get_oilActionCount() const {
	return oilActionCount;
}
void objInfoList::set_oilActionCount(int value) { oilActionCount = value; }

int objInfoList::get_oilActionLoopCount() const {
	return oilActionLoopCount;
}
void objInfoList::set_oilActionLoopCount(int value) { oilActionLoopCount = value; }

void objInfoList::set_NumOfSelected(int ns) {
	oilNumOfSelected = ns;
}
void objInfoList::set_ListSelected(short sh) {
	oilListSelected = sh;
}
void objInfoList::set_EventCount(int ec) {
	oilEventCount = ec;
}
void objInfoList::set_EventCountOR(int ec) {
	oilEventCountOR = ec;
}
short objInfoList::get_QualifierByIndex(const std::size_t index) {
	if (index >= MAX_QUALIFIERS) {
		LOGE(_T("Invalid qualifier index read: expected 0 to 7, got %zu.\n"), index);
		return -1;
	}
	return oilQualifiers[index];
}
CreateObjectInfo::Flags CreateObjectInfo::get_flags() const {
	return cobFlags;
}
std::int32_t CreateObjectInfo::get_X() const {
	return cobX;
}
std::int32_t CreateObjectInfo::get_Y() const {
	return cobY;
}
std::int32_t CreateObjectInfo::GetDir(RunObjectMultiPlatPtr rdPtr) const {
	if (cobDir != -1 && (cobFlags & CreateObjectInfo::Flags::CreatedAtStart) == CreateObjectInfo::Flags::None)
		return cobDir;
	const auto roc = rdPtr->get_roc();
	// This shouldn't be getting read, as it's in CreateObjectInfo, so it's current ext
	if (!roc)
	{
		LOGE(_T("This is not a moving extension, why are you reading direction?"));
		return -1;
	}
	return roc->get_dir();
}
std::int32_t CreateObjectInfo::get_layer() const {
	return cobLayer;
}
std::int32_t CreateObjectInfo::get_ZOrder() const {
	return cobZOrder;
}

int qualToOi::get_qoiCurrentOi() const { return CurrentOi; }
void qualToOi::set_qoiCurrentOi(int value) { CurrentOi = (short)value; }
int qualToOi::get_qoiNext() const { return Next; }
void qualToOi::set_qoiNext(int value) { Next = (short)value; }
int qualToOi::get_qoiActionPos() const { return ActionPos; }
void qualToOi::set_qoiActionPos(int value) { ActionPos = value; }
decltype(qualToOi::CurrentRountine) qualToOi::get_qoiCurrentRoutine() const { return CurrentRountine; }
void qualToOi::set_qoiCurrentRoutine(decltype(qualToOi::CurrentRountine) value) { CurrentRountine = value; }
int qualToOi::get_qoiActionCount() const { return ActionCount; }
void qualToOi::set_qoiActionCount(int value) { ActionCount = value; }
int qualToOi::get_qoiActionLoopCount() const { return ActionLoopCount; }
void qualToOi::set_qoiActionLoopCount(int value) { ActionLoopCount = value; }
bool qualToOi::get_qoiNextFlag() const { return NextFlag; }
void qualToOi::set_qoiNextFlag(bool value) { NextFlag = value; }
bool qualToOi::get_qoiSelectedFlag() const { return SelectedFlag; }
void qualToOi::set_qoiSelectedFlag(bool value) { SelectedFlag = value; }
short qualToOi::get_Oi(std::size_t idx) {
	return OiAndOiList[idx * 2];
}
short qualToOi::get_OiList(std::size_t idx) {
	return OiAndOiList[idx * 2 + 1];
}

std::uint8_t EventGroupMP::get_evgNCond() {
	return evgNCond;
}
std::uint8_t EventGroupMP::get_evgNAct() {
	return evgNAct;
}
std::uint16_t EventGroupMP::get_evgIdentifier() {
	return evgIdentifier;
}
std::uint16_t EventGroupMP::get_evgInhibit() {
	return evgInhibit;
}

event2 * EventGroupMP::GetCAByIndex(std::size_t index)
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (index >= (std::size_t)(evgNCond + evgNAct))
		return nullptr;

	event2 * ret = (event2*)(&this[1]);
	for (std::size_t i = 0; i < index && ret; ++i) {
		ret = ret->Next();
	}
	return ret;
}


extern HINSTANCE hInstLib;

void Edif::Runtime::WriteGlobal(const TCHAR * name, void * Value)
{
	RunHeader * rhPtr = hoPtr->hoAdRunHeader;

	while (rhPtr->rhApp->ParentApp)
		rhPtr = rhPtr->rhApp->ParentApp->Frame->rhPtr;

	EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->mvGetExtUserData(rhPtr->rhApp, hInstLib);

	if (!Global)
	{
		Global = new EdifGlobal;

		_tcscpy(Global->name, name);
		Global->Value = Value;

		Global->Next = 0;

		rhPtr->rh4.rh4Mv->mvSetExtUserData(rhPtr->rhApp, hInstLib, Global);

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
	RunHeader * rhPtr = hoPtr->hoAdRunHeader;

	while (rhPtr->rhApp->ParentApp)
		rhPtr = rhPtr->rhApp->ParentApp->Frame->rhPtr;

	EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->mvGetExtUserData(rhPtr->rhApp, hInstLib);

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
TCHAR * Edif::Runtime::CopyString(const TCHAR * String)
{
	// Allows exts to call into other exts to read the string returns,
	// without guessing if the returned memory was heap or not
	if (runtimeCopyHeapAlloc)
		return strdup(String);

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

#if TEXT_OEFLAG_EXTENSION
void * Edif::Runtime::GetRunObjectFont() const
{
	if (extFont == NULL)
		return LOGF(_T("Can't get object font: font was not set.")), nullptr;
#ifdef __ANDROID__
	return (jobject)extFont->cfontinfo;
#else
	return extFont->cfontinfo;
#endif
}
#endif // TEXT_OEFLAG_EXTENSION



#if defined(__ANDROID__)

jmethodID Edif::Runtime::generateEventMethodID, Edif::Runtime::destroyObjectMethodID;

Edif::Runtime::Runtime(Extension* ext, jobject javaExtPtr2) : ext(ext),
	javaExtPtr(javaExtPtr2, "Edif::Runtime::javaExtPtr from Edif::Runtime ctor"), ObjectSelection(NULL)
{
	SDKPointer = Edif::SDK;
	std::string exc;
	javaExtPtrClass = global(threadEnv->GetObjectClass(javaExtPtr), "Extension::javaExtPtrClass from Extension ctor");
	if (javaExtPtrClass.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaExtPtrClass, got exception %s.\n", exc.c_str());
	}

	jfieldID javaHoField = threadEnv->GetFieldID(javaExtPtrClass, "ho", "LObjects/CExtension;");
	if (javaHoField == NULL) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaHoField, got exception %s.\n", exc.c_str());
	}

	javaHoObject = global(threadEnv->GetObjectField(javaExtPtr, javaHoField), "Extension::javaHoObject from Extension ctor");
	if (javaHoObject.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaHoObject, got exception %s.\n", exc.c_str());
	}

	javaHoClass = global(threadEnv->GetObjectClass(javaHoObject), "Extension::javaHoClass from Extension ctor");
	if (javaHoClass.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not find javaHoClass method, got exception %s.\n", exc.c_str());
	}

	javaCEventClass = global(threadEnv->FindClass("Events/CEvent"), "CEvent class");
	if (javaCEventClass.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not find javaCEventClass method, got exception %s.\n", exc.c_str());
	}

	if (!generateEventMethodID) {
		generateEventMethodID = threadEnv->GetMethodID(javaExtPtrClass, "darkedif_jni_generateEvent", "(IZZ)V");
		JNIExceptionCheck();
		destroyObjectMethodID = threadEnv->GetStaticMethodID(javaExtPtrClass, "darkedif_jni_destroyObject", "(LObjects/CObject;)V");
		JNIExceptionCheck();
	}

	ext->rdPtr = std::make_shared<RunObject>(javaHoObject.ref, javaHoClass.ref, this);
	ext->rdPtr->Init(ext->rdPtr);
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

void Edif::Runtime::Rehandle()
{
	static jmethodID javaMethodID = threadEnv->GetMethodID(javaHoClass.ref, "reHandle", "()V");
	threadEnv->CallVoidMethod(javaHoObject.ref, javaMethodID);
	// GenEdifFunction reHandle
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

#ifdef DARKEDIF_SIGNAL_HANDLERS
#include <iostream>
#include <iomanip>
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

namespace DarkEdif::Android { void SetupSignalHandlers(const char*); void RemoveSignalHandlers(); }
struct BacktraceState
{
	void ** current;
	void ** end;
};

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context * context, void * arg)
{
	BacktraceState * state = static_cast<BacktraceState *>(arg);
	uintptr_t pc = _Unwind_GetIP(context);
	if (pc) {
		if (state->current == state->end) {
			return _URC_END_OF_STACK;
		}
		else {
			*state->current++ = reinterpret_cast<void *>(pc);
		}
	}
	return _URC_NO_REASON;
}

// Taken from https://stackoverflow.com/a/28858941
size_t captureBacktrace(void ** buffer, size_t max)
{
	BacktraceState state = { buffer, buffer + max };
	_Unwind_Backtrace(unwindCallback, &state);

	return state.current - buffer;
}
void dumpBacktrace(std::ostream & os, void ** buffer, size_t count)
{
	os << "Call stack, last function is bottommost:\n"sv;
	size_t outputMemSize = 512;
	char outputMem[outputMemSize];

	for (int idx = count - 1; idx >= 0; idx--) {
		//	for (size_t idx = 0; idx < count; ++idx) {
		const void * addr = buffer[idx];
		const char * symbol = "";

		Dl_info info;
		if (dladdr(addr, &info) && info.dli_sname) {
			symbol = info.dli_sname;
		}
		memset(outputMem, 0, outputMemSize);
		int status = 0;
		abi::__cxa_demangle(symbol, outputMem, &outputMemSize, &status);
		os << "  #"sv << std::setw(2) << idx << ": "sv << addr << "  "sv << (status == 0 ? outputMem : symbol) << '\n';
	}
}
//int signalCatches[] = { SIGABRT, SIGSEGV, SIGBUS, SIGPIPE };
struct Signal {
	int signalNum;
	const char * signalName;
	Signal(int s, const char * n) : signalNum(s), signalName(n) {}
};
static Signal signalCatches[] = {
	// sync signals (always sent to causing thread)
	{SIGSEGV, "SIGSEGV"},
	{SIGBUS, "SIGBUS"},
	{SIGFPE, "SIGFPE"},
	{SIGILL, "SIGILL"},
	{SIGSYS, "SIGSYS"},
	// SIGABRT: ignore, we want aborts to abort; if this is changed, note LOGF calls it
	// SIGTRAP: ignore, used for breakpoints; note DarkEdif::BreakIfDebuggerAttached() only raises SIGTRAP if debugger not detected
	// async signals:
	{SIGPIPE, "SIGPIPE"},
	// Don't try to catch 33, SIGLWP?, it's the signal used for dumping threads post-abort
};

#define ALT_STACK_SIZE (SIGSTKSZ + 4096)

stack_t alt_stack;

static void signal_handler(const int code, siginfo_t * const si, void * const sc)
{
	static size_t numCatches = 0;
	if (++numCatches > 3) {
		exit(0);
		return;
	}

#if DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_ERROR
	const char * signalName = "Unknown";
	for (std::size_t i = 0; i < std::size(signalCatches); ++i)
	{
		if (signalCatches[i].signalNum == code)
		{
			signalName = signalCatches[i].signalName;
			break;
		}
	}
#endif

	const size_t max = 30;
	void * buffer[max];
	std::ostringstream oss;

	dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));

	LOGE("signal code raised: %d, %s.\n", code, signalName);
	LOGI("%s\n", oss.str().c_str());
	// Breakpoint
#ifdef _DEBUG
	raise(SIGTRAP);
#endif
}
void DarkEdif::Android::SetupSignalHandlers(const char * threadName)
{
	if (DarkEdif::IsDebuggerAttached())
		return LOGI(_T("Not setting up signal handlers on thread %s: attached debugger detected."), threadName);

	alt_stack.ss_sp = malloc(ALT_STACK_SIZE);
	if (!alt_stack.ss_sp)
		LOGF(_T("Setup of alt stack failed, couldn't allocate %d"), ALT_STACK_SIZE);
	alt_stack.ss_size = ALT_STACK_SIZE;
	alt_stack.ss_flags = 0;

	if (sigaltstack(&alt_stack, NULL) != 0)
		LOGF(_T("Setup of alt stack failed, error %d"), errno);

	for (std::size_t i = 0; i < std::size(signalCatches); ++i)
	{
		struct sigaction sa;
		struct sigaction sa_old;
		memset(&sa, 0, sizeof(sa));
		sigemptyset(&sa.sa_mask);
#ifdef __INTELLISENSE__
		sa.sa_handler = (sig_t)(void*)signal_handler;
#else
		sa.sa_sigaction = signal_handler;
#endif
		// Flags: handler takes three params, and should run on alternative stack (good for memory corruption)
		sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
		if (sigaction(signalCatches[i].signalNum, &sa, &sa_old) != 0) {
			LOGW("Failed to set up %s sigaction.\n", signalCatches[i].signalName);
		}
		// sigaction64 is not needed; on 64-bit variants it is basically the same as sigaction
	}

	LOGI("Set up %zu signal handlers.\n", std::size(signalCatches));
}

void DarkEdif::Android::RemoveSignalHandlers()
{
	for (int i = 0; i < std::size(signalCatches); ++i)
		signal(signalCatches[i].signalNum, SIG_DFL);
}

#endif // DARKEDIF_SIGNAL_HANDLERS

void Edif::Runtime::AttachJVMAccessForThisThread(const char* threadName, bool asDaemon)
{
	const auto thisThreadID = std::this_thread::get_id();

	if (threadEnv != nullptr)
		return LOGF("Thread ID %s already has JNI access.\n", ThreadIDToStr(thisThreadID).c_str());

	pthread_setname_np(pthread_self(), threadName);

	JavaVMAttachArgs args = {};
	args.name = threadName;
	args.group = NULL;
	args.version = JNI_VERSION_1_6;
	// Daemon means the JVM won't keep the app running if this thread is still alive.
	// Do you want main thread exiting to choose whether the app is running or not?
	jint error;
	if (asDaemon)
		error = global_vm->AttachCurrentThreadAsDaemon((JNIEnv**)&threadEnv, &args);
	else
		error = global_vm->AttachCurrentThread((JNIEnv **)&threadEnv, &args);
	if (error != JNI_OK)
		return LOGF("Couldn't attach thread %s (ID %s) to JNI, AttachCurrentThread%s error %i.\n",
			threadName, ThreadIDToStr(thisThreadID).c_str(), asDaemon ? "AsDaemon" : "", error);
	LOGV("Attached thread %s (ID %s) to JNI.\n", threadName, ThreadIDToStr(thisThreadID).c_str());
	JNIExceptionCheck();

#ifdef DARKEDIF_SIGNAL_HANDLERS
	DarkEdif::Android::SetupSignalHandlers(threadName);
#endif // DARKEDIF_SIGNAL_HANDLERS
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

short Edif::Runtime::GetOIListIndexFromObjectParam(std::size_t paramIndex)
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (*(long *)&paramIndex < 0)
		raise(SIGTRAP);
	// read evtPtr.evtParams[paramIndex] as PARAM_OBJECT
	static jfieldID evtParamsFieldID = threadEnv->GetFieldID(this->javaCEventClass.ref, "evtParams", "[LParams/CParam;");
	JNIExceptionCheck();
	jobjectArray evtParams = (jobjectArray)threadEnv->GetObjectField(curCEvent, evtParamsFieldID);
	JNIExceptionCheck();
	jobject thisParam = threadEnv->GetObjectArrayElement(evtParams, paramIndex);
	JNIExceptionCheck();
#ifdef _DEBUG
	jint pParamCode = threadEnv->GetShortField(thisParam, threadEnv->GetFieldID(threadEnv->GetObjectClass(thisParam), "code", "S"));
	JNIExceptionCheck();
	if ((Params)pParamCode != Params::Object)
		LOGE(_T("GetOIListIndexFromObjectParam: Returning a OI for a non-Object parameter.\n"));
#endif
	// Read the equivalent of evp.W[0]
	jshort oiList = threadEnv->GetShortField(thisParam, threadEnv->GetFieldID(threadEnv->GetObjectClass(thisParam), "oiList", "S"));
	JNIExceptionCheck();
#if defined(_DEBUG) && (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_DEBUG)
	jshort oi = threadEnv->GetShortField(thisParam, threadEnv->GetFieldID(threadEnv->GetObjectClass(thisParam), "oi", "S"));
	JNIExceptionCheck();
	LOGD(_T("GetOIListIndexFromObjectParam: Returning OiList %hi (%#04hx; non-qual: %hi, %#04hx), oi is %hi (%#04hx; non-qual: %hi, %#04hx).\n"),
		oiList, oiList, (short)(oiList & 0x7FFF), (short)(oiList & 0x7FFF), oi, oi, (short)(oi & 0x7FFF), (short)(oi & 0x7FFF));
#endif
	return oiList;
}

int Edif::Runtime::GetCurrentFusionFrameNumber()
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	return 1 + hoPtr->get_AdRunHeader()->get_App()->get_nCurrentFrame();
}

#if TEXT_OEFLAG_EXTENSION
ProjectFunc jobject getRunObjectFont(JNIEnv*, jobject, jlong ext) {
	return (jobject)((Extension*)ext)->Runtime.GetRunObjectFont();
}
ProjectFunc jint getRunObjectTextColor(JNIEnv*, jobject, jlong ext) {
	return (jint)((Extension*)ext)->Runtime.GetRunObjectTextColor();
}
ProjectFunc void setRunObjectFont(JNIEnv*, jobject, jlong ext, jobject fontInfo, jobject rcPtr) {
	((Extension*)ext)->Runtime.SetRunObjectFont(fontInfo, rcPtr);
}
ProjectFunc void setRunObjectTextColor(JNIEnv*, jobject, jlong ext, int rgb) {
	((Extension*)ext)->Runtime.SetRunObjectTextColor(rgb);
}
#endif // TEXT_OEFLAG_EXTENSION


// Edit this to monitor specific jobject/jclass references. End with null.
const char * const globalToMonitor[] = { NULL };

// Gets the RH2 event count, used in object selection
int RunHeader::GetRH2EventCount()
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	// CEventProgram rh2EventCount
	// CRun stores this as rhEvtProg

	return get_EventProgram()->get_rh2EventCount();
}
// Gets the RH2 event count, used in object selection
void RunHeader::SetRH2EventCount(int newEventCount)
{
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), newEventCount);
	// CEventProgram rh2EventCount
	// CRun stores this as rhEvtProg

	return get_EventProgram()->set_rh2EventCount(newEventCount);
}
// Gets the RH4 event count for OR, used in object selection in OR-related events.
int RunHeader::GetRH4EventCountOR()
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	// CEventProgram rh4EventCountOR
	return get_EventProgram()->get_rh4EventCountOR();
}
void RunHeader::SetRH4EventCountOR(int value) {
	get_EventProgram()->set_rh4EventCountOR(value);
}


// Reads the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
int RunHeader::GetRH2ActionCount()
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	// CEventProgram rh2ActionCount
	return get_EventProgram()->get_rh2ActionCount();
}
// Sets the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
void RunHeader::SetRH2ActionCount(int newActionCount)
{
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), newActionCount);
	// CEventProgram rh2ActionCount
	return get_EventProgram()->set_rh2ActionCount(newActionCount);
}

// Reads the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
int RunHeader::GetRH2ActionLoopCount()
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	// CEventProgram rh2ActionLoopCount
	return get_EventProgram()->get_rh2ActionLoopCount();
}
// Sets the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
void RunHeader::SetRH2ActionLoopCount(int newActLoopCount)
{
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), newActLoopCount);
	// CEventProgram rh2ActionLoopCount
	return get_EventProgram()->set_rh2ActionLoopCount(newActLoopCount);
}

// Gets the current expression token index
int RunHeader::GetRH4CurToken()
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	int rh4CurToken = threadEnv->GetIntField(crun, rh4CurTokenFieldID);
	JNIExceptionCheck();
	return rh4CurToken;
}
// Sets the current expression token index
void RunHeader::SetRH4CurToken(int newCurToken)
{
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), newCurToken);
	threadEnv->SetIntField(crun, rh4CurTokenFieldID, newCurToken);
	JNIExceptionCheck();
}

objInfoList * RunHeader::GetOIListByIndex(std::size_t index)
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (OiList.invalid())
		GetOiList(); // ignore return
	if (index >= OiListLength)
	{
		LOGE(_T("RunHeader::GetOIListByIndex() was passed invalid OIList index %zu, are you passing a OI instead?\n"), index);
		return nullptr;
	}

	if ((long)index < 0)
		raise(SIGTRAP);

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
		jobject item = threadEnv->GetObjectArrayElement(OiList, i);
		JNIExceptionCheck();
		OiListArray.emplace_back(objInfoList{ i, this, item, runtime });
	}

	return OiList.ref;
}

event2 * RunHeader::GetRH4ActionStart() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
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
			if (evntGroup.has_value())
			{
				evntGroup.reset();
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
		if (evntGroup.has_value())
		{
			evntGroup.reset();
			// We load eventgroup from eventProgram, which is tied to the frame so program should be valid
			get_EventProgram()->eventGrp.reset();
		}

		rh4ActStart = std::make_unique<event2>(get_EventGroup(), event2::FindIndexMagicNum, runtime->curRH4ActBasedOnCEventOnly, runtime);
	}
	return rh4ActStart.get();
}
bool RunHeader::GetRH2ActionOn() {
	return get_EventProgram()->GetRH2ActionOn();
}
void RunHeader::SetRH2ActionOn(bool newActOn) {
	get_EventProgram()->SetRH2ActionOn(newActOn);
}

CEventProgram* RunHeader::get_EventProgram() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!eventProgram)
	{
		jobject eventProgramJava = threadEnv->GetObjectField(crun, eventProgramFieldID);
		JNIExceptionCheck();

		eventProgram = std::make_unique<CEventProgram>(eventProgramJava, runtime);
	}
	return eventProgram.get();
}
RunHeader::RunHeader(jobject me, jclass meClass, Edif::Runtime* runtime) :
	crun(me, "RunHeader's CRun"), crunClass(meClass, "RunHeader's CRun class"), runtime(runtime)
{
	if (rh4CurTokenFieldID == nullptr)
	{
		// CEventProgram rh4CurToken
		rh4CurTokenFieldID = threadEnv->GetFieldID(crunClass, "rh4CurToken", "I");
		JNIExceptionCheck();
		eventProgramFieldID = threadEnv->GetFieldID(crunClass, "rhEvtProg", "LEvents/CEventProgram;");
		JNIExceptionCheck();

		oiListFieldID = threadEnv->GetFieldID(crunClass, "rhOiList", "[LRunLoop/CObjInfo;");
		JNIExceptionCheck();
		oiListClass = global(threadEnv->FindClass("RunLoop/CObjInfo"), "RunHeader::GetOIListByIndex's rhOiList class");
		JNIExceptionCheck();
	}
}
// Static definitions - default inited to zero
jfieldID RunHeader::rh4CurTokenFieldID,
	RunHeader::eventProgramFieldID, RunHeader::oiListFieldID;

void RunHeader::InvalidatedByNewGeneratedEvent()
{
	rh4ActStart.reset();
	if (evntGroup)
		evntGroup.reset();
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
jfieldID CEventProgram::rh4ActStartFieldID, CEventProgram::rh2ActionOnFieldID;

EventGroupMP * CEventProgram::get_eventGroup() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!eventGrp)
	{
		jfieldID rhEventProgFieldID = threadEnv->GetFieldID(meClass, "rhEventGroup", "LEvents/CEventGroup;");
		JNIExceptionCheck();
		jobject eventGroupJava = threadEnv->GetObjectField(me, rhEventProgFieldID);
		JNIExceptionCheck();
		// This can be null, if running events from Handle tick.
		if (eventGroupJava != nullptr)
		{
			eventGrp = std::make_unique<EventGroupMP>(eventGroupJava, runtime);
			LOGV(_T("Running %s() - got a new eventGroup of %p, going to store it in eventGroup struct at %p.\n"), _T(__FUNCTION__), eventGroupJava, eventGrp.get());
		}
	}
	return eventGrp.get();
}
CEventProgram::CEventProgram(jobject me, Edif::Runtime* runtime) :
	me(me, "CEventProgram"), runtime(runtime)
{
	meClass = global(threadEnv->GetObjectClass(me), "CEventProgram class");
	JNIExceptionCheck();

	if (rh2ActionOnFieldID == NULL)
	{
		rh2ActionOnFieldID = threadEnv->GetFieldID(meClass, "rh2ActionOn", "Z");
		JNIExceptionCheck();
	}

	// rh4ActionStart is missing in unpatched and earlier Runtime versions (<= 295.10)
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
	rh2ActionCount.reset();
	rh2ActionLoopCount.reset();
	// rh2ActionOn.reset(); // currently not cached
	if (eventGrp)
		eventGrp.reset();
}
void CEventProgram::SetEventGroup(jobject grp)
{
	jfieldID rhEventProgFieldID = threadEnv->GetFieldID(meClass, "rhEventGroup", "LEvents/CEventGroup;");
	JNIExceptionCheck();
	threadEnv->SetObjectField(me, rhEventProgFieldID, grp);
	JNIExceptionCheck();
	eventGrp = grp ? std::make_unique<EventGroupMP>(grp, runtime) : nullptr;
	runtime->ObjectSelection.pExtension->rhPtr->evntGroup = grp ? eventGrp.get() : nullptr;
}


int CEventProgram::get_qualOilPtr() const {
	static jfieldID id = threadEnv->GetFieldID(meClass, "qualOilPtr", "I");
	JNIExceptionCheck();
	const jint value = threadEnv->GetIntField(me, id);
	JNIExceptionCheck();
	return value;
}
void CEventProgram::set_qualOilPtr(int value) {
	static jfieldID id = threadEnv->GetFieldID(meClass, "qualOilPtr", "I");
	JNIExceptionCheck();
	threadEnv->SetIntField(me, id, value);
	JNIExceptionCheck();
}
int CEventProgram::get_qualOilPos() const {
	static jfieldID id = threadEnv->GetFieldID(meClass, "qualOilPos", "I");
	JNIExceptionCheck();
	const jint value = threadEnv->GetIntField(me, id);
	JNIExceptionCheck();
	return value;
}
void CEventProgram::set_qualOilPos(int value) {
	static jfieldID id = threadEnv->GetFieldID(meClass, "qualOilPos", "I");
	JNIExceptionCheck();
	threadEnv->SetIntField(me, id, value);
	JNIExceptionCheck();
}
int CEventProgram::get_qualOilPtr2() const {
	static jfieldID id = threadEnv->GetFieldID(meClass, "qualOilPtr2", "I");
	JNIExceptionCheck();
	const jint value = threadEnv->GetIntField(me, id);
	JNIExceptionCheck();
	return value;
}
void CEventProgram::set_qualOilPtr2(int value) {
	static jfieldID id = threadEnv->GetFieldID(meClass, "qualOilPtr2", "I");
	JNIExceptionCheck();
	threadEnv->SetIntField(me, id, value);
	JNIExceptionCheck();
}
int CEventProgram::get_qualOilPos2() const {
	static jfieldID id = threadEnv->GetFieldID(meClass, "qualOilPos2", "I");
	JNIExceptionCheck();
	const jint value = threadEnv->GetIntField(me, id);
	JNIExceptionCheck();
	return value;
}
void CEventProgram::set_qualOilPos2(int value) {
	static jfieldID id = threadEnv->GetFieldID(meClass, "qualOilPos2", "I");
	JNIExceptionCheck();
	threadEnv->SetIntField(me, id, value);
	JNIExceptionCheck();
}
int CEventProgram::get_rh2EventCount() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
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
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), newEventCount);
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2EventCount", "I");
	JNIExceptionCheck();
	rh2EventCount = newEventCount;
	threadEnv->SetIntField(me, fieldID, newEventCount);
	JNIExceptionCheck();
}
int CEventProgram::get_rh4EventCountOR()
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
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
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), newEventCount);
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh4EventCountOR", "I");
	JNIExceptionCheck();
	rh4EventCountOR = newEventCount;
	threadEnv->SetIntField(me, fieldID, newEventCount);
	JNIExceptionCheck();
}

// Reads the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
int CEventProgram::get_rh2ActionCount()
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2ActionCount", "I");
	JNIExceptionCheck();
	int rh2ActionCount = threadEnv->GetIntField(me, fieldID);
	JNIExceptionCheck();
	return rh2ActionCount;
}
// Sets the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
void CEventProgram::set_rh2ActionCount(int newActionCount)
{
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), newActionCount);
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2ActionCount", "I");
	JNIExceptionCheck();
	rh2ActionCount = newActionCount;
	threadEnv->SetIntField(me, fieldID, newActionCount);
	JNIExceptionCheck();
}

// Reads the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
int CEventProgram::get_rh2ActionLoopCount()
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2ActionLoopCount", "I");
	JNIExceptionCheck();
	int rh2ActionLoopCount = threadEnv->GetIntField(me, fieldID);
	JNIExceptionCheck();
	return rh2ActionLoopCount;
}
// Sets the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
void CEventProgram::set_rh2ActionLoopCount(int newActLoopCount)
{
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), newActLoopCount);
	static jfieldID fieldID = threadEnv->GetFieldID(meClass, "rh2ActionLoopCount", "I");
	JNIExceptionCheck();
	rh2ActionLoopCount = newActLoopCount;
	threadEnv->SetIntField(me, fieldID, newActLoopCount);
	JNIExceptionCheck();
}

bool CEventProgram::GetRH2ActionOn() {
	// TODO: We can possibly optimize this by storing true/false Action/Condition jump funcs
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	bool yes = threadEnv->GetBooleanField(me, rh2ActionOnFieldID);
	JNIExceptionCheck();
	return yes;
}
void CEventProgram::SetRH2ActionOn(bool newSet) {
	// TODO: We can possibly optimize this by storing true/false Action/Condition jump funcs
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), newSet ? 1 : 0);
	threadEnv->SetBooleanField(me, rh2ActionOnFieldID, newSet);
	JNIExceptionCheck();
}

EventGroupMP * RunHeader::get_EventGroup() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));

	if (!evntGroup.has_value() || !this->evntGroup.value())
	{
		LOGV(_T("Running %s() - eventgroup out of date, updating it.\n"), _T(__FUNCTION__));
		evntGroup = get_EventProgram()->get_eventGroup();
	}

	return evntGroup.value();
}

std::size_t RunHeader::GetNumberOi() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!NumberOi.has_value())
	{
		// NumberOi is actually rhMaxOI
		// Note: EventProgram has short maxOi, but CRun has int maxOi
		JNIExceptionCheck();
		jfieldID fieldID = threadEnv->GetFieldID(crunClass, "rhMaxOI", "I");
		JNIExceptionCheck();
		NumberOi = (std::size_t)threadEnv->GetIntField(crun, fieldID);
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
	LOGV(_T("Running %s(%zu).\n"), _T(__FUNCTION__), index);

	// CQualToOiList[] CEventProgram.qualToOiList
	if (QualToOiList.invalid())
	{
		auto evp = this->get_EventProgram();
		jfieldID fieldID = threadEnv->GetFieldID(evp->meClass, "qualToOiList", "[LEvents/CQualToOiList;");
		JNIExceptionCheck();
		jobjectArray list = (jobjectArray)threadEnv->GetObjectField(evp->me, fieldID);
		JNIExceptionCheck();
		if (!list)
			return nullptr;
		QualToOiList = global(list, "RunHeader::GetQualToOiListByIndex's QualToOiList");
		threadEnv->DeleteLocalRef(list);
		// can't look up from OiList, that's the class of array, not array element
		QualToOiClass = global(threadEnv->FindClass("Events/CQualToOiList"), "RunHeader::GetQualToOiListByIndex's QualToOiList class");
		JNIExceptionCheck();
		QualToOiListLength = threadEnv->GetArrayLength(QualToOiList.ref);
		JNIExceptionCheck();

		for (std::size_t i = 0; i < QualToOiListLength; ++i)
		{
			jobject thisParam = threadEnv->GetObjectArrayElement(QualToOiList, i);
			JNIExceptionCheck();
			QualToOiListArray.emplace_back(qualToOi(this, i, thisParam, runtime));
		}
	}

	if (index >= QualToOiListLength)
		return nullptr;

	return &QualToOiListArray[index];
}
RunObjectMultiPlatPtr RunHeader::GetObjectListOblOffsetByIndex(std::size_t index) {
	LOGV(_T("Running %s(%zu).\n"), _T(__FUNCTION__), index);
	if (!ObjectList)
	{
		// CObject[] CRun.rhObjectsList
		static jfieldID fieldID = threadEnv->GetFieldID(crunClass, "rhObjectList", "[LObjects/CObject;");
		JNIExceptionCheck();
		jobjectArray jobArr = (jobjectArray)threadEnv->GetObjectField(crun, fieldID);
		JNIExceptionCheck();
		ObjectList = std::make_unique<objectsList>(jobArr, runtime);
	}

	return ObjectList->GetOblOffsetByIndex(index);
}

EventGroupFlags RunHeader::GetEVGFlags() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	return get_EventGroup()->get_evgFlags();
}
CRunAppMultiPlat* RunHeader::get_App() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!App)
	{
		jfieldID fieldID = threadEnv->GetFieldID(crunClass, "rhApp", "LApplication/CRunApp;");
		JNIExceptionCheck();
		jobject appJava = threadEnv->GetObjectField(crun, fieldID);
		JNIExceptionCheck();
		App = std::make_unique<CRunAppMultiPlat>(appJava, runtime);
	}
	return App.get();
}
size_t RunHeader::get_MaxObjects() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!MaxObjects.has_value())
	{
		// Note: CRun has int rhMaxObjects
		static jfieldID maxObjectsFieldID = threadEnv->GetFieldID(crunClass, "rhMaxObjects", "I");
		JNIExceptionCheck();
		MaxObjects = threadEnv->GetIntField(crun, maxObjectsFieldID);
		JNIExceptionCheck();
	}
	return MaxObjects.value();
}
// Gets number of valid object instances currently in frame
size_t RunHeader::get_NObjects() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!NObjects.has_value())
	{
		// Note: CRun has int rhMaxObjects
		static jfieldID nObjectsFieldID = threadEnv->GetFieldID(crunClass, "rhNObjects", "I");
		JNIExceptionCheck();
		NObjects = threadEnv->GetIntField(crun, nObjectsFieldID);
		JNIExceptionCheck();
	}
	return NObjects.value();
}
int RunHeader::get_WindowX() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	static jfieldID windowXFieldID;
	if (!windowXFieldID)
	{
		windowXFieldID = threadEnv->GetFieldID(crunClass, "rhWindowX", "I");
		JNIExceptionCheck();
	}
	const int windowX = threadEnv->GetIntField(crun, windowXFieldID);
	JNIExceptionCheck();
	return windowX;
}
int RunHeader::get_WindowY() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	static jfieldID windowYFieldID;
	if (!windowYFieldID)
	{
		windowYFieldID = threadEnv->GetFieldID(crunClass, "rhWindowY", "I");
		JNIExceptionCheck();
	}
	const int windowY = threadEnv->GetIntField(crun, windowYFieldID);
	JNIExceptionCheck();
	return windowY;
}


int HeaderObject::get_EventNumber() const {
	static jfieldID id = threadEnv->GetFieldID(meClass, "hoEventNumber", "I");
	JNIExceptionCheck();
	const int value = threadEnv->GetIntField(me, id);
	JNIExceptionCheck();
	return value;
}
void HeaderObject::set_EventNumber(int value) {
	static jfieldID id = threadEnv->GetFieldID(meClass, "hoEventNumber", "I");
	JNIExceptionCheck();
	threadEnv->SetIntField(me, id, value);
	JNIExceptionCheck();
	EventNumber.reset();
}
int HeaderObject::get_CurrentParam() const {
	static jfieldID id = threadEnv->GetFieldID(meClass, "hoCurrentParam", "I");
	JNIExceptionCheck();
	const int value = threadEnv->GetIntField(me, id);
	JNIExceptionCheck();
	return value;
}
void HeaderObject::set_CurrentParam(int value) {
	static jfieldID id = threadEnv->GetFieldID(meClass, "hoCurrentParam", "I");
	JNIExceptionCheck();
	threadEnv->SetIntField(me, id, value);
	JNIExceptionCheck();
}
short HeaderObject::get_NextSelected() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!NextSelected.has_value())
	{
		// Part of CObject
		jfieldID nextSelectedFieldID = threadEnv->GetFieldID(meClass, "hoNextSelected", "S");
		JNIExceptionCheck();
		NextSelected = threadEnv->GetShortField(me, nextSelectedFieldID);
		JNIExceptionCheck();
	}
	return NextSelected.value();
}
unsigned short HeaderObject::get_CreationId() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!CreationId.has_value())
	{
		// Part of CObject
		jfieldID CreationIdFieldID = threadEnv->GetFieldID(meClass, "hoCreationId", "S");
		JNIExceptionCheck();
		CreationId = threadEnv->GetShortField(me, CreationIdFieldID);
		JNIExceptionCheck();
	}
	return CreationId.value();
}
short HeaderObject::get_Number() {
	LOGV(_T("Running %s() on %p.\n"), _T(__FUNCTION__), this);
	if (!Number.has_value())
	{
		// Part of CObject
		jfieldID NumberFieldID = threadEnv->GetFieldID(meClass, "hoNumber", "S");
		JNIExceptionCheck();
		Number = threadEnv->GetShortField(me, NumberFieldID);
		JNIExceptionCheck();
	}
	return Number.value();
}
short HeaderObject::get_NumNext() {
	LOGV(_T("Running %s() on %p.\n"), _T(__FUNCTION__), this);
	if (!NumNext.has_value())
	{
		// Part of CObject
		static jfieldID NumNextFieldID = threadEnv->GetFieldID(meClass, "hoNumNext", "S");
		JNIExceptionCheck();
		NumNext = threadEnv->GetShortField(me, NumNextFieldID);
		JNIExceptionCheck();
	}
	return NumNext.value();
}
short HeaderObject::get_Oi() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!Oi.has_value())
	{
		// Part of CObject
		static jfieldID OiFieldID = threadEnv->GetFieldID(meClass, "hoOi", "S");
		JNIExceptionCheck();
		Oi = threadEnv->GetShortField(me, OiFieldID);
		JNIExceptionCheck();
	}
	return Oi.value();
}
objInfoList * HeaderObject::get_OiList() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
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
		raise(SIGTRAP);
	}

	return OiList;
}
bool HeaderObject::get_SelectedInOR() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!SelectedInOR.has_value())
	{
		// Part of CObject
		static jfieldID fieldID = threadEnv->GetFieldID(meClass, "hoSelectedInOR", "B");
		JNIExceptionCheck();
		SelectedInOR = threadEnv->GetByteField(me, fieldID) != 0;
		JNIExceptionCheck();
	}
	return SelectedInOR.value();
}
HeaderObjectFlags HeaderObject::get_Flags() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!Flags.has_value())
	{
		// Part of CObject
		static jfieldID fieldID = threadEnv->GetFieldID(meClass, "hoFlags", "S");
		JNIExceptionCheck();
		Flags = (HeaderObjectFlags)threadEnv->GetShortField(me, fieldID);
		JNIExceptionCheck();
	}
	return Flags.value();
}
void HeaderObject::set_Flags(HeaderObjectFlags value) {
	static jfieldID id = threadEnv->GetFieldID(meClass, "hoFlags", "S");
	JNIExceptionCheck();
	threadEnv->SetShortField(me, id, (jshort)value);
	JNIExceptionCheck();
	Flags = value;
}

RunHeader* HeaderObject::get_AdRunHeader() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!AdRunHeader)
	{
		// Part of CObject
		LOGV("Class name of meClass: \"%s\".\n", getClassName(meClass, true));
		static jfieldID fieldID = threadEnv->GetFieldID(meClass, "hoAdRunHeader", "LRunLoop/CRun;");
		JNIExceptionCheck();
		jobject rhObject = threadEnv->GetObjectField(me, fieldID);
		JNIExceptionCheck();
		jclass rhObjectClass = threadEnv->GetObjectClass(rhObject);
		AdRunHeader = std::make_unique<RunHeader>(rhObject, rhObjectClass, runtime);
		JNIExceptionCheck();
	}
	return AdRunHeader.get();
}

void HeaderObject::set_NextSelected(short ns) {
	LOGV(_T("Running %s(%hi).\n"), _T(__FUNCTION__), ns);
	NextSelected = ns;
	// Part of CObject
	static jfieldID nextSelectedFieldID = threadEnv->GetFieldID(meClass, "hoNextSelected", "S");
	JNIExceptionCheck();
	threadEnv->SetShortField(me, nextSelectedFieldID, ns);
	JNIExceptionCheck();
}
void HeaderObject::set_SelectedInOR(bool b) {
	// despite being a Java byte field, it's used as bool
	LOGV(_T("Running %s(bool %i).\n"), _T(__FUNCTION__), b ? 1 : 0);
	SelectedInOR = b;
	// Part of CObject
	static jfieldID selectedInORFieldID = threadEnv->GetFieldID(meClass, "hoSelectedInOR", "B");
	JNIExceptionCheck();
	threadEnv->SetByteField(me, selectedInORFieldID, b);
	JNIExceptionCheck();
}
int HeaderObject::get_X() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int x = threadEnv->GetIntField(me, xFieldID);
	JNIExceptionCheck();
	return x;
}
void HeaderObject::SetX(int x) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), x);
	if (!setXMethodID)
	{
		setXMethodID = threadEnv->GetMethodID(meClass, "setX", "(I)V");
		JNIExceptionCheck();
	}
	threadEnv->CallVoidMethod(me, setXMethodID, x);
	JNIExceptionCheck();
}
int HeaderObject::get_Y() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int y = threadEnv->GetIntField(me, yFieldID);
	JNIExceptionCheck();
	return y;
}
void HeaderObject::SetY(int y) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), y);
	if (!setYMethodID)
	{
		setYMethodID = threadEnv->GetMethodID(meClass, "setY", "(I)V");
		JNIExceptionCheck();
	}
	threadEnv->CallVoidMethod(me, setYMethodID, y);
	JNIExceptionCheck();
}
void HeaderObject::SetPosition(int x, int y) {
	LOGV(_T("Running %s(%d, %d).\n"), _T(__FUNCTION__), x, y);
	if (!setPosMethodID)
	{
		setPosMethodID = threadEnv->GetMethodID(meClass, "setPosition", "(II)V");
		JNIExceptionCheck();
	}
	threadEnv->CallVoidMethod(me, setPosMethodID, y);
	JNIExceptionCheck();
}
int HeaderObject::get_ImgWidth() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int imgWidth = threadEnv->GetIntField(me, imgWidthFieldID);
	JNIExceptionCheck();
	return imgWidth;
}
void HeaderObject::SetImgWidth(int w) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), w);
	if (!imgWidthMethodID)
	{
		imgWidthMethodID = threadEnv->GetMethodID(meClass, "setWidth", "(I)V");
		JNIExceptionCheck();
	}
	threadEnv->CallVoidMethod(me, imgWidthMethodID, w);
	JNIExceptionCheck();
}
int HeaderObject::get_ImgHeight() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int imgHeight = threadEnv->GetIntField(me, imgHeightFieldID);
	JNIExceptionCheck();
	return imgHeight;
}
void HeaderObject::SetImgHeight(int h) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), h);
	if (!imgHeightMethodID)
	{
		imgHeightMethodID = threadEnv->GetMethodID(meClass, "setHeight", "(I)V");
		JNIExceptionCheck();
	}
	threadEnv->CallVoidMethod(me, imgHeightMethodID, h);
	JNIExceptionCheck();
}
void HeaderObject::SetSize(int w, int h) {
	LOGV(_T("Running %s(%d, %d).\n"), _T(__FUNCTION__), w, h);
	if (!setSizeMethodID)
	{
		setSizeMethodID = threadEnv->GetMethodID(meClass, "setSize", "(II)V");
		JNIExceptionCheck();
	}
	threadEnv->CallVoidMethod(me, setSizeMethodID, w, h);
	JNIExceptionCheck();
}
int HeaderObject::get_ImgXSpot() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int xspot = threadEnv->GetIntField(me, imgXSpotFieldID);
	JNIExceptionCheck();
	return xspot;
}
int HeaderObject::get_ImgYSpot() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int yspot = threadEnv->GetIntField(me, imgYSpotFieldID);
	JNIExceptionCheck();
	return yspot;
}
int HeaderObject::get_Identifier() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int ident = threadEnv->GetIntField(me, identifierFieldID);
	JNIExceptionCheck();
	return ident;
}
OEFLAGS HeaderObject::get_OEFLAGS() const {
	return oeFlags;
}

// static definition
jfieldID HeaderObject::numberFieldID, HeaderObject::xFieldID, HeaderObject::yFieldID,
	HeaderObject::imgWidthFieldID, HeaderObject::imgHeightFieldID,
	HeaderObject::imgXSpotFieldID, HeaderObject::imgYSpotFieldID,
	HeaderObject::identifierFieldID, HeaderObject::oeFlagsFieldID;

HeaderObject::HeaderObject(RunObject * ro, jobject me, jclass meClass, Edif::Runtime* runtime) :
	me(me), meClass(meClass), runtime(runtime), runObj(ro)
{
	// May be looked up by accident
	if (numberFieldID == nullptr)
	{
		// Parts of CObject
		numberFieldID = threadEnv->GetFieldID(meClass, "hoNumber", "S");
		JNIExceptionCheck();
		xFieldID = threadEnv->GetFieldID(meClass, "hoX", "I");
		JNIExceptionCheck();
		yFieldID = threadEnv->GetFieldID(meClass, "hoY", "I");
		JNIExceptionCheck();
		imgWidthFieldID = threadEnv->GetFieldID(meClass, "hoImgWidth", "I");
		JNIExceptionCheck();
		imgHeightFieldID = threadEnv->GetFieldID(meClass, "hoImgHeight", "I");
		JNIExceptionCheck();
		imgXSpotFieldID = threadEnv->GetFieldID(meClass, "hoImgXSpot", "I");
		JNIExceptionCheck();
		imgYSpotFieldID = threadEnv->GetFieldID(meClass, "hoImgYSpot", "I");
		JNIExceptionCheck();
		identifierFieldID = threadEnv->GetFieldID(meClass, "hoIdentifier", "I");
		JNIExceptionCheck();
		oeFlagsFieldID = threadEnv->GetFieldID(meClass, "hoOEFlags", "I");
		JNIExceptionCheck();
	}
	oeFlags = (OEFLAGS) threadEnv->GetIntField(me, oeFlagsFieldID);
	JNIExceptionCheck();
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
	LOGV(_T("Running %s() on %p.\n"), _T(__FUNCTION__), obj);
	if (numberFieldID == nullptr)
	{
		// Part of CObject
		numberFieldID = threadEnv->GetFieldID(threadEnv->GetObjectClass(obj), "hoNumber", "S");
		JNIExceptionCheck();
	}
	const short num = threadEnv->GetShortField(obj, numberFieldID);
	JNIExceptionCheck();
	return num;
}

rCom::MovementID rCom::get_nMovement() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int rcNMovement = threadEnv->GetIntField(me, nMovementFieldID);
	JNIExceptionCheck();
	return (MovementID)rcNMovement;
}
int rCom::get_dir() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int rcDir = threadEnv->GetIntField(me, dirFieldID);
	JNIExceptionCheck();
	return rcDir;
}
int rCom::get_anim() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int rcAnim = threadEnv->GetIntField(me, animFieldID);
	JNIExceptionCheck();
	return rcAnim;
}
int rCom::get_image() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int rcImage = threadEnv->GetIntField(me, imageFieldID);
	JNIExceptionCheck();
	return rcImage;
}
float rCom::get_scaleX() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int rcScaleX = threadEnv->GetFloatField(me, scaleXFieldID);
	JNIExceptionCheck();
	return rcScaleX;
}
float rCom::get_scaleY() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int rcScaleY = threadEnv->GetFloatField(me, scaleYFieldID);
	JNIExceptionCheck();
	return rcScaleY;
}
float rCom::GetAngle() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const float rcAngle = threadEnv->GetFloatField(me, angleFieldID);
	JNIExceptionCheck();
	return rcAngle;
}
int rCom::get_speed() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int rcSpeed = threadEnv->GetIntField(me, speedFieldID);
	JNIExceptionCheck();
	return rcSpeed;
}
int rCom::get_minSpeed() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int rcMinSpeed = threadEnv->GetIntField(me, minSpeedFieldID);
	JNIExceptionCheck();
	return rcMinSpeed;
}
int rCom::get_maxSpeed() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int rcMaxSpeed = threadEnv->GetIntField(me, maxSpeedFieldID);
	JNIExceptionCheck();
	return rcMaxSpeed;
}
bool rCom::get_changed() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const bool rcChanged = threadEnv->GetBooleanField(me, changedFieldID);
	JNIExceptionCheck();
	return rcChanged;
}
bool rCom::get_checkCollides() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const bool rcChecksColides = threadEnv->GetBooleanField(me, checkCollidesFieldID);
	JNIExceptionCheck();
	return rcChecksColides;
}
void rCom::set_dir(int val) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), val);
	threadEnv->SetIntField(me, dirFieldID, val);
	JNIExceptionCheck();
}
void rCom::set_anim(int val) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), val);
	threadEnv->SetIntField(me, animFieldID, val);
	JNIExceptionCheck();
}
void rCom::set_image(int val) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), val);
	if ((short)val != val) {
		LOGE(_T("%s: Animation image too large!"), _T(__FUNCTION__));
	}
	threadEnv->SetShortField(me, imageFieldID, (jshort)val);
	JNIExceptionCheck();
}
void rCom::set_scaleX(float val) {
	LOGV(_T("Running %s(%f).\n"), _T(__FUNCTION__), val);
	threadEnv->SetFloatField(me, scaleXFieldID, val);
	JNIExceptionCheck();
}
void rCom::set_scaleY(float val) {
	LOGV(_T("Running %s(%f).\n"), _T(__FUNCTION__), val);
	threadEnv->SetFloatField(me, scaleYFieldID, val);
	JNIExceptionCheck();
}
void rCom::SetAngle(float val) {
	LOGV(_T("Running %s(%f).\n"), _T(__FUNCTION__), val);
	threadEnv->SetFloatField(me, angleFieldID, val);
	JNIExceptionCheck();
}
void rCom::set_speed(int val) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), val);
	threadEnv->SetIntField(me, speedFieldID, val);
	JNIExceptionCheck();
}
void rCom::set_minSpeed(int val) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), val);
	threadEnv->SetIntField(me, minSpeedFieldID, val);
	JNIExceptionCheck();
}
void rCom::set_maxSpeed(int val) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), val);
	threadEnv->SetIntField(me, maxSpeedFieldID, val);
	JNIExceptionCheck();
}
void rCom::set_changed(bool val) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), val);
	threadEnv->SetBooleanField(me, changedFieldID, val);
	JNIExceptionCheck();
}
void rCom::set_checkCollides(bool val) {
	LOGV(_T("Running %s(%d).\n"), _T(__FUNCTION__), val);
	threadEnv->SetBooleanField(me, checkCollidesFieldID, val);
	JNIExceptionCheck();
}

// static definition
jfieldID rCom::nMovementFieldID, rCom::dirFieldID, rCom::animFieldID, rCom::imageFieldID, rCom::scaleXFieldID,
	rCom::scaleYFieldID, rCom::angleFieldID, rCom::speedFieldID, rCom::minSpeedFieldID,
	rCom::maxSpeedFieldID, rCom::changedFieldID, rCom::checkCollidesFieldID;
rCom::rCom(RunObject* ro) : ro(ro)
{
	const jfieldID rocFieldID = threadEnv->GetFieldID(ro->meClass, "roc", "LObjects/CRCom;");
	JNIExceptionCheck();
	me = global(threadEnv->GetObjectField(ro->me, rocFieldID), "roc");
	meClass = global(threadEnv->GetObjectClass(me), "roc class");

	if (!nMovementFieldID)
	{
		nMovementFieldID = threadEnv->GetFieldID(meClass, "rcMovementType", "I");
		JNIExceptionCheck();
		dirFieldID = threadEnv->GetFieldID(meClass, "rcDir", "I");
		JNIExceptionCheck();
		animFieldID = threadEnv->GetFieldID(meClass, "rcAnim", "I");
		JNIExceptionCheck();
		imageFieldID = threadEnv->GetFieldID(meClass, "rcImage", "I");
		JNIExceptionCheck();
		scaleXFieldID = threadEnv->GetFieldID(meClass, "rcScaleX", "F");
		JNIExceptionCheck();
		scaleYFieldID = threadEnv->GetFieldID(meClass, "rcScaleY", "F");
		JNIExceptionCheck();
		angleFieldID = threadEnv->GetFieldID(meClass, "rcAngle", "F");
		JNIExceptionCheck();
		speedFieldID = threadEnv->GetFieldID(meClass, "rcSpeed", "I");
		JNIExceptionCheck();
		minSpeedFieldID = threadEnv->GetFieldID(meClass, "rcMinSpeed", "I");
		JNIExceptionCheck();
		maxSpeedFieldID = threadEnv->GetFieldID(meClass, "rcMaxSpeed", "I");
		JNIExceptionCheck();
		changedFieldID = threadEnv->GetFieldID(meClass, "rcChanged", "Z");
		JNIExceptionCheck();
		checkCollidesFieldID = threadEnv->GetFieldID(meClass, "rcCheckCollides", "Z");
		JNIExceptionCheck();
	}
}

// static definition
//jfieldID rMvt::nMovementFieldID;
rMvt::rMvt(RunObject* ro) : ro(ro)
{
	const jfieldID romFieldID = threadEnv->GetFieldID(ro->meClass, "rom", "LMovements/CRMvt;");
	JNIExceptionCheck();
	me = global(threadEnv->GetObjectField(ro->me, romFieldID), "rom");
	meClass = global(threadEnv->GetObjectClass(me), "rom class");

	/*
	if (!nMovementFieldID)
	{
	}*/
}

// static definition
//jfieldID rAni::nMovementFieldID;
rAni::rAni(RunObject* ro) : ro(ro)
{
	const jfieldID roaFieldID = threadEnv->GetFieldID(ro->meClass, "roa", "LAnimations/CRAni;");
	JNIExceptionCheck();
	me = global(threadEnv->GetObjectField(ro->me, roaFieldID), "roa");
	meClass = global(threadEnv->GetObjectClass(me), "roa class");

	/*
	if (!nMovementFieldID)
	{
	}*/
}

RunSpriteFlag RunSprite::get_Flags() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const jint flags = threadEnv->GetShortField(me, flagsFieldID);
	JNIExceptionCheck();
	return (RunSpriteFlag)flags;
}
int RunSprite::get_EffectShader() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const jint effectShader = threadEnv->GetIntField(me, effectShaderFieldID);
	JNIExceptionCheck();
	return effectShader;
}
BlitOperation RunSprite::get_Effect() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const jint effect = threadEnv->GetIntField(me, effectFieldID);
	JNIExceptionCheck();
	return (BlitOperation)effect;
}
int RunSprite::get_EffectParam() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const jint effectParam = threadEnv->GetIntField(me, effectParamFieldID);
	JNIExceptionCheck();
	return effectParam;
}
std::uint32_t RunSprite::get_layer() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const jshort layer = threadEnv->GetShortField(me, layerFieldID);
	JNIExceptionCheck();
	return layer;
}

// static definition
jfieldID RunSprite::flagsFieldID, RunSprite::effectFieldID, RunSprite::effectShaderFieldID,
	RunSprite::effectParamFieldID, RunSprite::layerFieldID;
RunSprite::RunSprite(RunObject* ro) : ro(ro)
{
	const jfieldID rosFieldID = threadEnv->GetFieldID(ro->meClass, "ros", "LSprites/CRSpr;");
	JNIExceptionCheck();
	me = global(threadEnv->GetObjectField(ro->me, rosFieldID), "ros");
	meClass = global(threadEnv->GetObjectClass(me), "ros class");

	if (!flagsFieldID)
	{
		flagsFieldID = threadEnv->GetFieldID(meClass, "rsFlags", "S");
		JNIExceptionCheck();
		effectFieldID = threadEnv->GetFieldID(meClass, "rsEffect", "I");
		JNIExceptionCheck();
		effectParamFieldID = threadEnv->GetFieldID(meClass, "rsEffectParam", "I");
		JNIExceptionCheck();
		layerFieldID = threadEnv->GetFieldID(meClass, "rsLayer", "S");
		JNIExceptionCheck();
		// Added in 296 beta
		effectShaderFieldID = threadEnv->GetFieldID(meClass, "rsEffectShader", "I");
		JNIExceptionCheck();
	}
}

// static definition
jfieldID AltVals::valuesFieldID, AltVals::stringsFieldID, AltVals::valueFlagsFieldID,
	AltVals::numValuesFieldID, AltVals::numStringsFieldID;
AltVals::AltVals(RunObject * ro) : ro(ro)
{
	const jfieldID rovFielID = threadEnv->GetFieldID(ro->meClass, "rov", "LValues/CRVal;");
	JNIExceptionCheck();
	me = global(threadEnv->GetObjectField(ro->me, rovFielID), "rov");
	meClass = global(threadEnv->GetObjectClass(me), "rov class");

	if (!valueFlagsFieldID)
	{
		valueFlagsFieldID = threadEnv->GetFieldID(meClass, "rvValueFlags", "I");
		JNIExceptionCheck();
		valuesFieldID = threadEnv->GetFieldID(meClass, "rvValues", "[LValues/CValue;");
		JNIExceptionCheck();
		stringsFieldID = threadEnv->GetFieldID(meClass, "rvStrings", "[Ljava/lang/String;");
		JNIExceptionCheck();
		numValuesFieldID = threadEnv->GetFieldID(meClass, "rvNumberOfValues", "I");
		JNIExceptionCheck();
		numStringsFieldID = threadEnv->GetFieldID(meClass, "rvNumberOfStrings", "I");
		JNIExceptionCheck();
	}
}

CRunFrameMultiPlat::CRunFrameMultiPlat(jobject me, Edif::Runtime* runtime) :
	me(me, "CRunFrame"), meClass(threadEnv->GetObjectClass(me), "CRunFrame class"), runtime(runtime)
{

}

CRunAppMultiPlat::CRunAppMultiPlat(jobject me, Edif::Runtime* runtime) :
	me(me, "CRunApp"), meClass(threadEnv->GetObjectClass(me), "CRunApp class"), runtime(runtime)
{
	// hmm
}

// static definition - zero inited
jfieldID event2::evtFlagsFieldID, event2::evtSizeFieldID, event2::evtCodeFieldID, event2::evtOiFieldID;

event2::event2(EventGroupMP * owner, int index, jobject evt, Edif::Runtime * run) :
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
	if (owner == nullptr)
		LOGE("CEvent being made on null!");
}
short event2::get_evtNum() {
	// In Windows, evtNum is high WORD of evtCode, in a union.
	// In Android, there is no union; just evtCode, so pull it out and shift it.

	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const jint evtCode = threadEnv->GetIntField(me, evtCodeFieldID);
	JNIExceptionCheck();
	return (jshort)(evtCode >> 16);
}
OINUM event2::get_evtOi() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const jshort evtOi = threadEnv->GetShortField(me, evtOiFieldID);
	JNIExceptionCheck();
	return evtOi;
}
/*short event2::get_evtSize() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const jshort evtSize = threadEnv->GetShortField(me, evtSizeFieldID);
	JNIExceptionCheck();
	return evtSize;
}*/
std::int8_t event2::get_evtFlags() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const jbyte evtFlags = threadEnv->GetByteField(me, evtFlagsFieldID);
	JNIExceptionCheck();
	return evtFlags;
}
void event2::set_evtFlags(std::int8_t evtF) {
	LOGV(_T("Running %s(%hhi).\n"), _T(__FUNCTION__), evtF);
	threadEnv->SetByteField(me, evtFlagsFieldID, evtF);
	JNIExceptionCheck();
}
std::unique_ptr<event2> event2::Next() {
	return owner->GetCAByIndex(index + 1);
}
int event2::GetIndex() {
	return index;
}

RunObject::RunObject(jobject meP, jclass meClassP, Edif::Runtime* runtime)
	: me(meP, "RunObject ctor holding CExtension"), meClass(meClassP, "RunObject class ctor holding CExtension")
{
	rHo = std::make_unique<HeaderObject>(this, me.ref, meClass.ref, runtime);
}
void RunObject::Init(std::shared_ptr<RunObject>& self)
{
	if (self.get() != this)
		LOGF(_T("Invalid use of RunObject::Init (%p != %p)\n"), self.get(), this);
	selfHolder = self;
}

HeaderObject * RunObject::get_rHo() {
	return rHo.get();
}
rCom* RunObject::get_roc() {
	if ((rHo->oeFlags & (OEFLAGS::MOVEMENTS | OEFLAGS::ANIMATIONS | OEFLAGS::SPRITES)) == OEFLAGS::NONE)
		return nullptr;
	if (!roc)
		roc = std::make_unique<rCom>(this);
	return roc.get();
}
rMvt* RunObject::get_rom() {
	if ((rHo->oeFlags & OEFLAGS::MOVEMENTS) == OEFLAGS::NONE)
		return nullptr;
	if (!rom)
		rom = std::make_unique<rMvt>(this);
	return rom.get();
}
rAni* RunObject::get_roa() {
	if ((rHo->oeFlags & OEFLAGS::ANIMATIONS) == OEFLAGS::NONE)
		return nullptr;
	if (!roa)
		roa = std::make_unique<rAni>(this);
	return roa.get();
}
RunSprite* RunObject::get_ros() {
	if ((rHo->oeFlags & OEFLAGS::SPRITES) == OEFLAGS::NONE)
		return nullptr;
	if (!ros)
		ros = std::make_unique<RunSprite>(this);
	return ros.get();
}
AltVals* RunObject::get_rov() {
	// read from Java: Values.CRVal rov
	if ((rHo->oeFlags & OEFLAGS::VALUES) != OEFLAGS::VALUES)
		return nullptr;
	if (!rov)
		rov = std::make_unique<AltVals>(this);
	return rov.get();
}

objectsList::objectsList(jobjectArray me, Edif::Runtime* runtime) :
	me(me, "objectList ctor from rhPtr"),
	meClass(threadEnv->GetObjectClass(this->me), "objectList array inside rhPtr's class"),
	runtime(runtime)
{
	JNIExceptionCheck();
	length = threadEnv->GetArrayLength(this->me);
	JNIExceptionCheck();
}

RunObjectMultiPlatPtr objectsList::GetOblOffsetByIndex(std::size_t index) {
#ifdef _DEBUG
	if (*(long*)&index < 0)
		raise(SIGTRAP);
#endif

	if (index >= length)
		return nullptr;

	//CObject cObj = run.rhObjectList[index];
	// The CObject is a CRunExtension as well, so we'll just return it directly.
	jobject rhObjEntry = threadEnv->GetObjectArrayElement(me, index);
	JNIExceptionCheck();
	if (rhObjEntry == nullptr)
	{
		LOGE(_T("Looked up object by hoNumber %zu, result was null.\n"), index);
		return nullptr;
	}
	jclass rhObjEntryClass = threadEnv->GetObjectClass(rhObjEntry);
	JNIExceptionCheck();
	auto ro = std::make_shared<RunObject>(rhObjEntry, rhObjEntryClass, runtime);
	activeObjs.push_back(ro);
#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_VERBOSE)
	auto ho = ro->get_rHo();
	ho->get_Number(); // load for log line
	LOGV(_T("Running %s(%zu), returning jobject %p; ho = %p; num = %hi [index in ObjList], oi = %hi [OI index], oilist = %s.\n"), _T(__FUNCTION__),
		index, rhObjEntry, ho, ho->get_Number(), ho->get_Oi(), ho->get_OiList()->get_name());
#endif
	return ro;
}

int objInfoList::get_EventCount() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!EventCount.has_value())
	{
		EventCount = threadEnv->GetIntField(me, eventCountFieldID);
		JNIExceptionCheck();
	}
#ifdef _DEBUG
	else
	{
		int newOilEventCount = threadEnv->GetIntField(me, eventCountFieldID);
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
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!EventCountOR.has_value())
	{
		EventCountOR = threadEnv->GetIntField(me, eventCountORFieldID);
		JNIExceptionCheck();
	}
#ifdef _DEBUG
	else
	{
		int newOilEventCount = threadEnv->GetIntField(me, eventCountORFieldID);
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
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!ListSelected.has_value())
	{
		ListSelected = threadEnv->GetShortField(me, listSelectedFieldID);
		JNIExceptionCheck();
	}
#ifdef _DEBUG
	else
	{
		short oldValue = ListSelected.value();
		short curValue = threadEnv->GetShortField(me, listSelectedFieldID);
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
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!NumOfSelected.has_value())
	{
		NumOfSelected = threadEnv->GetIntField(me, numOfSelectedFieldID);
		JNIExceptionCheck();
	}
	return NumOfSelected.value();
}
short objInfoList::get_Oi() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!Oi.has_value())
	{
		Oi = threadEnv->GetShortField(me, oiFieldID);
		JNIExceptionCheck();
	}
	return Oi.value();
}
int objInfoList::get_NObjects() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!NObjects.has_value())
	{
		NObjects = threadEnv->GetIntField(me, nObjectsFieldID);
		JNIExceptionCheck();
	}
	return NObjects.value();
}
short objInfoList::get_Object() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!Object.has_value())
	{
		Object = threadEnv->GetShortField(me, objectFieldID);
		JNIExceptionCheck();
	}
	return Object.value();
}
const TCHAR* objInfoList::get_name() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!name.has_value())
	{
		JavaAndCString str((jstring)threadEnv->GetObjectField(me, nameFieldID));
		name = str.str();
	}
	return name.value().c_str();
}
void objInfoList::set_NumOfSelected(int ns) {
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), ns);
	NumOfSelected = ns;
	threadEnv->SetIntField(me, numOfSelectedFieldID, ns);
	JNIExceptionCheck();
}
void objInfoList::set_ListSelected(short sh) {
	LOGV(_T("Running %s(%hi); %p; oilObject %hi, oilOi %hi, name %s.\n"), _T(__FUNCTION__), sh, this,
		Object.value_or(-2), Oi.value_or(-2), name.value_or("??"s).c_str());
	ListSelected = sh;
	threadEnv->SetShortField(me, listSelectedFieldID, sh);
	JNIExceptionCheck();
}
void objInfoList::set_EventCount(int ec) {
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), ec);
	EventCount = ec;
	threadEnv->SetIntField(me, eventCountFieldID, ec);
	JNIExceptionCheck();
}
void objInfoList::set_EventCountOR(int ec) {
	LOGV(_T("Running %s(%i).\n"), _T(__FUNCTION__), ec);
	EventCountOR = ec;
	threadEnv->SetIntField(me, eventCountORFieldID, ec);
	JNIExceptionCheck();
}
short objInfoList::get_QualifierByIndex(std::size_t index) {
	if (index >= MAX_QUALIFIERS) {
		LOGE(_T("Invalid qualifier index read: expected 0 to 7, got %zu.\n"), index);
		return -1;
	}
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
int objInfoList::get_oilNext() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int oilNext = threadEnv->GetIntField(me, nextFieldID);
	JNIExceptionCheck();
	return oilNext;
}
void objInfoList::set_oilNext(int value) {
	threadEnv->SetIntField(me, nextFieldID, value);
	JNIExceptionCheck();
}

// When an ActionLoop is active, this is whether to iterate further or not.
bool objInfoList::get_oilNextFlag() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const bool oilNextFlag = threadEnv->GetBooleanField(me, nextFlagFieldID);
	JNIExceptionCheck();
	return oilNextFlag;
}
void objInfoList::set_oilNextFlag(bool value) {
	threadEnv->SetBooleanField(me, nextFlagFieldID, value);
	JNIExceptionCheck();
}

int objInfoList::get_oilCurrentRoutine() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int oilCurrentRoutine = threadEnv->GetIntField(me, currentRoutineFieldID);
	JNIExceptionCheck();
	return oilCurrentRoutine;
}
void objInfoList::set_oilCurrentRoutine(int value) {
	threadEnv->SetIntField(me, currentRoutineFieldID, value);
	JNIExceptionCheck();
}

// When an Action is active, this specifies which object is currently being iterated. -1 if invalid.
int objInfoList::get_oilCurrentOi() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int oilCurrentOi = threadEnv->GetIntField(me, currentOiFieldID);
	JNIExceptionCheck();
	return oilCurrentOi;
}
void objInfoList::set_oilCurrentOi(int value) {
	threadEnv->SetIntField(me, currentOiFieldID, value);
	JNIExceptionCheck();
}

// When an Action is active, this applies oilCurrentRountine
int objInfoList::get_oilActionCount() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int oilActionCount = threadEnv->GetIntField(me, actionCountFieldID);
	JNIExceptionCheck();
	return oilActionCount;
}
void objInfoList::set_oilActionCount(int value) {
	threadEnv->SetIntField(me, actionCountFieldID, value);
	JNIExceptionCheck();
	ActionCount = value;
}

// When an ActionLoop is active (Action repeating in a fastloop), this applies oilCurrentRountine
int objInfoList::get_oilActionLoopCount() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int oilActionLoopCount = threadEnv->GetIntField(me, actionLoopCountFieldID);
	JNIExceptionCheck();
	return oilActionLoopCount;
}
void objInfoList::set_oilActionLoopCount(int value) {
	threadEnv->SetIntField(me, actionLoopCountFieldID, value);
	JNIExceptionCheck();
	ActionLoopCount = value;
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
	LOGV(_T("objInfoList made at index %i, ptr %p.\n"), index, this);
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
	LOGV(_T("objInfoList destroyed, index %i, ptr %p.\n"), index, this);
}

void objInfoList::InvalidatedByNewGeneratedEvent()
{
	// invalidated by new event: listselected, object, eventcount, numofselected, nobjects, actioncount, actionloopcount
	ListSelected.reset();
	Object.reset();
	EventCount.reset();
	EventCountOR.reset();
	NumOfSelected.reset();
	NObjects.reset();
	ActionCount.reset();
	ActionLoopCount.reset();
}

// static definition
jfieldID CreateObjectInfo::flagsFieldID, CreateObjectInfo::xFieldID, CreateObjectInfo::yFieldID,
	CreateObjectInfo::dirFieldID, CreateObjectInfo::layerFieldID, CreateObjectInfo::zOrderFieldID;
CreateObjectInfo::CreateObjectInfo(jobject o) :
	me(o, "CCreateObjectInfo")
{
	if (!flagsFieldID)
	{
		jclass meClass = threadEnv->GetObjectClass(o);
		flagsFieldID = threadEnv->GetFieldID(meClass, "cobFlags", "S");
		JNIExceptionCheck();
		xFieldID = threadEnv->GetFieldID(meClass, "cobX", "I");
		JNIExceptionCheck();
		yFieldID = threadEnv->GetFieldID(meClass, "cobY", "I");
		JNIExceptionCheck();
		flagsFieldID = threadEnv->GetFieldID(meClass, "cobDir", "I");
		JNIExceptionCheck();
		layerFieldID = threadEnv->GetFieldID(meClass, "cobLayer", "I");
		JNIExceptionCheck();
		zOrderFieldID = threadEnv->GetFieldID(meClass, "cobZOrder", "I");
		JNIExceptionCheck();
	}
}
CreateObjectInfo::Flags CreateObjectInfo::get_flags() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const Flags cobFlags = (Flags)(std::uint16_t)threadEnv->GetShortField(me, flagsFieldID);
	JNIExceptionCheck();
	return cobFlags;
}
std::int32_t CreateObjectInfo::get_X() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int cobX = threadEnv->GetIntField(me, xFieldID);
	JNIExceptionCheck();
	return cobX;
}
std::int32_t CreateObjectInfo::get_Y() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int cobY = threadEnv->GetIntField(me, yFieldID);
	JNIExceptionCheck();
	return cobY;
}
std::int32_t CreateObjectInfo::GetDir(RunObjectMultiPlatPtr rdPtr) const {
	LOGV(_T("Running %s(%p).\n"), _T(__FUNCTION__), &*rdPtr);
	const int cobDir = threadEnv->GetIntField(me, flagsFieldID);
	if (cobDir != -1 && get_flags() != Flags::None)
		return cobDir;
	const auto roc = rdPtr->get_roc();
	// This shouldn't be getting read, as it's in CreateObjectInfo, so it's current ext
	if (!roc)
	{
		LOGE(_T("This is not a moving extension, why are you reading direction?"));
		return -1;
	}
	return roc->get_dir();
}
std::int32_t CreateObjectInfo::get_layer() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int cobLayer = threadEnv->GetIntField(me, layerFieldID);
	JNIExceptionCheck();
	return cobLayer;
}
std::int32_t CreateObjectInfo::get_ZOrder() const {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	const int cobZOrder = threadEnv->GetIntField(me, zOrderFieldID);
	JNIExceptionCheck();
	return cobZOrder;
}


int qualToOi::get_qoiCurrentOi() const {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiCurrentOi", "S");
	JNIExceptionCheck();
	const jshort value = threadEnv->GetShortField(me, fieldID);
	JNIExceptionCheck();
	return value;
}
void qualToOi::set_qoiCurrentOi(int value) {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiCurrentOi", "S");
	JNIExceptionCheck();
	threadEnv->SetShortField(me, fieldID, (jshort)value);
	JNIExceptionCheck();
}
int qualToOi::get_qoiNext() const {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiNext", "S");
	JNIExceptionCheck();
	const jshort value = threadEnv->GetShortField(me, fieldID);
	JNIExceptionCheck();
	return value;
}
void qualToOi::set_qoiNext(int value) {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiNext", "S");
	JNIExceptionCheck();
	threadEnv->SetShortField(me, fieldID, (jshort)value);
	JNIExceptionCheck();
}
int qualToOi::get_qoiActionPos() const {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiActionPos", "S");
	JNIExceptionCheck();
	const jshort value = threadEnv->GetShortField(me, fieldID);
	JNIExceptionCheck();
	return value;
}
void qualToOi::set_qoiActionPos(int value) {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiActionPos", "S");
	JNIExceptionCheck();
	threadEnv->SetShortField(me, fieldID, (jshort)value);
	JNIExceptionCheck();
}
int qualToOi::get_qoiCurrentRoutine() const {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiCurrentRoutine", "I");
	JNIExceptionCheck();
	const jint value = threadEnv->GetIntField(me, fieldID);
	JNIExceptionCheck();
	return value;
}
void qualToOi::set_qoiCurrentRoutine(int value) {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiCurrentRoutine", "I");
	JNIExceptionCheck();
	threadEnv->SetIntField(me, fieldID, value);
	JNIExceptionCheck();
}
int qualToOi::get_qoiActionCount() const {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiActionCount", "I");
	JNIExceptionCheck();
	const jint value = threadEnv->GetIntField(me, fieldID);
	JNIExceptionCheck();
	return value;
}
void qualToOi::set_qoiActionCount(int value) {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiActionCount", "I");
	JNIExceptionCheck();
	threadEnv->SetIntField(me, fieldID, value);
	JNIExceptionCheck();
}
int qualToOi::get_qoiActionLoopCount() const {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiActionLoopCount", "I");
	JNIExceptionCheck();
	const jint value = threadEnv->GetIntField(me, fieldID);
	JNIExceptionCheck();
	return value;
}
void qualToOi::set_qoiActionLoopCount(int value) {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiActionLoopCount", "I");
	JNIExceptionCheck();
	threadEnv->SetIntField(me, fieldID, value);
	JNIExceptionCheck();
}
bool qualToOi::get_qoiNextFlag() const {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiNextFlag", "Z");
	JNIExceptionCheck();
	const jboolean value = threadEnv->GetBooleanField(me, fieldID);
	JNIExceptionCheck();
	return value;
}
void qualToOi::set_qoiNextFlag(bool value) {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiNextFlag", "Z");
	JNIExceptionCheck();
	threadEnv->SetBooleanField(me, fieldID, value);
	JNIExceptionCheck();
}
bool qualToOi::get_qoiSelectedFlag() const {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiSelectedFlag", "Z");
	JNIExceptionCheck();
	const jboolean value = threadEnv->GetBooleanField(me, fieldID);
	JNIExceptionCheck();
	return value;
}
void qualToOi::set_qoiSelectedFlag(bool value) {
	static jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiSelectedFlag", "Z");
	JNIExceptionCheck();
	threadEnv->SetBooleanField(me, fieldID, value);
	JNIExceptionCheck();
}
short qualToOi::get_Oi(std::size_t i) {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	// Update internal list
	if (OiAndOiListLength == SIZE_MAX)
		get_OiList(0);

	if (i * 2 >= OiAndOiListLength)
		return -1;

	return OiAndOiList[i * 2];
}
short qualToOi::get_OiList(std::size_t i) {
	LOGV(_T("Running qualToOi::%s().\n"), _T(__FUNCTION__));
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
		LOGV(_T("qualToOi::%s() - OiAndOiList for qual %i was populated OK with %zu entries.\n"), _T(__FUNCTION__), offsetInQualToOiList, OiAndOiListLength);
	}
	else
		LOGV(_T("qualToOi::%s() -  OiAndOiList for qual %i was already populated with %zu entries, returning index %zu.\n"), _T(__FUNCTION__), offsetInQualToOiList, OiAndOiListLength, i * 2 + 1);

	if (i * 2 + 1 >= OiAndOiListLength)
		return -1;

	return OiAndOiList[i * 2 + 1];
}

qualToOi::qualToOi(RunHeader* rh, int offset, jobject me, Edif::Runtime* runtime) :
	rh(rh), offsetInQualToOiList(offset), me(me, "qualToOi ctor"), runtime(runtime)
{
	LOGV(_T("qualToOi::%s() - OiAndOiList not populated, doing so.\n"), _T(__FUNCTION__));
	jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiList", "[S");
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
	jfieldID fieldID = threadEnv->GetFieldID(rh->QualToOiClass, "qoiList", "[S");
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

std::uint8_t EventGroupMP::get_evgNCond() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!evgNCond.has_value())
	{
		evgNCond = (std::uint8_t)threadEnv->GetShortField(me, evgNCondFieldID);
		JNIExceptionCheck();
	}
	return evgNCond.value();
}
std::uint8_t EventGroupMP::get_evgNAct() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!evgNAct.has_value())
	{
		evgNAct = (std::uint8_t)threadEnv->GetShortField(me, evgNActFieldID);
		JNIExceptionCheck();
	}
	return evgNAct.value();
}
std::int16_t EventGroupMP::get_evgIdentifier() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!evgIdentifier.has_value())
	{
		// We read identifier from evgLine
		evgIdentifier = threadEnv->GetShortField(me, evgLineFieldID);
		JNIExceptionCheck();
	}
	return evgIdentifier.value();
}

EventGroupFlags EventGroupMP::get_evgFlags() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (!evgFlags.has_value())
	{
		evgFlags = (EventGroupFlags)threadEnv->GetShortField(me, evgFlagsFieldID);
		JNIExceptionCheck();
	}
	return evgFlags.value();
}

std::unique_ptr<event2> EventGroupMP::GetCAByIndex(std::size_t index) {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (evgEvents.invalid())
		GetEventList(); // ignore return

	if (index >= evgEventsLength)
		return nullptr;

	jobject item = threadEnv->GetObjectArrayElement(evgEvents, index);
	JNIExceptionCheck();
	return std::make_unique<event2>(this, index, item, runtime);
}

jobjectArray EventGroupMP::GetEventList() {
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (evgEvents.invalid())
	{
		evgEvents = global((jobjectArray)threadEnv->GetObjectField(me, evgEventsFieldID), "evgEvents inside eventGroup");
		JNIExceptionCheck();
		evgEventsLength = threadEnv->GetArrayLength(evgEvents);
		JNIExceptionCheck();
		LOGV(_T("Running %s(), line %d, was invalid, new one is %p, size %d, evgID %hi.\n"), _T(__FUNCTION__), __LINE__, evgEvents.ref, evgEventsLength, get_evgIdentifier());
	}
	else
		LOGV(_T("Running %s(), line %d. Was already valid.\n"), _T(__FUNCTION__), __LINE__);
	return evgEvents;
}


// Static definition - default inited to zero, i.e. nullptr
jfieldID EventGroupMP::evgNCondFieldID, EventGroupMP::evgNActFieldID,
	EventGroupMP::evgFlagsFieldID, EventGroupMP::evgLineFieldID,
	EventGroupMP::evgEventsFieldID;
EventGroupMP::EventGroupMP(jobject me, Edif::Runtime * runtime) :
	me(me, "EventGroupMP ctor"), runtime(runtime)
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

#elif defined(__APPLE__)
#include "MMF2Lib/CRunExtension.h"
#include "MMF2Lib/CRun.h"
#include "MMF2Lib/CObject.h"
#include "MMF2Lib/CEventProgram.h"
#include "MMF2Lib/CObjInfo.h"
#include "MMF2Lib/CQualToOiList.h"
#include "MMF2Lib/CEvents.h"
#include "MMF2Lib/CActExtension.h"

Edif::Runtime::Runtime(Extension * ext, void * const objCExtPtr) : ext(ext),
	objCExtPtr(objCExtPtr), ObjectSelection((RunHeader *)((CRunExtension *)objCExtPtr)->rh)
{
	SDKPointer = Edif::SDK;
	this->hoPtr = (HeaderObject *)((CRunExtension *)objCExtPtr)->ho;

	ext->rdPtr = (RunObject*)hoPtr;
	ext->rhPtr = hoPtr->get_AdRunHeader();
	this->ObjectSelection.rhPtr = ext->rhPtr;
}


Edif::Runtime::~Runtime()
{
}

extern "C" void DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, generateEvent)(void * ext, int code, int param);
extern "C" void DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, reHandle)(void * ext);

void Edif::Runtime::Rehandle()
{
	DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, reHandle)(this->objCExtPtr);
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
	hoPtr->SetPosition(X, Y);
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

short Edif::Runtime::GetOIListIndexFromObjectParam(std::size_t paramIndex)
{
	// This still works for conditions, as they store params in same offset.
	const eventParam* curParam = ((CActExtension*)curCEvent)->pParams[0];
	for (int i = 0; i < paramIndex; ++i)
		curParam = (const eventParam*)((const std::uint8_t*)curParam + curParam->evpSize);
	if ((Params)curParam->evpCode != Params::Object)
		LOGE(_T("GetOIListIndexFromObjectParam: Returning a OI for a non-Object parameter."));
	LOGD(_T("GetOIListIndexFromObjectParam: Returning OiList %hi, oi is %hi.\n"), curParam->evp.evpW.evpW0, curParam->evp.evpW.evpW1);
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

#if TEXT_OEFLAG_EXTENSION
ProjectFunc void * PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _getRunObjectFont)(void* cppExtPtr) {
	return ((Extension*)cppExtPtr)->Runtime.GetRunObjectFont();
}
ProjectFunc int PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _getRunObjectTextColor)(void* cppExtPtr) {
	return ((Extension*)cppExtPtr)->Runtime.GetRunObjectTextColor();
}
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _setRunObjectFont)(void* cppExtPtr, void * fontInfo, void* rcPtr) {
	((Extension*)cppExtPtr)->Runtime.SetRunObjectFont(fontInfo, rcPtr);
}
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _setRunObjectTextColor)(void* cppExtPtr, int rgb) {
	((Extension*)cppExtPtr)->Runtime.SetRunObjectTextColor(rgb);
}
#endif // TEXT_OEFLAG_EXTENSION



CRunAppMultiPlat* RunHeader::get_App() {
	return (CRunAppMultiPlat*)((CRun*)this)->rhApp;
}

// Gets the RH2 event count, used in object selection
int RunHeader::GetRH2EventCount()
{
	return ((CRun*)this)->rhEvtProg->rh2EventCount;
}
void RunHeader::SetRH2EventCount(int newEventCount)
{
	((CRun*)this)->rhEvtProg->rh2EventCount = newEventCount;
}
// Gets the RH4 event count for OR, used in object selection in OR-related events.
int RunHeader::GetRH4EventCountOR()
{
	return ((CRun*)this)->rhEvtProg->rh4EventCountOR;
}
void RunHeader::SetRH4EventCountOR(int value) {
	((CRun*)this)->rhEvtProg->rh4EventCountOR = value;
}
int RunHeader::GetRH2ActionCount() { return ((CRun*)this)->rhEvtProg->rh2ActionCount; }
int RunHeader::GetRH2ActionLoopCount() { return ((CRun*)this)->rhEvtProg->rh2ActionLoopCount; }
void RunHeader::SetRH2ActionCount(int value) { ((CRun*)this)->rhEvtProg->rh2ActionCount = value; }
void RunHeader::SetRH2ActionLoopCount(int value) { ((CRun*)this)->rhEvtProg->rh2ActionLoopCount = value; }


objectsList* RunHeader::get_ObjectList() {
	return (objectsList*)((CRun*)this)->rhObjectList;
}

objInfoList* RunHeader::GetOIListByIndex(std::size_t index)
{
	if (index >= ((CRun*)this)->rhMaxOI)
	{
		LOGE(_T("RunHeader::GetOIListByIndex() was passed invalid OIList index %zu, are you passing a OI instead?\n"), index);
		return nullptr;
	}

#if _DEBUG
	int maxOi = ((CRun*)this)->rhMaxOI;
	assert(maxOi > index || (long)index < 0); // invalid OI
#endif
	return (objInfoList*)((((CRun*)this)->rhOiList)[index]);
}
event2* RunHeader::GetRH4ActionStart() {
	return (event2*)((CRun*)this)->rhEvtProg->rh4ActionStart;
}
bool RunHeader::GetRH2ActionOn() {
	return ((CRun*)this)->rhEvtProg->rh2ActionOn != 0;
}
void RunHeader::SetRH2ActionOn(bool newActOn) {
	((CRun*)this)->rhEvtProg->rh2ActionOn = newActOn ? 1 : 0;
}
FusionInternals::EventGroupMP* RunHeader::get_EventGroup() {
	return (EventGroupMP*)((CRun*)this)->rhEvtProg->rhEventGroup;
}

std::size_t RunHeader::GetNumberOi() {
	// rhMaxOi on non-Windows
	return ((CRun*)this)->rhMaxOI;
}
qualToOi* RunHeader::GetQualToOiListByOffset(std::size_t byteOffset) {
	CEventProgram* const evp = ((CRun*)this)->rhEvtProg;
	// Not initialized during start of frame's CreateRunObject; the qualToOiList is null here (not just its [0])
	// Unlike Windows' packed byte-offset table, Apple exposes an array and its count.
	const std::size_t index = byteOffset & 0x7FFF;
	if (!evp->qualToOiList || index >= (std::size_t)evp->nQualifiers)
		return nullptr;
	return (qualToOi*)evp->qualToOiList[index];
}
RunObjectMultiPlatPtr RunHeader::GetObjectListOblOffsetByIndex(std::size_t index) {
	if (index >= get_MaxObjects())
		return nullptr;
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
int RunHeader::get_WindowX() const {
	return ((CRun*)this)->rhWindowX;
}
int RunHeader::get_WindowY() const {
	return ((CRun*)this)->rhWindowY;
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
void HeaderObject::set_Flags(HeaderObjectFlags value) {
	((CObject*)this)->hoFlags = (short)value;
}
int HeaderObject::get_EventNumber() const {
	return ((const CObject*)this)->hoEventNumber;
}
int HeaderObject::get_CurrentParam() const {
	return ((const CObject*)this)->hoCurrentParam;
}
void HeaderObject::set_EventNumber(int value) {
	((CObject*)this)->hoEventNumber = value;
}
void HeaderObject::set_CurrentParam(int value) {
	((CObject*)this)->hoCurrentParam = value;
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
int HeaderObject::get_X() const {
	return [((CObject*)this) getX];
}
void HeaderObject::SetX(int x) {
	[((CObject*)this) setX: x];
}
int HeaderObject::get_Y() const {
	return ((CObject*)this)->hoY;
}
void HeaderObject::SetY(int y) {
	[((CObject*)this) setY: y];
}
int HeaderObject::get_ImgWidth() const {
	return ((CObject*)this)->hoImgWidth;
}
void HeaderObject::SetImgWidth(int w) {
	[((CObject*)this) setWidth: w];
}
int HeaderObject::get_ImgHeight() const {
	return ((CObject*)this)->hoImgHeight;
}
void HeaderObject::SetImgHeight(int h) {
	[((CObject*)this) setHeight:h];
}
void HeaderObject::SetPosition(int x, int y) {
	// TODO: Confirm what this sets in terms of rcChanged, rcCheckCollides, rmMoveFlag
	[((CObject*)this) setX: x];
	[((CObject*)this) setY: y];
}
int HeaderObject::get_ImgXSpot() const {
	return ((CObject*)this)->hoImgXSpot;
}
int HeaderObject::get_ImgYSpot() const {
	return ((CObject*)this)->hoImgYSpot;
}
int HeaderObject::get_Identifier() const {
	return ((CObject*)this)->hoIdentifier;
}
void HeaderObject::SetSize(int width, int height) {
	[((CObject*)this) setWidth: width];
	[((CObject*)this) setHeight: height];
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

RunSpriteFlag RunSprite::get_Flags() const {
	return (RunSpriteFlag)((CRSpr *)this)->rsFlags;
}
BlitOperation RunSprite::get_Effect() const {
	return (BlitOperation)((CRSpr*)this)->rsEffect;
}
std::uint32_t RunSprite::get_layer() const {
	return ((CRSpr*)this)->rsLayer;
}
// Returns a mix of alpha + color blend coefficient
int RunSprite::get_EffectParam() const {
	return ((CRSpr*)this)->rsEffectParam;
}
int RunSprite::get_EffectShader() const {
	return ((CRSpr*)this)->rsEffectShader;
}

rCom::MovementID rCom::get_nMovement() const { return (rCom::MovementID)((CRCom *)this)->rcMovementType; }
int rCom::get_anim() const { return ((CRCom*)this)->rcAnim; }
int rCom::get_image() const { return ((CRCom*)this)->rcImage; }
float rCom::get_scaleX() const { return ((CRCom*)this)->rcScaleX; }
float rCom::get_scaleY() const { return ((CRCom*)this)->rcScaleY; }
int rCom::get_dir() const { return ((CRCom*)this)->rcDir; }
float rCom::GetAngle() const {
	return ((CRCom*)this)->rcAngle;
}
int rCom::get_speed() const { return ((CRCom*)this)->rcSpeed; }
int rCom::get_minSpeed() const { return ((CRCom*)this)->rcMinSpeed; }
int rCom::get_maxSpeed() const { return ((CRCom*)this)->rcMaxSpeed; }
bool rCom::get_changed() const { return ((CRCom*)this)->rcChanged; }
bool rCom::get_checkCollides() const { return ((CRCom*)this)->rcCheckCollides; }
// Sets current direction (0-31, 0 is right, incrementing ccw)
void rCom::set_dir(const int val) {
	if ((val & 31) != val)
		LOGE(_T("Direction set to %i, outside of range 0-31.\n"), val);
	((CRCom*)this)->rcDir = val;
}
void rCom::set_anim(int val) { ((CRCom*)this)->rcAnim = val; }
void rCom::set_image(int val) { ((CRCom*)this)->rcImage = val; }
void rCom::set_scaleX(float val) { ((CRCom*)this)->rcScaleX = val; }
void rCom::set_scaleY(float val) { ((CRCom*)this)->rcScaleY = val; }
void rCom::SetAngle(float val) {
	((CRCom*)this)->rcAngle = val;
}
void rCom::set_speed(int val) { ((CRCom*)this)->rcSpeed = val; }
void rCom::set_minSpeed(int val) { ((CRCom*)this)->rcMinSpeed = val; }
void rCom::set_maxSpeed(int val) { ((CRCom*)this)->rcMaxSpeed = val; }
void rCom::set_changed(bool val) { ((CRCom*)this)->rcChanged = val; }
void rCom::set_checkCollides(bool val) { ((CRCom*)this)->rcCheckCollides = val; }

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
FusionInternals::RunSprite* RunObject::get_ros() {
	return (RunSprite*)((CObject*)this)->ros;
}
AltVals* RunObject::get_rov() {
	return (AltVals*)((CObject*)this)->rov;
}
RunObjectMultiPlatPtr objectsList::GetOblOffsetByIndex(std::size_t index) {
	return (RunObjectMultiPlatPtr)((CObject**)this)[index];
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

int objInfoList::get_oilNext() const { return ((CObjInfo*)this)->oilNext; }
void objInfoList::set_oilNext(int value) { ((CObjInfo*)this)->oilNext = value; }
bool objInfoList::get_oilNextFlag() const { return ((CObjInfo*)this)->oilNextFlag; }
void objInfoList::set_oilNextFlag(bool value) { ((CObjInfo*)this)->oilNextFlag = value; }
int objInfoList::get_oilCurrentRoutine() const { return ((CObjInfo*)this)->oilCurrentRoutine; }
void objInfoList::set_oilCurrentRoutine(int value) { ((CObjInfo*)this)->oilCurrentRoutine = value; }
int objInfoList::get_oilCurrentOi() const { return ((CObjInfo*)this)->oilCurrentOi; }
void objInfoList::set_oilCurrentOi(int value) { ((CObjInfo*)this)->oilCurrentOi = value; }
int objInfoList::get_oilActionCount() const { return ((CObjInfo*)this)->oilActionCount; }
void objInfoList::set_oilActionCount(int value) { ((CObjInfo*)this)->oilActionCount = value; }
int objInfoList::get_oilActionLoopCount() const { return ((CObjInfo*)this)->oilActionLoopCount; }
void objInfoList::set_oilActionLoopCount(int value) { ((CObjInfo*)this)->oilActionLoopCount = value; }
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
	if (index >= MAX_QUALIFIERS) {
		LOGE(_T("Invalid qualifier index read: expected 0 to 7, got %zu.\n"), index);
		return -1;
	}
	return ((CObjInfo*)this)->oilQualifiers[index];
}
int qualToOi::get_qoiCurrentOi() const { return ((CQualToOiList*)this)->qoiCurrentOi; }
void qualToOi::set_qoiCurrentOi(int value) { ((CQualToOiList*)this)->qoiCurrentOi = (short)value; }
int qualToOi::get_qoiNext() const { return ((CQualToOiList*)this)->qoiNext; }
void qualToOi::set_qoiNext(int value) { ((CQualToOiList*)this)->qoiNext = (short)value; }
int qualToOi::get_qoiActionPos() const { return ((CQualToOiList*)this)->qoiActionPos; }
void qualToOi::set_qoiActionPos(int value) { ((CQualToOiList*)this)->qoiActionPos = (short)value; }
int qualToOi::get_qoiCurrentRoutine() const { return ((CQualToOiList*)this)->qoiCurrentRoutine; }
void qualToOi::set_qoiCurrentRoutine(int value) { ((CQualToOiList*)this)->qoiCurrentRoutine = value; }
int qualToOi::get_qoiActionCount() const { return ((CQualToOiList*)this)->qoiActionCount; }
void qualToOi::set_qoiActionCount(int value) { ((CQualToOiList*)this)->qoiActionCount = value; }
int qualToOi::get_qoiActionLoopCount() const { return ((CQualToOiList*)this)->qoiActionLoopCount; }
void qualToOi::set_qoiActionLoopCount(int value) { ((CQualToOiList*)this)->qoiActionLoopCount = value; }
bool qualToOi::get_qoiNextFlag() const { return ((CQualToOiList*)this)->qoiNextFlag; }
void qualToOi::set_qoiNextFlag(bool value) { ((CQualToOiList*)this)->qoiNextFlag = value; }
bool qualToOi::get_qoiSelectedFlag() const { return ((CQualToOiList*)this)->qoiSelectedFlag; }
void qualToOi::set_qoiSelectedFlag(bool value) { ((CQualToOiList*)this)->qoiSelectedFlag = value; }

CreateObjectInfo::Flags CreateObjectInfo::get_flags() const {
	return (CreateObjectInfo::Flags)(((CCreateObjectInfo*)this)->cobFlags);
}
std::int32_t CreateObjectInfo::get_X() const {
	return ((CCreateObjectInfo*)this)->cobX;
}
std::int32_t CreateObjectInfo::get_Y() const {
	return ((CCreateObjectInfo*)this)->cobY;
}
std::int32_t CreateObjectInfo::GetDir(RunObjectMultiPlatPtr ptr) const {
	const int cobDir = ((CCreateObjectInfo*)this)->cobDir;
	if (cobDir != -1 && ((CCreateObjectInfo*)this)->cobFlags != 0)
		return cobDir;
	const auto roc = ptr->get_roc();
	// This shouldn't be getting read, as it's in CreateObjectInfo, so it's current ext
	if (!roc)
	{
		LOGE(_T("This is not a moving extension, why are you reading direction?"));
		return -1;
	}
	return roc->get_dir();
}
std::int32_t CreateObjectInfo::get_layer() const {
	return ((CCreateObjectInfo*)this)->cobLayer;
}
std::int32_t CreateObjectInfo::get_ZOrder() const {
	return ((CCreateObjectInfo*)this)->cobZOrder;
}

std::uint8_t EventGroupMP::get_evgNCond() const {
	return ((tagEVG*)this)->evgNCond;
}
std::uint8_t EventGroupMP::get_evgNAct() const {
	return ((tagEVG*)this)->evgNAct;
}
std::uint16_t EventGroupMP::get_evgIdentifier() const {
	return ((tagEVG*)this)->evgFree;
}
std::uint16_t EventGroupMP::get_evgInhibit() const {
	return *(std::uint16_t*)&((tagEVG*)this)->evgInhibit;
}
EventGroupFlags EventGroupMP::get_evgFlags() const {
	return *(EventGroupFlags*)&((tagEVG*)this)->evgFlags;
}


event2* EventGroupMP::GetCAByIndex(size_t index)
{
	LOGV(_T("Running %s().\n"), _T(__FUNCTION__));
	if (index >= (size_t)(((tagEVG*)this)->evgNCond) + ((tagEVG*)this)->evgNAct)
		return nullptr;

	event2* ret = (event2*)(&((event2*)this)[1]);
	for (size_t i = 0; i < index && ret; ++i) {
		ret = ret->Next();
	}
	return ret;
}

#else // not APPLE
	#error Unexpected platform
#endif // not APPLE

#endif // Apple or Android
