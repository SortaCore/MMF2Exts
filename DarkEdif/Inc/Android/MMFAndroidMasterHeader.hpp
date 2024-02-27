#pragma once

#ifndef __ANDROID__
#error Included the wrong header for this OS. Include MMFMasterHeader.h instead
#endif

// Cover up clang warnings about unused features we make available
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wcomment"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-variable"

#include "../Shared/AllPlatformDefines.hpp"
#include "../Shared/NonWindowsDefines.hpp"

#include <asm-generic\posix_types.h>
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <stddef.h>

#define SUBSTRIFY(X) #X
#define STRIFY(X) #X

void Sleep(unsigned int milliseconds);
#define _CrtCheckMemory() /* no op */

#include <signal.h>
#include <map>

#define ProjectFunc extern "C" JNIEXPORT
#define FusionAPI /* no declarator */
#include <fcntl.h>
#include <errno.h>
#include <jni.h>
#include <unistd.h>
#include <sys/resource.h>
#include <android/log.h>
#include <math.h>
#include <optional>

// Note: doesn't use underlying_type due to incompatibility with one of the Android C++ STL libraries (stlport_static).

typedef unsigned short ushort;
typedef unsigned int uint;

// Do not use everywhere! JNIEnv * are thread-specific. Use Edif::Runtime JNI functions to get a thread-local one.
extern JNIEnv * mainThreadJNIEnv;
extern JavaVM * global_vm;

struct CTransition;
struct CDebugger;
enum class KGI {
	VERSION,			// Version (required)
	NOTUSED,			// Not used
	PLUGIN,				// Version for plug-in (required)
	MULTIPLESUBTYPE,	// Allow sub-types
	NOTUSED2,			// Reserved
	ATX_CONTROL,		// Not used
	PRODUCT,			// Minimum product the extension is compatible with
	BUILD,				// Minimum build the extension is compatible with
	UNICODE_,			// Returns TRUE if the extension is in Unicode (UNICODE and _UNICODE are #defines)
};

// Surface.h pre-declaration
class cSurface;
class cSurfaceImplementation;
class CFillData;
class CInputFile;
struct sMask;

#define bit1  0x00000001
#define bit2  0x00000002
#define bit3  0x00000004
#define bit4  0x00000008
#define bit5  0x00000010
#define bit6  0x00000020
#define bit7  0x00000040
#define bit8  0x00000080
#define bit9  0x00000100
#define bit10 0x00000200
#define bit11 0x00000400
#define bit12 0x00000800
#define bit13 0x00001000
#define bit14 0x00002000
#define bit15 0x00004000
#define bit16 0x00008000
#define bit17 0x00010000
#define bit18 0x00020000
#define bit19 0x00040000
#define bit20 0x00080000
#define bit21 0x00100000
#define bit22 0x00200000
#define bit23 0x00400000
#define bit24 0x00800000
#define bit25 0x01000000
#define bit26 0x02000000
#define bit27 0x04000000
#define bit28 0x08000000
#define bit29 0x10000000
#define bit30 0x20000000
#define bit31 0x40000000
#define bit32 0x80000000

struct EDITDATA;
struct LevelObject;

typedef short OINUM;
typedef short HFII;

// Definitions for extensions
#define TYPE_LONG	0x0000
#define TYPE_INT	TYPE_LONG
#define TYPE_STRING	bit1
#define TYPE_FLOAT	bit2				// Pour les extensions
#define TYPE_DOUBLE bit2

// ------------------------------------------------------
// EXTENSION EDITION HEADER STRUCTURE
// ------------------------------------------------------
struct extHeader_v1
{
	short extSize,
		  extMaxSize,
		  extOldFlags,		// For conversion purpose
		  extVersion;		// Version number
};
//typedef extHeader_v1*	extHeader *V1;

// ------------------------------------------------------------
// EXTENSION OBJECT DATA ZONE
// ------------------------------------------------------------

// For GetFileInfos
enum class FILEINFO {
	DRIVE = 1,
	DIR,
	PATH,
	APP_NAME,
	TEMP_PATH
};
class CImageFilterMgr;

#define CNC_GetParameter(hoPtr)							{ }
#define CNC_GetIntParameter(hoPtr)						CallRunTimeFunction(hoPtr, RFUNCTION::GET_PARAM, 0, 0)
#define CNC_GetStringParameter(hoPtr)					CallRunTimeFunction(hoPtr, RFUNCTION::GET_PARAM, 0xFFFFFFFF, 0)
#define CNC_GetFloatValue(hoPtr, par)					CallRunTimeFunction(hoPtr, RFUNCTION::GET_PARAM_FLOAT, par, 0)


// Was EVGFLAGS_**
enum class EventGroupFlags : unsigned short
{
	Once = 0x1,
	NotAlways = 0x2,
	Repeat = 0x4,
	NoMore = 0x8,
	Shuffle = 0x10,
	EditorMark = 0x20,
	UndoMark = 0x40,
	ComplexGroup = 0x80,
	Breakpoint = 0x100,
	AlwaysClean = 0x200,
	OrInGroup = 0x400,
	StopInGroup = 0x800,
	OrLogical = 0x1000,
	Grouped = 0x2000,
	Inactive = 0x4000,
	NoGood = 0x8000,
	Limited = Shuffle | NotAlways | Repeat | NoMore,
	DefaultMask = Breakpoint | Grouped
};
enum_class_is_a_bitmask(EventGroupFlags);


// Converts u8str to UTF-8-Modified str. Expects no embedded nulls
jstring CStrToJStr(const char* u8str);
// Converts std::thread::id to a std::string
std::string ThreadIDToStr(std::thread::id);

extern thread_local JNIEnv* threadEnv;
// Gets and returns a Java Exception. Pre-supposes there is one. Clears the exception.
std::string GetJavaExceptionStr();

// JNI global ref wrapper for Java objects. You risk your jobject/jclass expiring without use of this.
extern const char* globalToMonitor[1];
template<class T>
struct global {
	static_assert(std::is_pointer<T>::value, "Must be a pointer!");
	T ref;
	const char * name;
	bool monitor = false;
	global(global<T> &p) = delete;

	global<T> swap_out() noexcept {
#if _DEBUG
		if (std::is_same_v<jobject, T> && ref && threadEnv->GetObjectRefType(ref) == 0)
		{
			LOGE("Invalid global ref at %p \"%s\" was moved!\n", this, name);
			raise(SIGTRAP);
		}
#endif
		if (ref == nullptr)
			return global<T>();
		global<T> newO(this->ref, this->name);
		newO.monitor = this->monitor;
		this->ref = nullptr;
		this->name = "unset [swapped out]";
		return std::move(newO);
	}
	global<T> & operator= (global<T> && p) noexcept {
		if (std::is_same_v<jobject, T> && p.ref && threadEnv->GetObjectRefType(p.ref) == 0)
		{
			LOGE("Invalid global ref at %p \"%s\" was moved!\n", this, name);
			raise(SIGTRAP);
		}
		this->ref = p.ref;
		this->name = p.name;
		this->monitor = p.monitor;
		p.ref = NULL;
		if (monitor) {
			LOGD("Thread %s: Moved global ref %p \"%s\" from holder %p to %p.\n",
				ThreadIDToStr(std::this_thread::get_id()).c_str(),
				this->ref, name, &p, this);
		}
		return *this;
	}

	global(T p, const char * name) {
		this->name = name;
#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_VERBOSE)
		// Check ref names to monitor
		for (std::size_t i = 0; i < std::size(globalToMonitor); ++i) {
			if (globalToMonitor[i] && !strcmp(globalToMonitor[i], name)) {
				monitor = true;
				break;
			}
		}
#endif

		ref = nullptr;
		if (p == nullptr) {
			LOGE("Couldn't make global ref from null (in \"%s\"). Check the calling function.\n", name);
			return;
		}
		assert(threadEnv != NULL);
		ref = (T)threadEnv->NewGlobalRef(p);
		if (ref == NULL) {
			std::string exc = GetJavaExceptionStr();
			LOGE("Couldn't make global ref from %p [1], error: %s.\n", p, exc.c_str());
		}
		if (monitor)
			LOGD("Thread %s: Creating global pointer %p \"%s\" in global() from original %p.\n", ThreadIDToStr(std::this_thread::get_id()).c_str(), ref, name, p);
		//threadEnv->DeleteLocalRef(p);
	}
	global() {
		ref = NULL;
		name = "unset";
		monitor = false;
	}
	bool invalid() const {
		return ref == NULL;
	}
	bool valid() const {
		return ref != NULL;
	}
	operator const T() const {
		if (ref == NULL) {
			LOGE("null global ref at %p \"%s\" was copied!\n", this, name);
			raise(SIGTRAP);
		}
#if _DEBUG
		if (monitor)
			LOGD("Monitored global ref at %p \"%s\" was used.\n", this, name);
		if (std::is_same_v<jobject, T> && ref && threadEnv->GetObjectRefType(ref) == 0)
		{
			LOGE("Invalid global ref at %p \"%s\" was used!\n", this, name);
			raise(SIGTRAP);
		}
#endif
		return ref;
	}
	~global() {
		if (ref)
		{
#if _DEBUG
			if (monitor) {
				LOGD("Thread %s: Freeing global pointer %p \"%s\" in ~global().\n",
					ThreadIDToStr(std::this_thread::get_id()).c_str(), ref, name);
			}
			if (std::is_same_v<jobject, T> && threadEnv->GetObjectRefType(ref) == 0)
			{
				LOGE("Invalid global ref at %p \"%s\" was moved!\n", this, name);
				raise(SIGTRAP);
			}
			assert(threadEnv != NULL);
#endif
			threadEnv->DeleteGlobalRef(ref);
			ref = NULL;
		}
	}
private:
	global(global<T>&& p) noexcept {
#if _DEBUG
		if (std::is_same_v<jobject, T> && p.ref && threadEnv->GetObjectRefType(p.ref) == 0)
		{
			LOGE("Invalid global ref at %p \"%s\" was moved!\n", this, name);
			raise(SIGTRAP);
		}
#endif
		this->ref = p.ref;
		this->name = p.name;
		this->monitor = p.monitor;
		p.ref = NULL;
		if (monitor) {
			LOGD("Thread %s: Moved global ref %p \"%s\" from holder %p to %p.\n",
				ThreadIDToStr(std::this_thread::get_id()).c_str(),
				this->ref, name, &p, this);
		}
	}

};

namespace Edif { class Runtime; }

struct eventGroup;
struct event2 {
	short get_evtNum();
	OINUM get_evtOi();
	//short get_evtSize();
	void set_evtFlags(std::int8_t);
	std::int8_t get_evtFlags();

	event2(eventGroup * inside, int index, jobject evt, Edif::Runtime* run);
	std::unique_ptr<event2> Next();
	int GetIndex();
protected:
	//friend class Edif::Runtime;
	friend struct RunHeader;
	global<jobject> me;
	global<jclass> meClass;
	Edif::Runtime* runtime;
	eventGroup* owner;
	int index;
	// Magic number to tell ctor to JNI-lookup the array index
	constexpr static int FindIndexMagicNum = -0xABCDEF;
	static jfieldID evtFlagsFieldID, evtSizeFieldID, evtCodeFieldID, evtOiFieldID;
};
///////////////////////////////////////////////////////////////////////
//
// DEFINITION OF THE DIFFERENT PARAMETERS
//
///////////////////////////////////////////////////////////////////////

struct ParamObject {
	unsigned short	OffsetListOI,	//
					Number,			//
					Type;			// Version > FVERSION_NEWOBJECTS
};

#define	CND_SIZE					sizeof(event2)
#define	ACT_SIZE					(sizeof(event2)-2) // Ignore Identifier

struct RunObject;
struct HeaderObject;
// RunObject
typedef std::shared_ptr<RunObject> RunObjectMultiPlatPtr;

struct RunHeader;

struct objectsList {
	NO_DEFAULT_CTORS(objectsList);
	//HeaderObject* get_oblOffset();

	RunObjectMultiPlatPtr GetOblOffsetByIndex(std::size_t);
	objectsList(jobjectArray me, Edif::Runtime* runtime);
protected:
	// For invalidating during events - active as in read from Java, not Active Object specifically
	std::vector<std::weak_ptr<RunObject>> activeObjs;
	friend Edif::Runtime;
	friend RunHeader;
	global<jobjectArray> me;
	global<jclass> meClass;
	int length;
	Edif::Runtime* runtime;
};
struct objInfoList {
	int get_EventCount();
	short get_ListSelected();
	int get_NumOfSelected();
	short get_Oi();
	int get_NObjects();
	short get_Object();
	const TCHAR* get_name();
	short get_QualifierByIndex(std::size_t);
	int get_oilNext();
	bool get_oilNextFlag();
	int get_oilCurrentRoutine();
	int get_oilCurrentOi();
	int get_oilActionCount();
	int get_oilActionLoopCount();
	void set_NumOfSelected(int);
	void set_ListSelected(short);
	void set_EventCount(int);

	objInfoList(int index, RunHeader* containerRH, jobject me, Edif::Runtime* runtime);
	objInfoList(objInfoList&&);
	~objInfoList();

	void InvalidatedByNewGeneratedEvent();
	void SelectNone(RunHeader* rhPtr);
	void SelectAll(RunHeader* rhPtr, bool explicitAll = false);
protected:
	// invalidated by new event: listselected, object, eventcount, numofselected, nobjects, actioncount, actionloopcount

	std::optional<short>	Oi,  			 // THE ObjectInfo number
							ListSelected,	 // First selection
							Type,			 // Type of the object
							Object;			 // First objects in the game
	std::optional<short>	EventCount,
							EventCountOR,
							NumOfSelected;

	std::optional<int>		NObjects,		 // Current number
							ActionCount,	 // Action loop counter
							ActionLoopCount; // Action loop counter
	std::optional<std::string> name;

	bool QualifiersLoaded = false;
	short Qualifiers[MAX_QUALIFIERS];

	friend Edif::Runtime;
	friend RunHeader;
	friend HeaderObject;
	int index;
	// CObjInfo
	global<jobject> me;
	global<jclass> meClass;
	RunHeader * containerRH;
	Edif::Runtime* runtime;

private:
	int get_EventCountOR();
	void set_EventCountOR(int);

	static jfieldID eventCountFieldID, actionLoopCountFieldID, objectFieldID, nameFieldID,
		numOfSelectedFieldID, listSelectedFieldID, eventCountORFieldID, qualifiersFieldID,
		nextFieldID, nextFlagFieldID, currentRoutineFieldID, currentOiFieldID, actionCountFieldID,
		typeFieldID, nObjectsFieldID, oiFieldID;
};
struct qualToOi {
	// returns the object in this qualifier
	short get_Oi(std::size_t idx);
	short get_OiList(std::size_t idx);
	qualToOi(RunHeader* rh, int index, jobject me, Edif::Runtime * runtime);
	qualToOi(qualToOi&& q);
	// Returns all OiList from internal array, used for looping through a qualifier's object IDs
	std::vector<short> GetAllOi();
	// Returns all OiList from internal array, used for looping through a qualifier's objInfoList
	std::vector<short> GetAllOiList();

	// Marks that the native Java side may have changed and cached internal variables should be reset.
	// @remarks Android only, and occurs during new A/C/E runs. Shouldn't need using by user.
	void InvalidatedByNewCondition();
protected:
	std::unique_ptr<short[]> OiAndOiList; // short oi, oiList, oi2, oilist2, etc
	std::size_t OiAndOiListLength = SIZE_MAX; // number of shorts, not short pairs nor byte size
	// CQualToOiList
	RunHeader* rh;
	int offsetInQualToOiList = -1;
	global<jobject> me;
	global<jshortArray> oiAndOiListJava;
	Edif::Runtime* runtime;
	friend struct RunHeader;
private:
	std::vector<short> HalfVector(std::size_t first);
};
struct CEventProgram;
struct eventGroup {
	NO_DEFAULT_CTORS(eventGroup);

	std::uint8_t get_evgNCond();
	std::uint8_t get_evgNAct();
	// std::int16_t get_evgInhibit();
	std::int16_t get_evgIdentifier();
	EventGroupFlags get_evgFlags();
	eventGroup(jobject me, Edif::Runtime * runtime);
	std::unique_ptr<event2> GetCAByIndex(std::size_t index);
protected:
	friend Edif::Runtime;
	friend RunHeader;
	friend struct CEventProgram;
	friend struct event2;
	jobjectArray GetEventList();

	std::optional<std::uint8_t> evgNCond, evgNAct;
	std::optional<std::int16_t> evgIdentifier;
	std::optional<EventGroupFlags> evgFlags;
	global<jobject> me;
	global<jclass> meClass;
	global<jobjectArray> evgEvents;
	int evgEventsLength = -1;
	Edif::Runtime* runtime;

private:
	static jfieldID evgNCondFieldID, evgNActFieldID, evgLineFieldID, evgFlagsFieldID, evgEventsFieldID;
};
struct CEventProgram {
	NO_DEFAULT_CTORS(CEventProgram);

	int get_rh2EventCount();
	void set_rh2EventCount(int eventCount);

	int get_rh4EventCountOR();
	void set_rh4EventCountOR(int eventCount);

	// Reads the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
	int get_rh2ActionCount();
	// Sets the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
	void set_rh2ActionCount(int newActionCount);

	// Reads the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
	int get_rh2ActionLoopCount();
	// Sets the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
	void set_rh2ActionLoopCount(int newActLoopCount);

	// Gets the current expression token number.
	int get_rh4CurToken();
	// Sets the current expression token number.
	void set_rh4CurToken(int newCurToken);
	// Gets the current expression token array; relevant in Android only. 
	jobject get_rh4Tokens();
	// Sets the current expression token array; relevant in Android only. 
	void set_rh4Tokens(jobject newTokensArray);

	eventGroup* get_eventGroup();
	// true: actions are being executed. False: conditions. Neither: undefined
	bool GetRH2ActionOn();

	CEventProgram(jobject me, Edif::Runtime* runtime);

	void InvalidatedByNewGeneratedEvent();
	void InvalidatedByNewCondition();
protected:
	friend Edif::Runtime;
	friend struct RunHeader;

	// invalidated by new event: eventGrp, rh2EventCount, rh4EventCountOr, rh2ActionOn
	// invalidated by new condition: rh4EventCountOr, rh2ActionOn

	std::unique_ptr<eventGroup> eventGrp;
	std::optional<int> rh2EventCount, rh4EventCountOR;
	std::optional<int> rh2ActionCount, rh2ActionLoopCount;

	void SetEventGroup(global<jobject>&& grp);

	global<jobject> me;
	global<jclass> meClass;
	Edif::Runtime* runtime;
};
struct CRunFrame {
	NO_DEFAULT_CTORS(CRunFrame);

	CEventProgram* get_eventProgram();

protected:
	friend class Edif::Runtime;
	friend struct RunHeader;
	Edif::Runtime* runtime;
	jobject me;
};


struct CRunAppMultiPlat {
	NO_DEFAULT_CTORS(CRunAppMultiPlat);
	int get_nCurrentFrame();
	CRunFrame* get_Frame();
	CRunAppMultiPlat * get_ParentApp();
	std::size_t GetNumFusionFrames();

	CRunAppMultiPlat(jobject app, Edif::Runtime * runtime);
protected:
	friend class Edif::Runtime;
	friend struct RunHeader;
	std::unique_ptr<CRunFrame> frame;
	std::optional<int> nCurrentFrame;
	std::size_t numTotalFrames = 0; // 0 if unset
	std::unique_ptr<CRunAppMultiPlat> parentApp;
	bool parentAppIsNull = false;
	jobject me;
	jclass meClass;
	Edif::Runtime* runtime;
};
typedef CRunAppMultiPlat CRunApp;

struct qualToOi;
struct RunHeader {
	NO_DEFAULT_CTORS(RunHeader);

	// Reads the EventCount variable from RunHeader2, used in object selection. DarkEdif-added function for cross-platform.
	int GetRH2EventCount();
	void SetRH2EventCount(int newEventCount);
	// Gets the EventCountOR, used in object selection in OR-related events. DarkEdif-added function for cross-platform.
	int GetRH4EventCountOR();
	event2* GetRH4ActionStart();

	// Reads the rh2.rh2ActionCount variable, used in an action with multiple instances selected, to repeat one action.
	int GetRH2ActionCount();
	// Reads the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
	int GetRH2ActionLoopCount();
	// Reads the current expression token index, used in the middle of expression evaluation.
	int GetRH4CurToken();
	// Reads the current expression token array, used in the middle of expression evaluation. Relevant in Android only.
	global<jobjectArray> GetRH4Tokens();

	// Sets the rh2.rh2ActionCount variable, used in an action with multiple instances selected, to repeat one action.
	void SetRH2ActionCount(int newActionCount);
	// Sets the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions in an event.
	void SetRH2ActionLoopCount(int newActLoopCount);
	// Sets the current expression token index, used in the middle of expression evaluation.
	void SetRH4CurToken(int newCurToken);
	// Sets the current expression token array, used in the middle of expression evaluation. Relevant in Android only.
	void SetRH4Tokens(jobjectArray newTokensArray);

	eventGroup * get_EventGroup();
	std::size_t GetNumberOi();
	//objectsList* get_ObjectList();
	// Returns max number of objects in the Fusion frame, set in frame properties
	std::size_t get_MaxObjects();
	std::size_t get_NObjects();

	objInfoList * GetOIListByIndex(std::size_t index);
	qualToOi * GetQualToOiListByOffset(std::size_t index);
	RunObjectMultiPlatPtr GetObjectListOblOffsetByIndex(std::size_t index);
	EventGroupFlags GetEVGFlags();
	CRunAppMultiPlat* get_App();
	CEventProgram* get_EventProgram();
	RunHeader(jobject me, jclass meClass, Edif::Runtime * runtime);
	void InvalidatedByNewGeneratedEvent();
protected:
	friend qualToOi;
	friend HeaderObject;
	friend struct ConditionOrActionManager_Android;
	friend CEventProgram;
	global<jobject> crun; // CRun seems to have majority of these variables
	global<jclass> crunClass;

	// There is only one EventProgram, even though there are multiple event sheets; they are all appended inside one EventProgram, in Android.
	std::unique_ptr<CEventProgram> eventProgram;
	Edif::Runtime* runtime;
	std::unique_ptr<event2> rh4ActStart;
	std::unique_ptr<CRunAppMultiPlat> App;
	std::optional<eventGroup *> EventGroup; // should be a part of eventProgram, so we don't own it ourselves
	std::unique_ptr<objectsList> ObjectList;
	global<jobjectArray> OiList;
	global<jobjectArray> QualToOiList;
	global<jclass> QualToOiClass;
	global<jclass> oiListClass;
	std::vector<objInfoList> OiListArray;
	int OiListLength = 0, QualToOiListLength = 0;
	std::vector<qualToOi> QualToOiListArray;

	// These don't change in the middle of frame, but we might not read them
	std::optional<int> NumberOi, MaxObjects;
	// This is number of object instances, so it does get invalidated
	std::optional<int> NObjects;

	// Called for getting the whole array during a oi find
	jobjectArray GetOiList();

private:
	static jfieldID rh4TokensFieldID, rh4CurTokenFieldID, eventProgramFieldID, oiListFieldID;
};

typedef jobject CCndExtension;
typedef jobject CActExtension;
typedef jobject CNativeExpInstance;
struct HeaderObject {
	short get_NextSelected();
	unsigned short get_CreationId();
	short get_Number();
	short get_NumNext();
	short get_Oi();
	bool get_SelectedInOR();
	HeaderObjectFlags get_Flags();
	objInfoList * get_OiList();
	EventGroupFlags GetEVGFlags();
	RunHeader* get_AdRunHeader();

	void set_NextSelected(short);
	void set_SelectedInOR(bool);
	HeaderObject(RunObject * ro, jobject me, jclass meClass, Edif::Runtime* runtime);

	void InvalidatedByNewGeneratedEvent();
	int GetFixedValue();
protected:

	// invalidated by event change: eventnumber, nextselected, numprev, numnext, selectedinor, Flags

	std::unique_ptr<RunHeader>			AdRunHeader;	// Run-header address
	std::optional<short>  				Number,			// Number of the object; its index in RunHeader::ObjectList
										NextSelected;	// Selected object list. Do not move from &NextSelected == (this+2).
	std::optional<short>				EventNumber;	// Last event number this selection was set; if matching rh2eventnumber, this selection applies
	std::optional<HeaderObjectFlags>	Flags;
	std::optional<short>				HFII,			// Number of LevObj
										Oi,				// Number of ObjInfo
										NumPrev,		// Same ObjInfo previous object
										NumNext,		// ... next
										Type;			// Type of the object
	std::optional<unsigned short>		CreationId;		// Number of creation
	objInfoList *						OiList = nullptr;			// Pointer to OILIST information
	int oiListIndex = -1; // stored when OiList is inited

	std::optional<bool>					SelectedInOR;
	// These are held by RunObject
	jobject me;
	jclass meClass;
	Edif::Runtime* runtime;
	RunObject* runObj;

	static jfieldID numberFieldID;

	friend struct ConditionOrActionManager_Android;
	// Short way to get number field from a jobject, used by ConditionOrActionManager::GetParamObject
	static short GetObjectParamNumber(jobject);
};

// Java memory pointer and a C memory pointer, for text held in Java memory
struct JavaAndCString
{
	jstring ctx;
	const char* ptr;
};

struct rCom;
struct rMvt;
struct rAni;
struct Sprite;
struct AltVals;

struct CValueMultiPlat {
	NO_DEFAULT_CTORS_OR_DTORS(CValueMultiPlat);
	unsigned int m_type;
	union
	{
		std::int32_t m_long;
		double m_double;
		TCHAR* m_pString;
	};
protected:
	friend AltVals;
	JavaAndCString str;
	CValueMultiPlat(unsigned int type, long value);
};
struct AltVals {
	NO_DEFAULT_CTORS_OR_DTORS(AltVals);
	std::size_t GetAltValueCount() const;
	std::size_t GetAltStringCount() const;
	const TCHAR* GetAltStringAtIndex(const std::size_t) const;
	const CValueMultiPlat * GetAltValueAtIndex(const std::size_t) const;
	void SetAltStringAtIndex(const std::size_t, const std::tstring_view&);
	void SetAltValueAtIndex(const std::size_t, const double);
	void SetAltValueAtIndex(const std::size_t, const int);
	std::uint32_t GetInternalFlags() const;
	void SetInternalFlags(std::uint32_t);
};
struct RunObject {
	HeaderObject* get_rHo();
	rCom* get_roc();
	rMvt* get_rom();
	rAni* get_roa();
	Sprite* get_ros();
	AltVals* get_rov();
	RunObject(jobject, jclass, Edif::Runtime *);
protected:
	std::unique_ptr<HeaderObject> rHo;
	global<jobject> me;
	global<jclass> meClass;
};
// Versions
#define MMFVERSION_MASK		0xFFFF0000
#define MMFBUILD_MASK		0x00000FFF		// MMF build
#define MMFVERFLAG_MASK		0x0000F000
#define MMFVERFLAG_HOME		bit16		// TGF
#define MMFVERFLAG_PRO		bit15		// MMF Pro
#define MMFVERFLAG_DEMO		bit14		// Demo
#define MMFVERFLAG_PLUGIN	bit13		// Plugin
#define MMFVERSION_15		0x01050000		// MMF 1.5
#define MMFVERSION_20		0x02000000		// MMF 2.0
#define	MMF_CURRENTVERSION	MMFVERSION_20

// WARNING: Android has a complete mismatch with actual SDK mV.
struct mv {
	//void * ReAllocEditData(EDITDATA * edPTr, unsigned int dwNewSize);
	//void InvalidateObject();
};

static int globalCount;

#define JAVACHKNULL(x) x; \
	if (threadEnv->ExceptionCheck()) { \
		std::string s = GetJavaExceptionStr(); \
		LOGE("Dead in %s, %i: %s.\n", __PRETTY_FUNCTION__, __LINE__, s.c_str()); \
	}

void Indirect_JNIExceptionCheck(const char * file, const char * func, int line);
#ifdef _DEBUG
	#define JNIExceptionCheck() Indirect_JNIExceptionCheck(__FILE__, __FUNCTION__, __LINE__)
#else
	#define JNIExceptionCheck() (void)0
#endif

struct ConditionOrActionManager_Android;
struct ExpressionManager_Android;

const int REFLAG_DISPLAY = 1;
const int REFLAG_ONESHOT = 2;


// Defined in DarkEdif.cpp with ASM instructions to embed the binary.
extern char darkExtJSON[];
extern unsigned darkExtJSONSize;

// Undo the warning disabling from earlier
#pragma clang diagnostic pop
