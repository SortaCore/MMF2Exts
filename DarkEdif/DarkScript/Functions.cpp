#include "Common.h"
#include <cstdlib>

#define CreateErrorExp(Ret, Error, ...) { CreateErrorT(Error, ##__VA_ARGS__); return Ret; }
#define CreateErrorExpOpt(Opt, Ret, Error, ...) { if (Opt) CreateErrorT(Error, ##__VA_ARGS__); return Ret; }

void Extension::CreateError(PrintFHintInside const TCHAR * format, ...)
{
	va_list v;
	va_start(v, format);

	static TCHAR backRAM[2048];

	try {
		if (_vstprintf_s(backRAM, std::size(backRAM), format, v) <= 0)
			DarkEdif::MsgBox::Error(_T("Error making error"), _T("Who watches the watchmen?\nError making error: %d"), errno);
	}
	catch (...) {
		DarkEdif::MsgBox::Error(_T("Error making error"), _T("Who watches the watchmen?\nError making error: %d"), errno);
	}

	va_end(v);

	LOGI(_T("DarkScript error: %s.\n"), backRAM);

	if (!curError.empty())
	{
		DarkEdif::MsgBox::Error(_T("Stack memory crash prevented"),
			_T("A recursive error occurred while handling a DarkScript error. Stack memory overflow bug prevented.\n"
				"Original error:\n%s\nNew error while handling original error:\n%s"),
			curError.c_str(), backRAM);
		return; // don't generate event
	}
	curError = backRAM; // will dup memory

	std::vector<FusionSelectedObjectListCache> list;
	evt_SaveSelectedObjects(list);
	Runtime.GenerateEvent(0);
	evt_RestoreSelectedObjects(list, true);

	curError.clear(); // to prevent recursion
}

static char errorStr[2048];

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
			CreateErrorExp(std::shared_ptr<Extension::FunctionTemplate>(), "%s: no function running and not a func template loop, so a function template name must be passed.", cppFuncName + sizeof("Extension::") - 1);
		return globals->runningFuncs.back()->funcTemplate;
	}

	// Match by name
	const std::tstring nameL(ToLower(funcNameOrBlank));
	const auto res = std::find_if(globals->functionTemplates.begin(), globals->functionTemplates.end(),
		[&](const std::shared_ptr<FunctionTemplate> & f) { return f->nameL == nameL;
	});
	if (res == globals->functionTemplates.end())
		CreateErrorExp(std::shared_ptr<Extension::FunctionTemplate>(), "%s: couldn't find a function template with name %s.", funcNameOrBlank);
	return *res;
}
std::shared_ptr<Extension::RunningFunction> Extension::Sub_GetRunningFunc(const TCHAR* cppFuncName, const TCHAR* funcNameOrBlank)
{
	if (globals->runningFuncs.empty())
		CreateErrorExp(std::shared_ptr<Extension::RunningFunction>(), "Use of %s without a function active.", cppFuncName + sizeof("Extension::") - 1);
	if (funcNameOrBlank[0] == _T('\0'))
		return globals->runningFuncs.back();

	// Match by name
	std::tstring nameL(ToLower(funcNameOrBlank));
	auto res = std::find_if(globals->runningFuncs.rbegin(), globals->runningFuncs.rend(),
		[&](const std::shared_ptr<RunningFunction>& f) { return !_tcscmp(f->funcTemplate->nameL.c_str(), nameL.c_str());
		});
	if (res == globals->runningFuncs.rend())
		CreateErrorExp(std::shared_ptr<Extension::RunningFunction>(), "%s: couldn't find a running function with name %s.", cppFuncName + sizeof("Extension::") - 1, funcNameOrBlank);
	return *res;
}

Extension::Value * Extension::Sub_CheckParamAvail(const TCHAR * cppFuncName, int paramNum)
{
	if (paramNum < 0)
		CreateErrorExp(NULL, "%s: param index below 0. (%i)", cppFuncName, paramNum);

	auto rf = Sub_GetRunningFunc(cppFuncName, _T(""));
	if (!rf)
		return NULL;

	if (rf->funcTemplate->params.size() < (size_t)paramNum)
		CreateErrorExp(NULL, "%s: param index out of range (0 < %i < %zu).",
			cppFuncName, paramNum, rf->funcTemplate->params.size());

	return &rf->paramValues[paramNum];
}
Extension::Value * Extension::Sub_CheckScopedVarAvail(const TCHAR * cppFuncName, const TCHAR * scopedVarName, Expected shouldBeParam, bool makeError, const Param ** paramTo /* = nullptr */)
{
	if (scopedVarName[0] == _T('\0'))
		CreateErrorExpOpt(makeError, NULL, "%s: param/scoped var name is blank.", cppFuncName);

	auto rf = Sub_GetRunningFunc(cppFuncName, _T(""));
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
			CreateErrorExpOpt(makeError, NULL, "%s: scoped var name \"%s\" not found.", cppFuncName, scopedVarName);
	}
	// shouldBeParam is either Either or Always, Never would've returned in ^ already

	const auto param = std::find_if(rf->funcTemplate->params.begin(), rf->funcTemplate->params.end(),
		[&](const Extension::Param & a) { return a.nameL == strL; }
	);
	if (param == rf->funcTemplate->params.end())
		CreateErrorExpOpt(makeError, NULL, "%s: param%s name \"%s\" not found. Anonymous functions use \"a\", \"b\" for parameters.",
			cppFuncName, shouldBeParam == Expected::Never ? _T("") : _T("/scoped var"), scopedVarName);
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

	if (f->params.size() < (size_t)paramNum)
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
		[&](Extension::Param & a) { return !_tcscmp(a.nameL.c_str(), strL.c_str()); }
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
		return Sub_ConvertToString(*(int *)&val.data).c_str();
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

#define lastNonDummyFunc (CurLang["Expressions"].u.array.length)
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

enum _Enum_is_bitflag_ Flags {
	None = 0b00,
	KeepObjSelection = 0b01,
	Repeat = 0b10,
	Both = KeepObjSelection + Repeat,
};
enum_class_is_a_bitmask(Flags);

// pow() in constexpr format. Ignore last parameter.
constexpr int64_t ipow(int64_t base, int exp, int64_t result = 1)
{
	return exp < 1 ? result : ipow(base*base, exp / 2, (exp % 2) ? result * base : result);
}
constexpr int64_t duet(int64_t base, int exp)
{
	int64_t sum = 1;
	for (int i = 0; i < exp; i++)
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
		numParamsToAllow)));
constexpr size_t numFuncVariantsToGenerateCF25 =
// Variants of return types...
(((Flags::Both + 1) * ((int)Extension::Type::NumCallableReturnTypes)) *
	// multiplied by variants of parameter types...
	((int)duet((int)Extension::Type::NumCallableParamTypes,
		// multiplied by max number of parameters
		6)));

#if EditorBuild

static void GenerateExpressionFuncFor(const int inputID)
{
	// inputID is func ID, - 40 not needed.

	// First, get repeat/delayed by bitmask of 4.

	const int NumVariants = Flags::Both + 1;
	int funcID = inputID;
	Flags flags = (Flags)(funcID & Flags::Both);
	funcID = (funcID / NumVariants); // remove last two bits (Flags) and shift remaining down in their place

	//for (int i = 0; i < 30; i++)
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

	auto GetParam = [&](int id) {
		assert(id < last->NumOfParams);
		if (last->Parameter[id].ep == ExpParams::String)
			return Extension::Type::String;
		else if (last->Parameter[id].ep == ExpParams::Integer)
			return last->FloatFlags & (1 << id) ? Extension::Type::Float : Extension::Type::Integer;
		else
		{
			MessageBoxA(NULL, "Parameter type not recognised", "", MB_OK);
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
		for (size_t i = 1; i < (size_t)last->NumOfParams; i++)
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
	for (size_t i = paramTypes.size(); i < (size_t)(last->NumOfParams + (needNewParam ? 1 : 0) + (flags & Flags::Repeat ? 1 : 0)); i++)
	{
		// too many params?
		if (intermediate < 0)
			DebugBreak();
		// While there's 4 valid parameter types (Any included),
		// the function can only be called with 3.
		paramTypes.push_back((Extension::Type)((intermediate % (int)Extension::Type::NumCallableParamTypes) + 1));

		if (paramTypes[i] < Extension::Type::Integer)
		{
			// miscalculated type
			DebugBreak();
			break;
		}
		intermediate = (intermediate / (int)Extension::Type::NumCallableParamTypes) - 1;
	}

	if (intermediate > 0)
		DebugBreak(); // missed a param?

	// Parameters checked; allocate new info
	ACEInfo* const ExpInfo = Edif::ACEInfoAlloc(paramTypes.size());

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

#if _DEBUG
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

#if _DEBUG
	if (false) // (inputID % 4 == 0)
	{
		sprintf_s(errorStr, "Outputted dynamic function, expression ID %i, funcID %i.\n%s", ExpInfo->ID, inputID, ret.c_str());
		OutputDebugStringA(errorStr);
	}
#endif
}

void GenerateDummyExpression(int inputID)
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
int Extension::GetNumExpressions()
{
	// CF2.5 b294.8+ has modification to allow more functions
	// +1 for 0-based ID to 1-based size
	assert(numFuncVariantsToGenerate > 0);
	return lastNonFuncID + numFuncVariantsToGenerate + 1;
}
void Extension::AutoGenerateExpressions()
{
	// CF2.5 b294.8+ has modification to allow more functions
	// Since all the expression infos are read back, we don't need to worry about undercutting later

	if (numFuncVariantsToGenerate == 0)
	{
		const int build = (Edif::SDK->mV->GetVersion() & MMFBUILD_MASK);
		const bool useMoreExpressions = DarkEdif::IsFusion25 &&
			(build >= 295 || (build == 294 && getFusionSubBuild() >= 8));
		numFuncVariantsToGenerate = (useMoreExpressions ? numFuncVariantsToGenerateCF25 : numFuncVariantsToGenerateMMF2);
	}

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

	// 1. Find end of Expressions
	assert(Edif::SDK->ExpressionInfos.size() <= lastNonFuncID);

	size_t i = Edif::SDK->ExpressionInfos.size();

	// 2. Add safety buffer (50 expressions?) pointing to dummy function
	// 21 used, reserve 20, intermediate index 40
	while (Edif::SDK->ExpressionInfos.size() <= lastNonFuncID)
	{
#ifdef _WIN32
		Edif::SDK->ExpressionJumps[i++] = (void *)Edif::ExpressionJump;
#endif
		GenerateDummyExpression(Edif::SDK->ExpressionInfos.size());
		Edif::SDK->ExpressionFunctions.push_back(0);
	}

	// 3. Generate IDs and infos following CreateNewExpressionInfo
	// formula is in excel
	
	while (Edif::SDK->ExpressionInfos.size() - 1 < lastNonFuncID + numFuncVariantsToGenerate)
	{
#ifdef _WIN32
		Edif::SDK->ExpressionJumps[i++] = (void *)Edif::ExpressionJump;
#endif
		GenerateExpressionFuncFor(Edif::SDK->ExpressionInfos.size() - 1 - lastNonFuncID);
		Edif::SDK->ExpressionFunctions.push_back(Edif::MemberFunctionPointer(&Extension::VariableFunction));
	}
}

void FusionAPI GetExpressionParam(mv* mV, short code, short param, TCHAR* strBuf, short maxLen)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
	{
		if ((size_t)code < lastNonDummyFunc)
			Edif::ConvertAndCopyString(strBuf, CurLang["Expressions"][code]["Parameters"][param][1], maxLen);
		else if ((size_t)code <= lastNonFuncID)
		{
			DarkEdif::MsgBox::Error(_T("Shouldn't happen"), _T("Should never happen. Param requested for dummy function."));
			Edif::ConvertAndCopyString(strBuf, "COUGH", maxLen);
		}
		else
		{
			int funcID = code - lastNonFuncID - 1;

			sprintf_s(errorStr, "Outputting param %i for dynamic function, expression ID %i, funcID %i.\n", param, code, funcID);
			OutputDebugStringA(errorStr);

			ACEInfo& exp = *Edif::SDK->ExpressionInfos[code];
			if (exp.NumOfParams < param + 1)
			{
				DarkEdif::MsgBox::Error(_T("Shouldn't happen"), _T("Param %i missing for function ID %i (expr ID %i)."), param, funcID, code);
				Edif::ConvertAndCopyString(strBuf, "Wowie", maxLen);
				return;
			}

			switch (exp.Parameter[param].ep)
			{
			case ExpParams::Float: // also integer
				if ((exp.FloatFlags & (1 << param)) != 0)
					sprintf_s(errorStr, "Float%i", param + 1);
				else // integer 
				{
					// Repeat is always 2nd param if just Repeat
					if (param == 1 && (funcID & Flags::Repeat) == Flags::Repeat)
						sprintf_s(errorStr, "Number of runs (1+)");
					else
						sprintf_s(errorStr, "Integer%i", param + 1);
				}
				Edif::ConvertAndCopyString(strBuf, errorStr, maxLen);
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
		std::string Return(CurLang["Expressions"][code]["Title"]);
		if (Return.back() != '(')
			Return.push_back('(');
		Edif::ConvertAndCopyString(strBuf, Return.c_str(), maxLen);
	}
	else if ((size_t)code <= lastNonFuncID)
	{
		Edif::ConvertAndCopyString(strBuf, "DummyFunc(", maxLen);
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

		for (; i < (size_t)exp.NumOfParams; i++)
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

	/* TODO : if running for parameter for one of the delayed function actions, record parameters but don't run.
		Don't allow anything but expression functions; no nesting. */

	// otherwise not variable func
	assert(exp.ID > lastNonFuncID && exp.ID <= lastNonFuncID + (short)numFuncVariantsToGenerate);
	const event2* callingAction = rhPtr->rh4.ActionStart;
	int actID = callingAction ? callingAction->evtNum - 80 : -1;
	bool runImmediately = true, isVoidRun = false;
	// We don't run simply if we have the Foreach or Delayed action being executed.
	if (actID >= 19 && actID <= 26 && callingAction->evtOi == rdPtr->rHo.Oi)
	{
		// dummy func: run immediately, but don't reset actID, we use it later in Sub_GetLocation()
		if (actID >= 19 && actID <= 20)
			LOGI(_T("Called for Dummy action! Action ID is %i.\n"), actID);
		else
		{
			runImmediately = false;
			LOGI(_T("Called for Foreach or Delayed action! Action ID is %i.\n"), actID);
		}
		isVoidRun = true;
	}
	else
	{
		actID = -1;
		LOGI(_T("NOT called from Foreach/Delayed action! Oi is %i, action ID is %i.\n"), callingAction ? callingAction->evtOi : -1, actID);
	}

	int funcID = exp.ID - lastNonFuncID - 1;
	std::tstring nameL(ToLower(funcName));

	auto funcTemplateIt = std::find_if(globals->functionTemplates.begin(), globals->functionTemplates.end(),
		[&](const auto &f) { return f->nameL == nameL; });

	size_t expParamIndex = 1; // skip func name (index 0), we already read it
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
#define CreateError2V(...) \
	{ \
		CreateErrorT(##__VA_ARGS__); \
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
		for (size_t i = expParamIndex; i < (size_t)exp.NumOfParams; i++, ++name[0])
		{
			// Note the ++name[0] in for(;;><), gives variable names a, b, c
			Type type = exp.Parameter[i].ep == ExpParams::String ? Type::String :
				exp.Parameter[i].ep == ExpParams::Integer ?
				(exp.FloatFlags & (1 << i) ? Type::Float : Type::Integer) : Type::Any;
			funcTemplate->params.push_back(Param(name, type));
		}
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

		if (!runImmediately && funcTemplate->delaying == Expected::Never)
			CreateError2V("Can't call function %s delayed, template does not allow delaying.", funcName);
		if (runImmediately && funcTemplate->delaying == Expected::Always)
			CreateError2V("Can't call function %s without delaying, template expects delaying.", funcName);

		if ((!funcTemplate->recursiveAllowed || !preventAllRecursion) &&
			std::find_if(globals->runningFuncs.begin(), globals->runningFuncs.end(),
				[&](std::shared_ptr<RunningFunction>& rf) { return rf->funcTemplate == funcTemplate; }
			) != globals->runningFuncs.end())
		{
			CreateError2V("Can't call function %s recursively, %s.", funcName,
				!preventAllRecursion ? _T("recursion has been prevented globally") :_T("template does not allow recursing")
			);
		}

		// Too many parameters (ignore func name and repeat count)
		if (((size_t)exp.NumOfParams - expParamIndex) > funcTemplate->params.size())
			CreateError2V("Can't call function %s with %hi parameters, template expects up to %zu parameters.", funcTemplate->name.c_str(), exp.NumOfParams, funcTemplate->params.size());
	}


	// Running while current func is aborting - that's confusing behaviour
	if (!globals->runningFuncs.empty() && !globals->runningFuncs.back()->active)
	{
		CreateError2V("Can't run function \"%s\", current function \"%s\" is aborted. Abort must be handled before functions can be run.",
			funcName, globals->runningFuncs.back()->funcTemplate->name.c_str());
	}


	auto newFunc = std::make_shared<RunningFunction>(funcTemplate, runImmediately, repeatTimes - 1);
	newFunc->keepObjectSelection = funcID & Flags::KeepObjSelection;
	newFunc->isVoidRun = isVoidRun;

	size_t numNotInParamsVector = expParamIndex;
	size_t numPassedExpFuncParams = exp.NumOfParams - numNotInParamsVector; // ignore func name and num repeats

	newFunc->numPassedParams = numPassedExpFuncParams;

	// User passed too many parameters
	if (funcTemplate->params.size() < numPassedExpFuncParams)
		CreateError2V("%s: called with %i parameters, but only expects %i.", funcName, numPassedExpFuncParams, funcTemplate->params.size());

	// don't confuse expParamIndex (A/C/E, will include funcName, may include numRepeats)
	// vs. paramIndex (index of paramValues)
	size_t paramIndex = 0;

	for (; paramIndex < numPassedExpFuncParams; paramIndex++)
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
					CreateError2V("Function %s: Parameter %i should have been an %s, but was called with a %s instead.",
						funcName, paramIndex, TypeToString(paramTypeInTemplate), "float");
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
					CreateError2V("Function %s: Parameter %i should have been an %s, but was called with a %s instead.",
						funcName, paramIndex, TypeToString(paramTypeInTemplate), "integer");
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
				CreateError2V("Function %s: Parameter %i should have been an %s, but was called with a %s instead.",
					funcName, paramIndex, TypeToString(paramTypeInTemplate), "string");
			}
			break;
		default:
			DarkEdif::MsgBox::Error(_T("Param not recognised"), _T("Param type for param index %zu not recognised for function ID %zu (expr ID %hu)."),
				paramIndex, (size_t)exp.ID - lastNonDummyFunc - 1, exp.ID);
			return DummyReturn;
		}
	}

	// User didn't pass these, but template expects them; load from param defaults
	for (; paramIndex < funcTemplate->params.size(); paramIndex++)
	{
		// No default, and user didn't pass one: no good.
		if (funcTemplate->params[paramIndex].type != Extension::Type::Any &&
			funcTemplate->params[paramIndex].defaultVal.type == Extension::Type::Any)
		{
			CreateError2V("%s: parameter %s (index %i) was not passed, but must be supplied.",
				funcName, funcTemplate->params[paramIndex].name.c_str(), paramIndex);
		}
		else
		{
			newFunc->paramValues[paramIndex] = newFunc->funcTemplate->params[paramIndex].defaultVal;
		}
	}

	if (newFunc->active)
	{
		/*
		const TCHAR* save = rhPtr->rh4.CurrentFastLoop;
		unsigned char actionLoop = rhPtr->rh2.ActionLoop;				// Flag boucle
		int actionLoopCount = rhPtr->rh2.ActionLoopCount;			// Numero de boucle d'actions
		unsigned char actionOn = rhPtr->rh2.ActionOn;
		auto eventGroup = rhPtr->EventGroup;
		auto posOnLoop = rhPtr->rh4.posOnLoop;
		LOGI(_T("Note: %s expression starting - cur fast loop \"%s\", actionLoop %hhu, actionLoopCount %d, actOn %hhu, evg %p, pOL=%p.\n"),
			funcName, save, actionLoop, actionLoopCount, actionOn, eventGroup, posOnLoop);
			*/
		newFunc->runLocation = Sub_GetLocation(actID);
		long l = ExecuteFunction(NULL, newFunc);
		newFunc->runLocation.clear();
		/*
		save = rhPtr->rh4.CurrentFastLoop;
		actionLoop = rhPtr->rh2.ActionLoop;				// Flag boucle
		actionLoopCount = rhPtr->rh2.ActionLoopCount;			// Numero de boucle d'actions
		actionOn = rhPtr->rh2.ActionOn;
		eventGroup = rhPtr->EventGroup;
		posOnLoop = rhPtr->rh4.posOnLoop;
		LOGI(_T("Note: %s expression finished - cur fast loop \"%s\", actionLoop %hhu, actionLoopCount %d, actOn %hhu, evg %p, pOL=%p\n"),
			funcName, save, actionLoop, actionLoopCount, actionOn, eventGroup, posOnLoop); */
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
		throw std::exception("Wrong call");


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
		void* exp = ext->rhPtr->rh4.ActionStart;
		if (exp == nullptr)
			return _T("a condition"s);
		char* cur = ((char*)ext->rhPtr->EventGroup) + sizeof(*ext->rhPtr->EventGroup);
		//cur = cur + -ext->rhPtr->EventGroup->evgSize;
		size_t i = 0;
		for (; i < ext->rhPtr->EventGroup->evgNCond; i++)
			cur = cur + ((event2*)cur)->evtSize;
		short lastOI = ((event2*)cur)->evtOi;
		std::tstringstream str;
		for (i = 0; i < ext->rhPtr->EventGroup->evgNAct; i++) {
			if (cur == exp) {
				// We're using 1-based for action index, so increment i
				++i;

				// If it's DarkScript, try not to say "DarkScript action running DarkScript".
				// We check the name, in case there's multiple DarkScripts named differently by user.
				const std::tstring_view name = ext->Runtime.ObjectSelection.GetOILFromOI(lastOI)->name;

				if (ext->rdPtr->rHo.Oi != lastOI || name != _T("DarkScript"sv))
					str << name << _T("'s action (index "sv) << i << _T(')');
				else
					str << _T("action index "sv) << i;
				return str.str();
			}
			cur = cur + ((event2*)cur)->evtSize;
			lastOI = ((event2*)cur)->evtOi;
		}
		if (cur == exp)
		{
			str << ext->Runtime.ObjectSelection.GetOILFromOI(lastOI)->name << _T("'s action (index "sv) << i + 1 << _T(')');
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

long Extension::ExecuteFunction(HeaderObject* objToRunOn, std::shared_ptr<RunningFunction> rf)
{
	// If running on another ext like subapp, we want to use its globals.
	auto globalsRunningOn = rf->funcTemplate->ext->globals;

	// TODO: Can optimize this by adding "isrunning" active to template
	if (preventAllRecursion || !rf->funcTemplate->recursiveAllowed)
	{
		auto olderIt = std::find_if(globalsRunningOn->runningFuncs.crbegin(), globalsRunningOn->runningFuncs.crend(),
			[&](const auto& f) { return Sub_FunctionMatches(rf, f); }
		);
		if (olderIt != globalsRunningOn->runningFuncs.crend())
		{
			rf->abortReason = _T("Aborted from DarkScript recursion error."sv);
			CreateErrorT("Running a function recursively, when not allowed. Older run was from %s; current, aborted run is from %s. Aborting.",
				(*olderIt)->runLocation.c_str(), rf->runLocation.c_str());
		}
	}

	rf->active = true;

	int objFixedValue = -1;
	size_t numScopedVarsBeforeCall = SIZE_MAX;
	if (objToRunOn)
		objFixedValue = (objToRunOn->CreationId << 16) + objToRunOn->Number;
	else
	{
		globalsRunningOn->runningFuncs.push_back(rf);
		numScopedVarsBeforeCall = globalsRunningOn->scopedVars.size();
		for (auto& sv : rf->funcTemplate->scopedVarOnStart)
		{
			if (sv.recursiveOverride || 1 == std::count_if(globalsRunningOn->runningFuncs.cbegin(), globalsRunningOn->runningFuncs.cend(),
				[&](const auto& f) { return Sub_FunctionMatches(rf, f); }))
			{
				globalsRunningOn->scopedVars.push_back(sv);
				globalsRunningOn->scopedVars.back().level = globals->runningFuncs.size();
			}
		}
	}

	// Set up subapp ext for running cross-frame, and in case they do a cross-frame themselves, we'll store what they're using
	Extension* orig = rf->funcTemplate->ext->errorExt;
	rf->funcTemplate->ext->errorExt = this;

	if (objToRunOn)
		LOGV(_T("Running On Foreach function \"%s\", passing object \"%s\", FV %i.\n"), rf->funcTemplate->name.c_str(), objToRunOn->OiList->name, rf->currentForeachObjFV);
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
					CreateErrorT("Function \"%s\" was not handled by any On Function \"%s\" for each %s events.", rf->funcTemplate->name.c_str(),
						rf->funcTemplate->name.c_str(), rf->funcTemplate->name.c_str(),
						globals->runningFuncs.back()->currentForeachOil < 0 ? _T("(qualifier)") : objToRunOn->OiList->name);
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
	} while (rf->active && rf->numRepeats > ++rf->index);

	// We need return value, no default return value, but we're returning with no return value set... uh oh
	if (!rf->isVoidRun && rf->returnValue.dataSize == 0 && rf->funcTemplate->defaultReturnValue.dataSize == 0)
	{
		if (objToRunOn)
		{
			CreateErrorT("Function \"%s\" has no default return value, and no return value was set by any On Function \"%s\" for each %s events.",
				rf->funcTemplate->name.c_str(), rf->funcTemplate->name.c_str(),
				rf->currentForeachOil < 0 ? _T("(qualifier)") : objToRunOn->OiList->name);
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
	if (rf->returnValue.type != Extension::Type::String)
		return (long)rf->returnValue.data.integer;

	// Clone string
	const TCHAR * tc = rf->returnValue.data.string;
	// assert(tc); // shouldn't have it as null under any scenario
	tc = tc ? tc : _T("");
	return (long)Runtime.CopyString(tc);
}

static void AddToList(Extension * ext, short oil, std::vector<HeaderObject*> & writeTo)
{
	// regular object
	assert(oil >= 0);

	auto poil = (objInfoList*)(((char*)ext->rhPtr->OiList) + ext->Runtime.ObjectSelection.oiListItemSize * oil);

	// Object's selected instance list wasn't set by conditions for this event.
	// It would be good if we could do Runtime.ObjectSelection.SelectAll(),
	// but actions can't modify selection, so we'll not bother.
	const bool selectAll = (poil->EventCount != ext->rhPtr->rh2.EventCount);

	short num = selectAll ? poil->Object : poil->ListSelected, lastNum = -1;
	// Iterate over list (until no other objects are found)
	while (num >= 0)
	{
		// Get the object instance
		auto pObl = ext->rhPtr->ObjectList + num;
		HeaderObject* pHo = pObl->oblOffset;
		if (pHo == nullptr)
			break; // we hit end of list

		// Make sure the obj wasn't already destroyed (e.g. Disappearing animation)
		if ((pHo->Flags & HeaderObjectFlags::Destroyed) == HeaderObjectFlags::None)
			writeTo.push_back(pHo);
		// Jump to next instance in list
		num = selectAll ? pHo->NumNext : pHo->NextSelected;
	}
}

// Save object selection
void Extension::evt_SaveSelectedObjects(std::vector<FusionSelectedObjectListCache>& selectedObjects)
{
	bool wasFound = false;
	for (int i = 0; i < rhPtr->NumberOi; ++i)
	{
		objInfoList* poil = (objInfoList*)(((char*)rhPtr->OiList) + Runtime.ObjectSelection.oiListItemSize * i);

		// Skip our ext, it'll always appear in selection because otherwise, how is this code right here in our ext running?
		if (poil->Oi == rhPtr->OiList->Oi)
			continue;

		// Selected?
		if (poil->EventCount == rhPtr->rh2.EventCount)
		{
			// Already in the list? 
			int j;
			for (j = 0; j < (int)selectedObjects.size(); j++)
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
				selectedObjects.push_back(sel);
				pSel = &selectedObjects[selectedObjects.size() - 1];
			}

			pSel->poil = poil;

			// Store selected objects
			short num = poil->ListSelected;
			while (num >= 0)
			{
				objectsList* pObl = rhPtr->ObjectList + num;
				HeaderObject* pHoFound = pObl->oblOffset;
				if (pHoFound == &rdPtr->rHo)
					wasFound = true;
				if (pHoFound == NULL)
					break;
				pSel->selectedObjects.push_back(num);
				num = pHoFound->NextSelected;
			}
		}
	}
}

// Restore objects selection
void Extension::evt_RestoreSelectedObjects(const std::vector<FusionSelectedObjectListCache>& selectedObjects, bool unselectAllExisting)
{
	// Unselect all objects
	if (unselectAllExisting)
		for (objInfoList* poil = rhPtr->OiList; poil->Oi != -1;
			poil = (objInfoList*)(((char*)poil) + Runtime.ObjectSelection.oiListItemSize))
	{
		// Skip our ext, it'll always appear in selection because otherwise, how is this code right here in our ext running?
		if (poil->Oi == rhPtr->OiList->Oi)
			continue;
		poil->EventCount = -1;
		//	poil->EventCountOR = -1;
		poil->NumOfSelected = 0;
		int listSel = poil->ListSelected;
		poil->ListSelected = -1;

		/*
		if (listSel < 0)
			continue;
		// Store selected objects
		short objectsListIndex = listSel;
		while (objectsListIndex >= 0)
		{
			objectsList* pObl = rhPtr->ObjectList + objectsListIndex;
			HeaderObject* pHoFound = pObl->oblOffset;
			if (pHoFound == NULL)
				break;
			pHoFound->SelectedInOR = 0;
			objectsListIndex = pHoFound->NextSelected;
		}*/
		LOGI(_T("Zero'd poil->Oi = %d, name = %hs.\n"), poil->Oi, poil->name);
	}

	for (int i = 0; i < (int)selectedObjects.size(); i++)
	{
		const FusionSelectedObjectListCache& sel = selectedObjects[i];
		objInfoList* poil = sel.poil;

		poil->EventCount = rhPtr->rh2.EventCount;
		//	poil->EventCountOR = rhPtr->rh4.EventCountOR;
		poil->ListSelected = -1;
		poil->NumOfSelected = 0;
		if (sel.selectedObjects.size() > 0)
		{
			objectsList* pObl = rhPtr->ObjectList + sel.selectedObjects[0];
			HeaderObject* pHoPrev = pObl->oblOffset;
			if (pHoPrev != NULL)
			{
				poil->ListSelected = sel.selectedObjects[0];
				//pHoPrev->SelectedInOR = 1;
				poil->NumOfSelected++;
				for (int j = 1; j < (int)sel.selectedObjects.size(); j++)
				{
					short num = sel.selectedObjects[j];
					pObl = rhPtr->ObjectList + num;
					HeaderObject* pHo = pObl->oblOffset;
					//pHo->SelectedInOR = 1;
					if (pHo == NULL)
						break;
					pHoPrev->NextSelected = num;
					poil->NumOfSelected++;
					pHoPrev = pHo;
				}
				pHoPrev->NextSelected = -1;
			}
		}
	}
}

void Extension::Sub_RunPendingForeachFunc(const short oil, std::shared_ptr<RunningFunction> runningFunc)
{
	// The issue is when manually looping, we only have the first instance of selected objects;
	// we don't know if a qualifier was selected.
	// newFunc->foreachOIL = oil;

	LOGI(_T("Sub_RunPendingForeachFunc executing for oil %hi, function \"%s\".\n"), oil, runningFunc->funcTemplate->name.c_str());

	std::vector<HeaderObject*> list;

	// regular object
	if (oil >= 0)
		AddToList(this, oil, list);
	// qualifier
	else if (oil != -1)
	{
		//Runtime.ObjectSelection.GetQualifierFromOI(oil);
		for (auto pqoi = (qualToOi*)((std::uint8_t*)rhPtr->QualToOiList + (oil & 0x7FFF));
			pqoi->OiList >= 0;
			pqoi = (qualToOi*)((std::uint8_t*)pqoi + 4))
		{
			AddToList(this, pqoi->OiList, list);
		}
	}
	// else -1, just leave list empty
	// oil could be -1 if valid object type, but no instances - or invalid obj type, e.g. global events but no corresponding frame obj

	// No instances available
	if (list.empty())
	{
		LOGI(_T("Foreach loop \"%s\" not starting! No object instances available when action was run.\n"), runningFunc->funcTemplate->name.c_str());
		return;
	}

	std::vector<FusionSelectedObjectListCache> selObjList;
	evt_SaveSelectedObjects(selObjList);

	LOGI(_T("Foreach func \"%s\" starting; has %i object instances selected.\n"), runningFunc->funcTemplate->name.c_str(), list.size());
	runningFunc->currentForeachOil = oil;
	for (auto pHo : list)
	{
		// One of the ExecuteFunctions may destroy some obj
		if ((pHo->Flags & HeaderObjectFlags::Destroyed) == HeaderObjectFlags::None)
		{
			LOGI(_T("Foreach func \"%s\" for object \"%s\", FV %i.\n"), runningFunc->funcTemplate->name.c_str(), pHo->OiList->name,
				(pHo->CreationId << 16) + pHo->Number);
			runningFunc->currentForeachObjFV = (pHo->CreationId << 16) + pHo->Number;
			ExecuteFunction(pHo, runningFunc);
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
	}
	evt_RestoreSelectedObjects(selObjList, true);
	LOGI(_T("End of foreach func \"%s\", OI %i.\n"), runningFunc->funcTemplate->name.c_str(), oil);
}

bool Extension::Sub_ParseParamValue(const TCHAR * cppFuncName, std::tstring& valueTextToParse, const Param & paramExpected, const size_t paramIndex, Value &writeTo)
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
		for (size_t i = valueTextToParse.find(_T('\\')); i != std::tstring::npos; i = valueTextToParse.find(_T('\\'), i + 1))
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
				defaultVal.insert(i, "\n"sv);
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
