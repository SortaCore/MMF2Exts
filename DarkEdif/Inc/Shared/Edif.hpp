#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <atomic>
#include "json.hpp"

#ifdef _WIN32
	#include "..\Windows\MMFWindowsMasterHeader.hpp"
	extern HINSTANCE hInstLib;
#elif defined (__ANDROID__)
	#include "..\Android\MMFAndroidMasterHeader.hpp"
#elif defined (__APPLE__)
#if MacBuild == 0
	#include "../iOS/MMFiOSMasterHeader.hpp"
#else
	#include "../Mac/MMFMacMasterHeader.hpp"
#endif
#endif

class Extension;

#include "ObjectSelection.hpp"

// DarkEdif provides C++11 type checking between JSON and C++ definition.
#if defined(_DEBUG) && defined(_WIN32) && !defined(FAST_ACE_LINK)
	#define LinkAction(ID, Function) \
		LinkActionDebug(ID, &Extension::Function);
	#define LinkCondition(ID, Function) \
		LinkConditionDebug(ID, &Extension::Function);
	#define LinkExpression(ID, Function) \
		LinkExpressionDebug(ID, &Extension::Function);
#else
	#define LinkAction(ID, Function) \
		Edif::SDK->ActionFunctions[ID] = Edif::MemberFunctionPointer(&Extension::Function);
	#define LinkCondition(ID, Function) \
		Edif::SDK->ConditionFunctions[ID] = Edif::MemberFunctionPointer(&Extension::Function);
	#define LinkExpression(ID, Function) \
		Edif::SDK->ExpressionFunctions[ID] = Edif::MemberFunctionPointer(&Extension::Function);
#endif

struct RUNDATA; // ghost define; we never dereference RUNDATA, just RunObject
struct EDITDATA;
struct ACEInfo;
namespace Edif
{
	// New access properties
	namespace Properties
	{
		// Synced with Names
		enum IDs {
			PROPTYPE_STATIC = 1,		// Simple static text
			PROPTYPE_FOLDER,			// Folder
			PROPTYPE_FOLDER_END,		// Folder End
			PROPTYPE_EDITBUTTON,		// Edit button, param1 = button text, or nullptr if Edit
			PROPTYPE_EDIT_STRING,		// Edit box for strings, parameter = max length
			PROPTYPE_EDIT_NUMBER,		// Edit box for numbers
			PROPTYPE_COMBOBOX,			// Combo box, parameters = list of strings, options (sorted, etc)
			PROPTYPE_SIZE,				// Size
			PROPTYPE_COLOR,				// Color
			PROPTYPE_LEFTCHECKBOX,		// Checkbox
			PROPTYPE_SLIDEREDIT,		// Edit + Slider, parameters = min value, max value
			PROPTYPE_SPINEDIT,			// Edit + Spin, parameters = min value, max value (min must be >= -32768, max <= 32767, and (max - min) <= 32767
			PROPTYPE_DIRCTRL,			// Direction Selector
			PROPTYPE_GROUP,				// Group
			PROPTYPE_LISTBTN,			// !Internal, do not use
			PROPTYPE_FILENAME,			// Edit box + browse file button, parameter = FilenameCreateParam
			PROPTYPE_FONT,				// Font dialog box
			PROPTYPE_CUSTOM,			// Custom property
			PROPTYPE_PICTUREFILENAME,	// Edit box + browse image file button
			PROPTYPE_COMBOBOXBTN,		// Combo box, parameters = list of strings, options (sorted, etc)
			PROPTYPE_EDIT_FLOAT,		// Edit box for floating point numbers, parameters = min value, max value, options
			PROPTYPE_EDIT_MULTILINE,	// Edit box for multiline texts, no parameter
			PROPTYPE_IMAGELIST,			// Image list
			PROPTYPE_ICONCOMBOBOX,		// Combo box with icons
			PROPTYPE_URLBUTTON,			// URL button
			PROPTYPE_DIRECTORYNAME,		// Directory pathname
			PROPTYPE_SPINEDITFLOAT,		// Edit + Spin, value = floating point number
			// For searches
			PROPTYPE_FIRST_ITEM = PROPTYPE_STATIC,
			PROPTYPE_LAST_ITEM = PROPTYPE_SPINEDITFLOAT,
		};

		// Synced with PropertyIDs
		static const char* Names[] = {
			"!Start item",		// IDs starts with 1
			"Text",
			"Folder",
			"FolderEnd",
			"Edit button",
			"Editbox String",
			"Editbox Number",
			"Combo Box",
			"Size",
			"Color",
			"Checkbox",
			"Edit slider",
			"Edit spin",
			"Edit direction",
			"Group",
			"!List button",			// Internal, do not use
			"Editbox File",
			"Font",
			"Custom",
			"Editbox Image File",
			"Combo Box Button",
			"Editbox Float",
			"Editbox String Multiline",
			"Image List",
			"Combo Box Icons",
			"URL button",
			"Editbox Folder",
			"Edit spin float",
		};
	};
	class SDK
	{
	public:

		json_value &json;

		SDK (mv * mV, json_value &);
		~SDK ();

		std::vector<ACEInfo *>	ActionInfos;
		std::vector<ACEInfo *>	ConditionInfos;
		std::vector<ACEInfo *>	ExpressionInfos;

		void ** ActionJumps;
		void ** ConditionJumps;
		void ** ExpressionJumps;

		std::vector<void *> ActionFunctions;
		std::vector<void *> ConditionFunctions;
		std::vector<void *> ExpressionFunctions;

		mv* mV;
		// A fnv1a hash of all changeable property names and types, all separated by pipe. Used for property upgrades.
		std::uint32_t jsonPropsNameAndTypesHash;
		// A fnv1a hash of all changeable property types, separated by pipe. Used for property upgrades.
		std::uint32_t jsonPropsTypesHash;
#if EditorBuild
		cSurface * Icon;
		std::unique_ptr<PropData[]> EdittimeProperties;
#endif
	};

	class Runtime
	{
	protected:
		friend RunHeader;

		HeaderObject* hoPtr;
#ifdef __ANDROID__
		friend ConditionOrActionManager_Android;
		friend ExpressionManager_Android;
		global<jobject> javaExtPtr;
		global<jclass> javaExtPtrClass;
		global<jobject> javaHoObject;
		global<jclass> javaHoClass;
		global<jclass> javaCEventClass;
		global<jobject> javaRhObject;
		global<jclass> javaCRunClass;
#elif defined(__APPLE__)
		void * objCExtPtr;
#endif

	public:
		long param1, param2;

#ifdef _WIN32
		Runtime(Extension * ext);
		EventParam* ParamZero;
#elif defined(__ANDROID__)
		Runtime(Extension* ext, jobject javaExtPtr);
		global<jobject> curCEvent;
		jobject curAct = nullptr;
		void InvalidateByNewACE();

		// Attaches current thread, and gets JNIEnv for it; errors are fatal
		static JNIEnv * AttachJVMAccessForThisThread(const char * threadName, bool asDaemon = false);
		static void DetachJVMAccessForThisThread();
		// Gets JNIEnv * for this thread, or null.
		static JNIEnv * GetJNIEnvForThisThread();
#else
		Runtime(Extension* ext, void * const objCExtPtr);
		void * curCEvent;
#endif

		~Runtime();

		void Rehandle();

		// Immediately creates an event, calling that condition ID in this object, and invalidating object selection.
		void GenerateEvent(int EventID);
		// Queues an event to run at the end of the actions, which will ccall this condition ID.
		void PushEvent(int EventID);

		void * Allocate(std::size_t);
		TCHAR * CopyString(const TCHAR *);
		char * CopyStringEx(const char *);
		wchar_t * CopyStringEx(const wchar_t *);

		void Pause();
		void Resume();

		void Redisplay();
		void Redraw();

		RunObjectMultiPlatPtr RunObjPtrFromFixed(int fixedValue);
		int FixedFromRunObjPtr(RunObjectMultiPlatPtr object);

		// For Object action parameters. Returns the object/qualifier OI used in the events; only necessary if you are looping the instances yourself.
		// @remarks This works for conditions too, but it should be unnecessary, as they're passed this OI directly.
		short GetOIFromObjectParam(std::size_t paramIndex);

		// For Object action parameters. Cancels other selected instances of the OI being looped through by Fusion runtime.
		// Only necessary if you are looping the instances yourself, or doing a singleton pattern.
		void CancelRepeatingObjectAction();

		// Returns current Fusion frame number, 1+.
		// @remarks shorthand for rhPtr->App->nCurrentFrame
		int GetCurrentFusionFrameNumber();

		void SetPosition(int X, int Y);
#ifdef _WIN32
		CallTables * GetCallTables();
#endif
		void CallMovement(int ID, long Parameter);

		void Destroy();

		void GetApplicationDrive(TCHAR * Buffer);
		void GetApplicationDirectory(TCHAR * Buffer);
		void GetApplicationPath(TCHAR * Buffer);
		void GetApplicationName(TCHAR * Buffer);
		void GetApplicationTempPath(TCHAR * Buffer);

#ifdef _WIN32
		void ExecuteProgram(ParamProgram * Program);

		long EditInteger(EditDebugInfo *);
		long EditText(EditDebugInfo *);

		event2 &CurrentEvent();
		bool IsHWACapableRuntime();
		SurfaceDriver GetAppDisplayMode();
#endif

		bool IsUnicode();

		DarkEdif::ObjectSelection ObjectSelection;

		void WriteGlobal(const TCHAR * name, void * Value);
		void * ReadGlobal(const TCHAR * name);
	};

	extern bool ExternalJSON;
#ifdef _WIN32
	void GetSiblingPath (TCHAR * Buffer, const TCHAR * Extension);
#endif

	const int DependencyNotFound	 = 0;
	const int DependencyWasFile	  = 1;
	const int DependencyWasResource  = 2;

	int GetDependency (char *& Buffer, size_t &size, const TCHAR * FileExtension, int Resource);

	TCHAR * ConvertString(const char* utf8String);
	TCHAR * ConvertAndCopyString(TCHAR* tstr, const char* utf8String, int maxLength);
	inline void FreeString(TCHAR* s)
	{
		free(s);
	}

	extern TCHAR LanguageCode[3];
	extern bool IsEdittime;

	int Init(mv * mV, bool fusionStartupScreen);
	void Init(mv * mV, EDITDATA * edPtr);

	void Free(mv * mV);
	void Free(EDITDATA * edPtr);

#ifdef _WIN32
#if EditorBuild
	extern HMENU ActionMenu, ConditionMenu, ExpressionMenu;

	HMENU LoadMenuJSON(int BaseID, const json_value& Source, HMENU Parent = 0);
#endif

	// These jump the Fusion runtime call into the right Extension call
	long FusionAPI ConditionJump(RUNDATA * rdPtr, long param1, long param2);
	short FusionAPI ActionJump(RUNDATA * rdPtr, long param1, long param2);
	long FusionAPI ExpressionJump(RUNDATA * rdPtr, long param);
#endif

	inline int ActionID(int ID) {
		return 25000 + ID;
	}

	inline int ConditionID(int ID) {
		return 26000 + ID;
	}

	inline int ExpressionID(int ID) {
		return 27000 + ID;
	}

	template<class T> inline void * MemberFunctionPointer(T Function)
	{
		T _Function = Function;
		return *(void **) &_Function;
	}

	[[deprecated("Use DarkEdif::GetMFXRelativeFolder()")]]
	std::string CurrentFolder();
	[[deprecated("Use PROJECT_NAME define")]]
	void GetExtensionName(char * const writeTo);

	class recursive_mutex {
#ifdef _DEBUG
		std::recursive_timed_mutex intern;
#define edif_lock_debugParams const char * file, const char * func, int line
#define edif_lock_debugParamDefs __FILE__, __FUNCTION__, __LINE__
		std::stringstream log;
#else
		std::recursive_mutex intern;
#define edif_lock_debugParams /* none */
#define edif_lock_debugParamDefs /* none */
#endif
	public:
		recursive_mutex();
		~recursive_mutex();

		// Don't use these directly! Use lock.edif_lock(), lock.edif_unlock(), etc.
		void lock(edif_lock_debugParams);
		bool try_lock(edif_lock_debugParams);
		void unlock(edif_lock_debugParams);

#define edif_lock() lock(edif_lock_debugParamDefs)
#define edif_try_lock() try_lock(edif_lock_debugParamDefs)
#define edif_unlock() unlock(edif_lock_debugParamDefs)
	};

};

namespace Edif {
	class SDK;
	extern SDK* SDK;
}
