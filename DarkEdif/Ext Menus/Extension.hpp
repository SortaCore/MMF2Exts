#pragma once
#include <sstream>
class Extension
{
public:
	#ifdef MULTI_THREADING
		SaveExtInfo ThreadData; // Must be first variable in Extension class
		std::vector<SaveExtInfo *> Saved;
		SaveExtInfo & AddEvent(int Event, bool UseLastData = false);
		void NewEvent(SaveExtInfo *);
		// TODO: Threadsafe with a CRITICAL_SECTION
	#endif

	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr; // you should not need to access this

	Edif::Runtime Runtime;

	static const int MinimumBuild = 254;
	static const int Version = 2;

	static const OEFLAGS OEFLAGS = OEFLAGS::VALUES;
	static const OEPREFS OEPREFS = OEPREFS::NONE;

	static const int WindowProcPriority = 100;

	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
	~Extension();


	/*  Add any data you want to store in your extension to this class
		(eg. what you'd normally store in rdPtr).

		For those using multi-threading, any variables that are modified
		by the threads should be set in ExtVariables.
		See MultiThreading.h.

		Unlike rdPtr, you can store real C++ objects with constructors
		and destructors, without having to call them manually or store
		a pointer.
	*/


	std::tstring filename;
	std::tstringstream error;
	std::tstring menuStr[3];



	/*  Add your actions, conditions and expressions as real class member
		functions here. The arguments (and return type for expressions) must
		match EXACTLY what you defined in the JSON.

		Remember to link the actions, conditions and expressions to their
		numeric IDs in the class constructor (Extension.cpp)
	*/

	/// Actions

		void SetMFXFile(const TCHAR * filename);

	/// Expressions

		const TCHAR * GetError();
		const TCHAR * GetActionMenu2();
		const TCHAR * GetConditionMenu2();
		const TCHAR * GetExpressionMenu2();

		const TCHAR * GetAllActionInfos();
		const TCHAR * GetAllConditionInfos();
		const TCHAR * GetAllExpressionInfos();



	/* These are called if there's no function linked to an ID */

	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);




	/*  These replace the functions like HandleRunObject that used to be
		implemented in Runtime.cpp. They work exactly the same, but they're
		inside the extension class.
	*/

	REFLAG Handle();
	REFLAG Display();

	short FusionRuntimePaused();
	short FusionRuntimeContinued();

	bool SaveFramePosition(HANDLE File);
	bool LoadFramePosition(HANDLE File);

};
