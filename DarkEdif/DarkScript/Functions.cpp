#include "Common.hpp"
#include <cstdlib>
#include <algorithm>

#define CreateErrorExp(Ret, Error, ...) { CreateErrorT(Error, ##__VA_ARGS__); return Ret; }
#define CreateErrorExpOpt(Opt, Ret, Error, ...) { if (Opt) CreateErrorT(Error, ##__VA_ARGS__); return Ret; }

void Extension::CreateError(PrintFHintInside const TCHAR * format, ...)
{
	va_list v;
	va_start(v, format);

	static TCHAR backRAM[2048], prefix[256];

	try {
		if (_vstprintf_s(backRAM, std::size(backRAM), format, v) <= 0)
			DarkEdif::MsgBox::Error(_T("Error making error"), _T("Who watches the watchmen?\nError making error: %d"), errno);
	}
	catch (...) {
		DarkEdif::MsgBox::Error(_T("Error making error"), _T("Who watches the watchmen?\nError making error: %d"), errno);
	}

	va_end(v);

	LOGI(_T("DarkScript error: %s\n"), backRAM);

	if (!curError.empty())
	{
		DarkEdif::MsgBox::Error(_T("Stack memory crash prevented"),
			_T("A recursive error occurred while handling a DarkScript error. Stack memory overflow bug prevented.\n"
				"Original error:\n%s\nNew error while handling original error:\n%s"),
			curError.c_str(), backRAM);
		return; // don't generate event
	}
	int fusionEventNum = DarkEdif::GetCurrentFusionEventNum(this);
	prefix[0] = _T('\0');
	if (fusionEventNum != -1)
		_stprintf_s(prefix, std::size(prefix), _T("[Fusion event #%d] "), fusionEventNum);
	curError = prefix;
	curError += backRAM; // will dup memory

	std::vector<FusionSelectedObjectListCache> list;
	evt_SaveSelectedObjects(list);
	darkScriptErrorRead = false;
	Runtime.GenerateEvent(0);
	evt_RestoreSelectedObjects(list, true);
	if (!darkScriptErrorRead)
	{
		DarkEdif::MsgBox::Error(_T("Error unhandled"),
			_T("DarkScript event occurred, but you have no \"DarkScript > On Error\" event to handle it. That is BAD PRACTICE. Error message:\n%s"),
			curError.c_str());
	}

	curError.clear(); // to prevent recursion
}

Extension::GlobalData* Extension::ReadGlobalDataByID(const std::tstring& str2)
{
	return (GlobalData*)Runtime.ReadGlobal((_T("DarkScript"s) + str2).c_str());
}
std::tstring Extension::ToLower(const std::tstring_view str2)
{
	std::tstring str(str2);
	std::transform(str.begin(), str.end(), str.begin(), ::_totlower);
	return str;
}

static const TCHAR * typeStrs[] = {
	_T("Any"),
	_T("Integer"),
	_T("Float"),
	_T("String")
};
const TCHAR * const Extension::TypeToString(Extension::Type type)
{
	return typeStrs[(int)type];
}

bool Extension::StringToType(Type &type, const TCHAR * typeStr)
{
	std::tstring typeStrL(ToLower(typeStr));
	if (typeStrL == _T("any"sv) || typeStrL == _T("a"sv))
		type = Type::Any;
	else if (typeStrL == _T("integer"sv) || typeStrL == _T("int"sv) || typeStrL == _T("i"sv))
		type = Type::Integer;
	else if (typeStrL == _T("float"sv) || typeStrL == _T("f"sv))
		type = Type::Float;
	else if (typeStrL == _T("string"sv) || typeStrL == _T("funcName"sv) || typeStrL == _T("text"sv) || typeStrL == _T("s"sv))
		type = Type::String;
	else
		return false;

	return true;
}

std::shared_ptr<Extension::FunctionTemplate> Extension::Sub_GetFuncTemplateByName(const TCHAR * cppFuncName, const TCHAR * funcNameOrBlank)
{
	// Blank func: use current on-going
	if (funcNameOrBlank[0] == _T('\0'))
	{
		if (curFuncTemplateLoop)
			return curFuncTemplateLoop;
		if (globals->runningFuncs.empty())
			CreateErrorExp(std::shared_ptr<Extension::FunctionTemplate>(), "%s: no function running and not a func template loop, so a function template name must be passed.", cppFuncName);
		return globals->runningFuncs.back()->funcTemplate;
	}

	// Match by name
	const std::tstring nameL(ToLower(funcNameOrBlank));
	const auto res = std::find_if(globals->functionTemplates.begin(), globals->functionTemplates.end(),
		[&](const std::shared_ptr<FunctionTemplate> & f) { return f->nameL == nameL;
	});
	if (res == globals->functionTemplates.end())
		CreateErrorExp(std::shared_ptr<Extension::FunctionTemplate>(), "%s: couldn't find a function template with name %s.", cppFuncName, funcNameOrBlank);
	return *res;
}
std::shared_ptr<Extension::RunningFunction> Extension::Sub_GetRunningFunc(const TCHAR* cppFuncName, const TCHAR* funcNameOrBlank)
{
	if (globals->runningFuncs.empty())
		CreateErrorExp(std::shared_ptr<Extension::RunningFunction>(), "Use of %s without a function active.", cppFuncName);
	if (funcNameOrBlank[0] == _T('\0'))
		return globals->runningFuncs.back();

	// Match by name
	std::tstring nameL(ToLower(funcNameOrBlank));
	auto res = std::find_if(globals->runningFuncs.rbegin(), globals->runningFuncs.rend(),
		[&](const std::shared_ptr<RunningFunction>& f) { return f->funcTemplate->nameL == nameL;
		});
	if (res == globals->runningFuncs.rend())
		CreateErrorExp(std::shared_ptr<Extension::RunningFunction>(), "%s: couldn't find a running function with name %s.", cppFuncName, funcNameOrBlank);
	return *res;
}

Extension::Value * Extension::Sub_CheckParamAvail(const TCHAR * cppFuncName, int paramNum)
{
	if (paramNum < 0)
		CreateErrorExp(NULL, "%s: param index below 0. (%i)", cppFuncName, paramNum);

	auto rf = Sub_GetRunningFunc(cppFuncName, _T(""));
	if (!rf)
		return NULL;

	if (rf->funcTemplate->params.size() <= (size_t)paramNum)
		CreateErrorExp(NULL, "%s: param index out of range (0 < %i < %zu).",
			cppFuncName, paramNum, rf->funcTemplate->params.size());

	return &rf->paramValues[paramNum];
}
std::tstring Extension::Sub_GetAvailableVars(std::shared_ptr<RunningFunction>& rf, Expected includeParam) const {
	std::vector<std::tstring_view> availableVars;
	std::tstringstream str;
	if (includeParam != Expected::Never)
	{
		for (size_t i = 0; i < rf->funcTemplate->params.size(); ++i)
		{
			const auto& tp = rf->funcTemplate->params[i];
			const auto& pv = rf->paramValues[i];

			// Already outputted; a later function could overwrite an earlier scoped variable, so skip any we're doing in reverse
			if (std::find_if(availableVars.crbegin(), availableVars.crend(), [&](const auto& t) { return t == tp.nameL; }) != availableVars.crend())
				continue;
			availableVars.push_back(tp.nameL);
			if (pv.type == Type::Integer)
				str << _T("[int]    "sv) << tp.name << _T(" = "sv) << pv.data.integer << _T('\n');
			else if (pv.type == Type::Float)
				str << _T("[float]  "sv) << tp.name << _T(" = "sv) << pv.data.decimal << _T('\n');
			else if (pv.type == Type::String)
				str << _T("[string] "sv) << tp.name << _T(" = \""sv) << std::tstring_view(pv.data.string, (pv.dataSize / sizeof(TCHAR)) - 1) << _T("\"\n"sv);
			else
				str << _T("[any]    "sv) << tp.name << _T(" = unset ("sv) << pv.dataSize << _T(" bytes)\n"sv);
		}
	}
	if (includeParam != Expected::Always)
	{
		for (auto it = globals->scopedVars.crbegin(); it != globals->scopedVars.crend(); ++it)
		{
			// Already outputted; a later function could overwrite an earlier scoped variable, so skip any we're doing in reverse
			if (std::find_if(availableVars.crbegin(), availableVars.crend(), [&](const auto& t) { return t == it->nameL; }) != availableVars.crend())
				continue;
			availableVars.push_back(it->nameL);
			if (it->defaultVal.type == Type::Integer)
				str << _T("[int]    "sv) << it->name << _T(" = "sv) << it->defaultVal.data.integer << _T('\n');
			else if (it->defaultVal.type == Type::Float)
				str << _T("[float]  "sv) << it->name << _T(" = "sv) << it->defaultVal.data.decimal << _T('\n');
			else if (it->defaultVal.type == Type::String)
				str << _T("[string] "sv) << it->name << _T(" = \""sv) << std::tstring_view(it->defaultVal.data.string, (it->defaultVal.dataSize / sizeof(TCHAR)) - 1) << _T("\"\n"sv);
			else
				str << _T("[any]    "sv) << it->name << _T(" = unset ("sv) << it->defaultVal.dataSize << _T(" bytes)\n"sv);
		}
	}

	std::tstring availableVarsStr = str.str();
	str.str(std::tstring());

	std::tstring_view types;
	if (includeParam == Expected::Either)
		types = _T("params/scoped vars"sv);
	else if (includeParam == Expected::Always)
		types = _T("params"sv);
	else
		types = _T("scoped vars"sv);

	if (availableVarsStr.empty())
		str << _T("No "sv) << types << _T(" available."s);
	else
	{
		str << _T("Available "sv) << types << _T(":\n"sv)
			<< std::tstring_view(availableVarsStr.data(), availableVarsStr.size() - 1);
	}
	return str.str();
}
Extension::Value * Extension::Sub_CheckScopedVarAvail(const TCHAR * cppFuncName, const TCHAR * scopedVarName, Expected shouldBeParam, bool makeError, const Param ** paramTo /* = nullptr */)
{
	if (scopedVarName[0] == _T('\0'))
		CreateErrorExpOpt(makeError, NULL, "%s: param/scoped var name is blank.", cppFuncName);

	auto rf = globals->runningFuncs.empty() && !makeError ? nullptr : Sub_GetRunningFunc(cppFuncName, _T(""));
	if (!rf)
		return NULL;

	const std::tstring strL(ToLower(scopedVarName));

	if (shouldBeParam != Expected::Always)
	{
		const auto scopedVar = std::find_if(globals->scopedVars.begin(), globals->scopedVars.end(),
			[&](const Extension::ScopedVar & a) { return a.nameL == strL; }
		);
		if (scopedVar != globals->scopedVars.end())
		{
			if (paramTo)
				*paramTo = &*scopedVar;
			return &scopedVar->defaultVal;
		}

		// Must be scoped var, not param, and not found - ded
		if (shouldBeParam == Expected::Never)
		{
			CreateErrorExpOpt(makeError, NULL, "%s: %s: scoped var name \"%s\" not found.\n%s",
				cppFuncName, rf->funcTemplate->name.c_str(), scopedVarName, Sub_GetAvailableVars(rf, shouldBeParam).c_str());
		}
	}
	// shouldBeParam is either Either or Always, Never would've returned in ^ already

	const auto param = std::find_if(rf->funcTemplate->params.begin(), rf->funcTemplate->params.end(),
		[&](const Extension::Param & a) { return a.nameL == strL; }
	);
	if (param == rf->funcTemplate->params.end())
	{
		CreateErrorExpOpt(makeError, NULL, "%s: %s: param%s name \"%s\" not found.%s\n%s",
			cppFuncName, rf->funcTemplate->name.c_str(), shouldBeParam == Expected::Never ? _T("") : _T("/scoped var"), scopedVarName,
			!rf->funcTemplate->isAnonymous ? _T("") : _T(" Anonymous functions use \"a\", \"b\" for parameters.\n"),
			Sub_GetAvailableVars(rf, shouldBeParam).c_str()
		);
	}
	if (paramTo)
		*paramTo = &*param;
	const size_t index = std::distance(rf->funcTemplate->params.begin(), param);
	return &rf->paramValues[index];
}
Extension::ScopedVar * Extension::Sub_GetOrCreateTemplateScopedVar(const TCHAR* cppFuncName, const TCHAR * funcName, const TCHAR* scopedVarName)
{
	if (scopedVarName[0] == _T('\0'))
		CreateErrorExp(NULL, "%s: scoped var name is blank.", cppFuncName);
	if (funcName[0] == _T('\0'))
		CreateErrorExp(NULL, "%s: func name is blank.", cppFuncName);

	const auto f = Sub_GetFuncTemplateByName(cppFuncName, funcName);
	if (!f)
		return NULL;

	const std::tstring strL(ToLower(scopedVarName));
	const auto scopedVar = std::find_if(f->scopedVarOnStart.begin(), f->scopedVarOnStart.end(),
		[&](const Extension::ScopedVar & a) { return a.nameL == strL; }
	);
	if (scopedVar == f->scopedVarOnStart.end())
	{
		f->scopedVarOnStart.push_back(ScopedVar(scopedVarName, Type::Any, false, -1));
		return &f->scopedVarOnStart.back();
	}

	return &*scopedVar;
}
Extension::Param* Extension::Sub_GetTemplateParam(const TCHAR* cppFuncName, const std::shared_ptr<FunctionTemplate> f, int paramNum)
{
	if (!f)
		return NULL;

	if (paramNum < 0)
		CreateErrorExp(NULL, "%s: param index below 0. (%i)", cppFuncName, paramNum);

	if (f->params.size() <= (size_t)paramNum)
		CreateErrorExp(NULL, "%s: param index out of range (0 < %i < %zu).",
			cppFuncName, paramNum, f->params.size());

	return &f->params[paramNum];
}
Extension::Param * Extension::Sub_GetTemplateParam(const TCHAR * cppFuncName, const std::shared_ptr<FunctionTemplate> f, const TCHAR * paramName)
{
	if (!f)
		return NULL;

	if (paramName[0] == '\0')
		CreateErrorExp(NULL, "%s: param name is blank.", cppFuncName);

	const std::tstring strL(ToLower(paramName));
	const auto param = std::find_if(f->params.begin(), f->params.end(),
		[&](Extension::Param & a) { return a.nameL == strL; }
	);
	if (param == f->params.end())
		CreateErrorExp(NULL, "%s: param name \"%s\" not found.",
			cppFuncName, paramName);

	return &*param;
}

float Extension::Sub_GetValAsFloat(const Extension::Value &val)
{
	switch (val.type)
	{
	case Type::String:
	{
		const double ret = _ttof(val.data.string);
		if (errno == ERANGE)
			CreateErrorExp((float)ret, "RunningFunc_GetParamValueByIndexF: Error while converting value to float (too large/small to return as 32-bit float).");
		return (float)ret;
	}
	case Type::Float:
		return *(float *)&val.data;
	case Type::Integer:
		return (float)*(int *)&val.data;
	case Type::Any:
		CreateErrorT("Failure at line %i, file %s.", __LINE__, _T(__FILE__));
		// while the default can be unset and thus remain any, it should be replaced when running
	}

	CreateErrorT("Failure at line %i, file %s.", __LINE__, _T(__FILE__));
	return 0.f;
}
int Extension::Sub_GetValAsInteger(const Extension::Value &val)
{
	switch (val.type)
	{
	case Type::String:
	{
		int ret = _ttoi((TCHAR *)val.data.string);
		if (errno == ERANGE)
			CreateErrorExp(ret, "Error while converting value to integer (too large/small to return as 32-bit signed integer).");
		return ret;
	}
	case Type::Float:
		return (int)*(float *)&val.data; // casting to float will cause round-down
	case Type::Integer:
		return *(int *)&val.data;
	case Type::Any:
		CreateErrorT("Failure at line %i, file %s.", __LINE__, _T(__FILE__));
		// while the default can be unset and thus remain any, it should be replaced when running
	}

	CreateErrorT("Failure at line %i, file %s.", __LINE__, _T(__FILE__));
	return 0;
}
std::tstring Extension::Sub_ConvertToString(float f)
{
	static TCHAR wow[128];
	if (_stprintf_s(wow, sizeof(wow) / sizeof(*wow), _T("%f"), f) < 0)
		CreateErrorT("Failed to convert float. Error %i.", errno);
	return std::tstring(wow);
}
std::tstring Extension::Sub_ConvertToString(int i)
{
	static TCHAR wow[128];
	if (_stprintf_s(wow, sizeof(wow) / sizeof(*wow), _T("%i"), i) <= 0)
		CreateErrorT("Failed to convert integer. Error %i.", errno);
	return std::tstring(wow);
}

std::tstring Extension::Sub_GetValAsString(const Extension::Value &val)
{
	switch (val.type)
	{
	case Type::String:
		return val.data.string ? val.data.string : _T("");
	case Type::Float:
		return Sub_ConvertToString(*(float *)&val.data);
	case Type::Integer:
		return Sub_ConvertToString(*(int *)&val.data);
	case Type::Any:
		CreateErrorT("Failure at line %i, file %s.", __LINE__, _T(__FILE__));
		// while the default can be unset and thus remain any, it should be replaced when running
	}

	CreateErrorT("Failure at line %i, file %s.", __LINE__, _T(__FILE__));
	return std::tstring();
}

#include <assert.h>

// ID < 40: nothing
// ID - 40 = true ID
// ID % num return types = return type
// ID - return type % num param types / num param types

// Defined in DarkEdif.cpp
ACEInfo * Edif::ACEInfoAlloc(unsigned int NumParams);

#define lastNonDummyFunc (CurLang["Expressions"sv].u.array.length)
constexpr short lastNonFuncID = 59;
// 5 parameters seems to be the maximum Fusion supports, because there is a hard cap of around 5,582 expression IDs.
// Any further and CF2.5 crashes when you try to add an action. MMF2 Standard staggers after 5632, and dies completely at 5647.
// Yves modified the CF2.5 b294.8 to allow more expressions, giving up to max ID 32392.
//
// 4,368 IDs for up to 5 params when there's 3 param types, 3 return types, 4 variants.
// 13,116 IDs for up to 6 params when there's 3 param types, 3 return types, 4 variants.
// 39,360 IDs for up to 7 params when there's 3 param types, 3 return types, 4 variants.
// 118,092 IDs for up to 8 params when there's 3 param types, 3 return types, 4 variants.
// ^ which is impossible since IDs are 16-bit ints, so max out at 65,535.
//
// We could potentially merge float and int, make all int returns/param parameters floats instead, giving us up to 8 parameters.
// The SDK can be set to switch at runtime and get either for a numeric parameter, but we'd not be able to tell what the type is
// before we request it, so we'd basically have to assume float. So it's less like a clever merge and more like a dumb assumption.
// That said,
// 4,088 IDs for up to 8 params when there's 2 param types, 2 return types, 4 variants.
//
// We could also spin out the "Keep selection" and "Repeat" flags into obvious actions, reducing the variants to 2 or 1.
// However, the less inline this object's functions are, the less useful it is.
constexpr int numParamsToAllow = 5;

// If this is updated, look everywhere for KR func regexes, like running from script and declaration/template setup
enum Flags {
	None = 0b00,
	KeepObjSelection = 0b01,
	Repeat = 0b10,
	Both = KeepObjSelection + Repeat,
};

// pow() in constexpr format. Ignore last parameter.
constexpr int64_t ipow(int64_t base, int exp, int64_t result = 1)
{
	return exp < 1 ? result : ipow(base*base, exp / 2, (exp % 2) ? result * base : result);
}
constexpr int64_t duet(int64_t base, int exp)
{
	int64_t sum = 1;
	for (int i = 0; i < exp; ++i)
		sum += ipow(base, i + 1);
	return sum;
}
size_t numFuncVariantsToGenerate = 0;

constexpr size_t numFuncVariantsToGenerateMMF2 =
	// Variants of return types...
	(((Flags::Both + 1) * ((int)Extension::Type::NumCallableReturnTypes)) *
	// multiplied by variants of parameter types...
	((int)duet((int)Extension::Type::NumCallableParamTypes,
		// multiplied by max number of parameters
		std::min(numParamsToAllow, 5))));
constexpr size_t numFuncVariantsToGenerateCF25 =
// Variants of return types...
(((Flags::Both + 1) * ((int)Extension::Type::NumCallableReturnTypes)) *
	// multiplied by variants of parameter types...
	((int)duet((int)Extension::Type::NumCallableParamTypes,
		// multiplied by max number of parameters
		std::min(numParamsToAllow, 6))));

static void GenerateExpressionFuncFor(const int inputID)
{
	// inputID is func ID, - 40 not needed.

	// First, get repeat/delayed by bitmask of 4.

	const int NumVariants = Flags::Both + 1;
	int funcID = inputID;
	Flags flags = (Flags)(funcID & Flags::Both);
	funcID = (funcID / NumVariants); // remove last two bits (Flags) and shift remaining down in their place

	//for (int i = 0; i < 30; ++i)
	//	LOGI(_T("Anticipating that for function ID %i, %i is the template.\n"), i, (i - 4 - (i % 4)));

	ACEInfo * last;
	// Get last function before this template, with None variant.
	// For example, FuncS should look up FuncI.
	if (inputID >= NumVariants)
		last = Edif::SDK->ExpressionInfos[lastNonFuncID + 1 + (inputID - NumVariants - (inputID % NumVariants))];
	else // We're on the first set of functions
	{
		last = (ACEInfo*)alloca(sizeof(ACEInfo) + (1 * sizeof(short) * 2));
		last->ID = lastNonFuncID;
		last->FloatFlags = 0;
		last->Flags.ef = ExpReturnType::Integer;
		// First parameter of expression function is the function name
		last->NumOfParams = 1;
		last->Parameter[0].ep = ExpParams::String;
		last->Parameter[1].ep = (ExpParams)0; // extra short of 0 follows parameter
	}

	auto GetParam = [&](std::size_t id) {
		assert((short)id < last->NumOfParams);
		if (last->Parameter[id].ep == ExpParams::String)
			return Extension::Type::String;
		else if (last->Parameter[id].ep == ExpParams::Integer)
			return last->FloatFlags & (1 << id) ? Extension::Type::Float : Extension::Type::Integer;
		else
		{
			DarkEdif::MsgBox::Error(_T("Param type"), _T("Parameter type not recognised"));
			return Extension::Type::Any;
		}
	};


	bool needNewParam = false;
	// If we're progressing to add a new parameter, we're on last variant, and highest value ExpReturnType, float,
	// and all parameters are string
	if (last->Flags.ef == ExpReturnType::Float)
	{
		needNewParam = true;
		// Skip parameter 0 (function ID) - bear in mind last is None variant, so no repeat count
		for (std::size_t i = 1; i < (std::size_t)last->NumOfParams; ++i)
		{
			if (GetParam(i) != Extension::Type::Float)
			{
				needNewParam = false;
				break;
			}
		}
	}

	// Fourth, what remains is an index with parameters being passed.
	// Don't forget to take into account repeat count.

	std::vector<Extension::Type> paramTypes;
	paramTypes.push_back(Extension::Type::String); // function name
	if (flags & Flags::Repeat)
		paramTypes.push_back(Extension::Type::Integer); // number of repeats

	// Get return type by modulus num return types
	const Extension::Type returnType = (Extension::Type)((funcID % (int)Extension::Type::NumCallableReturnTypes) + 1);

	// If we're adding a new parameter, we start with int-returning funcs
	assert(!needNewParam || returnType == Extension::Type::Integer);

	int intermediate = (funcID / (int)Extension::Type::NumCallableReturnTypes) - 1;
	for (std::size_t i = paramTypes.size(); i < (std::size_t)(last->NumOfParams + (needNewParam ? 1 : 0) + ((flags & Flags::Repeat) != Flags::None ? 1 : 0)); ++i)
	{
		// too many params?
		if (intermediate < 0)
			DarkEdif::BreakIfDebuggerAttached();
		// While there's 4 valid parameter types (Any included),
		// the function can only be called with 3.
		paramTypes.push_back((Extension::Type)((intermediate % (int)Extension::Type::NumCallableParamTypes) + 1));

		if (paramTypes[i] < Extension::Type::Integer)
		{
			// miscalculated type
			DarkEdif::BreakIfDebuggerAttached();
			break;
		}
		intermediate = (intermediate / (int)Extension::Type::NumCallableParamTypes) - 1;
	}

	if (intermediate > 0)
		DarkEdif::BreakIfDebuggerAttached(); // missed a param?

	// Parameters checked; allocate new info
	ACEInfo* const ExpInfo = Edif::ACEInfoAlloc((unsigned int)paramTypes.size());

	// Could not allocate memory
	if (!ExpInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating expression info"), _T("Could not allocate memory for expression ID %zu return."), Edif::SDK->ExpressionInfos.size());

	// If a non-triggered condition, set the correct flags
	ExpInfo->ID = inputID + lastNonFuncID + 1;
	ExpInfo->NumOfParams = (short)paramTypes.size();

	std::stringstream str;
	switch (returnType)
	{
	case Extension::Type::Integer:
		ExpInfo->Flags.ef = ExpReturnType::Integer;
		str << "int"sv;
		break;
	case Extension::Type::Float:
		ExpInfo->Flags.ef = ExpReturnType::Float;
		str << "float"sv;
		break;
	case Extension::Type::String:
		ExpInfo->Flags.ef = ExpReturnType::String;
		str << "string"sv;
		break;
	default:
		DarkEdif::MsgBox::Error(_T("Error creating expression info"), _T("Can't use return type %i, not set up."), (int)returnType);
		return;
	}

	std::stringstream str2;

	str2 <<
		(flags & Flags::KeepObjSelection ? "K" : "") <<
		(flags & Flags::Repeat ? "R" : "") <<
		(ExpInfo->Flags.ef == ExpReturnType::Float ? "F" : "") <<
		"Func"sv;

	str << " " <<
		(flags & Flags::KeepObjSelection ? "k" : "") <<
		(flags & Flags::Repeat ? "r" : "") <<
		"func_"sv << inputID << '(';

	str << "string_funcname, "sv;
	ExpInfo->Parameter[0].ep = ExpParams::String;

	// Set up each parameter (there's 1+, since func name is there)
	for (size_t c = 1, d = 1; c < paramTypes.size(); ++c, ++d)
	{
		if (c == 1 && (flags & Flags::Repeat))
		{
			str << "int_repeatcount, "sv;
			ExpInfo->Parameter[1].ep = ExpParams::Integer;
			--d;
			continue;
		}
		switch (paramTypes[c])
		{
		case Extension::Type::Integer:
			ExpInfo->Parameter[c].ep = ExpParams::Integer;
			str << "int_"sv << d << ", "sv;
			str2 << 'I';
			break;
		case Extension::Type::Float:
			ExpInfo->Parameter[c].ep = ExpParams::Float;
			ExpInfo->FloatFlags |= (1 << c);
			str << "float_"sv << d << ", "sv;
			str2 << 'F';
			break;
		case Extension::Type::String:
			ExpInfo->Parameter[c].ep = ExpParams::String;
			str << "string_"sv << d << ", "sv;
			str2 << 'S';
			break;
		default:
			DarkEdif::MsgBox::Error(_T("Error creating expression info"), _T("Can't use parameter type %i, not set up."), (int)paramTypes[c]);
			return;
		}
	}

#if defined(_DEBUG) && defined(_WIN32)
	std::string ret = str.str();
	str.str(std::string());
	str << ret.substr(0, ret.size() - 2U) << ") [ID "sv << ExpInfo->ID << ", param count "sv << ExpInfo->NumOfParams << "]\n"sv;
	ret = str.str();

	str2 << (ExpInfo->Flags.ef == ExpReturnType::String ? "$" : "")
		<< "() [expression ID "sv << ExpInfo->ID << ", "sv << std::hex << ExpInfo->ID << "].\n"sv;
	ret = str2.str();
	OutputDebugStringA(ret.c_str());
#endif

	// Extra short possibly used for menu resource ID, which we don't use
	memset(&ExpInfo->Parameter[ExpInfo->NumOfParams], 0, ExpInfo->NumOfParams * sizeof(short));

	// Add to table
	Edif::SDK->ExpressionInfos.push_back(ExpInfo);

#if _DEBUG && defined(_WIN32)
	if (false) // (inputID % 4 == 0)
		 LOGV(_T("Outputted dynamic function, expression ID %i, funcID %i.\n%s"), ExpInfo->ID, inputID, DarkEdif::UTF8ToTString(ret).c_str());
#endif
}

void GenerateDummyExpression(short inputID)
{
	ACEInfo * ExpInfo = Edif::ACEInfoAlloc(0);

	// Could not allocate memory
	if (!ExpInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating expression info"), _T("Could not allocate memory for dummy expression ID %zu return."), Edif::SDK->ExpressionInfos.size());

	ExpInfo->ID = inputID;
	ExpInfo->NumOfParams = 0;
	ExpInfo->Flags.ef = ExpReturnType::Integer;

	// Add to table
	Edif::SDK->ExpressionInfos.push_back(ExpInfo);
}
#ifdef _WIN32
#pragma comment (lib, "Version.lib")

static int getFusionSubBuild() {
	LPBYTE adfifo;
	VS_FIXEDFILEINFO* lpVfifo;
	DWORD dwHnd = 0;
	const std::tstring_view tsv = DarkEdif::GetRunningApplicationPath(DarkEdif::GetRunningApplicationPathType::FullPath);
	unsigned int lsize = GetFileVersionInfoSize(tsv.data(), &dwHnd);
	if (lsize != 0 && (adfifo = (LPBYTE)malloc(lsize)) != NULL)
	{
		if (GetFileVersionInfo(tsv.data(), dwHnd, lsize, adfifo) &&
			VerQueryValue(adfifo, _T("\\"), (void**)&lpVfifo, &lsize))
		{
			return LOWORD(lpVfifo->dwFileVersionLS);
		}
		free(adfifo);
	}

	return -1;
}
#endif // _WIN32

int Extension::GetNumExpressions()
{
	// CF2.5 b294.8+ has modification to allow more functions
	// +1 for 0-based ID to 1-based size
	assert(numFuncVariantsToGenerate > 0);
	return lastNonFuncID + (int)numFuncVariantsToGenerate + 1;
}
void Extension::AutoGenerateExpressions()
{
	// CF2.5 b294.8+ has modification to allow more functions
	// Since all the expression infos are read back, we don't need to worry about undercutting later

	if (numFuncVariantsToGenerate == 0)
	{
#ifdef _WIN32
		const int build = (Edif::SDK->mV->GetVersion() & MMFBUILD_MASK);
		const bool useMoreExpressions = DarkEdif::IsFusion25 &&
			(build >= 295 || (build == 294 && getFusionSubBuild() >= 8));
#else
		const bool useMoreExpressions = true;
#endif

		numFuncVariantsToGenerate = (useMoreExpressions ? numFuncVariantsToGenerateCF25 : numFuncVariantsToGenerateMMF2);
	}

#ifdef _WIN32
	// Jumps uses new[], not malloc(), otherwise we'd realloc
	{
		delete[] Edif::SDK->ExpressionJumps;

		// We want to get the last index, then add one for null pointer at end
		const size_t maxFuncIndex = lastNonFuncID + numFuncVariantsToGenerate;
		Edif::SDK->ExpressionJumps = new void* [maxFuncIndex + 2]; // +1 for index -> length, +1 cos we want an extra for null
		for (size_t i = 0; i < Edif::SDK->ExpressionInfos.size(); ++i)
			Edif::SDK->ExpressionJumps[i] = &Edif::ExpressionJump;
		Edif::SDK->ExpressionJumps[maxFuncIndex + 1] = NULL;
	}
#endif // _WIN32

	// 1. Find end of Expressions
	assert(Edif::SDK->ExpressionInfos.size() <= lastNonFuncID);

#ifdef _WIN32
	size_t i = Edif::SDK->ExpressionInfos.size();
#endif

	// 2. Add safety buffer (50 expressions?) pointing to dummy function
	// 21 used, reserve 20, intermediate index 40
	while (Edif::SDK->ExpressionInfos.size() <= lastNonFuncID)
	{
#ifdef _WIN32
		Edif::SDK->ExpressionJumps[i++] = (void *)Edif::ExpressionJump;
#endif
		GenerateDummyExpression((short)Edif::SDK->ExpressionInfos.size());
		Edif::SDK->ExpressionFunctions.push_back(0);
	}

	// 3. Generate IDs and infos following CreateNewExpressionInfo
	// formula is in excel

	while (Edif::SDK->ExpressionInfos.size() - 1 < lastNonFuncID + numFuncVariantsToGenerate)
	{
#ifdef _WIN32
		Edif::SDK->ExpressionJumps[i++] = (void *)Edif::ExpressionJump;
#endif
		GenerateExpressionFuncFor((int)Edif::SDK->ExpressionInfos.size() - 1 - lastNonFuncID);
		Edif::SDK->ExpressionFunctions.push_back(Edif::MemberFunctionPointer(&Extension::VariableFunction));
	}
}

#ifdef _WIN32
void FusionAPI GetExpressionParam(mv* mV, short code, short param, TCHAR* strBuf, short maxLen)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
	{
		if ((size_t)code < lastNonDummyFunc)
			Edif::ConvertAndCopyString(strBuf, CurLang["Expressions"sv][code]["Parameters"sv][param][1], maxLen);
		else if ((size_t)code <= lastNonFuncID)
		{
			DarkEdif::MsgBox::Error(_T("Shouldn't happen"), _T("Should never happen. Param requested for dummy function."));
			Edif::ConvertAndCopyString(strBuf, "COUGH"sv, maxLen);
		}
		else
		{
			int funcID = code - lastNonFuncID - 1;

			LOGV(_T("Outputting param %i for dynamic function, expression ID %i, funcID %i.\n"), param, code, funcID);

			ACEInfo& exp = *Edif::SDK->ExpressionInfos[code];
			if (exp.NumOfParams < param + 1)
			{
				DarkEdif::MsgBox::Error(_T("Shouldn't happen"), _T("Param %i missing for function ID %i (expr ID %i)."), param, funcID, code);
				Edif::ConvertAndCopyString(strBuf, "Wowie", maxLen);
				return;
			}

			char ret[64];
			switch (exp.Parameter[param].ep)
			{
			case ExpParams::Float: // also integer
				if ((exp.FloatFlags & (1 << param)) != 0)
					sprintf_s(ret, "Float%i", param + 1);
				else // integer
				{
					// Repeat is always 2nd param if just Repeat
					if (param == 1 && (funcID & Flags::Repeat) == Flags::Repeat)
						sprintf_s(ret, "Number of runs (1+)");
					else
						sprintf_s(ret, "Integer%i", param + 1);
				}
				Edif::ConvertAndCopyString(strBuf, ret, maxLen);
				return;
			case ExpParams::String:
				// Either function (param 0) or string
				Edif::ConvertAndCopyString(strBuf, param == 0 ? "Function" : "String", maxLen);
				return;
			default:
				DarkEdif::MsgBox::Error(_T("Shouldn't happen"), _T("Param %i unrecognised for function ID %i (expr ID %i)."), param, funcID, code);
				Edif::ConvertAndCopyString(strBuf, "Unrecog", maxLen);
				return;
			}

		}
	}
}

void FusionAPI GetExpressionTitle(mv* mV, short code, TCHAR* strBuf, short maxLen)
{
#pragma DllExportHint
	if (!Edif::IS_COMPATIBLE(mV))
		return;

	if ((size_t)code < lastNonDummyFunc)
	{
		std::string Return(CurLang["Expressions"sv][code]["Title"sv]);
		if (Return.back() != '(')
			Return.push_back('(');
		Edif::ConvertAndCopyString(strBuf, Return.c_str(), maxLen);
	}
	else if ((size_t)code <= lastNonFuncID)
	{
		Edif::ConvertAndCopyString(strBuf, "DummyFunc("sv, maxLen);
	}
	else
	{
		const int origFuncID = code - lastNonFuncID - 1;

		ACEInfo& exp = *Edif::SDK->ExpressionInfos[code];
		std::stringstream str;
		if (origFuncID & Flags::KeepObjSelection)
			str << 'K';
		if (origFuncID & Flags::Repeat)
			str << 'R';
		if (exp.Flags.ef == ExpReturnType::Float)
			str << 'F';
		str << "Func"sv;

		size_t i = 1; // func name is first paramater
		if (origFuncID & Flags::Repeat)
			++i; // skip repeat count parameter if present

		for (; i < (size_t)exp.NumOfParams; ++i)
		{
			if (exp.Parameter[i].ep == ExpParams::String)
				str << 'S';
			else if (exp.Parameter[i].ep == ExpParams::Float)
			{
				if ((exp.FloatFlags & (1 << i)) != 0)
					str << 'F';
				else // integer
					str << 'I';
			}
			else
			{
				DarkEdif::MsgBox::Error(_T("GetExpressionTitle"), _T("Param %i unrecognised for function ID %i (expr ID %i)."), i, origFuncID, code);
				str.str("UnrecognizedError"s);
				str << code;
				break;
			}
		}
		if (exp.Flags.ef == ExpReturnType::String)
			str << '$';
		str << '(';
		Edif::ConvertAndCopyString(strBuf, str.str().c_str(), maxLen);
	}
}
void FusionAPI GetExpressionString(mv* mV, short code, TCHAR* strPtr, short maxLen)
{
#pragma DllExportHint
	if (!Edif::IS_COMPATIBLE(mV))
		return;
	GetExpressionTitle(mV, code, strPtr, maxLen);
}

#endif

// Set in Edif.cpp for DARKSCRIPT_EXTENSION; needed to read the Action parameters.
// Last action parameter is a FuncX expression, which when evaluated, will call VariableFunction()
extern long* actParameters;

long Extension::VariableFunction(const TCHAR* funcName, const ACEInfo& exp, long *args)
{
#define DummyReturn ( exp.Flags.ef == ExpReturnType::String ? (long)Runtime.CopyString(_T("")) : 0L)
#define LastReturn (lastReturn.type == Type::String ? (long)Runtime.CopyString(lastReturn.data.string ? lastReturn.data.string : _T("")) : (long)lastReturn.data.string)
#define CreateError2V(...) { \
	CreateErrorT(##__VA_ARGS__); \
	lastReturn = Value((Extension::Type)((int)exp.Flags.ef + 1)); \
	return DummyReturn; }
#define ReadNextArgAs(x) (*(x *)&args[expParamIndex++])

	std::shared_ptr<FunctionTemplate> funcTemplate;

	// otherwise not variable func
	assert(exp.ID > lastNonFuncID && exp.ID <= lastNonFuncID + (short)numFuncVariantsToGenerate);
	event2* callingAction = rhPtr->GetRH4ActionStart();
	int actID = callingAction ? callingAction->get_evtNum() - 80 : -1;
	bool runImmediately = true, isVoidRun = false, isDelayed = false;
	// We don't run now if we have the Foreach or Delayed action being executed.
	// Instead, parameters are collected here, then when the Action func runs, it triggers On Function.
	if (actID >= 19 && actID <= 26 && callingAction->get_evtOi() == rdPtr->get_rHo()->get_Oi())
	{
		// dummy func: run immediately, but don't reset actID, we use it later in Sub_GetLocation()
		if (actID >= 19 && actID <= 20)
			LOGV(_T("Called for Dummy action! Action ID is %i.\n"), actID);
		else
		{
			runImmediately = false;
			isDelayed = actID >= 23; // foreach is 21, 22, delayed 23-26
			LOGD(_T("Called for Foreach or Delayed action! Action ID is %i; qual act cause singl cond = %s, singl act cause qual cond = %s.\n"), actID,
				allowQualifierToTriggerSingularForeach ? _T("yes") : _T("no"), allowSingularToTriggerQualifierForeach ? _T("yes") : _T("no"));
		}
		isVoidRun = true;
	}
	else
	{
		LOGV(_T("NOT called from Foreach/Delayed action! Oi is %i, action ID is %i.\n"), callingAction ? callingAction->get_evtOi() : -1, actID);
		actID = -1;
	}

	int funcID = exp.ID - lastNonFuncID - 1;
	std::tstring nameL(ToLower(funcName)), redirectedFromName;

	auto funcTemplateIt = std::find_if(globals->functionTemplates.begin(), globals->functionTemplates.end(),
		[&](const auto &f) { return f->nameL == nameL; });

	// Handles redirection to another function
	if (funcTemplateIt != globals->functionTemplates.end() && !(**funcTemplateIt).redirectFunc.empty())
	{
		LOGV(_T("Redirecting from function \"%s\" to \"%s\".\n"), funcName, (**funcTemplateIt).redirectFunc.c_str());
		auto funcTemplateIt2 = std::find(globals->functionTemplates.begin(), globals->functionTemplates.end(), (**funcTemplateIt).redirectFuncPtr);

		// To redirect, the function must exist as a template, even with anonymous functions allowed. So if it doesn't, that's a pretty big problem
		if (funcTemplateIt2 == globals->functionTemplates.end() /* && funcsMustHaveTemplate */)
		{
			CreateErrorT("Can't call function \"%s\"; was redirected to function \"%s\", which was defined when redirection was set, but does not exist in templates now. "
				"Returning the default return value of \"%s\".",
				funcName, (**funcTemplateIt).redirectFunc.c_str(), funcName);
			lastReturn = (*funcTemplateIt)->defaultReturnValue;
			return lastReturn.type == Type::String ? (long)Runtime.CopyString(lastReturn.data.string ? lastReturn.data.string : _T("")) : (long)lastReturn.data.string;
		}
		redirectedFromName = funcName;
		funcName = (**funcTemplateIt2).name.c_str();
		nameL = (**funcTemplateIt2).nameL;
		funcTemplateIt = funcTemplateIt2;
	}

	std::size_t expParamIndex = 1; // skip func name (index 0), we already read it
	// If 0, then function does not run
	int repeatTimes = 1;

	if (funcID & Flags::Repeat)
	{
		repeatTimes = ReadNextArgAs(int);
		if (repeatTimes < 0)
		{
			CreateErrorT("Can't call function %s with a negative number of repeats (you supplied %i repeats).", funcName, repeatTimes);
			if (funcTemplateIt == globals->functionTemplates.end())
				lastReturn = Value((Extension::Type)((int)exp.Flags.ef + 1));
			else
				lastReturn = (*funcTemplateIt)->defaultReturnValue;
			return lastReturn.type == Type::String ? (long)Runtime.CopyString(lastReturn.data.string ? lastReturn.data.string : _T("")) : (long)lastReturn.data.string;
		}
	}

	// Switch from returning dummy to returning whatever's in lastReturn
#undef CreateError2V
#define CreateError2V(a, ...) \
	{ \
		CreateErrorT(a, ##__VA_ARGS__); \
		if (runImmediately) \
			return LastReturn; \
		return DummyReturn; \
	}

	// Function template doesn't exist; generate a temporary one
	if (funcTemplateIt == globals->functionTemplates.end())
	{
		lastReturn = Value((Extension::Type)((int)exp.Flags.ef + 1));
		if (funcsMustHaveTemplate)
			CreateError2V("Function with name \"%s\" not found.", funcName);
		if (repeatTimes == 0)
			return LastReturn;

		LOGW(_T("Generating anonymous function \"%s\".\n"), funcName);
		funcTemplate = std::make_shared<FunctionTemplate>(this, funcName, Expected::Either, Expected::Either, true, (Extension::Type)((int)exp.Flags.ef + 1));
		TCHAR name[3] = { _T('a'), _T('\0') };
		for (std::size_t i = expParamIndex; i < (size_t)exp.NumOfParams; ++i, ++name[0])
		{
			// Note the ++name[0] in for(;;><), gives variable names a, b, c
			Type type = exp.Parameter[i].ep == ExpParams::String ? Type::String :
				exp.Parameter[i].ep == ExpParams::Integer ?
				(exp.FloatFlags & (1 << i) ? Type::Float : Type::Integer) : Type::Any;
			funcTemplate->params.push_back(Param(name, type));
		}
		funcTemplate->isAnonymous = true;
	}
	else
	{
		funcTemplate = *funcTemplateIt;
		if (runImmediately)
			lastReturn = funcTemplate->defaultReturnValue;

		if ((funcID & Flags::Repeat) == Flags::Repeat && funcTemplate->repeating == Expected::Never)
			CreateError2V("Can't call function %s on repeat, template does not allow repeating.", funcName);
		if ((funcID & Flags::Repeat) == Flags::None && funcTemplate->repeating == Expected::Always)
			CreateError2V("Can't call function %s without repeating, template expects repeating.", funcName);

		if (isDelayed && funcTemplate->delaying == Expected::Never)
			CreateError2V("Can't call function %s delayed, template does not allow delaying.", funcName);
		if (!isDelayed && funcTemplate->delaying == Expected::Always)
			CreateError2V("Can't call function %s without delaying, template expects delaying.", funcName);

		if ((!funcTemplate->recursiveAllowed || preventAllRecursion) &&
			std::find_if(globals->runningFuncs.begin(), globals->runningFuncs.end(),
				[&](std::shared_ptr<RunningFunction>& rf) { return rf->funcTemplate == funcTemplate; }
			) != globals->runningFuncs.end())
		{
			CreateError2V("Can't call function %s recursively, %s.", funcName,
				preventAllRecursion ? _T("recursion has been prevented globally") :_T("template does not allow recursing")
			);
		}

		// Too many parameters (ignore func name and repeat count)
		if (((std::size_t)exp.NumOfParams - expParamIndex) > funcTemplate->params.size())
			CreateError2V("Can't call function %s with %hi parameters, template expects up to %zu parameters.", funcTemplate->name.c_str(), exp.NumOfParams, funcTemplate->params.size());

		// This template was made with a valid global ID that had an ext, but the ext vanished
		if (funcTemplate->ext == NULL)
			CreateError2V("Can't call function %s, it runs on a now non-existent global ID \"%s\".",
				funcTemplate->name.c_str(), funcTemplate->globalID.c_str());
	}


	// Running while current func is aborting - that's confusing behaviour
	if (!globals->runningFuncs.empty() && !globals->runningFuncs.back()->active)
	{
		CreateError2V("Can't run function \"%s\", current function \"%s\" is aborted. Abort must be handled before functions can be run.",
			funcName, globals->runningFuncs.back()->funcTemplate->name.c_str());
	}

	// User requested a looping function to run 0 times - return default value from function
	if (repeatTimes == 0)
	{
		if (runImmediately)
			return LastReturn;
		return DummyReturn;
	}

	auto newFunc = std::make_shared<RunningFunction>(funcTemplate, runImmediately, repeatTimes - 1);
	newFunc->keepObjectSelection = funcID & Flags::KeepObjSelection;
	newFunc->isVoidRun = isVoidRun;
	newFunc->redirectedFromFunctionName = redirectedFromName;

	std::size_t numNotInParamsVector = expParamIndex;
	std::size_t numPassedExpFuncParams = exp.NumOfParams - numNotInParamsVector; // ignore func name and num repeats

	newFunc->numPassedParams = numPassedExpFuncParams;

	// User passed too many parameters
	if (funcTemplate->params.size() < numPassedExpFuncParams)
		CreateError2V("%s: called with %zu parameters, but only expects %zu.", funcName, numPassedExpFuncParams, funcTemplate->params.size());

	// don't confuse expParamIndex (A/C/E, will include funcName, may include numRepeats)
	// vs. paramIndex (index of paramValues)
	std::size_t paramIndex = 0;

	for (; paramIndex < numPassedExpFuncParams; ++paramIndex)
	{
		Extension::Type paramTypeInTemplate = newFunc->funcTemplate->params[paramIndex].type;

		// Param type has default string, free it so we can replace with user-supplied param
		if (newFunc->paramValues[paramIndex].type == Extension::Type::String)
		{
			free(newFunc->paramValues[paramIndex].data.string);
			newFunc->paramValues[paramIndex].data.integer = 0;
		}

		switch (exp.Parameter[expParamIndex].ep)
		{
		case ExpParams::Float: // also integer
			if ((exp.FloatFlags & (1 << expParamIndex)) != 0) // float passed
			{
				if (paramTypeInTemplate == Extension::Type::Float || paramTypeInTemplate == Extension::Type::Any)
				{
					newFunc->paramValues[paramIndex].type = Extension::Type::Float;
					newFunc->paramValues[paramIndex].dataSize = sizeof(float);
					newFunc->paramValues[paramIndex].data.decimal = ReadNextArgAs(float);
				}
				else
				{
					CreateError2V("Function %s: Parameter %zu should have been an %s, but was called with a %s instead.",
						funcName, paramIndex, TypeToString(paramTypeInTemplate), _T("float"));
				}
			}
			else // integer passed
			{
				if (paramTypeInTemplate == Extension::Type::Integer || paramTypeInTemplate == Extension::Type::Any)
				{
					newFunc->paramValues[paramIndex].type = Extension::Type::Integer;
					newFunc->paramValues[paramIndex].dataSize = sizeof(int);
					newFunc->paramValues[paramIndex].data.integer = ReadNextArgAs(int);
				}
				else
				{
					CreateError2V("Function %s: Parameter %zu should have been an %s, but was called with a %s instead.",
						funcName, paramIndex, TypeToString(paramTypeInTemplate), _T("integer"));
				}
			}
			break;
		case ExpParams::String:
			if (paramTypeInTemplate == Extension::Type::String || paramTypeInTemplate == Extension::Type::Any)
			{
				newFunc->paramValues[paramIndex].type = Extension::Type::String;
				// pre-freed earlier
				const TCHAR * param = ReadNextArgAs(const TCHAR *);
				newFunc->paramValues[paramIndex].dataSize = (_tcslen(param) + 1) * sizeof(TCHAR);
				newFunc->paramValues[paramIndex].data.string = _tcsdup(param);
			}
			else
			{
				CreateError2V("Function %s: Parameter %zu should have been an %s, but was called with a %s instead.",
					funcName, paramIndex, TypeToString(paramTypeInTemplate), _T("string"));
			}
			break;
		default:
			DarkEdif::MsgBox::Error(_T("Param not recognised"), _T("Param type for param index %zu not recognised for function ID %zu (expr ID %hu)."),
				paramIndex, (size_t)exp.ID - lastNonDummyFunc - 1, exp.ID);
			return DummyReturn;
		}
	}

	// User didn't pass these, but template expects them; load from param defaults
	for (; paramIndex < funcTemplate->params.size(); ++paramIndex)
	{
		// No default, and user didn't pass one: no good.
		if (funcTemplate->params[paramIndex].type != Extension::Type::Any &&
			funcTemplate->params[paramIndex].defaultVal.type == Extension::Type::Any)
		{
			CreateError2V("%s: parameter %s (index %zu) was not passed, but must be supplied.",
				funcName, funcTemplate->params[paramIndex].name.c_str(), paramIndex);
		}
		else
		{
			newFunc->paramValues[paramIndex] = newFunc->funcTemplate->params[paramIndex].defaultVal;
		}
	}

	if (newFunc->active)
	{
		// Always save selection and restore after action, even for non-K func;
		// if newFunc->keepObjectSelection is set (K-func), then the On Function
		// condition will restore for start of each On Function event
		evt_SaveSelectedObjects(newFunc->selectedObjects);

		newFunc->runLocation = Sub_GetLocation(actID);
		newFunc->expectedReturnType = (Type)((int)exp.Flags.ef + 1);
		// TODO: Conversion strictness checks, e.g.
		// if (newFunc->funcTemplate->returnType != newFunc->expectedReturnType)

		long l = ExecuteFunction(nullptr, newFunc);
		newFunc->runLocation.clear();

		evt_RestoreSelectedObjects(newFunc->selectedObjects, true);
		return l;
	}

	// else Foreach or Delayed func
	// ForEach
	if (Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Foreach_Num) ||
		Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Foreach_String))
	{
		if (foreachFuncToRun)
			LOGE(_T("Possible crash! Foreach function is already set, but is being set again. Original foreach: \"%s\", current foreach: \"%s\"."), foreachFuncToRun->funcTemplate->name.c_str(), newFunc->funcTemplate->name.c_str());

		// Can't run a foreach on another frame; how could we select?
		if (funcTemplate->ext != this && funcTemplate->ext->fusionFrameNum != this->fusionFrameNum)
			CreateError2V("Can't run a foreach loop on function %s, its events are in DarkScript global ID %s, in another frame.",
				funcName, funcTemplate->ext->globalID.c_str());


		foreachFuncToRun = newFunc;
		newFunc->isVoidRun = true;
		newFunc->runLocation = Sub_GetLocation(actID);
		// actParameters[1] is being calculated right now by this call of VariableFunc.

	}
	// Delayed func
	else if (Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Delayed_Num_MS) ||
		Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Delayed_String_MS) ||
		Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Delayed_Num_Ticks) ||
		Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Delayed_String_Ticks))
	{
		int firstRunDelayedFor = *(int*)&actParameters[0];
		int numRepeats = *(int*)&actParameters[1];
		int delayBetweenRuns = *(int*)&actParameters[2];
		int keepIfFusionFrameChanged = *(int*)&actParameters[3];
		// actParameters[4] is being calculated right now by this call of VariableFunc().

		auto delayFunc = std::make_shared<DelayedFunction>(newFunc, curFrame);
		delayFunc->keepAcrossFrames = keepIfFusionFrameChanged != 0;
		delayFunc->numRepeats = numRepeats;
		delayFunc->numUnitsUntilRun = delayBetweenRuns;
		delayFunc->useTicks =
			Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Delayed_Num_Ticks) ||
			Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Delayed_String_Ticks);
		if (delayFunc->useTicks)
			delayFunc->runAtTick = curFrame + firstRunDelayedFor;
		else
			delayFunc->runAtTime = decltype(delayFunc->runAtTime)::clock::now() + std::chrono::milliseconds(firstRunDelayedFor);
		delayFunc->fusionStartEvent = Sub_GetLocation(-1);
		globals->pendingFuncs.push_back(delayFunc);

		Runtime.Rehandle();
	}
	else
		throw std::runtime_error("Wrong call");


	return DummyReturn;
#undef DummyReturn
}

bool Extension::Sub_FunctionMatches(std::shared_ptr<RunningFunction> a, std::shared_ptr<RunningFunction> b)
{
	if (a == b || a->funcTemplate == b->funcTemplate)
		return true;

	// If templates must exist, there cannot be lambda, so the func templates are always shared
	// by being in globals->funcTemplates, so the above if ^ would have triggered from template matching
	if (funcsMustHaveTemplate)
		return false;

	// Possible lambda, have to check func name
	return a->funcTemplate->nameL == b->funcTemplate->nameL;
}
void Extension::Sub_ReplaceAllString(std::tstring& str, const std::tstring_view from, const std::tstring_view to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::tstring::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
}
std::tstring Extension::Sub_GetLocation(int actID)
{
	const auto GetActionIndex = [](Extension* ext)->std::tstring {
		event2* exp = ext->rhPtr->GetRH4ActionStart();
		if (exp == nullptr)
			return _T("a condition"s);
		auto evg = ext->rhPtr->get_EventGroup();
		if (evg == nullptr)
			return _T("unknown (action?)"s);

		auto cur = evg->GetCAByIndex(0);
		std::size_t i = 0;
		for (; i < evg->get_evgNCond(); ++i)
			cur = cur->Next();
		short lastOI = cur->get_evtOi();
		std::tstringstream str;

		for (i = 0; i < evg->get_evgNAct(); i++) {
			if (cur->GetIndex() == exp->GetIndex()) {
				lastOI = cur->get_evtOi();
				// We're using 1-based for action index, so increment i
				++i;

				// If it's DarkScript, try not to say "DarkScript action running DarkScript".
				// We check the name, in case there's multiple DarkScripts named differently by user.
				const std::tstring_view name = ext->Runtime.ObjectSelection.GetOILFromOI(lastOI)->get_name();

				if (((RunObject *)&*ext->rdPtr)->get_rHo()->get_Oi() != lastOI || name != _T("DarkScript"sv))
					str << name << _T("'s action (index "sv) << i << _T(')');
				else
					str << _T("action index "sv) << i;
				return str.str();
			}
			cur = cur->Next();
		}
		if (cur && cur->GetIndex() == exp->GetIndex())
		{
			str << ext->Runtime.ObjectSelection.GetOILFromOI(lastOI)->get_name() << _T("'s action (index "sv) << i + 1 << _T(')');
			return str.str();
		}
		return _T("unknown"s);
	};
	std::shared_ptr<RunningFunction> callerOrNull =
		globals->runningFuncs.empty() ? nullptr : globals->runningFuncs.back();
	// we're not multithreaded, so either it's the Handle() from a delayed function, or it's a Fusion event.
	const int curFusionEvent = DarkEdif::GetCurrentFusionEventNum(this);
	std::tstringstream str;

	// Not run inside a DarkScript action, so we'll assume it's run by expression
	if (actID == -1)
	{
		if (curFusionEvent != -1)
		{
			str << _T("DarkScript expression in Fusion event #"sv) << curFusionEvent << _T(", in "sv)
				<< GetActionIndex(this);
			return str.str();
		}

		// If this is running, it wasn't detected as being run in a DarkScript action,
		// and there is no current Fusion event, so it must be getting triggered by Handle(),
		// but we don't look up location for delayed func then, we look it up when delayed func is queued.

		DarkEdif::BreakIfDebuggerAttached();
		return _T("Unknown event"s);
	}

	if (Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_ActionDummy_Num) ||
		Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_ActionDummy_String))
	{
		// currently this overrides when a delayed funcA calls a non-delayed funcB which calls a non-delayed funcC.
		//callerOrNull->funcToRun->level
		str << _T("DarkScript dummy action in Fusion event #"sv) << curFusionEvent << _T(", in "sv)
			<< GetActionIndex(this);
		return str.str();
	}
	if (Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Foreach_Num) ||
		Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Foreach_String))
	{
		str << _T("DarkScript foreach action in Fusion event #"sv) << curFusionEvent << _T(", in "sv)
			<< GetActionIndex(this);
		return str.str();
	}
	if (Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Script))
	{
		str << _T("DarkScript scripting action in Fusion event #"sv) << curFusionEvent << _T(", in "sv)
			<< GetActionIndex(this);
		return str.str();
	}
	if (Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Delayed_Num_MS) ||
		Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Delayed_Num_Ticks) ||
		Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Delayed_String_MS) ||
		Edif::SDK->ActionFunctions[actID] == Edif::MemberFunctionPointer(&Extension::RunFunction_Delayed_String_Ticks))
	{
		str << _T("DarkScript delayed function call, queued by "sv)
			<< curDelayedFunc->fusionStartEvent.substr("DarkScript expression in "sv.size());
		return str.str(); // _T("delayed function \""s) + callerOrNull->funcToRun->funcTemplate->name + _T("\" queued by "s) + callerOrNull->fusionStartEvent;
	}

	assert(false && "Unrecognised action ID");
	return _T(""s);
}

long Extension::ExecuteFunction(RunObjectMultiPlatPtr objToRunOn, const std::shared_ptr<RunningFunction>& rf)
{
	// If running on another ext like subapp, we want to use its globals.
	auto globalsRunningOn = rf->funcTemplate->ext->globals;

	// TODO: Can optimize this by adding "isrunning" active to template
	if ((preventAllRecursion || !rf->funcTemplate->recursiveAllowed) && globalsRunningOn->runningFuncs.size() > 1)
	{
		auto endIt = --globalsRunningOn->runningFuncs.crend();
		auto olderIt = std::find_if(globalsRunningOn->runningFuncs.crbegin(), endIt,
			[&](const auto& f) { return Sub_FunctionMatches(rf, f); }
		);
		if (olderIt != endIt)
		{
			rf->abortReason = _T("Aborted from DarkScript recursion error."sv);
			CreateErrorT("Running a function recursively, when not allowed. Older run was from %s; current, aborted run is from %s. Aborting.",
				(*olderIt)->runLocation.c_str(), rf->runLocation.c_str());
		}
	}

	rf->active = true;

	// If not foreach, we want to save and restore scoped variables?
	// TODO: Shouldn't we do this for foreach too, or is it a optimization based on not actually running here?
	std::size_t numScopedVarsBeforeCall = SIZE_MAX;
	if (!objToRunOn)
	{
		numScopedVarsBeforeCall = globalsRunningOn->scopedVars.size();
		if (inheritParametersAsScopedVariables && !globalsRunningOn->runningFuncs.empty())
		{
			auto& lastFunc = globalsRunningOn->runningFuncs.back();
			for (std::size_t i = 0; i < lastFunc->funcTemplate->params.size(); ++i)
			{
				globalsRunningOn->scopedVars.push_back(ScopedVar(lastFunc->funcTemplate->params[i].name, Type::Any, true, globalsRunningOn->runningFuncs.size()));
				globalsRunningOn->scopedVars.back().defaultVal = lastFunc->paramValues[i];
			}
		}
		globalsRunningOn->runningFuncs.push_back(rf);
		for (auto& sv : rf->funcTemplate->scopedVarOnStart)
		{
			if (sv.recursiveOverride || 1 == std::count_if(globalsRunningOn->runningFuncs.cbegin(), globalsRunningOn->runningFuncs.cend(),
				[&](const auto& f) { return Sub_FunctionMatches(rf, f); }))
			{
				globalsRunningOn->scopedVars.push_back(sv);
				globalsRunningOn->scopedVars.back().level = globalsRunningOn->runningFuncs.size();
			}
		}
	}

	// Set up subapp ext for running cross-frame, and in case they do a cross-frame themselves, we'll store what they're using
	Extension* orig = rf->funcTemplate->ext->errorExt;
	rf->funcTemplate->ext->errorExt = this;
	//int origEventCount = rhPtr->GetRH2EventCount();
	//rhPtr->SetRH2EventCount(origEventCount + 1);

	if (objToRunOn)
		LOGV(_T("Running On Foreach function \"%s\", passing object \"%s\", FV %i.\n"), rf->funcTemplate->name.c_str(), objToRunOn->get_rHo()->get_OiList()->get_name(), rf->currentForeachObjFV);
	else
		LOGV(_T("Running On Function \"%s\". Set to repeat %i times.\n"), rf->funcTemplate->name.c_str(), rf->numRepeats);
	do
	{
		// On Obj Function, or On Function
		rf->funcTemplate->ext->Runtime.GenerateEvent(objToRunOn ? 2 : 1);

		if (enableOnAnyFunctions)
			rf->funcTemplate->ext->Runtime.GenerateEvent(11); // On Any Function

		// No On XX Function event ran
		if (!rf->eventWasHandled)
		{
			if (createErrorForUnhandledEvents)
			{
				if (objToRunOn)
				{
					CreateErrorT("Function \"%s\" was not handled by any On Function \"%s\" for each %s events.",
						rf->funcTemplate->name.c_str(), rf->funcTemplate->name.c_str(),
						globalsRunningOn->runningFuncs.back()->currentForeachOil < 0 ? _T("(qualifier)") : objToRunOn->get_rHo()->get_OiList()->get_name());
				}
				else
					CreateErrorT("Function \"%s\" was not handled by any On Function \"%s\" events.", rf->funcTemplate->name.c_str(),
						rf->funcTemplate->name.c_str());
			}
			// No events are triggering, no point running the rest
			rf->foreachTriggering = false;
			break;
		}

		// User cancelled the loop
		if (!rf->nextRepeatIterationTriggering)
			break;

		// In case they cancelled current iteration's On Function, reset back to normal for next loop
		rf->currentIterationTriggering = true;
	} while (rf->active && rf->numRepeats >= ++rf->index);

	// We need return value, no default return value, but we're returning with no return value set... uh oh
	if (rf->returnValue.dataSize == 0 && rf->funcTemplate->defaultReturnValue.dataSize == 0)
	{
		// Anonymous functions can have no return value set, and we'll just return 0 or ""
		// If void run, it's a delayed or foreach action
		if (whenAllowNoRVSet == AllowNoReturnValue::AllFunctions ||
			(rf->isVoidRun && whenAllowNoRVSet >= AllowNoReturnValue::ForeachDelayedActionsOnly) ||
			(rf->funcTemplate->isAnonymous && whenAllowNoRVSet >= AllowNoReturnValue::AnonymousForeachDelayedActions))
		{
			LOGV(_T("%sFunction \"%s\" has no default return value, and no return value was set by any On %sFunction \"%s\" events. "
				"Due to \"allow no default value\" property, ignoring it and returning 0/\"\".\n"),
				objToRunOn ? _T("Foreach ") : _T(""), rf->funcTemplate->name.c_str(),
				objToRunOn ? _T("Foreach ") : _T(""), rf->funcTemplate->name.c_str());
			// Return to calling expression - return int and float directly as they occupy same memory address
			rf->returnValue.type = rf->expectedReturnType;
			if (rf->returnValue.type != Extension::Type::String)
				rf->returnValue.dataSize = sizeof(int); // or sizeof float, same thing
			else
			{
				rf->returnValue.dataSize = sizeof(TCHAR);
				rf->returnValue.data.string = _tcsdup(_T(""));
			}
		}
		// else error: do foreach error, or do normal error
		else if (objToRunOn)
		{
			CreateErrorT("Function \"%s\" has no default return value, and no return value was set by any On Function \"%s\" for each %s events.",
				rf->funcTemplate->name.c_str(), rf->funcTemplate->name.c_str(),
				rf->currentForeachOil < 0 ? _T("(qualifier)") : objToRunOn->get_rHo()->get_OiList()->get_name());
		}
		else
			CreateErrorT("Function \"%s\" has no default return value, and no return value was set by any On Function \"%s\" events.",
				rf->funcTemplate->name.c_str(), rf->funcTemplate->name.c_str());
	}

	// No abort
	if (rf->abortReason.empty())
	{
		if (enableOnAnyFunctions)
			rf->funcTemplate->ext->Runtime.GenerateEvent(12); // On any function completed OK
	}
	else // Function was aborted
	{
		rf->funcTemplate->ext->Runtime.GenerateEvent(3); // On function aborted
		if (enableOnAnyFunctions)
			rf->funcTemplate->ext->Runtime.GenerateEvent(13); // On any function aborted

		if (!rf->abortWasHandled && createErrorForUnhandledAborts)
			CreateErrorT("Function abort \"%s\" was not handled by any On Function \"%s\" Aborted events.", rf->funcTemplate->name.c_str(), rf->funcTemplate->name.c_str());
	}

	// Reset error handling ext back to what it was
	rf->funcTemplate->ext->errorExt = orig;

	// If not a foreach (which keeps runningFunc itself), delete our func
	if (!objToRunOn)
		globalsRunningOn->runningFuncs.erase(--globalsRunningOn->runningFuncs.cend());

	// Remove all scoped vars we added on this level
	// Since any function we call from this one will have erased their own, in theory this will only delete ours
	if (numScopedVarsBeforeCall != SIZE_MAX)
		globalsRunningOn->scopedVars.erase(globalsRunningOn->scopedVars.cbegin() + numScopedVarsBeforeCall, globalsRunningOn->scopedVars.cend());

	// Store return value, in case we're running a foreach and later actions need the return
	lastReturn = rf->returnValue;

	// Return to calling expression - return int and float directly as they occupy same memory address
	if (rf->expectedReturnType == Extension::Type::String)
		return (long)Runtime.CopyString(Sub_GetValAsString(rf->returnValue).c_str());
	if (rf->expectedReturnType == Extension::Type::Float)
	{
		const float f = Sub_GetValAsFloat(rf->returnValue);
		return *(int*)&f;
	}
	if (rf->expectedReturnType == Extension::Type::Integer)
		return Sub_GetValAsInteger(rf->returnValue);
	return 0;
}

// Save object selection
void Extension::evt_SaveSelectedObjects(std::vector<FusionSelectedObjectListCache>& selectedObjects, short excludeOi)
{
	bool wasFound = false;
	const short DSOi = rdPtr->get_rHo()->get_Oi();
	std::vector<short> excludeOis;
	if (excludeOi > -1)
	{
		excludeOis.push_back(excludeOi);
		LOGD(_T("Should skip oi %hi (%s), it should be a foreach target?\n"),
			excludeOi, rhPtr->GetOIListByIndex(excludeOi)->get_name());
	}
	else if (excludeOi != -1)
	{
		excludeOis = rhPtr->GetQualToOiListByOffset(excludeOi)->GetAllOi();
		LOGD(_T("Should skip qualifier oi %hi, it should be a foreach target?\n"), excludeOi);
	}
	else
		LOGD(_T("No objects to skip.\n"));

	std::size_t numAllOi = 0;
	short oiListIndex = -1;
	for (auto poil : DarkEdif::AllOIListIterator(rhPtr))
	{
		++oiListIndex;
		++numAllOi;

		// Skip our ext, it'll always appear in selection as it's how this code right here in our ext is running
		// Since every generated condition will use DarkScript, we shouldn't need to re-select after generated events finish
		// although... if the user is strange enough to mix DS with other loop-type objects, maybe.
		if (poil->get_Oi() == DSOi)
		{
			LOGD(_T("Skipping object \"%s\", oi %i, it should be DarkScript.\n"), poil->get_name(), poil->get_Oi());
			wasFound = true;
			continue;
		}
		if (!excludeOis.empty() &&
			std::find(excludeOis.cbegin(), excludeOis.cend(), poil->get_Oi()) != excludeOis.cend())
		{
			LOGD(_T("Skipping object \"%s\", oi %i, it should be a foreach target and cannot be saved in K.\n"), poil->get_name(), poil->get_Oi());
			continue;
		}

		// Selection was caused by condition, or by action
		if (poil->get_EventCount() != rhPtr->GetRH2EventCount())
		{
			LOGD(_T("Skipping object \"%.*s\", event count differs (it is %i, but should be %i) so it is not selected.\n"),
				std::isprint(poil->get_name()[0] & 0x7F) ? 24 : 3,
				poil->get_name(), poil->get_EventCount(), rhPtr->GetRH2EventCount());
			continue;
		}
		LOGD(_T("Adding an object \"%.*s\", event count same (oil ec %i, rh ec %i) so it is selected.\n"),
			std::isprint(poil->get_name()[0] & 0x7F) ? 24 : 3,
			poil->get_name(), poil->get_EventCount(), rhPtr->GetRH2EventCount());

		// Already in the list?
		// TODO: Is this even possible? Multiple CRuns, maybe, but rhPtr is one CRun.
		int j;
		for (j = 0; j < (int)selectedObjects.size(); ++j)
		{
			if (selectedObjects[j].poil == poil)
				break;
		}

		FusionSelectedObjectListCache* pSel;
		if (j < (int)selectedObjects.size())
		{
			// In the list already => replace selection
			pSel = &selectedObjects[j];
			pSel->selectedObjects.clear();
		}
		else
		{
			// Not in the list yet, add new selection
			FusionSelectedObjectListCache sel;
			selectedObjects.emplace_back(std::move(sel));
			pSel = &selectedObjects[selectedObjects.size() - 1];
		}

		pSel->poil = std::move(poil);

		// Store explicitly selected objects; we ignore Implicit via eventcount check anyway
		std::size_t numSelected = 0;
		for (auto pHoFound : DarkEdif::ObjectIterator(rhPtr, oiListIndex, DarkEdif::Selection::Explicit))
		{
			//auto&& pHoFound = rhPtr->GetObjectListOblOffsetByIndex(num);
			// TODO: With multiple CRuns revealed, is this still functional across subapps?
			if (pHoFound == rdPtr)
				wasFound = true;
			++numSelected;
			pSel->selectedObjects.push_back(pHoFound->get_rHo()->get_Number());
			LOGV(_T("Added object selection instance: object \"%s\" hoNumber %hi. ListSelected number is %hi,"
				" Object is %hi, ho eventCount %i ==? oil %i.\n"),
				pSel->poil->get_name(), pSel->selectedObjects.back(), pSel->poil->get_ListSelected(), pSel->poil->get_Object(),
				rhPtr->GetRH2EventCount(), pSel->poil->get_EventCount());
		}
		if (numSelected > 0)
			LOGI(_T("Added object selection: object \"%s\" num instances %zu.\n"), pSel->poil->get_name(), pSel->selectedObjects.size());
	}
	LOGD(_T("Save obj select done: %zu objects.\n"), selectedObjects.size());

	if (numAllOi == 0)
		LOGE(_T("Save obj select FAILED: %zu oilist.\n"), numAllOi);
	else
		LOGD(_T("Save obj select note: %zu oilist total.\n"), numAllOi);

	if (!wasFound)
		LOGE(_T("Couldn't find DarkScript in selected objects!\n"));
}

// Restore objects selection
void Extension::evt_RestoreSelectedObjects(const std::vector<FusionSelectedObjectListCache>& selectedObjects, bool unselectAllExisting)
{
	// Unselect all objects
	if (unselectAllExisting)
	{
		for (auto poil : DarkEdif::AllOIListIterator(rhPtr))
		{
			// Skip our ext, it'll always appear in selection because otherwise, how is this code right here in our ext running?
			if (poil->get_Oi() == rdPtr->get_rHo()->get_Oi())
				continue;

			// If we're manually selecting, then don't reset selection
			if (std::find_if(selectedObjects.cbegin(), selectedObjects.cend(),
				[&poil](const FusionSelectedObjectListCache& f) {
					return f.poil == poil;
				}) != selectedObjects.cend())
			{
				continue;
			}

			// Invalidate the selection by making the event count not match, as opposed to explicitly selecting all
			poil->SelectAll(rhPtr, false);
		}
	}

	const int rh2EventCount = rhPtr->GetRH2EventCount();
	for (std::size_t i = 0; i < (int)selectedObjects.size(); ++i)
	{
		const FusionSelectedObjectListCache& sel = selectedObjects[i];
		auto & poil = sel.poil;
		LOGD(_T("Restoring obj select: running for %s, with %zu saved instances.\n"), poil->get_name(),
			sel.selectedObjects.size());

		poil->set_EventCount(rh2EventCount);
		poil->set_ListSelected(-1);
		poil->set_NumOfSelected(0);
		if (sel.selectedObjects.size() > 0)
		{
			auto&& pHoPrev = rhPtr->GetObjectListOblOffsetByIndex(sel.selectedObjects[0]);
			if (pHoPrev != NULL)
			{
				poil->set_ListSelected(sel.selectedObjects[0]);
				poil->set_NumOfSelected(poil->get_NumOfSelected()+1);
				for (std::size_t j = 1; j < (int)sel.selectedObjects.size(); ++j)
				{
					short num = sel.selectedObjects[j];
					auto&& pHo = rhPtr->GetObjectListOblOffsetByIndex(num);
					//pHo->SelectedInOR = 1;
					if (pHo == NULL)
						break;
					pHoPrev->get_rHo()->set_NextSelected(num);
					poil->set_NumOfSelected(poil->get_NumOfSelected() + 1);
					pHoPrev = std::move(pHo);
				}
				pHoPrev->get_rHo()->set_NextSelected(-1);
			}
		}
	}

	LOGE(_T("Restoring object selection done.\n"));
}

void Extension::Sub_RunPendingForeachFunc(const short oil, const std::shared_ptr<RunningFunction> &runningFunc)
{
	LOGI(_T("Sub_RunPendingForeachFunc executing on event %i for oil %hi (non-qual: %hi), function \"%s\".\n"),
		DarkEdif::GetCurrentFusionEventNum(this),
		oil, (short)(oil & 0x7FFF), runningFunc->funcTemplate->name.c_str());

	std::vector<RunObjectMultiPlatPtr> list;

	for (auto pqoi : DarkEdif::ObjectIterator(rhPtr, oil, DarkEdif::Selection::Implicit, false))
		list.push_back(pqoi);
	// else -1, just leave list empty
	// oil could be -1 if valid object type, but no instances - or invalid obj type, e.g. global events but no corresponding frame obj

	// No instances available
	if (list.empty())
	{
		LOGI(_T("Foreach loop \"%s\" not starting! No object instances available when action was run.\n"), runningFunc->funcTemplate->name.c_str());
		return;
	}

	std::vector<FusionSelectedObjectListCache> selObjList;
	evt_SaveSelectedObjects(selObjList, oil);

#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_INFO)
	size_t totalSel = 0;
	for (const auto& a : selObjList)
		totalSel += a.selectedObjects.size();
	LOGI(_T("Event %i starting foreach func \"%s\"; selection was saved with %zu context objects, %zu instances within. Will for-each over %zu instances.\n"),
		DarkEdif::GetCurrentFusionEventNum(this), runningFunc->funcTemplate->name.c_str(), selObjList.size(), totalSel, list.size());
#endif
	runningFunc->currentForeachOil = oil;
	HeaderObject* pHo;
	for (auto& runObj : list)
	{
		pHo = runObj->get_rHo();
		// One of the ExecuteFunctions may destroy obj we haven't looped through yet
		if ((pHo->get_Flags() & HeaderObjectFlags::Destroyed) != HeaderObjectFlags::None)
			continue;

		LOGI(_T("Event %i running current foreach func \"%s\" for object \"%s\", FV %i, num %hi [object list], oi %hi [oilist id].\n"),
			DarkEdif::GetCurrentFusionEventNum(this),
			runningFunc->funcTemplate->name.c_str(), pHo->get_OiList()->get_name(),
			pHo->GetFixedValue(), pHo->get_Number(), pHo->get_Oi());
		runningFunc->currentForeachObjFV = pHo->GetFixedValue();
		ExecuteFunction(runObj, runningFunc);
		// User cancelled the foreach loop or aborted it
		if (!runningFunc->foreachTriggering || !runningFunc->active)
		{
			if (!runningFunc->active)
				LOGI(_T("Aborted foreach func \"%s\" with reason \"%s\".\n"), runningFunc->funcTemplate->name.c_str(), runningFunc->abortReason.c_str());
			else
				LOGI(_T("Foreach func \"%s\" was cancelled.\n"), runningFunc->funcTemplate->name.c_str());
			evt_RestoreSelectedObjects(selObjList, true);
			return;
		}
		lastReturn = runningFunc->returnValue;
	}
	LOGI(_T("End of foreach func \"%s\", OI %i, restoring object selection.\n"), runningFunc->funcTemplate->name.c_str(), oil);
	evt_RestoreSelectedObjects(selObjList, true);
}

bool Extension::Sub_ParseParamValue(const TCHAR * cppFuncName, std::tstring valueTextToParse, const Param & paramExpected, const size_t paramIndex, Value &writeTo)
{
	// String type
	if (valueTextToParse[0] == '"')
	{
		if (paramExpected.type != Type::Any && paramExpected.type != Type::String)
		{
			return CreateErrorT("%s: Parameter \"%s\" (index %zu) has a string default value, but is set to type %s.",
				cppFuncName, paramExpected.name.c_str(), paramIndex, TypeToString(paramExpected.type)), false;
		}

		// Starts with " but no ending "; or is just a " by itself.
		if (valueTextToParse.back() != _T('"') || valueTextToParse.size() == 1)
		{
			return CreateErrorT("%s: Parameter \"%s\" (index %zu) has corrupt default value [%s]. Make sure your string parameters end with quotes and are escaped properly.",
				cppFuncName, paramExpected.name.c_str(), paramIndex, valueTextToParse.c_str()), false;
		}

		// remove starting and ending quotes
		valueTextToParse.erase(valueTextToParse.cbegin());
		valueTextToParse.erase(--valueTextToParse.cend());

		// Double-quote within the string, without a \ before it.
		// This turned up during a test and was incredibly annoying, as it silently merged 2nd and 3rd parameter as 2nd didn't correctly end with a ".
		// Example case: foo("bar",""yes")
		for (std::size_t i = valueTextToParse.find(_T('"')); i != std::tstring::npos; i = valueTextToParse.find(_T('"'), i + 1))
		{
			if (valueTextToParse[i] == '"' && (i == 0 || valueTextToParse[i - 1] != '\\'))
			{
				return CreateErrorT("%s: Parameter \"%s\" (index %zu) has unescaped double-quote at index %zu [%s%.20s%s].",
					cppFuncName, paramExpected.name.c_str(), paramIndex, i, i < 10 ? _T("...") : _T(""), valueTextToParse.c_str() + std::max(0, (int)i - 10), (int)valueTextToParse.size() - 20 > 0 ? _T("...") : _T("")), false;
			}
		}

		for (std::size_t i = valueTextToParse.find(_T('\\')); i != std::tstring::npos; i = valueTextToParse.find(_T('\\'), i + 1))
		{
			// ends with a '\' and nothing following... ignore it
			if (i == valueTextToParse.size())
				break;

			// escaped backslash, or quote: just remove the escaping
			if (valueTextToParse[i + 1] == _T('\\') || valueTextToParse[i + 1] == _T('"'))
				valueTextToParse.erase(i--, 1);
			// We remove \\r entirely
			else if (valueTextToParse[i + 1] == _T('r'))
				valueTextToParse.erase(i--, 2);
			// \\n is replaced with native newline
			else if (valueTextToParse[i + 1] == _T('n'))
			{
				valueTextToParse.erase(i, 2);
#ifdef _WIN32
				valueTextToParse.insert(i, _T("\r\n"sv));
#else
				valueTextToParse.insert(i, "\n"sv);
#endif
			}
			else if (valueTextToParse[i + 1] == _T('t'))
			{
				valueTextToParse[i + 1] = _T('\t');
				valueTextToParse.erase(i--, 1);
			}
		}

		writeTo.data.string = _tcsdup(valueTextToParse.c_str());
		writeTo.dataSize = (valueTextToParse.size() + 1) * sizeof(TCHAR);
		writeTo.type = Type::String;
		return true;
	}

	TCHAR* endPos;
	// expected float
	const bool endsWithF = valueTextToParse.back() == _T('f') || valueTextToParse.back() == _T('F');
	if (endsWithF || valueTextToParse.find(_T('.')) != std::tstring::npos)
	{
		if (paramExpected.type != Type::Any && paramExpected.type != Type::Float)
		{
			return CreateErrorT("%s: Parameter \"%s\" (index %zu) has a float default value, but is set to type %s.",
				cppFuncName, paramExpected.name.c_str(), paramIndex, TypeToString(paramExpected.type)), false;
		}

		if (endsWithF)
			valueTextToParse.erase(--valueTextToParse.cend());

		float f = _tcstof(valueTextToParse.c_str(), &endPos);
		// Returns 0 and endPos should be set to start pos if failed to convert
		// If out of bounds of float, should return ERANGE
		if (f == 0.0f && (endPos == valueTextToParse.c_str() || errno == ERANGE))
		{
			return CreateErrorT("%s: Parameter \"%s\" (index %zu) has corrupt default float value [%s].",
				cppFuncName, paramExpected.name.c_str(), paramIndex, valueTextToParse.c_str()), false;
		}

		writeTo.type = Type::Float;
		writeTo.data.decimal = f;
		writeTo.dataSize = sizeof(float);
		return true;
	}

	if (paramExpected.type != Type::Any && paramExpected.type != Type::Integer)
	{
		return CreateErrorT("%s: Parameter \"%s\" (index %zu) has an integer default value, but is set to type %s.",
			cppFuncName, paramExpected.name.c_str(), paramIndex, TypeToString(paramExpected.type)), false;
	}

	const long d = _tcstol(valueTextToParse.c_str(), &endPos, 0);
	// Returns 0 and endPos should be set to start pos if failed to convert
	// If out of bounds of long, should return ERANGE
	// If 64-bit long, make sure value within is in bounds of 32-bit long
	if ((d == 0 && (endPos == valueTextToParse.c_str() || errno == ERANGE)) ||
		(sizeof(d) == 8 && (d > INT32_MAX || d < INT32_MIN)))
	{
		return CreateErrorT("%s: Parameter \"%s\" (index %zu) has corrupt default integer value [%s].",
			cppFuncName, paramExpected.name.c_str(), paramIndex, valueTextToParse.c_str()), false;
	}

	writeTo.type = Type::Integer;
	writeTo.data.integer = (int)d;
	writeTo.dataSize = sizeof(int);
	return true;
}
