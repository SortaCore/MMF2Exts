
class Extension
{
public:

    LPRDATA rdPtr;
    LPRH    rhPtr;

    Edif::Runtime Runtime;

    static const int MinimumBuild = 249;
	static const int Version = 2;

    static const int OEFLAGS = 0;
    static const int OEPREFS = 0;
    
    static const int WindowProcPriority = 100;

    Extension(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr);
    ~Extension();


    /*
        Add any data you want to store in your extension to this class
        (eg what you'd normally store in rdPtr)

        Unlike rdPtr, you can store real C++ objects with constructors
        and destructors, without having to call them manually or store
        a pointer

    */

    // int MyVariable;




    /*
        Add your actions, conditions and expressions as real class member
        functions here. The arguments (and return type for expressions) must
        match EXACTLY what you defined in the JSON.

        You miss an argument, you crash. Edif shows no mercy.

        Remember to link the actions, conditions and expressions to their
        numeric IDs in the class constructor (Extension.cpp)

     */

    /// Actions

        void SetRandomSeed(int Seed);
        void SetRandomSeedToTimer();

    /// Conditions

        bool Compare(float FirstNumber, float SecondNumber, int ComparisonType);

    /// Expressions
        
        float GenerateRandom(float Minimum, float Maximum);
        float Limit(float Value, float Minimum, float Maximum);
        float Nearest(float Value, float Minimum, float Maximum);
        float Normalise(float Value, float Minimum, float Maximum, int LimitRange);
        float ModifyRange(float Value, float Minimum, float Maximum, float NewMinimum, float NewMaximum, int LimitRange);
        float Wave(int Waveform, float Value, float CycleStart, float CycleEnd, float Minimum, float Maximum);
		float EuclideanMod(float Dividend, float Divisor);
		float UberMod(float Dividend, float Lower, float Upper);
		float Interpolate(float Value, float From, float To, int LimitRange);
		float Mirror(float Value, float From, float To);
		float ExpressionCompare(float First, float Second, int ComparisonType, float ReturnIfTrue, float ReturnIfFalse);
		float Approach(float Value, float Amount, float Target);
		int IntGenerateRandom(float Minimum, float Maximum);
		int IntLimit(float Value, float Minimum, float Maximum);
		int IntNearest(float Value, float Minimum, float Maximum);
        int IntNormalise(float Value, float Minimum, float Maximum, int LimitRange);
        int IntModifyRange(float Value, float Minimum, float Maximum, float NewMinimum, float NewMaximum, int LimitRange);
        int IntWave(int Waveform, float Value, float CycleStart, float CycleEnd, float Minimum, float Maximum);
		int IntEuclideanMod(float Dividend, float Divisor);
		int IntUberMod(float Dividend, float Lower, float Upper);
		int IntInterpolate(float Value, float From, float To, int LimitRange);
		int IntMirror(float Value, float From, float To);
		int IntExpressionCompare(float First, float Second, int ComparisonType, float ReturnIfTrue, float ReturnIfFalse);
		int IntApproach(float Value, float Amount, float Target);
		const char * Substr(const char * String, int Start, int Length);
		const char * StrExpressionCompare(float First, float Second, int ComparisonType, const char * ReturnIfTrue, const char * ReturnIfFalse);


    // These are called if there's no function linked to an ID

    void Action(int ID, LPRDATA rdPtr, long param1, long param2);
    long Condition(int ID, LPRDATA rdPtr, long param1, long param2);
    long Expression(int ID, LPRDATA rdPtr, long param);




    /*
        These replace the functions like HandleRunObject that used to be
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