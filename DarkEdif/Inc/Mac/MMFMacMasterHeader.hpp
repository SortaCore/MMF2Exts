#pragma once

// This file is a duplicate of iOSMasterHeader, with the main difference being its location,
// altering its #include/#import search preferences.
// 
// On Mac, we want to read the Fusion internal headers from MMF2Lib.framework in DarkEdif/Lib/Mac,
// and on iOS, we want to read them from DarkEdif/Inc/iOS/MMF2Lib folder.
// As #include prefers the current folder, using the Inc/iOS header would cause iOS/MMF2Lib to be preferred.

#if defined(_WIN32) || defined (__ANDROID__) || !defined(__APPLE__) || MacBuild==0
	#error Included the wrong header for this OS.
#endif

// All C/C++ files are expected to be built with Objective-C or Objective-C++ mode. 
// This should be handled by FusionSDK.props, but for specific files, check
// project/file property C/C++ > Advanced > Compile As.
// It should be noted that IntelliSense does not realise they are coded for Objective-C, and will get upset.
#if !defined(__OBJC__) && !defined(__INTELLISENSE__)
	#error Objective-C compatibility required!
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

// Beware that these MMF2Lib files are organised differently.
// In Mac, there's a framework with Headers folder, and Apple Clang has a Darwin GCC feature to ignore the .framework/Headers part of the path.
// In iOS, MMF2Lib headers are just headers copied from a generated xcodeproj, so there is no MMF2Lib.framework we're linking to;
// instead we only declare things, and let the Fusion-generated iOS xcodeproj link things later when building.
//
// It should also be noted that IntelliSense does not recognise Objective-C, or the #import directive. All headers were modified to add #pragma once,
// which is protection #import has by design.
#ifdef __OBJC__
#import <Foundation/Foundation.h>

#import "MMF2Lib/CObjInfo.h"
#import "MMF2Lib/CQualToOiList.h"
#import "MMF2Lib/CRunExtension.h"
#import "MMF2Lib/CActExtension.h"
#import "MMF2Lib/CCndExtension.h"
@class CRunApp;
#elif defined(__INTELLISENSE__)
// These are dummies so the VS IntelliSense is happy; the actual compiler won't read this,
// and compiled files will never use these
class CRunApp {};
class tagEVT {};
class CObject {};
class tagEVG {};
class CEventProgram { public: tagEVG * rhEventGroup; };
class CRun { public: CEventProgram* rhEvtProg; };
class CActExtension { public: class eventParam* pParams[]; };
class CCndExtension {};
#endif

// Used to make the DLL function be visible externally, and in C function signature, not C++
#define ProjectFunc extern "C" __attribute__((visibility ("default")))
// Used for __stdcall in Windows, useless here
#define FusionAPI /* no declarator */
#define DLLExport __attribute__ ((stdcall))
// Used for declspec(dllimport), useless here
#define PhiDLLImport

// Surface.h pre-declaration
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
struct mv;

namespace Edif {
	Params ReadActionOrConditionParameterType(const char*, bool&);
	ExpParams ReadExpressionParameterType(const char*, bool&);
	ExpReturnType ReadExpressionReturnType(const char* text);
}

// This namespace is #defined to different per ext, preventing multiple of the iOS/Mac DarkEdif exts
// exported functions in their static library builds from conflicting when the full iOS/Mac app is built.
inline namespace FusionInternals
{
	struct HeaderObject;
	struct RunObject;
	struct RunHeader;
	// Runtime data for object - it is a std::shared_ptr on Android, beware!
	typedef RunObject* RunObjectMultiPlatPtr;

	struct objectsList {
		NO_DEFAULT_CTORS_OR_DTORS(objectsList);
		RunObjectMultiPlatPtr GetOblOffsetByIndex(std::size_t);
	};
	struct objInfoList {
		NO_DEFAULT_CTORS_OR_DTORS(objInfoList);
		int get_EventCount() const;
		int get_EventCountOR() const;
		short get_ListSelected() const;
		int get_NumOfSelected() const;
		short get_Oi() const;
		int get_NObjects() const;
		short get_Object() const;
		const TCHAR* get_name();
		void set_NumOfSelected(int);
		void set_ListSelected(short);
		void set_EventCount(int);
		void set_EventCountOR(int);
		short get_QualifierByIndex(std::size_t) const;

		void SelectNone(RunHeader* rhPtr);
		void SelectAll(RunHeader* rhPtr, bool explicitAll = false);
	};
	struct qualToOi {
		NO_DEFAULT_CTORS_OR_DTORS(qualToOi);
		short get_Oi(std::size_t idx);
		short get_OiList(std::size_t idx);
		// Returns all OiList from internal array, used for looping through a qualifier's object IDs
		std::vector<short> GetAllOi();
		// Returns all OiList from internal array, used for looping through a qualifier's objInfoList
		std::vector<short> GetAllOiList();
	private:
		std::vector<short> HalfVector(std::size_t first);
	};

	// Differs from eventV1 by union variable types
	struct event2 {
		NO_DEFAULT_CTORS_OR_DTORS(event2);
		short get_evtNum();
		OINUM get_evtOi();
		short get_evtSize();
		std::int8_t get_evtFlags();
		void set_evtFlags(std::int8_t);

		event2* Next();
		int GetIndex();
	};
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

	struct eventGroup {
		NO_DEFAULT_CTORS_OR_DTORS(eventGroup);

		std::uint8_t get_evgNCond() const;
		std::uint8_t get_evgNAct() const;
		std::uint16_t get_evgInhibit() const;
		std::uint16_t get_evgIdentifier() const;
		EventGroupFlags get_evgFlags() const;
		event2 * GetCAByIndex(std::size_t index);
	};

	struct CRunFrameMultiPlat {

	};
	struct CRunAppMultiPlat {
		NO_DEFAULT_CTORS(CRunAppMultiPlat);
		int get_nCurrentFrame();
		CRunFrameMultiPlat* get_Frame();
		CRunAppMultiPlat* get_ParentApp();
		std::size_t GetNumFusionFrames();
	};

	struct RunHeader {
		NO_DEFAULT_CTORS_OR_DTORS(RunHeader);
		objectsList* get_ObjectList();

		qualToOi* GetQualToOiListByOffset(std::size_t byteOffset);
		RunObjectMultiPlatPtr GetObjectListOblOffsetByIndex(std::size_t index);
		// Reads the EventCount variable from RunHeader2, used in object selection. DarkEdif-added function for cross-platform.
		int GetRH2EventCount();
		// Gets the EventCountOR, used in object selection in OR-related events. DarkEdif-added function for cross-platform.
		int GetRH4EventCountOR();
		objInfoList* GetOIListByIndex(std::size_t index);
		short GetOIListIndexFromOi(const short oi);
		std::size_t GetNumberOi();
		std::size_t get_MaxObjects();
		std::size_t get_NObjects();
		event2* GetRH4ActionStart();
		EventGroupFlags GetEVGFlags();
		eventGroup* get_EventGroup();
		CRunAppMultiPlat* get_App();
		//CEventProgram* get_EventProgram();
	};

	struct HeaderObject {
		NO_DEFAULT_CTORS_OR_DTORS(HeaderObject);

		void set_NextSelected(short);
		void set_SelectedInOR(bool);
		short get_NextSelected();
		short get_Number();
		short get_NumNext();
		short get_Oi();
		objInfoList* get_OiList();
		unsigned short get_CreationId();
		RunHeader* get_AdRunHeader();
		bool get_SelectedInOR();
		HeaderObjectFlags get_Flags();

		int GetFixedValue();
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
		CValueMultiPlat(unsigned int type, long memory);
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
		NO_DEFAULT_CTORS_OR_DTORS(RunObject);
		HeaderObject* get_rHo();
		rCom* get_roc();
		rMvt* get_rom();
		rAni* get_roa();
		Sprite* get_ros();
		AltVals* get_rov();
	};
} // namespace FusionInternals

// Defined in DarkEdif.cpp with ASM instructions to embed the binary.
//extern char darkExtJSON[];
//extern unsigned int darkExtJSONSize;
#define PROJ_FUNC_GEN2(x,y) x##y
#define PROJ_FUNC_GEN(x,y) PROJ_FUNC_GEN2(x,y)

ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _init());
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _dealloc());
ProjectFunc int PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW,_getNumberOfConditions());
ProjectFunc long PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW,_condition(void * cppExtPtr, int ID, void * paramReader));
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW,_action(void * cppExtPtr, int ID, void * paramReader));
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW,_expression(void * cppExtPtr, int ID));
ProjectFunc void * PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW,_createRunObject(void * file, int cob, int version, void * objCExtPtr));
ProjectFunc short PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW,_handleRunObject(void * cppExtPtr));
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW,_destroyRunObject(void * cppExtPtr, bool bFast));

#define DarkEdifObjCFunc2(a,b) DarkEdif_##a##_##b
#define DarkEdifObjCFunc(a,b) DarkEdifObjCFunc2(a,b)

// Stolen from IncBin GitHub: https://github.com/graphitemaster/incbin
// Edited to remove g prefix.
#ifndef INCBIN_HDR
#define INCBIN_HDR
#include <limits.h>

#if defined(__SSE__) || defined(__neon__)
# define INCBIN_ALIGNMENT 16
#else
# if ULONG_MAX == 0xffffffffu
#  define INCBIN_ALIGNMENT 4
# else
#  define INCBIN_ALIGNMENT 8
# endif
#endif

#define INCBIN_ALIGN __attribute__((aligned(INCBIN_ALIGNMENT)))

#ifdef __cplusplus
#  define INCBIN_EXTERNAL extern "C"
#else
#  define INCBIN_EXTERNAL extern
#endif

#define INCBIN_STR(X) #X
#define INCBIN_STRINGIZE(X) INCBIN_STR(X)

#define INCBIN_EXTERN(NAME1, NAME2) \
	INCBIN_EXTERNAL const INCBIN_ALIGN unsigned char NAME1 ## NAME2[]; \
	INCBIN_EXTERNAL const unsigned int NAME1 ## NAME2 ## Size

#define INCBIN(NAME1, NAME2, FILENAME) INCBIN2(NAME1, NAME2, FILENAME)

#define INCBIN2(NAME1, NAME2, FILENAME) \
	__asm__(".const_data\n" \
			".globl _" #NAME1 #NAME2 "\n"	  \
			".align " INCBIN_STRINGIZE(INCBIN_ALIGNMENT) "\n" \
			"_" #NAME1 #NAME2 ":\n" \
				".incbin \"" FILENAME "\"\n" \
			".globl _" #NAME1 #NAME2 "Size\n"	  \
			".align " INCBIN_STRINGIZE(INCBIN_ALIGNMENT) "\n" \
			"_" #NAME1 #NAME2 "Size:\n" \
			".long _" #NAME1 #NAME2 "Size - _" #NAME1 #NAME2 "\n" \
	); \
	INCBIN_EXTERN(NAME1, NAME2)
#define INCBIN_EXTERN2(a, b) INCBIN_EXTERN(a, b);
INCBIN_EXTERN2(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _darkExtJSON);
#endif

// Undo warnings
#pragma clang diagnostic pop
