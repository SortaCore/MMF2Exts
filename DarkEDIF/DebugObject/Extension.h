
class Extension
{
public:

    RUNDATA *	rdPtr;
    RunHeader *	rhPtr;

    Edif::Runtime Runtime;

	GlobalData * Data;
    static const int MinimumBuild = 256;
    static const int Version = 8;
	// v8: Fixed non-closing console when console was disabled.
	// v7: Fixed subapp "MFX Not Found" issue in DarkEDIF
	// v6: Release for edittime property fix, added runtime property, new minidump events
	// v5: Release for bug fixes and global settings, and unhandled exception event
	// v4: First release with console
	// v3: Release due to bug fixes
	// v2: Release due to new features
	// v1: First release
	static const int OEFLAGS = OEFLAGS::NEVER_KILL | OEFLAGS::NEVER_SLEEP | OEFLAGS::RUN_BEFORE_FADE_IN;
	static const int OEPREFS = OEPREFS::GLOBAL;
    
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
		void OutputNow(int Intensity, int Line, const char * TextToOutput);

    /// Actions
        void SetOutputFile(char * FileP, int DescribeAppI);
        void Output(int Intensity, int Line, const char * TextToOutput);
		void SetOutputTimeFormat(char * Format);
		void SetOutputOnOff(int OnOff);
		void SetHandler(int Reaction, int ContinuesCount);
		void CauseCrash_ZeroDivisionInt();
		void CauseCrash_ZeroDivisionFloat();
		void CauseCrash_WriteAccessViolation();
		void CauseCrash_ReadAccessViolation();
		void CauseCrash_ArrayOutOfBoundsRead();
		void CauseCrash_ArrayOutOfBoundsWrite();
		void SetConsoleOnOff(int OnOff);
		void SetDumpFile(const char * path, int flags);

	/// Conditions
		bool OnSpecificConsoleInput(char * Command);
		bool OnAnyConsoleInput();
		bool OnUnhandledException();
		bool OnCtrlCEvent();
		bool OnCtrlBreakEvent();
		bool OnConsoleCloseEvent();

	/// Expressions
		const char * FullCommand();
		const char * CommandMinusName();

		void LoadDataVariable();

    /* These are called if there's no function linked to an ID */

    void Action(int ID, RUNDATA * rdPtr, long param1, long param2);
    long Condition(int ID, RUNDATA * rdPtr, long param1, long param2);
    long Expression(int ID, RUNDATA * rdPtr, long param);




    /*  These replace the functions like HandleRunObject that used to be
        implemented in Runtime.cpp. They work exactly the same, but they're
        inside the extension class.
    */

    short Handle();
    short Display();

    short Pause();
    short Continue();

    bool Save(HANDLE File);
    bool Load(HANDLE File);

};