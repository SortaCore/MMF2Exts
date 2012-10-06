

class Extension
{
public:

    RUNDATA *	rdPtr;
    RunHeader *	rhPtr;

    Edif::Runtime Runtime;

    static const int MinimumBuild = 254;
    static const int Version = 4;
	// v4: First release with console
	// v3: Release due to bug fixes
	// v2: Release due to new features
	// v1: First release
    static const int OEFLAGS = 0;
    static const int OEPREFS = 0;
    
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
		void CauseCrash_ZeroDivisionInt(void);
		void CauseCrash_ZeroDivisionFloat(void);
		void CauseCrash_WriteAccessViolation(void);
		void CauseCrash_ReadAccessViolation(void);
		void CauseCrash_ArrayOutOfBoundsRead(void);
		void CauseCrash_ArrayOutOfBoundsWrite(void);
		void SetConsoleOnOff(int OnOff);

	/// Conditions
		bool OnSpecificConsoleInput(char * Command);
		bool OnAnyConsoleInput(void);

	/// Expressions
		const char * FullCommand(void);
		const char * CommandMinusName(void);



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