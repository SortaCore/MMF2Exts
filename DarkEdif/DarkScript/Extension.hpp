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
	static const int Version = 4;

	static constexpr OEFLAGS OEFLAGS = OEFLAGS::VALUES;
	static constexpr OEPREFS OEPREFS = OEPREFS::NONE;

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

	DarkEdif::FusionDebugger FusionDebugger;

	// Fusion frame number; used for subapps
	int fusionFrameNum;

	// Object properties
	std::tstring globalID;

	// Manages discrepancies between supplied type and expected type
	enum class ConversionStrictness
	{
		// Requested type must match input type
		Exact,
		// Float and integers can be exchanged, text <-> number is errors
		Numeric,
		// Strings containing numbers can be converted to numbers and vice versa
		// If conversion fails, errors is used
		AnyWithAborts,
		// Strings containing numbers can be converted to numbers and vice versa
		// If conversion fails, default is used
		AnyWithDefaults
	};
	ConversionStrictness conversionStrictness;
	// Sets contraint where no return value set is allowed when function finishes
	enum class AllowNoReturnValue
	{
		// All functions must have a return value set when they return, even if they don't use them
		Never,
		// Allow foreach actions and delayed function calls, which don't use the func return directly
		// to not set a return value
		ForeachDelayedActionsOnly,
		// Allow foreach actions, delayed function calls, and anonymous functions
		AnonymousForeachDelayedActions,
		// Any function can forgo setting return value and use the defaults of "" or 0
		AllFunctions
	};
	// If funcsMustHaveTemplate is false, allows no return value set at all.
	AllowNoReturnValue whenAllowNoRVSet;

	// If true, a function can't be run without a pre-existing template
	bool funcsMustHaveTemplate;
	// If true, sends template list to new Fusion frames after frame switch.
	// Requires global ID of receiving object to match.
	bool keepTemplatesAcrossFrames;
	// If true, sends scoped vars that are global to new Fusion frames after frame switch.
	// Requires global ID of receiving object to match.
	bool keepGlobalScopedVarsAcrossFrames;
	// If true, creates error if no On Function event matches when a function is run
	// If false, ignores it.
	// On Any Function will also work.
	bool createErrorForUnhandledEvents;
	// If true, creates error if no On Function Aborted event matches when a function is run
	// If false, ignores it.
	// On Any Function Aborted will also work.
	bool createErrorForUnhandledAborts;
	// If true, enables On Any Function XX events.
	bool enableOnAnyFunctions;
	// If true, when hiding scoped variables by virtue of a lower level scoped var having same name
	// as a higher level, create an error. This is usually unintended behaviour.
	bool createErrorOverridingScopedVars;
	// If true, prevents any recursive function calls. Recursion is usually a bad design choice,
	// and 99% of recursive functions can be written without recursion.
	// A noteable exception is deleting a folder tree; but even those have system commands or Sphax File-Folder.
	bool preventAllRecursion;
	// If true, when a function runs on another ext, the aborts the function generates
	// are run on the destination ext.
	// If false, aborts/errors are run on the extension that starts the function (default).
	// Errors are usually the fault of the caller, some syntax error - so they'll be wherever is local.
	bool runAbortsOnDestination;
	// If true, allows a ForEach run on a qualifier to trigger for objects in qualifier, not just On Each qualifier.
	bool allowQualifierToTriggerSingularForeach;
	// If true, allows a ForEach run on an object to trigger On Each Qualifier for qualifiers containing that object.
	bool allowSingularToTriggerQualifierForeach;
	// If true, parents' parameters are passed as scoped variables, but read-only.
	bool inheritParametersAsScopedVariables;

	// Function settings passed by action -> expression

	// If more than 0, current function is being executed by delay action
	int lastDelaySetting = 0;
	// If true, all functions available are being triggered to analyse event numbers
	bool selfAwareness = false;
	// Per object list of selected instances
	struct FusionSelectedObjectListCache final
	{
		objInfoList* poil = nullptr;
		// List of object instance numbers
		std::vector<short> selectedObjects;

		// If running in a foreach, it's necessary to back up the action-level selection sometimes.
		// Otherwise we run the risk of objects not being selected in actions between a DS function
		// and later actions in the same event.

		// Should we backup OR-related selection? I think not, because conditions will alter them anyway...
		// maybe if DS function is in an OR event?
		// Should probably just flat out refuse to run a DS event that uses OR in the On Function

		// TODO: Investigate how ActionCount works and when it's gonna bork things.
	};

	enum class Type
	{
		// Any allows weakly-typed functions. Not sure if that'd ever be useful, but there ya go.
		Any = 0,
		Integer,
		String, // String must be before float, because ExpReturnType has it that way
		Float,

		// UPDATE THESE WHEN CHANGING THE ABOVE
		NumCallableReturnTypes = 3,
		MaskReturnTypes = 0x3,
		NumCallableParamTypes = 3,
		MaskParamTypes = 0x3
	};

	struct Value final
	{
		Type type;
		union
		{
			TCHAR* string;
			int integer;
			float decimal;
		} data;
		std::size_t dataSize;

		Value(Type type) : type(type), dataSize(0)
		{
			dataSize = 0;
			data.string = nullptr; // string is 64-bit, so it's bigger on some platforms
			//if (type == Type::Integer || type == Type::Float)
			//	dataSize = sizeof(int); // same as sizeof(float)
		}
		Value(const Value& v) : type(v.type), data(v.data), dataSize(v.dataSize)
		{
			if (type == Type::String && data.string)
				data.string = _tcsdup(data.string);
		}
		Value(Value&& v) noexcept : type(v.type), data(v.data), dataSize(v.dataSize)
		{
			v.type = Type::Any;
			v.data.string = NULL;
			v.dataSize = 0;
		}
		Value & operator = (const Value v) {
			data.string = v.data.string;
			dataSize = v.dataSize;
			type = v.type;
			if (type == Type::String && data.string)
				data.string = _tcsdup(v.data.string);
			return *this;
		}

		~Value()
		{
			if (type == Type::String)
				free(data.string);
		}
	};

	struct Param
	{
		std::tstring name;
		std::tstring nameL;
		// Type can be Any to allow any type of defaultVal; if defaultVal.type == Any, it is unset
		Type type;
		// Type as Any for no default
		Value defaultVal;

		Param(const std::tstring_view name, const Type typ) :
			name(name), nameL(ToLower(name)), type(typ), defaultVal(Type::Any)
		{
		}
	};
	struct ScopedVar final : Param
	{
		bool recursiveOverride;
		std::size_t level; // Indicates how deep in runningFunc vector this is set; 0 is global, 1 is first func layer, etc
		ScopedVar(const std::tstring_view name, Type typ, bool recursiveOverride, std::size_t level) :
			Param(name, typ), recursiveOverride(recursiveOverride), level(level)
		{
			// all done above
		}
	};

	enum class Expected : std::uint8_t
	{
		Never,
		Always,
		Either,
	};
	struct FunctionTemplate final
	{
		std::tstring name;
		std::tstring nameL;
		Expected repeating, delaying;
		bool recursiveAllowed;
		// if false, no events are generated, it returns default return value instantly to caller
		bool isEnabled = true;
		// return type can be Any, or match defaultReturnValue
		Type returnType;
		Value defaultReturnValue;
		std::vector<Param> params;
		std::vector<ScopedVar> scopedVarOnStart;
		// Extension to generate events on - blank if ext points to the globals that holds this template
		std::tstring globalID;
		Extension* ext = nullptr; // NULL is invalid! See GlobalInfo::updateTheseGlobalsWhenMyExtCycles
		// Function name to redirect to. Redirects do not combine.
		std::tstring redirectFunc;
		std::shared_ptr<FunctionTemplate> redirectFuncPtr;
		bool isAnonymous = false;

		FunctionTemplate(Extension* ext, const TCHAR * funcName, Expected delayable, Expected repeatable, bool recursable, Type returnType)
			: name(funcName), repeating(repeatable), delaying(delayable),
			recursiveAllowed(recursable), returnType(returnType), defaultReturnValue(Type::Any), ext(ext)
		{
			nameL = Extension::ToLower(name);
		}
	};
	struct RunningFunction final
	{
		std::shared_ptr<FunctionTemplate> funcTemplate;
		// If true, currently running; if false, waiting to be run, or aborting.
		bool active = false;
		// Current abort reason. Can't double-abort.
		std::tstring abortReason;
		// If true, an On Function Aborted that matched this func name ran
		bool abortWasHandled = false;

		// If true, an On Function that matched this func name ran
		bool eventWasHandled = false;

		// For all functions: If true, current iteration's On Function will be triggered.
		bool currentIterationTriggering = true;
		// For repeating functions: If true, next iteration of a repeating event will run.
		bool nextRepeatIterationTriggering = true;
		// For foreach functions: If false, following foreach object will not be looped through.
		bool foreachTriggering = true;

		// 0+: index of this function. Once index > numRepeats, function is removed.
		int index = 0;
		// 0 = no repeats, 1 run in total.
		int numRepeats = 0;

		// If true, is a K-type of function, keeping object selection.
		bool keepObjectSelection = false;
		// All selected objects from the starting function.
		// Note: in a foreach function, does NOT include the foreach object list.
		std::vector<FusionSelectedObjectListCache> selectedObjects;
		// Current object fixed value in a foreach
		int currentForeachObjFV = 0;
		// Current object OIL (including qualifier)
		short currentForeachOil = -1;
		// If true, return value does not need to be set, as it's being run inside an action
		// It still *can* be set, though.
		bool isVoidRun = false;

		// Includes both passed and default parameters - see numPassedParams
		std::vector<Value> paramValues;
		// Number of parameters explicitly passed by user in expression
		size_t numPassedParams = -1;
		// Current return value; if Any, not set yet.
		Value returnValue;
		// Desired return type, for an expression. If Any, not being run from expression.
		Type expectedReturnType = Type::Any;

		// Fusion event #(num) or delayed event.
		std::tstring runLocation;

		// If non-empty, the original function name before it was redirected to this one
		std::tstring redirectedFromFunctionName;

		RunningFunction(std::shared_ptr<FunctionTemplate> funcTemplate, bool active, int numRepeats)
			: funcTemplate(funcTemplate), active(active), numRepeats(numRepeats), returnValue(funcTemplate->defaultReturnValue)
		{
			for (std::size_t i = 0; i < funcTemplate->params.size(); ++i)
				paramValues.push_back(funcTemplate->params[i].defaultVal);
		}
	};
	struct DelayedFunction final
	{
		// Tick where this function was queued
		const int startFrame;
		// Units may be milliseconds or ticks; see useTicks
		int numUnitsUntilRun = 0;
		// If true, numUnitsUntilRun is frame ticks, if false, it's milliseconds
		bool useTicks = true;
		// If true, this delayed function remains in the queue and should trigger even if Fusion frame changes
		// (requires the new frame to have a DarkScript with a matching global ID)
		bool keepAcrossFrames = false;
		// Number of times this delayed event ticks
		int numRepeats = 0;
		// Tick count until this is run (might be N/A, see useTicks)
		int runAtTick = 0;
		// Time when this is run (might be N/A, see useTicks)
		std::chrono::time_point<std::chrono::system_clock> runAtTime;

		std::shared_ptr<RunningFunction> funcToRun;

		// Fusion event #(num) or delayed event.
		std::tstring fusionStartEvent;

		DelayedFunction(std::shared_ptr<RunningFunction> func, int startFrame)
			: startFrame(startFrame), funcToRun(func)
		{
			// Can't keep object selection in a delayed func
			assert(!func->keepObjectSelection);
		}
	};

	struct GlobalData final
	{
		// Extensions using this GlobalData
		std::vector<Extension *> exts;
		// Extensions using func templates registered in this global's exts will register themselves here,
		// to get template->ext updated when this global's ext instance the template would've run on is destroyed
		std::vector<GlobalData *> updateTheseGlobalsWhenMyExtCycles;
		// Templates, otherwise called declarations
		std::vector<std::shared_ptr<FunctionTemplate>> functionTemplates;
		// Functions delayed but will run later
		std::vector<std::shared_ptr<DelayedFunction>> pendingFuncs;
		// Functions that are running
		std::vector<std::shared_ptr<RunningFunction>> runningFuncs;
		// All scoped vars available at all levels
		std::vector<ScopedVar> scopedVars;
		// The curFrame number on frame end
		int curFrameOnFrameEnd = 0;
		// If runtime is paused, this is set to pause time
		decltype(DelayedFunction::runAtTime) runtimepausedtime;
	};
	// Function set up, in case we're sending templates across frames
	GlobalData* globals;
	// current DarkScript error
	std::tstring curError;
	// All the juicy details
	std::tstringstream curLog;
	// Internal loop name
	std::tstring curLoopName;
	// If calling subapp ext for running our expression, we generate errors from caller side,
	// not from the subapp ext
	Extension* errorExt = nullptr;
	// Last return value - useful
	Value lastReturn = Value(Type::Any);
	// Make errors a mandatory event.
	bool darkScriptErrorRead = true;

	// Current frame index (while functions pending, this number accumulates by 1 per event loop via Handle)
	int curFrame = 0;

	static void AutoGenerateExpressions();
	static int GetNumExpressions();
	long VariableFunction(const TCHAR * funcName, const ACEInfo &exp, long * args);

	void evt_SaveSelectedObjects(std::vector<FusionSelectedObjectListCache>& selectedObjects, short excludeOi = -1);
	void evt_RestoreSelectedObjects(const std::vector<FusionSelectedObjectListCache>& selectedObjects, bool unselectAllExisting);

	std::shared_ptr<DelayedFunction> curDelayedFuncLoop, curDelayedFunc;
	std::shared_ptr<FunctionTemplate> curFuncTemplateLoop;
	const Param * curParamLoop = nullptr;
	const ScopedVar * curScopedVarLoop = nullptr;
	std::size_t internalLoopIndex;

	std::shared_ptr<RunningFunction> foreachFuncToRun;
	GlobalData * ReadGlobalDataByID(const std::tstring & globalID);
	static std::tstring ToLower(const std::tstring_view str2);

	bool StringToType(Type &type, const TCHAR * typeStr);
	static const TCHAR * const TypeToString(Extension::Type type);

	std::shared_ptr<FunctionTemplate> Sub_GetFuncTemplateByName(const TCHAR * cppFuncName, const TCHAR * funcNameOrBlank);
	std::shared_ptr<RunningFunction> Sub_GetRunningFunc(const TCHAR* cppFuncName, const TCHAR * funcNameOrBlank);
	std::tstring Sub_GetAvailableVars(std::shared_ptr<RunningFunction>& rf, Expected includeParam) const;
	Value * Sub_CheckParamAvail(const TCHAR * const cppFuncName, int paramIndex);
	Value * Sub_CheckScopedVarAvail(const TCHAR * const cppFuncName, const TCHAR * scopedVarOrParamName, Expected shouldBeParam, bool makeError, const Param ** optParam = nullptr);
	Param * Sub_GetTemplateParam(const TCHAR * const cppFuncName, const std::shared_ptr<FunctionTemplate> f, int paramIndex);
	Param * Sub_GetTemplateParam(const TCHAR * const cppFuncName, const std::shared_ptr<FunctionTemplate> f, const TCHAR * paramName);
	ScopedVar* Sub_GetOrCreateTemplateScopedVar(const TCHAR* cppFuncName, const TCHAR* funcName, const TCHAR* scopedVarName);

	int Sub_GetValAsInteger(const Value &val);
	float Sub_GetValAsFloat(const Value &val);
	std::tstring Sub_GetValAsString(const Value& val);
	std::tstring Sub_ConvertToString(const float val);
	std::tstring Sub_ConvertToString(const int val);

	bool Sub_FunctionMatches(std::shared_ptr<RunningFunction> a, std::shared_ptr<RunningFunction> b);
	std::tstring Sub_GetLocation(int actID);
	static void Sub_ReplaceAllString(std::tstring& str, const std::tstring_view from, const std::tstring_view to);

	long ExecuteFunction(RunObjectMultiPlatPtr obj, const std::shared_ptr<RunningFunction> &rf);
	void Sub_RunPendingForeachFunc(const short oil, const std::shared_ptr<RunningFunction> &rf);
	bool Sub_ParseParamValue(const TCHAR* cppFuncName, std::tstring valueTextToParse, const Param& paramExpected, const size_t paramIndex, Value& writeTo);


	void CreateError(PrintFHintInside const TCHAR* format, ...) PrintFHintAfter(2, 3);
	#define CreateErrorT(x, ...) CreateError(_T(x), ##__VA_ARGS__)


	/// Actions

	void Template_SetFuncSignature(const TCHAR * funcSig, int delayable, int repeatable, int recursable);
	void Template_SetDefaultReturnN(const TCHAR * funcName);
	void Template_SetDefaultReturnI(const TCHAR * funcName, int value);
	void Template_SetDefaultReturnF(const TCHAR * funcName, float value);
	void Template_SetDefaultReturnS(const TCHAR * funcName, const TCHAR * value);
	void Template_Param_SetDefaultValueN(const TCHAR * funcName, const TCHAR* paramName, int useTheAnyType);
	void Template_Param_SetDefaultValueI(const TCHAR * funcName, const TCHAR* paramName, int paramValue, int useTheAnyType);
	void Template_Param_SetDefaultValueF(const TCHAR * funcName, const TCHAR* paramName, float paramValue, int useTheAnyType);
	void Template_Param_SetDefaultValueS(const TCHAR * funcName, const TCHAR* paramName, const TCHAR * paramValue, int useTheAnyType);
	void Template_SetScopedVarOnStartI(const TCHAR* funcName, const TCHAR* scopedVarName, int paramValue, int overrideWhenRecursing);
	void Template_SetScopedVarOnStartF(const TCHAR* funcName, const TCHAR* scopedVarName, float paramValue, int overrideWhenRecursing);
	void Template_SetScopedVarOnStartS(const TCHAR* funcName, const TCHAR* scopedVarName, const TCHAR* paramValue, int overrideWhenRecursing);
	void Template_CancelScopedVarOnStart(const TCHAR* funcName, const TCHAR* scopedVarName);
	void Template_SetGlobalID(const TCHAR* funcName, const TCHAR* globalIDToRunOn);
	void Template_SetEnabled(const TCHAR* funcName, int funcEnabled);
	void Template_RedirectFunction(const TCHAR* funcName, const TCHAR* redirectFuncName);
	void Template_Loop(const TCHAR* loopName);
	void Template_ImportFromAnotherFrame(const TCHAR* funcName, const TCHAR* globalIDToImportFrom);

	void DelayedFunctions_Loop(const TCHAR* loopName);
	void DelayedFunctions_CancelByPrefix(const TCHAR* funcNamePrefix);

	void RunFunction_ActionDummy_Num(int result); // dummy
	void RunFunction_ActionDummy_String(const TCHAR * result); // dummy
	// The dummies must be last parameter, so when last parameter is evaluated, we have the previous parameters we can read from
	void RunFunction_Foreach_Num(RunObject* obj, int dummy);
	void RunFunction_Foreach_String(RunObject* obj, const TCHAR* dummy);
	void RunFunction_Delayed_Num_MS(int timeFirst, int numRepeats, int timeSubsequent, int crossFrames, int funcDummy);
	void RunFunction_Delayed_String_MS(int timeFirst, int numRepeats, int timeSubsequent, int crossFrames, const TCHAR* funcDummy);
	void RunFunction_Delayed_Num_Ticks(int timeFirst, int numRepeats, int timeSubsequent, int crossFrames, int funcDummy);
	void RunFunction_Delayed_String_Ticks(int timeFirst, int numRepeats, int timeSubsequent, int crossFrames, const TCHAR* funcDummy);
	void RunFunction_Script(const TCHAR* script);

	void RunningFunc_SetReturnI(int value);
	void RunningFunc_SetReturnF(float value);
	void RunningFunc_SetReturnS(const TCHAR * value);
	void RunningFunc_ScopedVar_SetI(const TCHAR * paramName, int newVal);
	void RunningFunc_ScopedVar_SetF(const TCHAR * paramName, float newVal);
	void RunningFunc_ScopedVar_SetS(const TCHAR * paramName, const TCHAR * newVal);
	void RunningFunc_Params_Loop(const TCHAR* loopName, int includeNonPassed);
	void RunningFunc_ScopedVar_Loop(const TCHAR* loopName, int includeInherited);
	void RunningFunc_StopFunction(int cancelCurrentIteration, int cancelNextIterations, int cancelForeachLoop);
	void RunningFunc_ChangeRepeatSetting(int newRepeatIndex, int newRepeatCount, int ignoreExistingCancel);
	void RunningFunc_Abort(const TCHAR * error, const TCHAR * funcToUnwindTo);

	void Logging_SetLevel(const TCHAR* funcNames, const TCHAR* logLevel);


	/// Conditions

	bool AlwaysTrue() const;
	bool LoopNameMatch(const TCHAR* loopName) const;

	bool OnDarkScriptError();
	bool OnFunction(const TCHAR * funcName);
	bool OnForeachFunction(const TCHAR* funcName, int objOiList);
	bool OnFunctionAborted(const TCHAR* funcName);
	bool IsRunningFuncStillActive();

	bool DoesFunctionHaveTemplate(const TCHAR* funcName);
	bool IsFunctionInCallStack(const TCHAR* funcName);
	bool IsLastRepeatOfFunction(const TCHAR* funcName);

	// OnFuncTemplateLoop -> LoopNameMatch
	// OnPendingFunctionLoop -> LoopNameMatch
	// OnFuncParamLoop -> LoopNameMatch
	// OnScopedVarLoop -> LoopNameMatch

	bool Logging_OnAnyFunction();
	bool Logging_OnAnyFunctionCompletedOK();
	bool Logging_OnAnyFunctionAborted();

	/// Expressions

	const TCHAR * Logging_GetDarkScriptError();
	const TCHAR * Logging_GetAbortReason();
	const TCHAR * Logging_GetCurrentLog(int clearLog);

	int RunningFunc_GetRepeatIndex(const TCHAR * funcName);
	int RunningFunc_GetNumRepeatsLeft(const TCHAR * funcName);
	int RunningFunc_GetNumRepeatsTotal(const TCHAR * funcName);
	int RunningFunc_ForeachFV(const TCHAR * funcName);
	int RunningFunc_NumParamsPassed(const TCHAR * funcName);
	int RunningFunc_ScopedVar_GetI(const TCHAR * paramName);
	float RunningFunc_ScopedVar_GetF(const TCHAR* paramName);
	const TCHAR * RunningFunc_ScopedVar_GetS(const TCHAR* paramName);
	int RunningFunc_GetParamValueByIndexI(int paramIndex);
	float RunningFunc_GetParamValueByIndexF(int paramIndex);
	const TCHAR * RunningFunc_GetParamValueByIndexS(int paramIndex);
	const TCHAR * RunningFunc_GetAllParamsAsText(const TCHAR * funcNameOrBlank, const TCHAR* separator, int includeAnnotation);
	const TCHAR * RunningFunc_GetCallStack(int mostRecentAtBottom, int rewindCount);
	const TCHAR * RunningFunc_GetCalledFuncName(const TCHAR * funcNameOrBlank);

	int InternalLoop_GetIndex();
	const TCHAR * InternalLoop_GetVarName();
	const TCHAR * InternalLoop_GetVarType();

	const TCHAR * FuncTemplate_GetFunctionName();
	int FuncTemplate_GetNumRequiredParams(const TCHAR * funcNameOrBlank);
	int FuncTemplate_GetNumPossibleParams(const TCHAR* funcNameOrBlank);
	const TCHAR * FuncTemplate_GetReturnType(const TCHAR* funcNameOrBlank);
	int FuncTemplate_ShouldRepeat(const TCHAR * funcNameOrBlank);
	int FuncTemplate_ShouldBeDelayed(const TCHAR * funcNameOrBlank);
	int FuncTemplate_RecursionAllowed(const TCHAR * funcNameOrBlank);
	int FuncTemplate_IsEnabled(const TCHAR* funcNameOrBlank); // can be false for running func if disabled while running
	const TCHAR * FuncTemplate_GetRedirectFuncName(const TCHAR* funcNameOrBlank);
	const TCHAR * FuncTemplate_GlobalID(const TCHAR* funcNameOrBlank);

	const TCHAR * FuncTemplate_ParamNameByIndex(const TCHAR * funcNameOrBlank, int paramIndex);
	const TCHAR * FuncTemplate_ParamTypeByIndex(const TCHAR * funcNameOrBlank, int paramIndex);
	const TCHAR * FuncTemplate_ParamDefaultValByIndex(const TCHAR * funcNameOrBlank, int paramIndex);
	int FuncTemplate_ParamIndexByName(const TCHAR * funcNameOrBlank, const TCHAR * paramName);
	const TCHAR * FuncTemplate_ParamTypeByName(const TCHAR * funcNameOrBlank, const TCHAR * paramName);
	const TCHAR * FuncTemplate_ParamDefaultValByName(const TCHAR * funcNameOrBlank, const TCHAR * paramName);

	int LastReturn_AsInt();
	float LastReturn_AsFloat();
	const TCHAR * LastReturn_AsString();
	const TCHAR * LastReturn_Type();

	const TCHAR * TestFunc(int param);

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

	// These are called if there's no function linked to an ID
	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);

#if PAUSABLE_EXTENSION == 0
#error DarkScript should be pausable
#endif
	void FusionRuntimePaused();
	void FusionRuntimeContinued();
};
