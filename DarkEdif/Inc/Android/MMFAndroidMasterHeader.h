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

#include "AllPlatformDefines.hpp"
#include "NonWindowsDefines.hpp"

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

// Callback function identifiers for CallFunction
enum class CallFunctionIDs {
	// Editor only
	INSERTPROPS = 1,		// Insert properties into Property window
	REMOVEPROP,				// Remove property
	REMOVEPROPS,			// Remove properties
	REFRESHPROP,			// Refresh propery
	REALLOCEDITDATA,		// Reallocate edPtr
	GETPROPVALUE,			// Get object's property value
	GETAPPPROPVALUE,		// Get application's property value
	GETFRAMEPROPVALUE,		// Get frame's property value
	SETPROPVALUE,			// Set object's property value
	SETAPPPROPVALUE,		// Set application's property value
	SETFRAMEPROPVALUE,		// Set frame's property value
	GETPROPCHECK,			// Get object's property check state
	GETAPPPROPCHECK,		// Get application's property check state
	GETFRAMEPROPCHECK,		// Get frame's property check state
	SETPROPCHECK,			// Set object's property check state
	SETAPPPROPCHECK,		// Set application's property check state
	SETFRAMEPROPCHECK,		// Set frame's property check state
	INVALIDATEOBJECT,		// Refresh object in frame editor
	RECALCLAYOUT,			// Recalc runtime layout (docking)
	GETNITEMS,				// Get number of items - not yet implemented
	GETNEXTITEM,			// Get next item - not yet implemented
	GETNINSTANCES,			// Get number of item instances - not yet implemented
	GETNEXTINSTANCE,		// Get next item instance - not yet implemented

	// Editor & runtime
	MALLOC = 100,			// Allocate memory
	CALLOC,					// Allocate memory & set it to 0
	REALLOC,				// Re-allocate memory
	FREE,					// Free memory
	GETSOUNDMGR,			// Get sound manager
	CLOSESOUNDMGR,			// Close sound manager
	ENTERMODALLOOP,			// Reserved
	EXITMODALLOOP,			// Reserved
	CREATEEFFECT,			// Create effect (runtime only)
	DELETEEFFECT,			// Delete effect (runtime only)
	CREATEIMAGEFROMFILEA,	// Create image from file (runtime only)
	NEEDBACKGROUNDACCESS,	// HWA : tell the frame the frame surface can be read (runtime only)
	ISHWA,					// Returns TRUE if HWA version (editor and runtime)
	ISUNICODE,				// Returns TRUE if the editor or runtime is in Unicode mode
	ISUNICODEAPP,			// Returns TRUE if the application being loaded is a Unicode application
	GETAPPCODEPAGE,			// Returns the code page of the application
	CREATEIMAGEFROMFILEW,	// Create image from file (runtime only)
};



// ------------------------------------------------------------
// EXTENSION OBJECT DATA ZONE
// ------------------------------------------------------------

#define IsRunTimeFunctionPresent(num)	(num < KPX_MAXFUNCTIONS && hoPtr->AdRunHeader->rh4.rh4KpxFunctions[(int)num].routine != NULL)
#define	CallRunTimeFunction(hoPtr,num,wParam,lParam)	0L/*(hoPtr->AdRunHeader->rh4.rh4KpxFunctions[(int)num].routine(hoPtr, wParam, lParam) )*/
#define	CallRunTimeFunction2(hoPtr,num,wParam,lParam)	0L/*(hoPtr->AdRunHeader->rh4.rh4KpxFunctions[(int)num].routine(hoPtr, wParam, lParam) )*/
#define	CallRunTimeFunction3(rh4_,num,wParam,lParam)	0L/*(rh4_.rh4KpxFunctions[num].routine(hoPtr, wParam, lParam) )*/

enum class RFUNCTION {
	//
	REHANDLE,				// Re-enable the call to Handle() every frame
	GENERATE_EVENT,			// Immediately create a triggered condition (do not call in functions that MMF hasn't began the call to, i.e. threads)
	PUSH_EVENT,				// Generates the event after the next event loop, safer for calling from threads and such.
	GET_STRING_SPACE_EX,	// Allocates memory from MMF for a char * or wchar_t *, letting you store a string.
	GET_PARAM_1,			// Retrieves the value of the first parameter of an a/c/e.
	GET_PARAM_2,			// Retrieves the value of 2nd+ parameter (first call with this is 2nd, next 3rd, etc).
	PUSH_EVENT_STOP,		// Removes event created by Push Event. Cannot do this with Generate Event as it happens immediately.
	PAUSE,
	CONTINUE,
	REDISPLAY,
	GET_FILE_INFOS,			// Retrieve information about the current app (see FILEINFOS namespace and Edif.Runtime.cpp)
	SUB_CLASS_WINDOW,
	REDRAW,					// Causes the object to redraw [a certain part or 100%?].
	DESTROY,
	GET_STRING_SPACE,		// Deprecated GET_STRING_SPACE_EX
	EXECUTE_PROGRAM,
	GET_OBJECT_ADDRESS,
	GET_PARAM,
	GET_EXP_PARAM,
	GET_PARAM_FLOAT,
	EDIT_INT,
	EDIT_TEXT,
	CALL_MOVEMENT,
	SET_POSITION,
	GET_CALL_TABLES
};
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

struct runHeader2 {
	short EventCount;
};
struct runHeader3 {
};
struct runHeader4;
struct HeaderObject;
struct objectsList {
	NO_DEFAULT_CTORS(objectsList);
	HeaderObject  *	oblOffset;
};
struct objInfoList {
	NO_DEFAULT_CTORS(objInfoList);
	short			Oi,  			 // THE ObjectInfo number
					ListSelected,	 // First selection
					Type,			 // Type of the object
					Object;			 // First objects in the game
	short			EventCount,
					NumOfSelected;
	objectsList *	ObjectList;			// Object list address

	int				NObjects,		 // Current number
					ActionCount,	 // Action loop counter
					ActionLoopCount; // Action loop counter
};
struct qualToOi {
	NO_DEFAULT_CTORS(qualToOi);
	short OiList;
};

struct RunHeader {
	NO_DEFAULT_CTORS(RunHeader);
	unsigned short GetEventCount();
};

struct CreateObjectInfo {

};
typedef jobject CCndExtension;
typedef jobject CActExtension;
typedef jobject CNativeExpInstance;
struct HeaderObject {

	RunHeader *			AdRunHeader;	// Run-header address
	short  				Number,			// Number of the object
						NextSelected;	// Selected object list. Do not move from &NextSelected == (this+2).
	short				EventNumber;
	HeaderObjectFlags	Flags;
	short				HFII,			// Number of LevObj
						Oi,				// Number of ObjInfo
						NumPrev,		// Same ObjInfo previous object
						NumNext,		// ... next
						Type;			// Type of the object
	unsigned short		CreationId;		// Number of creation
	objInfoList *		OiList;			// Pointer to OILIST information

	qualToOi *			QualToOiList;
};
struct RunObject {
	HeaderObject  	roHo;		  		// Common structure
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
/*
__inline void * mvReAllocEditData(mv * mV, EDITDATA * edPtr, unsigned int dwNewSize) {
	return mV->ReAllocEditData(edPtr, dwNewSize);
}*/

struct RuntimeFunctions
{
	void * ext;

	struct string
	{
		void * ctx;
		const char * ptr;
	};

	void(*generateEvent) (void * ext, int code, int param);

	int(*act_getParamExpression) (void * ext, void * act);
	string(*act_getParamExpString) (void * ext, void * act);
	float(*act_getParamExpFloat) (void * ext, void * act);

	int(*cnd_getParamExpression) (void * ext, void * cnd);
	string(*cnd_getParamExpString) (void * ext, void * cnd);
	float(*cnd_getParamExpFloat) (void * ext, void * cnd);

	int(*exp_getParamInt) (void * ext, void * exp);
	string(*exp_getParamString) (void * ext, void * exp);
	float(*exp_getParamFloat) (void * ext, void * exp);

	void(*exp_setReturnInt) (void * ext, void * exp, int);
	void(*exp_setReturnString) (void * ext, void * exp, const char *);
	void(*exp_setReturnFloat) (void * ext, void * exp, float);

	void(*freeString) (void * ext, string);
};


const int REFLAG_DISPLAY = 1;
const int REFLAG_ONESHOT = 2;

struct ACE
{
	RuntimeFunctions * fn;
	void * ext;

	RuntimeFunctions::string strings[8];
	int stringIndex;

	inline const char * trackString(RuntimeFunctions::string s)
	{
		strings[stringIndex++] = s;
		return s.ptr;
	}

	inline ACE()
	{
		stringIndex = 0;
	}

	inline ~ACE()
	{
		while (--stringIndex >= 0)
			fn->freeString(ext, strings[stringIndex]);
	}
};

class Action : public ACE
{
	void * act;

public:

	inline Action(RuntimeFunctions * fn, void * ext, void * _act)
		: act(_act)
	{
		this->ext = ext;
		this->fn = fn;
	}

	inline int getParamExpression()
	{
		return fn->act_getParamExpression(ext, act);
	}

	inline const char * getParamExpString()
	{
		return trackString(fn->act_getParamExpString(ext, act));
	}

	inline float getParamExpFloat()
	{
		return fn->act_getParamExpFloat(ext, act);
	}
};

class Condition : public ACE
{
	void * cnd;

public:

	inline Condition(RuntimeFunctions * fn, void * ext, void * _cnd)
		: cnd(_cnd)
	{
		this->ext = ext;
		this->fn = fn;
	}

	inline int getParamExpression()
	{
		return fn->cnd_getParamExpression(ext, cnd);
	}

	inline const char * getParamExpString()
	{
		return trackString(fn->cnd_getParamExpString(ext, cnd));
	}

	inline float getParamExpFloat()
	{
		return fn->cnd_getParamExpFloat(ext, cnd);
	}
};

class Expression : public ACE
{
	void * exp;

public:

	inline Expression(RuntimeFunctions * fn, void * ext, void * _exp)
		: exp(_exp)
	{
		this->ext = ext;
		this->fn = fn;
	}

	inline int getParamInt()
	{
		return fn->exp_getParamInt(ext, exp);
	}

	inline const char * getParamString()
	{
		return trackString(fn->exp_getParamString(ext, exp));
	}

	inline float getParamFloat()
	{
		return fn->exp_getParamFloat(ext, exp);
	}

	inline void setReturnInt(int value)
	{
		fn->exp_setReturnInt(ext, exp, value);
	}

	inline void setReturnString(const char * value)
	{
		fn->exp_setReturnString(ext, exp, value);
	}

	inline void setReturnFloat(float value)
	{
		fn->exp_setReturnFloat(ext, exp, value);
	}
};


// Gets and returns a Java Exception. Pre-supposes there is one. Clears the exception.
std::string GetJavaExceptionStr();

/*struct monitor {
	jobject dat;
	size_t numRefs;
};
std::vector<monitor> monitors;*/

extern thread_local JNIEnv * threadEnv;


void LOGF(const TCHAR * x, ...);

// Converts u8str to UTF-8Modified str. Expects no embedded nulls
jstring CStrToJStr(const char * u8str);
// Converts std::thread::id to a std::string
std::string ThreadIDToStr(std::thread::id);

static int globalCount;

// JNI global ref wrapper for Java objects. You risk your jobject/jclass expiring without use of this.
template<class T>
struct global {
	static_assert(std::is_pointer<T>::value, "Must be a pointer!");
	T ref;
	const char * name;
	global(global<T> &&p) = delete;
	global(global<T> &p) = delete;

	global<T> & operator= (global<T> && p) noexcept {
		this->ref = p.ref;
		this->name = p.name;
		p.ref = NULL;
		LOGV("Thread %s: Moved global ref %p \"%s\" from holder %p to %p.",
			ThreadIDToStr(std::this_thread::get_id()).c_str(),
			this->ref, name, &p, this);
		return *this;
	}

	global(T p, const char * name) {
		this->name = name;
		ref = nullptr;
		if (p == nullptr) {
			LOGE("Couldn't make global ref from null (in \"%s\"). Check the calling function.", name);
			return;
		}
		assert(threadEnv != NULL);
		ref = (T)threadEnv->NewGlobalRef(p);
		if (ref == NULL) {
			std::string exc = GetJavaExceptionStr();
			LOGE("Couldn't make global ref from %p [1], error: %s.", p, exc.c_str());
		}
		LOGV("Thread %s: Creating global pointer %p \"%s\" in global() from original %p.", ThreadIDToStr(std::this_thread::get_id()).c_str(), ref, name, p);
		//threadEnv->DeleteLocalRef(p);
	}
	global() {
		ref = NULL;
		name = "unset";
	}
	bool invalid() const {
		return ref == NULL;
	}
	bool valid() const {
		return ref != NULL;
	}
	operator const T() const {
		if (ref == NULL) {
			LOGE("null global ref at %p \"%s\" was copied!", this, name);
			raise(SIGTRAP);
		}
		return ref;
	}
	~global() {
		if (ref)
		{
			LOGV("Thread %s: Freeing global pointer %p \"%s\" in ~global().",
				ThreadIDToStr(std::this_thread::get_id()).c_str(), ref, name);
			assert(threadEnv != NULL);
			threadEnv->DeleteGlobalRef(ref);
			ref = NULL;
		}
	}
};

#define JAVACHKNULL(x) x; \
	if (threadEnv->ExceptionCheck()) { \
		std::string s = GetJavaExceptionStr(); \
		LOGE("Dead in %s, %i: %s.", __PRETTY_FUNCTION__, __LINE__, s.c_str()); \
	}

void Indirect_JNIExceptionCheck(const char * file, const char * func, int line);
#ifdef _DEBUG
	#define JNIExceptionCheck() Indirect_JNIExceptionCheck(__FILE__, __FUNCTION__, __LINE__)
#else
	#define JNIExceptionCheck() (void)0
#endif

// Defined in DarkEdif.cpp with ASM instructions to embed the binary.
extern char darkExtJSON[];
extern unsigned darkExtJSONSize;

// Undo the warning disabling from earlier
#pragma clang diagnostic pop
