
class Extension
{
public:

	RUNDATA *	rdPtr;
	RunHeader *	rhPtr;

	Edif::Runtime Runtime;

	GlobalData * data;
	static const int MinimumBuild = 256;
	static const int Version = 13;
	// b13: Fixed message box about properties failing to convert
	// b12: Fixed use of tcsdup in expressions, upgrade to SDK v5
	// b11: Fixed minidump handle not being passed
	// b10: Use of DarkEdif SDK properties, formatting changes
	// b9: Fixed lockup when disabling console and debug was already disabled
	// b8: Fixed non-closing console when console was disabled
	// b7: Fixed subapp "MFX Not Found" issue in DarkEdif
	// b6: Release for edittime property fix, added runtime property, new minidump events
	// b5: Release for bug fixes and global settings, and unhandled exception event
	// b4: First release with console
	// b3: Release due to bug fixes
	// b2: Release due to new features
	// b1: First release
	static const OEFLAGS OEFLAGS = OEFLAGS::NEVER_KILL | OEFLAGS::NEVER_SLEEP | OEFLAGS::RUN_BEFORE_FADE_IN;
	static const OEPREFS OEPREFS = OEPREFS::GLOBAL;
	
	static const int WindowProcPriority = 100;

	Extension(RUNDATA * rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr);
	~Extension();


	/*  Add any data you want to store in your extension to this class
		(eg. what you'd normally store in rdPtr)

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
	/// Hidden
		void OutputNow(int intensity, int line, std::string textToOutputU8);

	/// Actions
		void SetOutputFile(const TCHAR * fileP, int describeAppI);
		void Output(int intensity, int line, const TCHAR * textToOutput);
		void SetOutputTimeFormat(TCHAR * format);
		void SetOutputOnOff(int onOff);
		void SetHandler(int reaction, int continuesRemaining);
		void CauseCrash_ZeroDivisionInt();
		void CauseCrash_ZeroDivisionFloat();
		void CauseCrash_WriteAccessViolation();
		void CauseCrash_ReadAccessViolation();
		void CauseCrash_ArrayOutOfBoundsRead();
		void CauseCrash_ArrayOutOfBoundsWrite();
		void SetConsoleOnOff(int onOff);
		void SetDumpFile(const TCHAR * path, int flags);

	/// Conditions
		const bool AlwaysTrue() const;
		bool OnSpecificConsoleInput(TCHAR * command);

	/// Expressions
		const TCHAR * FullCommand();
		const TCHAR * CommandMinusName();

		void LoadDataVariable();

	/* These are called if there's no function linked to an ID */

	void Action(int ID, RUNDATA * rdPtr, long param1, long param2);
	long Condition(int ID, RUNDATA * rdPtr, long param1, long param2);
	long Expression(int ID, RUNDATA * rdPtr, long param);




	/*  These replace the functions like HandleRunObject that used to be
		implemented in Runtime.cpp. They work exactly the same, but they're
		inside the extension class.
	*/

	REFLAG Handle();
	REFLAG Display();

	short Pause();
	short Continue();

	bool Save(HANDLE File);
	bool Load(HANDLE File);

};
