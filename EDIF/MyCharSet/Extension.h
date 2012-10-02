class Extension
{
public:

	#define MsgBox(text) MessageBoxA(NULL, text, "MyCharSet Object - Debug information", MB_OK|MB_ICONINFORMATION)
	#define FatalBox()	MessageBoxA(NULL, "Fatal error has not been repaired; bypassing erroneous code.", "MyCharSet Object - Bypass notification", MB_OK|MB_ICONERROR);

    LPRDATA rdPtr;
    LPRH    rhPtr;

    Edif::Runtime Runtime;

    static const int MinimumBuild = 252;
    static const int Version = 1;

    static const int OEFLAGS = 0;
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
		std::wstring WideString;		// Unicode string
		std::string ThinString;		// ASCII string
		long long locale;		// Current locale?
		
    /*  Add your actions, conditions and expressions as real class member
        functions here. The arguments (and return type for expressions) must
        match EXACTLY what you defined in the JSON.

        Remember to link the actions, conditions and expressions to their
        numeric IDs in the class constructor (Extension.cpp)
    */

		/// Actions
		void SetBOMMarkASC(const char * FileToAddTo, int TypeOfBOM, int IgnoreCurrentBOM);
        
	/// Conditions

		bool IsUnicode();

    /// Expressions
        
	char * GetASCIIStringFromUnicodeString(wchar_t * Unicode, int Size, int ASCIIOrUTF8);
	wchar_t * GetUnicodeStringFromASCIIString(char * ASCII, int Size, int ASCIIOrUTF8);
	char * GetASCIIStringFromUnicodeMemory(int Address, int Size, int ASCIIOrUTF8);
	wchar_t * GetUnicodeStringFromASCIIMemory(int Address, int Size, int ASCIIOrUTF8);

	size_t GetASCIIMemoryFromUnicodeString(wchar_t * Unicode, int Size, int ASCIIOrUTF8);
	size_t GetUnicodeMemoryFromASCIIString(char * ASCII, int Size, int ASCIIOrUTF8);
	size_t GetASCIIMemoryFromUnicodeMemory(int Address, int Size, int ASCIIOrUTF8);
	size_t GetUnicodeMemoryFromASCIIMemory(int Address, int Size, int ASCIIOrUTF8);
	
	/* These are called if there's no function linked to an ID */

    void Action(int ID, LPRDATA rdPtr, long param1, long param2);
    long Condition(int ID, LPRDATA rdPtr, long param1, long param2);
    long Expression(int ID, LPRDATA rdPtr, long param);

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