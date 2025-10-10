#pragma once
#include "DarkEdif.hpp"

class Extension final
{
public:
	// ======================================
	// Required variables + functions
	// Variables here must not be moved or swapped around or it can cause future issues
	// ======================================
	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr;
#ifdef __ANDROID__
	global<jobject> javaExtPtr;
#elif defined(__APPLE__)
	void* const objCExtPtr;
#endif

	Edif::Runtime Runtime;

	static const int MinimumBuild = 254;
	static const int Version = 1;

	// Warning: OEFLAGS/OEPREFS cannot be freely modified when you have used them in MFAs.
	static constexpr OEFLAGS OEFLAGS = OEFLAGS::NONE;
	static constexpr OEPREFS OEPREFS = OEPREFS::NONE;
	// If OEFLAGS::WINDOW_PROC (otherwise you can delete)
	// static constexpr int WindowProcPriority = 100;
	// If OEFLAGS::TEXT (otherwise you can delete)
	// static constexpr TextCapacity TextCapacity = TextCapacity::None;

#ifdef _WIN32
	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
#elif defined(__ANDROID__)
	Extension(const EDITDATA* const edPtr, const jobject javaExtPtr, const CreateObjectInfo* const cobPtr);
#else
	Extension(const EDITDATA* const edPtr, void* const objCExtPtr, const CreateObjectInfo* const cobPtr);
#endif
	~Extension();

	// ======================================
	// Extension data
	// ======================================

	// To add items to the Fusion Debugger, just uncomment this line.
	DarkEdif::FusionDebugger FusionDebugger;
	// After enabling it, you run FusionDebugger.AddItemToDebugger() inside Extension's constructor
	// As an example:
	std::tstring exampleDebuggerTextItem;


	/*  Add any data you want to store in your extension to this class
		(eg. what you'd normally store in rdPtr in old SDKs).

		Unlike rdPtr, you can store real C++ objects with constructors
		and destructors, without having to call them manually or store
		a pointer.
	*/

	// int MyVariable;


	/*  Add your actions, conditions and expressions as real class member
		functions here. The arguments (and return type for expressions) must
		match EXACTLY what you defined in the JSON.

		Remember to link the actions, conditions and expressions to their
		numeric IDs in the class constructor (Extension.cpp)
	*/

	// Actions

	void ActionExample(int ExampleParameter);
	void SecondActionExample();

	// Conditions

	bool AreTwoNumbersEqual(int FirstNumber, int SecondNumber);

	// Expressions

	int Add(int FirstNumber, int SecondNumber);
	const TCHAR * HelloWorld();

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

#if TEXT_OEFLAG_EXTENSION
	// Extension text struct. Required for text exts.
	DarkEdif::FontInfoMultiPlat font;
	void OnFontChanged(bool colorEdit, DarkEdif::Rect* rc);
#endif
#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
	// Extension display surface ptr. Required for simple display exts.
	DarkEdif::Surface * surf = nullptr;
#elif DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_MANUAL
	void Display();
	void GetZoneInfos();
	DarkEdif::Surface * GetDisplaySurface();
	DarkEdif::CollisionMask * GetCollisionMask(std::uint32_t flags);
#endif

	// These are called if there's no function linked to an ID
	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);

#if PAUSABLE_EXTENSION
	// Called when Fusion runtime is pausing - not just the F3 pause dialog
	void FusionRuntimePaused();
	// Called when Fusion runtime is resuming after a pause
	void FusionRuntimeContinued();
#endif // PAUSABLE_EXTENSION
};
