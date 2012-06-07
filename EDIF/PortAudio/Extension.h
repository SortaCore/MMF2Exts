
class Extension
{
public:
// Custom tokeniser
/*inline vector<tstring> ParseStringToVector(tstring source, tstring delimiters)
{
	vector <tstring> v;
	tstring s = _T("");
 
	for (unsigned int i=0; i<source.length(); i++)
	{
		tstring charat = _T("");
		charat.push_back(source[i]);
 
		if (charat.find_first_of(delimiters) != -1)
		{
			if (s.length() > 0)
				v.push_back(s);
			s.clear();
		}
 
		else
		{
			s.push_back(charat[0]);
		}
	}
 
	if (s.length() > 0)
		v.push_back(s);
 
	return v;
}


*/
	#define MsgBox(text) MessageBoxA(NULL, text, "RangeHandler Object - Debug information", MB_OK|MB_ICONINFORMATION)
	#define FatalBox()	MessageBoxA(NULL, "Fatal error has not been repaired; bypassing erroneous code.", "RangeHandler Object - Bypass notification", MB_OK|MB_ICONERROR);
    
	LPRDATA rdPtr;
    LPRH    rhPtr;

    Edif::Runtime Runtime;

    static const int MinimumBuild = 251;
    static const int Version = 1;

    static const int OEFLAGS = OEFLAG_VALUES;
    static const int OEPREFS = 0;
    
    static const int WindowProcPriority = 100;

    Extension(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr);
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

    /// Actions

        void InitialiseRecordingToMemory();
		void TestReportAndExplode();
		
	/// Conditions

		const bool OnError();
		const bool OnReport();

    /// Expressions
        
		tchar * GetError(int clear);
		tchar * GetReport(int clear);

	/// Unreferenced (not called by MMF2)

		void Unreferenced_Report(tchar * report, int ThreadID);
		void Unreferenced_Error(tchar * error, int ThreadID);
		
		string LastLockFile;
		int LastLockLine;
		volatile bool threadsafe;
		uchar NewThreadID;
		bool UsePopups;

    /* These are called if there's no function linked to an ID */

    void Action(int ID, LPRDATA rdPtr, long param1, long param2);
    long Condition(int ID, LPRDATA rdPtr, long param1, long param2);
    long Expression(int ID, LPRDATA rdPtr, long param);

	int CurrentVal;
	bool ReturnBool;
	bool RecordingAlreadyStarted;
	tstring LastError;
	tstring CompleteStatus;


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