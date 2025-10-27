#include "Common.hpp"
#include "Extension.hpp"
#include <atomic>
#include <math.h>

#ifdef _WIN32
extern HINSTANCE hInstLib;
// For props that alter other props with some expression
#include <regex>
#elif defined (__APPLE__)
#include "MMF2Lib/CFontInfo.h"
#endif

#if EditorBuild

static const _json_value * StoredCurrentLanguage = &json_value_none;
static std::tstring StoredCurrentLanguageName = _T("<default>"s);

static const _json_value * DefaultLanguageIndex()
{
	// Misuse of function; called before ::SDK is valid
	if (!Edif::SDK)
	{
		DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("Premature function call!\n  Called DefaultLanguageIndex() before ::SDK was a valid pointer."));
		return &json_value_none;
	}

	for (unsigned int i = 0; i < Edif::SDK->json.u.object.length; ++i)
	{
		auto& lang = Edif::SDK->json.u.object.values[i];
		if ((*lang.value).type == json_object
			&& (*lang.value)["About"sv]["Name"sv].type == json_string)
		{
			StoredCurrentLanguageName = DarkEdif::UTF8ToTString(lang.name);
			return lang.value;
		}
	}

	// Fallback on last object
	auto& lang = Edif::SDK->json.u.object.values[Edif::SDK->json.u.object.length - 1];
	StoredCurrentLanguageName = DarkEdif::UTF8ToTString(lang.name);
	return lang.value;
}

const json_value & DarkEdif::JSON::LanguageJSON()
{
	if (StoredCurrentLanguage->type != json_none)
		return *StoredCurrentLanguage;

	// Not in editor mode; the fancy language features are not necessary
	if (DarkEdif::RunMode == DarkEdif::MFXRunMode::RunApplication || DarkEdif::RunMode == DarkEdif::MFXRunMode::BuiltEXE)
		return *(StoredCurrentLanguage = DefaultLanguageIndex());

	std::string langList = DarkEdif::GetIniSetting("Languages"sv);
	if (langList.empty())
		return *DefaultLanguageIndex();

	langList.append("\r");

	size_t readIndex = 0;
	// Read string until newline. Expect ";" or "; "-delimited list of languages.
	while (langList.find_first_of(";\r", readIndex) != std::string::npos) // Is the semi-colon after an end-of-line character?
	{
		// Read individual token
		std::string langItem(langList.substr(readIndex, langList.find_first_of(";\r", readIndex)-readIndex));

		// If languages are separated by "; " not ";"
		if (langItem.front() == ' ')
		{
			++readIndex;
			langItem.erase(langItem.begin());
		}

		// langItem matched, get index of language in JSON
		for (unsigned int i = 0; i < Edif::SDK->json.u.object.length; ++i)
		{
			// Return index
			if ((*Edif::SDK->json.u.object.values[i].value).type == json_object &&
				!_stricmp(Edif::SDK->json.u.object.values[i].name, langItem.c_str()))
			{
				StoredCurrentLanguage = Edif::SDK->json.u.object.values[i].value;
				StoredCurrentLanguageName = DarkEdif::UTF8ToTString(Edif::SDK->json.u.object.values[i].name);
				return *StoredCurrentLanguage;
			}
		}
		readIndex += langItem.size()+1;
		if (langList.at(readIndex-1) == '\r')
			break;
	}

	return *DefaultLanguageIndex();
}
const TCHAR* const DarkEdif::JSON::LanguageName()
{
	return StoredCurrentLanguageName.c_str();
}
#else // not EditorBuild
#define CurLang (*Edif::SDK->json.u.object.values[Edif::SDK->json.u.object.length - 1].value)

// In runtime, only one langauge is included, so we'll use a single string to contain it
static std::tstring jsonLangName;
const TCHAR* const DarkEdif::JSON::LanguageName()
{
	if (jsonLangName.empty())
		jsonLangName = UTF8ToTString(Edif::SDK->json.u.object.values[Edif::SDK->json.u.object.length - 1].name);
	return jsonLangName.c_str();
}

#endif // EditorBuild

// Case-insensitive comparison of texts. true if same. Uses stricmp or strcasecmp.
bool DarkEdif::SVICompare(const std::string_view& first, const std::string_view& second)
{
	// std::string_view() default ctor makes with null ptr, so if empty, _stricmp may crash from null
	return first.size() == second.size() && (first.empty() || !_stricmp(first.data(), second.data()));
}
// Case-insensitive comparison of texts. true if first param starts with second. Uses strnicmp or strncasecmp.
bool DarkEdif::SVIComparePrefix(const std::string_view& text, const std::string_view& prefix)
{
	assert(!prefix.empty());
	// std::string_view() default ctor makes with null ptr, so if empty, _strnicmp may crash from null
	return text.size() >= prefix.size() && !text.empty() && !_strnicmp(text.data(), prefix.data(), prefix.size());
}


ACEInfo * Edif::ACEInfoAlloc(unsigned int NumParams)
{
	// Allocate space for ACEInfo struct, plus Parameter[NumParams] so it has valid memory
	return (ACEInfo *)calloc(sizeof(ACEInfo) + (NumParams * sizeof(short) * 2), 1);	// The *2 is for reserved variables
}

bool Edif::CreateNewActionInfo()
{
	// Get ID and thus properties by counting currently existing actions.
	const json_value & UnlinkedAction = CurLang["Actions"sv][(std::int32_t)Edif::SDK->ActionInfos.size()];

	// Invalid JSON reference
	if (UnlinkedAction.type != json_object)
		return DarkEdif::MsgBox::Error(_T("Error reading action JSON"), _T("Invalid JSON reference for action ID %zu, expected object."), Edif::SDK->ActionInfos.size()), false;

	const json_value & Param = UnlinkedAction["Parameters"sv];

	// Num of parameters is beyond number of bits in FloatFlags
	if (sizeof(ACEInfo::FloatFlags)*CHAR_BIT < Param.u.object.length)
		return DarkEdif::MsgBox::Error(_T("Error reading action JSON"), _T("Too many parameters in action ID %zu."), Edif::SDK->ActionInfos.size()), false;

	// Parameters checked; allocate new info
	ACEInfo * ActInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!ActInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating action info"), _T("Could not allocate memory for action ID %zu return."), Edif::SDK->ActionInfos.size()), false;

	ActInfo->ID = (short)Edif::SDK->ActionInfos.size();
	ActInfo->NumOfParams = Param.u.object.length;

	if (ActInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (std::uint8_t c = 0; c < ActInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			ActInfo->Parameter[c].p = ReadActionOrConditionParameterType(Param[c][0], IsFloat);	// Store parameter type
			ActInfo->FloatFlags |= (IsFloat << c);								// Store whether it is a float or not with a single bit
		}

		// For some reason in Edif an extra short is provided, initialized to 0, so duplicate that
		memset(&ActInfo->Parameter[ActInfo->NumOfParams], 0, ActInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	Edif::SDK->ActionInfos.push_back(ActInfo);
	return true;
}

bool Edif::CreateNewConditionInfo()
{
	// Get ID and thus properties by counting currently existing conditions.
	const json_value & Condition = CurLang["Conditions"sv][(std::int32_t)Edif::SDK->ConditionInfos.size()];

	// Invalid JSON reference
	if (Condition.type != json_object)
		return DarkEdif::MsgBox::Error(_T("Error reading condition JSON"), _T("Invalid JSON reference for condition ID %zu, expected a JSON object."), Edif::SDK->ConditionInfos.size()), false;

	const json_value & Param = Condition["Parameters"sv];

	// Num of parameters is beyond size of FloatFlags
	if (sizeof(ACEInfo::FloatFlags) *CHAR_BIT < Param.u.object.length)
		return DarkEdif::MsgBox::Error(_T("Error reading condition JSON"), _T("Too many parameters in condition ID %zu."), Edif::SDK->ConditionInfos.size()), false;

	// Parameters checked; allocate new info
	ACEInfo * CondInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!CondInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating condition info"), _T("Could not allocate memory for condition ID %zu return."), Edif::SDK->ConditionInfos.size()), false;

	// If a non-triggered condition, set the correct flags
	CondInfo->ID = (short)Edif::SDK->ConditionInfos.size();
	CondInfo->NumOfParams = Param.u.object.length;
	CondInfo->Flags.ev = ((bool) Condition["Triggered"sv]) ? EVFLAGS::NONE : (EVFLAGS::ALWAYS | EVFLAGS::NOTABLE);

	if (CondInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (std::uint8_t c = 0; c < CondInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			CondInfo->Parameter[c].p = ReadActionOrConditionParameterType(Param[c][0], IsFloat);	// Store parameter type
			CondInfo->FloatFlags |= (IsFloat << c);								// Store whether it is a float or not with a single bit
		}

		// For some reason in Edif an extra short is provided, initialized to 0, so duplicate that
		memset(&CondInfo->Parameter[CondInfo->NumOfParams], 0, CondInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	Edif::SDK->ConditionInfos.push_back(CondInfo);
	return true;
}

bool Edif::CreateNewExpressionInfo()
{
	// Get ID and thus properties by counting currently existing conditions.
	const json_value & Expression = CurLang["Expressions"sv][(std::int32_t)Edif::SDK->ExpressionInfos.size()];

	// Invalid JSON reference
	if (Expression.type != json_object)
		return DarkEdif::MsgBox::Error(_T("Error reading expression JSON"), _T("Invalid JSON reference for expression ID %zu, expected a JSON object."), Edif::SDK->ExpressionInfos.size()), false;

	const json_value & Param = Expression["Parameters"sv];

	// Num of parameters is beyond size of FloatFlags
	if (sizeof(ACEInfo::FloatFlags)*CHAR_BIT < Param.u.object.length)
		return DarkEdif::MsgBox::Error(_T("Error reading expression JSON"), _T("Too many JSON parameters in expression ID %zu."), Edif::SDK->ExpressionInfos.size()), false;

	// Parameters checked; allocate new info
	ACEInfo * ExpInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!ExpInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating expression info"), _T("Could not allocate memory for expression ID %zu return."), Edif::SDK->ExpressionInfos.size()), false;

	// If a non-triggered condition, set the correct flags
	ExpInfo->ID = (short)Edif::SDK->ExpressionInfos.size();
	ExpInfo->NumOfParams = Param.u.object.length;
	ExpInfo->Flags.ef = Edif::ReadExpressionReturnType(Expression["Returns"sv]);

	if (ExpInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (std::uint8_t c = 0; c < ExpInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			ExpInfo->Parameter[c].ep = Edif::ReadExpressionParameterType(Param[c][0], IsFloat);	// Store parameter type
			ExpInfo->FloatFlags |= (IsFloat << c);											// Store whether it is a float or not with a single bit
		}

		// For some reason in Edif an extra short is provided, initialized to 0, so duplicate that
		memset(&ExpInfo->Parameter[ExpInfo->NumOfParams], 0, ExpInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	Edif::SDK->ExpressionInfos.push_back(ExpInfo);
	return true;
}

using namespace DarkEdif;

static void DebugProp_OutputString(PrintFHintInside const TCHAR* msgFormat, ...)
{
	if constexpr (!Properties::DebugProperties)
		return;
	va_list v;
	va_start(v, msgFormat);
	LogV(DARKEDIF_LOG_INFO, msgFormat, v);
	va_end(v);
}

// Any type that has no changeable value (excludes checkbox)
static bool IsUnchangeablePropExclCheckbox(const std::string_view& curStr)
{
	return SVICompare(curStr, "Text"sv) || SVIComparePrefix(curStr, "Folder"sv)
		|| SVICompare(curStr, "Edit button"sv) || SVICompare(curStr, "Group"sv);
}
// Any type that has no changeable value (includes checkbox, JSON statics, and buttons)
static bool IsUnchangeablePropInclCheckbox(const std::string_view & curStr)
{
	return SVICompare(curStr, "Checkbox"sv) || IsUnchangeablePropExclCheckbox(curStr);
}
// If true, a DarkEdif property with a string value in Data. Used for iteration and zero-length checks.
static bool IsStringPropType(const std::string_view & typeStr, bool includeComboBoxes)
{
	return SVIComparePrefix(typeStr, "Editbox String"sv) ||
		SVICompare(typeStr, "Editbox File"sv) || SVICompare(typeStr, "Editbox Image File"sv) ||
		SVICompare(typeStr, "URL button"sv) || SVICompare(typeStr, "Editbox Folder"sv) ||
		(includeComboBoxes && SVIComparePrefix(typeStr, "Combo Box"sv));
}
#ifndef NOPROPS
static bool IsStringPropType(int TypeID)
{
	return TypeID == Edif::Properties::IDs::PROPTYPE_EDIT_STRING ||
		TypeID == Edif::Properties::IDs::PROPTYPE_EDIT_MULTILINE ||
		TypeID == Edif::Properties::IDs::PROPTYPE_FILENAME ||
		TypeID == Edif::Properties::IDs::PROPTYPE_PICTUREFILENAME ||
		TypeID == Edif::Properties::IDs::PROPTYPE_DIRECTORYNAME;
}
static bool IsComboBoxType(int TypeID)
{
	return TypeID == Edif::Properties::IDs::PROPTYPE_COMBOBOX ||
		TypeID == Edif::Properties::IDs::PROPTYPE_COMBOBOXBTN ||
		TypeID == Edif::Properties::IDs::PROPTYPE_ICONCOMBOBOX;
}
#endif // NOPROPS

#pragma pack(push, 1)
struct DarkEdif::Properties::Data
{
	// Size of property's data relative from start pos in dataForProps.
	std::uint32_t sizeBytes; // for jumping past in scrolling; includes this entire Data
	// Property type ID - not Unicode incremented!
	std::uint16_t propTypeID;
	// New in prop v2: this is necessary if this property is part of a repeating set, and
	// its index in EDITDATA::Props does not match the JSON property index
	std::uint16_t propJSONIndex;

	// Data includes:
	// 1) Prop name as retrieved from JSON, but in lowercase for faster string compare.
	// 2) Actual data for property (prop value)
	std::uint8_t propNameSize;
	std::uint8_t data[];

	inline std::string ReadPropName() const {
		// Must be nicely padded to 1 byte, or there be shenanigans
		static_assert((sizeof(Data) - sizeof(sizeBytes) - sizeof(propTypeID) - sizeof(propJSONIndex) - sizeof(propNameSize)) == 0, "data[] is not 0 bytes");

		return std::string((const char*)data, (std::size_t)propNameSize);
	}

	inline const std::uint8_t* const ReadPropValue() const {
		return data + propNameSize;
	}
	inline std::uint8_t* ReadPropValue() {
		return data + propNameSize;
	}
	inline size_t ReadPropValueSize() const {
		return sizeBytes - sizeof(Data) - propNameSize;
	}

	Data* Next() const {
		return (Data*)(((char*)this) + sizeBytes);
	}
};
// Old smart properties
struct DarkEdif::Properties::V1Data
{
	// Size of property's data relative from start pos in dataForProps.
	std::uint32_t sizeBytes; // for jumping past in scrolling; includes this entire Data
	// Property type ID - not Unicode incremented!
	std::uint16_t propTypeID;

	// Data includes:
	// 1) Prop name as retrieved from JSON, but in lowercase for faster string compare.
	// 2) Actual data for property (prop value)
	std::uint8_t propNameSize;
	std::uint8_t data[];

	inline std::string ReadPropName() const {
		// Must be nicely padded to 1 byte, or there be shenanigans
		static_assert((sizeof(V1Data) - sizeof(sizeBytes) - sizeof(propTypeID) - sizeof(propNameSize)) == 0, "data[] is not 0 bytes");

		return std::string((const char*)data, (std::size_t)propNameSize);
	}

	inline const std::uint8_t* const ReadPropValue() const {
		return data + propNameSize;
	}
	inline std::uint8_t* ReadPropValue() {
		return data + propNameSize;
	}
	inline size_t ReadPropValueSize() const {
		return sizeBytes - sizeof(V1Data) - propNameSize;
	}

	V1Data* Next() const {
		return (V1Data*)(((char*)this) + sizeBytes);
	}
};
#pragma pack(pop)

// Backing struct for an image list property. Varying size.
struct ImgListProperty {
	// Number of images stored in this image list
	std::uint16_t numImages;
	// Fusion image bank IDs
	std::uint16_t imageIDs[];
};

#if EditorBuild

// Hash function; we can't use std::hash as it returns size_t, which varies in size per platform
static std::uint32_t fnv1a(std::string_view const& text)
{
	// 32 bit params
	constexpr std::uint32_t fnv_prime = 16777619U;
	constexpr std::uint32_t fnv_offset_basis = 2166136261U;

	const std::basic_string_view<std::uint8_t> text2((std::uint8_t*)text.data(), text.size());
	std::uint32_t hash = fnv_offset_basis;
	for (const std::uint8_t& c : text2) {
		hash ^= c;
		hash *= fnv_prime;
	}

	return hash;
}
#endif // EditorBuild

#ifndef NOPROPS

#if EditorBuild

// Converts ID to remove the ext custom item prefix, or returns false if not in range
static bool GetPropRealID(std::uint32_t &fromFusion)
{
	if (fromFusion < PROPID_EXTITEM_CUSTOM_FIRST || CurLang["Properties"sv].type == json_null)
		return false;

	if (fromFusion >= PROPID_EXTITEM_CUSTOM_FIRST + CurLang["Properties"sv].u.array.length)
	{
		DebugProp_OutputString(_T("Accessed property ID %u, outside of custom extension range; ignoring it.\n"),
			fromFusion - PROPID_EXTITEM_CUSTOM_FIRST);
		return false;
	}

	// Ext properties start from 0x80000, PROPID_EXTITEM_CUSTOM_FIRST
	fromFusion -= PROPID_EXTITEM_CUSTOM_FIRST;
	return true;
}

DarkEdif::Properties& GetRealPropsAddress(EDITDATA* edPtr);

namespace DarkEdif {
	namespace DLL
	{
		struct ConverterReturnAccessor
		{
			Properties::ConvReturnType convRetType;
			const void * data;
			size_t dataSize;
			int checkboxState; // 0, 1, or -1 if unset

			// If non-null, a pointer to a freeing function.
			void (*freeData)(const void *);
		};
	}
}

// Return a value with optional deleter, and optional checkbox state
void Properties::ConverterReturn::Return_OK(const void * data, size_t dataSize,
	void (*freeData)(const void *) /* = nullptr */, int checkboxState /* = -1 */)
{
	this->data = data;
	this->dataSize = dataSize;
	this->freeData = freeData;
	this->checkboxState = checkboxState;

	if (checkboxState < -1 || checkboxState > 1)
		MsgBox::Error(_T("Checkbox error"), _T("Checkbox state of %i is invalid."), checkboxState);

	convRetType = ConvReturnType::OK;

	// This happens when upgrading from an old smart property version that didn't store any data.
	// Can't just modify GetPropValue() because it might be used to get offset to write in Properties.
	if (data && dataSize == 0)
		data = nullptr;
}

// Failed to read, but it should be okay to use converter after
// Passing to next level of converter is requested
void Properties::ConverterReturn::Return_Pass()
{
	convRetType = ConvReturnType::Pass;
}

// Was okay to return it, but had a third-party error
// Report to user
static TCHAR outputBuff[2048];
void Properties::ConverterReturn::Return_Error(PrintFHintInside const TCHAR * err, ...)
{
	convRetType = ConvReturnType::Error;
	va_list v;
	va_start(v, err);
	assert(_vsntprintf_s(outputBuff, std::size(outputBuff), (size_t)-1, err, v) > 0);
	va_end(v);
	data = outputBuff;
	DebugProp_OutputString(_T("%s"), outputBuff);
}

// The source data doesn't have this, and clearly the source is unreliable
// This converter should no longer be used
void Properties::ConverterReturn::Return_ConverterUnsuitable()
{
	convRetType = ConvReturnType::ConverterUnsuitable;
}

Properties::ConverterReturn::ConverterReturn()
{
	this->data = nullptr;
	this->dataSize = 0;
	this->freeData = nullptr;
	this->checkboxState = -1;
	this->convRetType = ConvReturnType::Unset;
}
#endif // EditorBuild

DarkEdif::DLL::PropAccesser & Elevate(const DarkEdif::Properties &p)
{
	return *(DarkEdif::DLL::PropAccesser *)&p;
}

#pragma pack (push, 1)
// Reads DarkEdif::Properties without the fanfare.
struct DarkEdif::DLL::PropAccesser
{
	NO_DEFAULT_CTORS_OR_DTORS(PropAccesser);
	// Type of DarkEdif::Properties
	decltype(Properties::propVersion) propVersion;

	// Used to read JSON properties to see if a change has been made.
	decltype(Properties::hash) hash; // fnv1a hash of property titles and types (with NOT applied)
	decltype(Properties::hashTypes) hashTypes; // fnv1a hash of property types only (with NOT applied)
	// Number of properties
	decltype(Properties::numProps) numProps;
	// VS decided to pad this struct, so let's continue the idiocy officially
	decltype(Properties::pad) pad;
	// Size of DataForProps - including EDITDATA (and thus EDITDATA::Properties)
	decltype(Properties::sizeBytes) sizeBytes;
	union {
		// If properties are visible. Included but invalid in non-Editor builds.
		decltype(Properties::visibleEditorProps) visibleEditorProps;
		// Padding for if Fusion editor is 64-bit
		decltype(Properties::ptrPad) ptrPad;
	};
	// The actual data for properties, merged together
	// Starts with checkboxes, then data, which is Data struct: type ID followed by binary.
	decltype(DarkEdif::Properties::dataForProps) dataForProps; /* [], inherited from decltype */;

	// Note: There is a single bit for each checkbox.
	// Use numProps / 8 for num of bytes used by checkboxes.
	static_assert(std::is_same<decltype(DarkEdif::DLL::PropAccesser::dataForProps), decltype(DarkEdif::Properties::dataForProps)>::value, "Invalid type");

	const DarkEdif::Properties::Data * Internal_FirstData() const
	{
		return ((DarkEdif::Properties *)this)->Internal_FirstData();
	}
	const DarkEdif::Properties::Data * Internal_DataAt(unsigned int ID, bool idIsJSON) const
	{
		return ((DarkEdif::Properties *)this)->Internal_DataAt(ID, idIsJSON);
	}
	std::size_t PropIdxFromJSONIdx(std::size_t ID, const DarkEdif::Properties::Data** dataPtr = nullptr, const DarkEdif::Properties::Data** rsContainer = nullptr) const
	{
		return ((DarkEdif::Properties*)this)->PropIdxFromJSONIdx(ID, dataPtr, rsContainer);
	}

#if EditorBuild
	DarkEdif::Properties::Data* Internal_FirstData()
	{
		return ((DarkEdif::Properties*)this)->Internal_FirstData();
	}
	DarkEdif::Properties::Data * Internal_DataAt(unsigned int ID, bool idIsJSON = true)
	{
		return ((DarkEdif::Properties *)this)->Internal_DataAt(ID, idIsJSON);
	}
	std::size_t PropIdxFromJSONIdx(std::size_t ID, DarkEdif::Properties::Data** dataPtr = nullptr, DarkEdif::Properties::Data** rsContainer = nullptr)
	{
		return ((DarkEdif::Properties*)this)->PropIdxFromJSONIdx(ID, dataPtr, rsContainer);
	}

	Prop * GetProperty(size_t ID)
	{
		return ((DarkEdif::Properties *)this)->GetProperty(ID);
	}
	static void Internal_PropChange(mv * mV, EDITDATA * &edPtr, unsigned int PropID, const void * newData, size_t newSize)
	{
		DarkEdif::Properties::Internal_PropChange(mV, edPtr, PropID, newData, newSize);
	}
#endif // EditorBuild
};
#pragma pack (pop)

#if EditorBuild
// Reads the real offset of EDITDATA::Props from extPrivateData offset, used to find Props
// for an EDITDATA that may not match C++ current version. Works with DarkEdif smart props v1.
static DarkEdif::Properties& GetRealPropsAddress(EDITDATA* edPtr)
{
	std::size_t propOffset = edPtr->eHeader.extPrivateData;
	std::uint8_t* edPtrFake = (std::uint8_t*)edPtr;
	if (propOffset < sizeof(edPtr->eHeader) || propOffset > edPtr->eHeader.extSize)
	{
		DarkEdif::MsgBox::WarningOK(_T("Old DarkEdif props"), _T("DarkEdif smart props offset not found. Falling back to default. Resetting properties recommended."));
		propOffset = ((char*)&edPtr->Props) - ((char*)edPtr);
	}
	int darVer = Elevate(*(DarkEdif::Properties*)(edPtrFake + propOffset)).propVersion;
	if (darVer != 'DAR1' && darVer != 'DAR2')
	{
		bool found = false;
		for (propOffset = sizeof(extHeader); propOffset < edPtr->eHeader.extSize - sizeof(DarkEdif::Properties); ++propOffset)
		{
			darVer = Elevate(*(DarkEdif::Properties*)(edPtrFake + propOffset)).propVersion;
			if (darVer == 'DAR1' && darVer == 'DAR2')
			{
				propOffset = ((std::uint8_t*)&edPtr->Props) - ((std::uint8_t*)edPtr);
				found = true;
			}
		}
		if (!found)
			DarkEdif::MsgBox::Error(_T("DarkEdif offset invalid"), _T("DarkEdif smart properties was not found in edittime data. The MFA is likely corrupt."));
		else
			DarkEdif::MsgBox::WarningOK(_T("DarkEdif offset invalid"), _T("DarkEdif smart properties offset was invalid and corrected. The MFA is likely corrupt."));
	}
	return *(DarkEdif::Properties*)(((std::uint8_t*)edPtr) + propOffset);
}

#endif // EditorBuild

std::uint16_t RuntimePropSet::GetNumPropsInThisEntry(Properties::Data* const propHoldingThisRS) const
{
	// Due to the prop name preceding, and the set name following, we don't know where the Data is that holds this PropSet
	assert((RuntimePropSet*)propHoldingThisRS->ReadPropValue() == this);

	DarkEdif::Properties::Data* data = propHoldingThisRS;
	while (data->propJSONIndex < firstSetJSONPropIndex)
		data = data->Next();
	std::uint16_t i = 1;
	while (data->propJSONIndex != lastSetJSONPropIndex)
	{
		data = data->Next();
		++i;
	}
	return i;
}

// Gets the set name
std::string RuntimePropSet::GetPropSetName(const Properties::Data * const propHoldingThisRS) const
{
	assert((RuntimePropSet*)propHoldingThisRS->ReadPropValue() == this);
	assert(propHoldingThisRS->propTypeID == Edif::Properties::IDs::PROPTYPE_COMBOBOX);
	const std::size_t setNameSize = propHoldingThisRS->ReadPropValueSize() - sizeof(RuntimePropSet);
	return std::string(setName, setNameSize);
}

#if defined(_WIN32) && !defined(NOPROPS)

void LoadObjectWipe(EDITDATA* edPtr)
{
	// TODO: Does LoadObject happen before editdata can be upgraded v1 -> v2?
	auto& Props = Elevate(edPtr->Props);
	if (Props.propVersion != 'DAR2')
		DarkEdif::MsgBox::Error(_T("DarkEdif property error"), _T("DarkEdif smart properties were not created correctly."));
	if (Props.visibleEditorProps)
		Props.visibleEditorProps = nullptr;
}

#endif // NOPROPS

#if EditorBuild

EdittimePropSet& ESFROMRS(RuntimePropSet* rs)
{
	auto edPropSetIt = std::find_if(
		Edif::SDK->EdittimePropertySets.begin(),
		Edif::SDK->EdittimePropertySets.end(),
		[rs](EdittimePropSet& es) {
			return es.startSetIdx == rs->firstSetJSONPropIndex;
		});
	assert(edPropSetIt != Edif::SDK->EdittimePropertySets.end());
	return *edPropSetIt;
}

void ReloadPropSet(mv* mV, EDITDATA* edPtr, Properties::Data * dataContainingRS, std::uint16_t newSetIdx)
{
	RuntimePropSet* const rs = (RuntimePropSet*)dataContainingRS->ReadPropValue();
	assert(rs->setIndicator == 'S');
	DebugProp_OutputString(_T("Reload of prop set from JSON ID range %zu to %zu.\n"),
		rs->firstSetJSONPropIndex, rs->lastSetJSONPropIndex);
	// There are three ways to refresh the set.
	// 1) mvRefreshProp on all properties
	// Seems not to work consistently.
	// 2) mvRemoveProps/InsertProps on just the set's props.
	// Using this at the mo.
	// 3) mvRemoveProps/InsertProps from start of set to end of all props, not just set props.
	// Was using this due to issues with index when re-inserting set props using BEFORE = false.
	// Now using insert with AFTER = true.

	constexpr bool refreshOnly = false;
	if constexpr (refreshOnly)
	{
		rs->setIndexSelected = newSetIdx;
		for (std::size_t i = rs->firstSetJSONPropIndex + PROPID_EXTITEM_CUSTOM_FIRST, j = rs->GetNumPropsInThisEntry(dataContainingRS); i < j; ++i)
			mvRefreshProp(mV, edPtr, i, TRUE);
	}
	else
	{
		constexpr bool onlySection = true;

		PropData* start = &Edif::SDK->EdittimeProperties[rs->firstSetJSONPropIndex];
		PropData* afterEnd, copyOfAfterEnd;
		if constexpr (onlySection)
		{
			afterEnd = &Edif::SDK->EdittimeProperties[rs->lastSetJSONPropIndex + 1];
			copyOfAfterEnd = *afterEnd;
			memset(afterEnd, 0, sizeof(PropData));
		}
		mvRemoveProps(mV, edPtr, &Edif::SDK->EdittimeProperties[rs->firstSetJSONPropIndex]);
		rs->setIndexSelected = newSetIdx;
		mvInsertProps(mV, edPtr, start, rs->firstSetJSONPropIndex - 1 + PROPID_EXTITEM_CUSTOM_FIRST, TRUE);
		if (onlySection)
			*afterEnd = copyOfAfterEnd;
		DebugProp_OutputString(_T("Reload of prop set from JSON ID range %zu to %zu (inclusive) completed.\n"),
			rs->firstSetJSONPropIndex, rs->lastSetJSONPropIndex);
	}

	// Refresh the buttons and name list
	const auto& es = ESFROMRS(rs);
	mvRefreshProp(mV, edPtr, es.addButtonIdx + PROPID_EXTITEM_CUSTOM_FIRST, FALSE);
	mvRefreshProp(mV, edPtr, es.deleteButtonIdx + PROPID_EXTITEM_CUSTOM_FIRST, FALSE);
	mvRefreshProp(mV, edPtr, es.nameListIdx + PROPID_EXTITEM_CUSTOM_FIRST, TRUE);
	mvRefreshProp(mV, edPtr, es.nameEditboxIdx + PROPID_EXTITEM_CUSTOM_FIRST, TRUE);
}

void DarkEdif::Properties::Internal_PropChange(mv* mV, EDITDATA*& edPtr, unsigned int PropID,
	const void* newPropValue, size_t newPropValueSize)
{
	const char* curTypeStr = CurLang["Properties"sv][PropID]["Type"sv].c_str();
	Data* const oldPropDataPtr = edPtr->Props.Internal_DataAt(PropID);
	if (!oldPropDataPtr)
	{
		MsgBox::Error(_T("DarkEdif property error"), _T("Couldn't find property %s (JSON type %hs), failed to edit it."),
			UTF8ToTString(CurLang["Properties"sv][PropID]["Title"sv]).c_str(), curTypeStr);
		return;
	}
	bool rearrangementRequired = false;
	const std::size_t oldPropValueSize = oldPropDataPtr->ReadPropValueSize();
	std::string newComboBoxValue = "L"s;

	// Fusion sends combo box changes as UInts, item index, but we store them as the item text.
	if (oldPropDataPtr->propTypeID == Edif::Properties::IDs::PROPTYPE_COMBOBOX ||
		oldPropDataPtr->propTypeID == Edif::Properties::IDs::PROPTYPE_COMBOBOXBTN)
	{
		assert(newPropValueSize == sizeof(unsigned int));
		const json_value& propjson = CurLang["Properties"sv][PropID];

		// It's a set-switcher combo box, we use index
		if (oldPropDataPtr->ReadPropValue()[0] == 'S')
		{
			// RuntimePropSet starts with S, and finishes with set name
			assert(oldPropValueSize > sizeof(RuntimePropSet));
			std::uint32_t newSetIdx = *(std::uint32_t*)newPropValue;
			RuntimePropSet* rs = (RuntimePropSet*)oldPropDataPtr->ReadPropValue();
			assert(newSetIdx < rs->numRepeats);
			ReloadPropSet(mV, edPtr, oldPropDataPtr, newSetIdx);
			return;
		}

		assert(oldPropDataPtr->ReadPropValue()[0] == 'L');

		// It's a regular combo box entry, so we take the passed index and store its data as text
		newComboBoxValue += std::string_view(
			(const char*)propjson["Items"sv][*(unsigned int*)newPropValue].u.string.ptr,
			propjson["Items"sv][*(unsigned int*)newPropValue].u.string.length);
		newPropValue = newComboBoxValue.data();
		newPropValueSize = newComboBoxValue.size();
	}

	std::size_t nameListToRefresh = -1;
	for (std::size_t i = 0; i < Edif::SDK->EdittimePropertySets.size(); ++i)
	{
		if (Edif::SDK->EdittimePropertySets[i].nameEditboxIdx == PropID)
		{
			nameListToRefresh = Edif::SDK->EdittimePropertySets[i].nameListIdx;
			break;
		}
	}

	if (newPropValueSize == oldPropValueSize)
	{
		// The Data is unchangeable, but the container EDITDATA isn't, so really the Data is changeable.
		memcpy(oldPropDataPtr->ReadPropValue(), newPropValue, newPropValueSize);
		if (nameListToRefresh != -1)
			mvRefreshProp(mV, edPtr, nameListToRefresh + PROPID_EXTITEM_CUSTOM_FIRST, TRUE);
		return;
	}

	// Only empty string properties should be 0 length. Originally I planned to include null JIC,
	// but as each prop has a preceding value size there's not really a need for that.
	if (newPropValueSize == 0)
	{
		if (!IsStringPropType(curTypeStr, false))
		{
			MsgBox::Error(_T("DarkEdif property error"), _T("Property size of property %s (JSON type %hs) is 0!"),
				UTF8ToTString(CurLang["Properties"sv][PropID]["Title"sv]).c_str(), curTypeStr);
		}
		if (nameListToRefresh != -1)
		{
			MsgBox::Error(_T("DarkEdif property error"), _T("Set names cannot be blank."));
			return; // cancel the edit
		}
	}

	// Shouldn't happen unless ext dev starts messing with ext properties without changing version
	if (edPtr->eHeader.extSize != edPtr->Props.sizeBytes)
	{
		if (MsgBox::WarningYesNo(_T("DarkEdif property warning"), _T("Change to property \"%s\" (JSON type \"%hs\") caused reserved size to be different to actual size. "
			"Press Yes to set it to Fusion's size (may corrupt properties).\n"
			"Press No to cancel property change."),
			UTF8ToTString(CurLang["Properties"sv][PropID]["Title"sv]).c_str(), curTypeStr) == IDYES)
		{
			edPtr->Props.sizeBytes = edPtr->eHeader.extSize;
		}
		else
			return;
	}

	// Where P is value of data. Note the metadata of Properties::Data is included in beforeOldSize.
	const size_t beforeOldSize = oldPropDataPtr->ReadPropValue() - ((std::uint8_t*)edPtr);	// Pointer to O|<P|O
	const size_t afterOldSize = edPtr->Props.sizeBytes - beforeOldSize - oldPropValueSize;	// Pointer to O|P>|O

	// Duplicate memory to another buffer (if new arragement is smaller - we can't just copy from old buffer after realloc)
	const size_t newEdPtrSize = edPtr->Props.sizeBytes - oldPropValueSize + newPropValueSize;
	std::unique_ptr<std::uint8_t[]> newEdPtr = std::make_unique<std::uint8_t[]>(newEdPtrSize);

	if (!newEdPtr)
	{
		DarkEdif::MsgBox::Error(_T("DarkEdif property error"), _T("Out of memory attempting to rewrite properties!"));
		return;
	}

	// Copy the part before new data into new address
	memcpy(newEdPtr.get(), edPtr, beforeOldSize);

	// Copy the new data into new address
	memcpy(newEdPtr.get() + beforeOldSize, newPropValue, newPropValueSize);

	// Copy the part after new data into new address
	memcpy(newEdPtr.get() + beforeOldSize + newPropValueSize,
		oldPropDataPtr->ReadPropValue() + oldPropValueSize,
		afterOldSize);

	// Update EDITDATA size
	((EDITDATA*)newEdPtr.get())->Props.sizeBytes = newEdPtrSize;

	// Update the new property's sizeBytes.
	// To get newEdPtr -> sizeBytes, we can re-use the offset of oldEdPtr -> prop::sizeBytes,
	// as obviously no properties before it in the queue were shifted, which is faster than using Internal_DataAt which jumps through Datas.
	// ((EDITDATA *)newEdPtr.get())->Props.Internal_DataAt(PropID)->sizeBytes = VV;
	*(std::uint32_t*)(newEdPtr.get() + (((std::uint8_t*)&oldPropDataPtr->sizeBytes) - (std::uint8_t*)edPtr))
		= (std::uint32_t)(sizeof(DarkEdif::Properties::Data) + oldPropDataPtr->propNameSize + newPropValueSize);

	static_assert(std::is_same_v<decltype(((EDITDATA*)newEdPtr.get())->Props.Internal_DataAt(PropID)->sizeBytes), std::uint32_t>, "Types invalid");

	// Reallocate edPtr
	EDITDATA* fusionNewEdPtr = (EDITDATA*)mvReAllocEditData(mV, edPtr, newEdPtrSize);
	if (!fusionNewEdPtr)
	{
		MsgBox::Error(_T("Property Error"), _T("NULL returned from EDITDATA reallocation. Property changed cancelled."));
		return;
	}

	// Copy into edPtr (copy everything after eHeader, leave eHeader alone)
	// eHeader::extSize and such will be changed by Fusion by mvReAllocEditData,
	// so should not be considered ours to interact with
	memcpy(&GetRealPropsAddress(fusionNewEdPtr),
		&GetRealPropsAddress((EDITDATA*)newEdPtr.get()),
		newEdPtrSize - ((EDITDATA*)newEdPtr.get())->eHeader.extPrivateData);

	edPtr = fusionNewEdPtr; // Inform caller of new address
	if (nameListToRefresh != -1)
		mvRefreshProp(mV, edPtr, nameListToRefresh + PROPID_EXTITEM_CUSTOM_FIRST, TRUE);
}

void FusionAPI DuplicateObject(mv* mV, ObjInfo* oiPtr, EDITDATA* edPtr)
{
#pragma DllExportHint
	auto& Props = Elevate(edPtr->Props);
	if (Props.propVersion != 'DAR2')
		MsgBox::Error(_T("DarkEdif property error"), _T("DarkEdif smart properties were not created correctly."));
	if (Props.visibleEditorProps)
		Props.visibleEditorProps = 0;
}

LPARAM DarkEdif::DLL::DLL_GetPropCreateParam(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	if (!GetPropRealID(PropID))
		return 0;

	LOGV(_T("DLL_GetPropCreateParam call for prop ID %u.\n"), PropID);

	auto& Props = Elevate(edPtr->Props);
	if (!std::any_of(Edif::SDK->EdittimePropertySets.cbegin(), Edif::SDK->EdittimePropertySets.cend(),
		[PropID](const auto& i) { return i.nameListIdx == PropID; }))
	{
		return NULL;
	}

	DarkEdif::Properties::Data* data = Props.Internal_DataAt(PropID);
	const auto rs = (RuntimePropSet*)data->ReadPropValue();
	assert(rs->setIndicator == 'S');

	// Create one long text underlying all the item strings
	std::tstring strList;
	std::vector<std::size_t> lengths;
	for (std::size_t numLeft = rs->numRepeats; ;)
	{
		if (data->propJSONIndex == rs->setNameJSONPropIndex)
		{
			assert(data->propTypeID == Edif::Properties::IDs::PROPTYPE_EDIT_STRING);
			const std::tstring name = DarkEdif::UTF8ToTString(std::string_view((char*)data->ReadPropValue(), data->ReadPropValueSize()));
			strList.append(name.data(), name.size() + 1);
			lengths.push_back(data->ReadPropValueSize());
			if (--numLeft == 0)
				break;
		}
		data = data->Next();
		if (data->propJSONIndex == 0 || data->propJSONIndex > rs->lastSetJSONPropIndex)
		{
			MsgBox::Error(_T("Property Error"), _T("Went past the end of the sets while looping set name entries."));
			return NULL;
		}
	}

	TCHAR * strListCopy = (TCHAR *)malloc(sizeof(TCHAR) * strList.size());
	if (!strListCopy)
	{
		MsgBox::Error(_T("Property Error"), _T("Out of memory allocating a list"));
		return NULL;
	}
	memcpy(strListCopy, strList.data(), strList.size() * sizeof(TCHAR));
	// Create list of string pointers pointing inside the long text
	TCHAR** listCopy2 = (TCHAR **)malloc((1 + lengths.size() + 1) * sizeof(TCHAR *));
	for (std::size_t i = 0, j = 0; i < lengths.size(); ++i)
	{
		listCopy2[i + 1] = &strListCopy[j];
		j += lengths[i] + 1;
	}
	// Null required at both start and end of array
	listCopy2[0] = listCopy2[lengths.size() + 1] = NULL;
	return (LPARAM)listCopy2;
}
void DarkEdif::DLL::DLL_ReleasePropCreateParam(mv * mV, EDITDATA * edPtr, unsigned int PropID, LPARAM lParam)
{
	if (!GetPropRealID(PropID) || !lParam)
		return;

	// Not a prop set, so we not freeing this
	if (!std::any_of(Edif::SDK->EdittimePropertySets.cbegin(), Edif::SDK->EdittimePropertySets.cend(),
		[PropID](const auto& i) { return i.nameListIdx == PropID; }))
	{
		return;
	}

	// Else it's a name switch prop, containing one long string ptr, with a ptr of string ptrs
	TCHAR** const strArray = (TCHAR**)lParam;
	// skip past null at start, free the one long text underlying all
	free(strArray[1]);
	// free the array of text
	free(strArray);
	return;
}

int DarkEdif::DLL::DLL_CreateObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr)
{
	LOGV(_T("Call to %s with edPtr %p.\n"), _T(__FUNCTION__), edPtr);
	// To make it easier, we delegate to UpdateEditStructure. That returns a HGLOBAL, so we'll pull the original pointer back out.
	HGLOBAL gblPtr = DLL_UpdateEditStructure(mV, nullptr);
	if (gblPtr == NULL)
		return -1;

	EDITDATA * gblEdPtr = (EDITDATA *)GlobalLock(gblPtr);
	if (gblEdPtr == NULL)
	{
		GlobalFree(gblPtr);
		return -1;
	}

	// The one thing gbl can't have set yet, because edPtr wasn't passed.
	// WARNING: If this is not set properly, Fusion will not display object properties correctly!
	gblEdPtr->eHeader.extID = edPtr->eHeader.extID;

	// Do we need more size now? We shouldn't, but check.
	std::size_t numBytes = Elevate(gblEdPtr->Props).sizeBytes;
	EDITDATA* fusionEdPtr = edPtr;
	errno_t err = 0;

	if (numBytes != edPtr->eHeader.extSize)
	{
		MsgBox::Error(_T("EDITDATA size mismatch"), _T("Fusion provided an EDITDATA of %zu bytes, Props sizeBytes is %zu, but GetEDITDATASizeFromJSON calculation was %hu bytes expected.\nThis is a DarkEdif bug!"),
			edPtr->eHeader.extSize, numBytes, Internal_GetEDITDATASizeFromJSON());
		fusionEdPtr = (EDITDATA*)mvReAllocEditData(mV, edPtr, numBytes);

		if (fusionEdPtr == NULL)
			err = ENOMEM;
	}
	else
	{
		// Copy out any extra EDITDATA variables between eHeader and Props
		std::size_t extraDataSize = ((char*)&edPtr->Props) - (((char*)&edPtr->eHeader) + sizeof(edPtr->eHeader));
		char* extraData = NULL;
		if (extraDataSize > 0)
		{
			extraData = (char*)malloc(extraDataSize);
			err = extraData ? memcpy_s(extraData, extraDataSize, ((char*)&edPtr->eHeader) + sizeof(edPtr->eHeader), extraDataSize) : ENOMEM;
		}

		if (err == 0)
			err = memcpy_s(fusionEdPtr, fusionEdPtr->eHeader.extSize, gblEdPtr, numBytes);

		// Copy back extra EDITDATA variables
		if (err == 0 && extraData)
		{
			err = memcpy_s(((char *)fusionEdPtr)+sizeof(fusionEdPtr->eHeader),
				fusionEdPtr->eHeader.extSize - sizeof(fusionEdPtr->eHeader),
				extraData, extraDataSize);
			free(extraData);
		}
	}
	GlobalUnlock(gblPtr);
	GlobalFree(gblPtr);

	if (err)
	{
		MsgBox::Error(_T("Initialisation failure"), _T("Initialising extension failed: error %d."), err);
		return -1;
	}

	return 0;
}
void ScanForDynamicPropChange(mv* mV, EDITDATA* edPtr, unsigned int PropID);

BOOL DarkEdif::DLL::DLL_GetProperties(mv * mV, EDITDATA * edPtr, bool masterItem)
{
	LOGV(_T("Call to %s with edPtr %p.\n"), _T(__FUNCTION__), edPtr);
	DebugProp_OutputString(_T("Call to GetProperties with edPtr version %u.\n"), edPtr->eHeader.extVersion);

	if (Edif::SDK->EdittimeProperties == nullptr)
		MsgBox::Error(_T("Property error"), _T("Call to DLL_GetProperties without valid EdittimeProperties."));

	// Insert all properties, then let ScanForDynamic remove invisible ones selectively

	// TODO: Selectively call mvInsertProps, rather than this bulk insert.
	// Note that EdittimePropertiesVisible is holding old instance visibility,
	// not updated for newly selected instance's visibility.
	// Update can be triggered by ScanForDynamicPropChange, but that assumes Visible ones
	// are displayed, and non-Visible ones are not present in prop data.
	// So basically we need to run Scan and pass back the results here.
	// I'm not sure what will happen if multiple instances are selected,
	// but I expect that selective insertion will do better than bulk insert and selective remove.
#ifndef NOPROPS
	auto& Props = Elevate(edPtr->Props);
	if (Props.propVersion != 'DAR2')
		MsgBox::Error(_T("DarkEdif property error"), _T("DarkEdif smart properties were not created correctly."));

	// CURRENT: the visibleEditorProps pointer is kept across MFA saves,
	// and since Fusion doesn't do anything to indicate pointer invalid...
	// PrepareToWriteObject is called but modifies original EDITDATA.
	//			Could we set an invalid checkbox?
	// Otherwise, could we store the checkbox array as a kv pair in SDK?
	// LoadObject is only called for first object in frame. Possibly runtime.
	//			Will need to check.
	// Also note DuplicateObject will also clone the pointer.
	// Honestly, storing it in EDITDATA is technically correct validity lifetime,
	// but saving it to MFA is not so.

	if (!Props.visibleEditorProps)
		Props.visibleEditorProps = new bool[CurLang["Properties"sv].u.array.length];

	for (std::size_t i = 0; i < CurLang["Properties"sv].u.array.length; ++i)
		Props.visibleEditorProps[i] = true;
#endif

	mvInsertProps(mV, edPtr, Edif::SDK->EdittimeProperties.get(), PROPID_TAB_GENERAL, TRUE);
	ScanForDynamicPropChange(mV, edPtr, UINT32_MAX);
	return TRUE; // OK
}
void DarkEdif::DLL::DLL_ReleaseProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
	LOGV(_T("Call to %s with edPtr %p.\n"), _T(__FUNCTION__), edPtr);
	// do nothing because SDK is static and properties are re-used
}

// This is passed Prop/Fusion ID with prefix, not JSON ID
Prop * DarkEdif::Properties::GetProperty(std::size_t PropID)
{
	// Not our responsibility; ID unrecognised
	if (!GetPropRealID(PropID))
		return nullptr;

	// Premature call
	if (sizeBytes == 0)
	{
		MsgBox::Error(_T("Premature function call"), _T("GetProperty() called without edPtr->Props being valid."));
		return nullptr;
	}

	const Data* Current = Internal_DataAt(PropID);
	if (!Current)
	{
		MsgBox::WarningOK(_T("DarkEdif property error"), _T("Warning: Returned null Data. Properties are corrupt; "
			"replace the existing " PROJECT_NAME ", by creating a new one and using \"Replace by another object\" in event editor."));
		return nullptr;
	}

	const json_value &propJSON = CurLang["Properties"sv][Current->propJSONIndex];
	const std::string_view curStr = propJSON["Type"sv];
	Prop * ret = nullptr;
	bool allConv;

	// Unchanging contents, so read the text from JSON.
	if (SVICompare(curStr, "Text"sv) || SVICompare(curStr, "Edit button"sv))
	{
		ret = new Prop_Str(UTF8ToTString(propJSON["DefaultState"sv], &allConv).c_str());
		if (!allConv)
		{
			MsgBox::WarningOK(_T("DarkEdif property error"), _T("Warning: The property's Unicode string \"%s\" couldn't be converted to ANSI. "
				"Characters will be replaced with filler."), UTF8ToTString(propJSON["DefaultState"sv]).c_str());
		}
		return ret;
	}

	// Unchanging contents, null is fine for data.
	if (IsUnchangeablePropInclCheckbox(curStr))
		return nullptr;

	if (IsStringPropType(curStr, false))
	{
		ret = new Prop_Str(UTF8ToTString(std::string_view((const char *)Current->ReadPropValue(), Current->ReadPropValueSize()), &allConv).c_str());
		if (!allConv)
		{
			// Failed to convert to ANSI (UTF-8 -> UTF-16 won't reasonably fail); if we report it with %s, we'll likely repeat the problem, so use %hs and report as-is.
			MsgBox::WarningOK(_T("DarkEdif property error"), _T("GetProperty warning: The property's Unicode string \"%hs\" couldn't be converted to ANSI. "
				"Characters will be replaced with filler."), std::string((const char*)Current->ReadPropValue(), Current->ReadPropValueSize()).c_str());
		}
	}
	else if (SVICompare(curStr, "Editbox Number"sv) || SVICompare(curStr, "Edit Spin"sv) ||
		SVICompare(curStr, "Edit Slider") || SVICompare(curStr, "Color") || SVICompare(curStr, "Edit direction"))
	{
		ret = new Prop_SInt(*(const int*)Current->ReadPropValue());
	}
	else if (SVICompare(curStr, "Editbox Float"sv) || SVICompare(curStr, "Edit spin float"sv))
	{
		ret = new Prop_Float(*(const float *)Current->ReadPropValue());
	}
	else if (SVICompare(curStr, "Size"sv))
	{
		const DarkEdif::Size& size = *(DarkEdif::Size *)Current->ReadPropValue();
		ret = new Prop_Size(size.width, size.height);
	}
	else if (SVICompare(curStr, "Image List"sv)) {
		ret = new Prop_Buff(Current->ReadPropValueSize(),
			(void *)Current->ReadPropValue());
	}
	// Combo boxes return index, not their item text
	else if (SVIComparePrefix(curStr, "Combo Box"sv))
	{
		// Combo box is a list of a prop set, so index is stored within
		const RuntimePropSet* const rs = (RuntimePropSet*)Current->ReadPropValue();
		if (rs->setIndicator == 'S')
			return new Prop_UInt(rs->setIndexSelected);

		// Combo box is stored as its item text, so items can be altered between versions.
		// Fusion expects the returned property to be the index of the item in combo box, so we have to do a lookup.
		assert(rs->setIndicator == 'L');

		std::string str((const char *)&Current->ReadPropValue()[1], Current->ReadPropValueSize() - 1);
		const json_value & propJSONItems = propJSON["Items"sv];
		if (propJSONItems.type != json_array)
		{
			MsgBox::Error(_T("Property error"), _T("GetProperty error: JSON Items not valid in Combo Box property \"%s\", ID %i, language %s."),
				UTF8ToTString(propJSON["Title"sv]).c_str(), PropID, JSON::LanguageName());
			return nullptr;
		}
		std::size_t itemIndex = -1;
		for (std::size_t i = 0; i < propJSONItems.u.array.length; ++i)
		{
			assert(propJSONItems[i].type == json_string);
			if (str == (std::string_view)propJSONItems[i])
			{
				itemIndex = i;
				break;
			}
		}
		if (itemIndex == -1)
		{
			MsgBox::Error(_T("Property error"), _T("GetProperty error: JSON Items does not contain given value %s (Combo Box property \"%s\", ID %zu, JSON language %s)."),
				UTF8ToTString(str).c_str(), UTF8ToTString(propJSON["Title"sv]).c_str(), PropID, JSON::LanguageName());
			return nullptr;
		}
		ret = new Prop_UInt(itemIndex);
	}
	else
		MsgBox::Error(_T("Property error"), _T("GetProperty error: JSON property type \"%hs\" is unexpected."), curStr.data());

	return ret;
}

void * DarkEdif::DLL::DLL_GetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID_)
{
	LOGV(_T("Call to %s with edPtr %p.\n"), _T(__FUNCTION__), edPtr);

	constexpr std::uint32_t curExtVersion = Properties::VersionFlags | Extension::Version;

	if (edPtr->eHeader.extVersion != curExtVersion)
	{
		// MFA is older than current ext - we should've upgraded already
		if (edPtr->eHeader.extVersion < curExtVersion)
		{
			MsgBox::Error(_T("Version upgrade failure"), _T("Extension version mismatch (MFA is %lu, real ext is %i). "
				"Fusion should have called UpdateEditStructure, but didn't."), edPtr->eHeader.extVersion & ~Properties::VersionFlags, Extension::Version);
		}
		else // MFA is newer than current version - old ext MFX?
		{
			MsgBox::Error(_T("Version upgrade failure"), _T("Extension version mismatch (MFA is %lu, real ext is %i). "
				"You should update the %s extension immediately, WITHOUT saving the MFA first."), edPtr->eHeader.extVersion & ~Properties::VersionFlags, Extension::Version, _T("" PROJECT_NAME));
		}
		return nullptr;
	}
	if (edPtr->eHeader.extPrivateData == 0)
	{
		MsgBox::Error(_T("No smart properties"), _T("MFA ext version %lu was saved WITHOUT smart properties, but current extension has same version %d WITH smart properties.\n\n"
			"Did the extension developer upgrade the extension to smart properties without incrementing Extension::Version?"), edPtr->eHeader.extVersion, Extension::Version);
		return nullptr;
	}
	auto& Props = Elevate(edPtr->Props);
	return Props.GetProperty(PropID_);
}

void DarkEdif::DLL::DLL_SetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID, void * Param)
{
	LOGV(_T("Call to %s with edPtr %p.\n"), _T(__FUNCTION__), edPtr);
	if (!GetPropRealID(PropID))
		return;

	Prop* prop = (Prop*)Param;
	std::uint32_t i = prop->GetClassID();
	auto& Props = Elevate(edPtr->Props);

	switch (i)
	{
		case 'DATA': // Buffer or string
		{
			const json_value & propjson = CurLang["Properties"sv][PropID];
			// Buff can be used for a string property
			if (IsStringPropType(propjson["Type"sv], false))
			{
				std::string utf8Str = DarkEdif::TStringToUTF8(((Prop_Str *)prop)->String);
				Props.Internal_PropChange(mV, edPtr, PropID, utf8Str.c_str(), utf8Str.size());
			}
			// If we get a Buff and it's not a string property, DarkEdif doesn't know how to handle it.
			else if (SVICompare(propjson["Type"sv], "Image List"sv))
			{
				DebugProp_OutputString(_T("Setting image list: size %zu."), ((Prop_Buff*)prop)->Size);
				Props.Internal_PropChange(mV, edPtr, PropID, ((Prop_Buff*)prop)->Address, ((Prop_Buff*)prop)->Size);
			}
			else
			{
				MsgBox::Error(_T("DarkEdif property error"), _T("Got Buff type for non-string property \"%s\"."),
					UTF8ToTString(propjson["Title"sv]).c_str());
			}
			break;
		}
		case 'STRA': // ANSI string
		{
			std::string utf8Str = DarkEdif::ANSIToUTF8(((Prop_AStr *)prop)->String);
			Props.Internal_PropChange(mV, edPtr, PropID, utf8Str.c_str(), utf8Str.size());
			break;
		}
		case 'STRW': // Unicode string
		{
			std::string utf8Str = DarkEdif::WideToUTF8(((Prop_WStr *)prop)->String);
			Props.Internal_PropChange(mV, edPtr, PropID, utf8Str.c_str(), utf8Str.size());
			break;
		}
		case 'INT ': // 4-byte signed int
		{
			Prop_SInt * prop2 = (Prop_SInt *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(int));
			break;
		}
		case 'DWRD': // 4-byte unsigned int
		{
			Prop_UInt * prop2 = (Prop_UInt *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(unsigned int));
			break;
		}
		case 'INT2': // 8-byte signed int
		{
			Prop_Int64 * prop2 = (Prop_Int64 *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(std::int64_t));
			break;
		}
		case 'DBLE': // 8-byte floating point var
		{
			Prop_Double * prop2 = (Prop_Double *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(double));
			break;
		}
		case 'FLOT': // 4-byte floating point var
		{
			Prop_Float * prop2 = (Prop_Float *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(float));
			break;
		}
		case 'SIZE': // Two ints depicting a size
		{
			Prop_Size * prop2 = (Prop_Size *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->X, sizeof(int) * 2);
			break;
		}
		default: // Custom property
		{
			Prop_Custom * prop2 = (Prop_Custom *)prop;
			//Internal_PropChange(mV, edPtr, PropID, prop2->GetPropValue(), prop2->GetPropValueSize());

			MsgBox::Info(_T("Custom property"), _T("Assuming class ID %i is custom - but no custom code yet written."), i);
			break;
		}
	}
	ScanForDynamicPropChange(mV, edPtr, PropID);
}
BOOL DarkEdif::DLL::DLL_GetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	LOGV(_T("Call to %s with edPtr %p.\n"), _T(__FUNCTION__), edPtr);

	if (!GetPropRealID(PropID))
		return FALSE;

	// converts ID from JSON index to real within IsPropChecked
	return edPtr->Props.IsPropChecked(PropID);
}
// Evaluates the passed property, reading the nth JSON Properties, and the text of the item in that Property
// and evaluating boolean and text expression.
int EvaluateProperty(mv* mV, EDITDATA* edPtr, std::size_t i, const std::string_view & text)
{
	auto& elevProps = Elevate(edPtr->Props);
	const json_value& props = CurLang["Properties"sv];
	const json_value& p = props[i];
	if (p[text].type != json_string)
		return 1;

	std::string req(p[text].u.string.ptr, p[text].u.string.length);
	std::size_t start;
	while ((start = req.find('[')) != std::string::npos)
	{
		std::size_t end = req.find(']', start);
		if (end == std::string::npos)
		{
			MsgBox::Error(_T("DarkEdif property error"), _T("Malformed JSON: %s value \"%s\" is invalid."),
				UTF8ToTString(text).c_str(), UTF8ToTString(p[text]).c_str());
			return -1;
		}
		std::string propName = req.substr(start + 1, end - (start + 1));
		std::size_t index = 0;
		for (std::size_t i = 0; i < props.u.array.length; ++i)
		{
			if (SVICompare(props[i]["Title"sv], propName.c_str()))
			{
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			MsgBox::Error(_T("DarkEdif property error"), _T("Malformed JSON: %s property name \"%s\" does not exist."),
				UTF8ToTString(text).c_str(), UTF8ToTString(propName).c_str());
			return -1;
		}
		const Properties::Data* data = elevProps.Internal_DataAt(index);
		std::string result;
		if (IsStringPropType(props[index]["Type"sv], false))
		{
			result = '"';
			result.append((const char*)data->ReadPropValue(), data->ReadPropValueSize());
			result += '"';
		}
		// Combo box (non-set) is L, then item text
		else if (SVIComparePrefix(props[index]["Type"sv], "Combo Box"sv))
		{
			assert(data->ReadPropValue()[0] == 'L'); // if it's S, that's a prop set list, no good
			result = '"';
			result.append((const char*)&data->ReadPropValue()[1], data->ReadPropValueSize() - 1);
			result += '"';
		}
		else if (data->propTypeID == Edif::Properties::IDs::PROPTYPE_LEFTCHECKBOX)
		{
			const int byteIndex = index / CHAR_BIT, bitIndex = index % CHAR_BIT;
			result = (elevProps.dataForProps[byteIndex] & (1 << bitIndex)) != 0 ? "true"sv : "false"sv;
		}
		else
		{
			MsgBox::Error(_T("DarkEdif property error"), _T("Malformed JSON: evaluation of \"%s\" included "
				"property name \"%s\", which is not usable in evaluation."),
				UTF8ToTString(text).c_str(), UTF8ToTString(propName).c_str());
			return -1;
		}

		std::string newReq = req.substr(0, start) + result + req.substr(end + 1);
		req = newReq;
	}

	while (true)
	{
		if (req == "true"sv || req == "false"sv)
		{
			DebugProp_OutputString(_T("Evaluated property %s (JSON ID %zu) item %s: result %s.\n"),
				UTF8ToTString(p["Title"sv]).c_str(), i, UTF8ToTString(text).c_str(), UTF8ToTString(req).c_str());
			return (req == "true"sv) ? 1 : 0;
		}

		// Boolean flip, !true, !false
		if (req._Starts_with("!true"sv))
		{
			req.replace(0, "!true"sv.size(), "false"sv);
			continue;
		}
		if (req._Starts_with("!false"sv))
		{
			req.replace(0, "!false"sv.size(), "true"sv);
			continue;
		}

		// "text"=="sometext", permitting spaces around operator, and backslash escaped double quotes
		std::regex reg(R"X(("(?:(?=(?:\\?)).)*?")\s*(==|!=)\s*("(?:(?=(?:\\?)).)*?")\s*)X"s);
		std::match_results<std::string::const_iterator> results;
		if (std::regex_search(req, results, reg, std::regex_constants::match_any))
		{
			DebugProp_OutputString(_T("regex search found results:\n"));
			DebugProp_OutputString(_T("results=%s\n"), UTF8ToTString(results.str()).c_str());
			DebugProp_OutputString(_T("LHS results[1]=%s\n"), UTF8ToTString(results[1].str()).c_str());
			DebugProp_OutputString(_T("op  results[2]=%s\n"), UTF8ToTString(results[2].str()).c_str());
			DebugProp_OutputString(_T("RHS results[3]=%s\n"), UTF8ToTString(results[3].str()).c_str());

			// results[1] = "text"
			// results[2] = ==
			// results[3] = "sometext"
			std::string partA = results[1].str();
			std::string op = results[2].str();
			std::string partB = results[3].str();
			if (op != "!="sv && op != "=="sv)
			{
				MsgBox::Error(_T("DarkEdif property error"), _T("Malformed JSON: %s property name \"%s\" "
					"uses invalid comparison operator \"%s\"."),
					UTF8ToTString(text).c_str(), UTF8ToTString(p["Title"sv]).c_str(), UTF8ToTString(op).c_str());
				return -1;
			}
			std::transform(partA.begin(), partA.end(), partA.begin(),
				[](std::uint8_t c) { return std::tolower(c); });
			std::transform(partB.begin(), partB.end(), partB.begin(),
				[](std::uint8_t c) { return std::tolower(c); });

			if ((op == "=="sv && partA == partB) ||
				(op == "!="sv && partA != partB))
			{
				DebugProp_OutputString(_T("Will trim from index %zu, so \"%s\".\n"),
					results.length(), UTF8ToTString(req.substr(results.position(0) + results.length())).c_str());
				req = req.substr(0, results.position(0)) + "true"s + req.substr(results.position(0) + results[0].length());
			}
			else
			{
				DebugProp_OutputString(_T("Will trim from index %zu, so \"%s\".\n"),
					results.length(), UTF8ToTString(req.substr(results.position(0) + results.length())).c_str());
				req = req.substr(0, results.position(0)) + "false"s + req.substr(results.position(0) + results[0].length());
			}
			DebugProp_OutputString(_T("Trim to:\n"));
			DebugProp_OutputString(_T("%s\n"), UTF8ToTString(req).c_str());
			continue;
		}

		// bool compare: true || false, or true && false; nothing more complicated than that.
		// The text should already be converted to trues and false when this is reached.
		// Possibly this could be optimized by allowing a true returned in above if to
		// scan for a following || and shortcut the entire expression into true;
		// as brackets are not supported in expression evaluator - but again, I don't think
		// performance is that much of a concern.

		std::regex regBoolean(R"X((true|false)\s*(\|\||\&\&)\s*(true|false)\s*)X"s);
		if (std::regex_search(req, results, regBoolean, std::regex_constants::match_any))
		{
			DebugProp_OutputString(_T("regex search found results:\n"));
			DebugProp_OutputString(_T("results=%s\n"), UTF8ToTString(results.str()).c_str());
			DebugProp_OutputString(_T("LHS results[1]=%s\n"), UTF8ToTString(results[1].str()).c_str());
			DebugProp_OutputString(_T("op  results[2]=%s\n"), UTF8ToTString(results[2].str()).c_str());
			DebugProp_OutputString(_T("RHS results[3]=%s\n"), UTF8ToTString(results[3].str()).c_str());

			// results[1] = "text"
			// results[2] = ==
			// results[3] = "sometext"
			std::string partA = results[1].str();
			std::string op = results[2].str();
			std::string partB = results[3].str();
			if (op != "||"sv && op != "&&"sv)
			{
				MsgBox::Error(_T("DarkEdif property error"), _T("Malformed JSON: %s property name \"%s\" "
					"uses invalid combination operator \"%s\"."),
					UTF8ToTString(text).c_str(), UTF8ToTString(p["Title"sv]).c_str(), UTF8ToTString(op).c_str());
				return -1;
			}
			std::transform(partA.begin(), partA.end(), partA.begin(),
				[](std::uint8_t c) { return std::tolower(c); });
			std::transform(partB.begin(), partB.end(), partB.begin(),
				[](std::uint8_t c) { return std::tolower(c); });

			if ((partA != "true"sv && partA != "false"sv) ||
				(partB != "true"sv && partB != "false"sv))
			{
				MsgBox::Error(_T("DarkEdif property error"), _T("Malformed JSON: %s property name \"%s\" "
					"became invalid boolean comparison \"%s\"."),
					UTF8ToTString(text).c_str(), UTF8ToTString(p["Title"sv]).c_str(),
					UTF8ToTString(results.str()).c_str());
				return -1;
			}
			const bool partATrue = partA == "true"sv,
				partBTrue = partB == "true"sv;

			if ((op == "||"sv && (partATrue || partBTrue)) ||
				(op == "&&"sv && (partATrue && partBTrue)))
			{
				DebugProp_OutputString(_T("Will trim from index %zu, so \"%s\".\n"),
					results.length(), UTF8ToTString(req.substr(results.position(0) + results.length())).c_str());
				req = req.substr(0, results.position(0)) + "true"s + req.substr(results.position(0) + results[0].length());
			}
			else
			{
				DebugProp_OutputString(_T("Will trim from index %zu, so \"%s\".\n"),
					results.length(), UTF8ToTString(req.substr(results.position(0) + results.length())).c_str());
				req = req.substr(0, results.position(0)) + "false"s + req.substr(results.position(0) + results[0].length());
			}
			DebugProp_OutputString(_T("Trim to:\n"));
			DebugProp_OutputString(_T("%s\n"), UTF8ToTString(req).c_str());
			continue;
		}

		MsgBox::Error(_T("DarkEdif property error"), _T("%s property \"%s\" was parsed into\n%s\n... which is unparseable."),
			UTF8ToTString(text).c_str(), UTF8ToTString(p["Title"sv]).c_str(), UTF8ToTString(req).c_str());
		return -1;
	}

	// Still to do: combo boxes comparison, ideally with an "IN" type operator
	// Can we edit combo box items based on a comparison?
}

void TriggerShowForRange(const json_value &props, DLL::PropAccesser &elevProps, mv * mV, EDITDATA * edPtr,
	std::size_t & startRange, std::size_t& endRange, bool recursiveShow = false)
{
	DebugProp_OutputString(_T("Doing a show of property range %hs to %hs (ID %zu to %zu).\n"),
		props[startRange]["Title"sv].c_str(), props[endRange]["Title"sv].c_str(), startRange, endRange);
	for (std::size_t i = startRange; i <= endRange; ++i)
		elevProps.visibleEditorProps[i] = true;

	// if recursive, delete all properties that are already after this one, re-add after
	std::vector<std::pair<std::size_t, std::size_t>> toAddRanges;

	if (!recursiveShow)
	{
		std::size_t subStartRange = -1, subEndRange = -1;
		for (std::size_t i = endRange + 1; i < props.u.array.length; ++i)
		{
			if (elevProps.visibleEditorProps[i])
			{
				DebugProp_OutputString(_T("Removing %s (ID %zu).\n"),
					UTF8ToTString(props[i]["Title"sv]).c_str(), i);
				mvRemoveProp(mV, edPtr, i + PROPID_EXTITEM_CUSTOM_FIRST);

				if (subEndRange == -1)
				{
					subStartRange = subEndRange = i;
					continue;
				}
				if (subEndRange == i - 1)
				{
					++subEndRange;
					continue;
				}

				toAddRanges.push_back(std::make_pair(subStartRange, subEndRange));
				subStartRange = subEndRange = -1;
			}
			if (subEndRange != -1)
			{
				toAddRanges.push_back(std::make_pair(subStartRange, subEndRange));
				subStartRange = subEndRange = -1;
			}
		}
		if (subEndRange != -1)
			toAddRanges.push_back(std::make_pair(subStartRange, subEndRange));
	}

	PropData* end = &Edif::SDK->EdittimeProperties[props.u.array.length];
	// Pass a list with all-zero prop data after
	// There's always a PropData to write to in EdittimeProperties, as it
	// ends with an all-zero PropData by Fusion editor design
	PropData temp = Edif::SDK->EdittimeProperties[endRange + 1];
	memset(&Edif::SDK->EdittimeProperties[endRange + 1], 0, sizeof(PropData));
	mvInsertProps(mV, edPtr, &Edif::SDK->EdittimeProperties[startRange], startRange + PROPID_EXTITEM_CUSTOM_FIRST - 1, TRUE);
	Edif::SDK->EdittimeProperties[endRange + 1] = temp;

	for (auto r : toAddRanges)
	{
		DebugProp_OutputString(_T("Doing re-add of property range %hs to %hs (ID %zu to %zu).\n"),
			props[std::get<0>(r)]["Title"sv].c_str(), props[std::get<1>(r)]["Title"sv].c_str(), std::get<0>(r), std::get<1>(r));
		TriggerShowForRange(props, elevProps, mV, edPtr, std::get<0>(r), std::get<1>(r), true);
	}

	startRange = endRange = -1;
};

void ScanForDynamicPropChange(mv* mV, EDITDATA* edPtr, unsigned int JSONPropIDAltered = UINT32_MAX)
{
	// We should scan only the property that this property affects,
	// but this property doesn't store it - the ones affected do.
	(void)JSONPropIDAltered;

	// TODO: We could cache which properties affect which. However, the number of properties
	// surely can't be high enough to make that optimization necessary...
	// A second optimization would be to check all props, but scan for this property name in the text.

	auto& elevProps = Elevate(edPtr->Props);
	const json_value& props = CurLang["Properties"sv];

	auto diffArr = std::make_unique<std::uint8_t[]>(props.u.array.length);

	std::size_t startRange = -1, endRange = -1;

	for (std::size_t i = 0; i < props.u.array.length; ++i)
	{
		const json_value& p = props[i];
		int res = EvaluateProperty(mV, edPtr, i, "ShowIf");
		// If -1, some error occurred and was reported
		if (res == -1)
			break;
		if (res == 1)
		{
			if (elevProps.visibleEditorProps[i])
			{
				DebugProp_OutputString(_T("Detected property %s (JSON ID %zu) should be visible, and already is. Doing nothing.\n"),
					UTF8ToTString(p["Title"sv]).c_str(), i);
				if (endRange != -1)
					TriggerShowForRange(props, elevProps, mV, edPtr, startRange, endRange);
				continue;
			}

			DebugProp_OutputString(_T("Detected property %s (JSON ID %zu) should be visible, and isn't. Enabling it.\n"),
				UTF8ToTString(p["Title"sv]).c_str(), i);
			// We either track or extend a range of newly shown properties
			// Once we've finished the range, TriggerShowForRange will run and insert all the properties at once.
			if (startRange == -1)
			{
				startRange = endRange = i;
				continue;
			}
			assert(endRange == i - 1);
			++endRange;
		}
		else // res == 0
		{
			if (endRange != -1)
				TriggerShowForRange(props, elevProps, mV, edPtr, startRange, endRange);

			if (elevProps.visibleEditorProps[i])
			{
				elevProps.visibleEditorProps[i] = false;

				DebugProp_OutputString(_T("Detected property %s should be disabled, and isn't. Disabling it.\n"),
					UTF8ToTString(p["Title"sv]).c_str());
				mvRemoveProp(mV, edPtr, i + PROPID_EXTITEM_CUSTOM_FIRST);
			}
			DebugProp_OutputString(_T("Detected property %s should be invisible, and already is. Doing nothing.\n"),
				UTF8ToTString(p["Title"sv]).c_str());
			//mvRefreshProp(mV, edPtr, i + 1 + PROPID_EXTITEM_CUSTOM_FIRST);
		}
	}
	if (endRange != -1)
		TriggerShowForRange(props, elevProps, mV, edPtr, startRange, endRange);

	// Trigger IsPropEnabled for all properties that specify EnableIf,
	// as this may affect them
	for (std::size_t i = 0; i < props.u.array.length; ++i)
		if (props[i]["EnableIf"sv].type == json_string)
			mvRefreshProp(mV, edPtr, i + PROPID_EXTITEM_CUSTOM_FIRST, FALSE);
}

void DarkEdif::DLL::DLL_SetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID, BOOL checked)
{
	LOGV(_T("Call to %s with edPtr %p.\n"), _T(__FUNCTION__), edPtr);
	if (!GetPropRealID(PropID))
		return;

	auto& elevProps = Elevate(edPtr->Props);
	const unsigned int RealPropID = elevProps.PropIdxFromJSONIdx(PropID);

	// The dataForProps consists of a set of chars, whereby each bit in the char is the "checked"
	// value for the Prop ID specified. Thus each char supports 8 properties.
	const int byteIndex = RealPropID / CHAR_BIT, bitIndex = RealPropID % CHAR_BIT;

	if (checked != FALSE)
		elevProps.dataForProps[byteIndex] |= 1 << bitIndex;
	else
		elevProps.dataForProps[byteIndex] &= ~(1 << bitIndex);

	ScanForDynamicPropChange(mV, edPtr, PropID);
	LOGV(_T("Call to %s with edPtr %p DONE.\n"), _T(__FUNCTION__), edPtr);
}
BOOL DarkEdif::DLL::DLL_IsPropEnabled(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	if (!GetPropRealID(PropID))
		return FALSE;

	LOGV(_T("Call to %s with edPtr %p, prop ID %u.\n"), _T(__FUNCTION__), edPtr, PropID);

	// Safety abort - properties failed to init, and we'll crash if we go much further
	if (Elevate(edPtr->Props).propVersion == 0)
		return FALSE;

	int res = EvaluateProperty(mV, edPtr, PropID, "EnableIf");
	// If -1, some error occurred and was reported
	if (res == -1)
		return TRUE; // Show by default

	const std::string_view propName = CurLang["Properties"sv][PropID]["Title"sv];
	(void)propName;
	// A button for deleting set also needs us to check our set isn't the last entry
	if (res == 1)
	{
		auto& elevProps = Elevate(edPtr->Props);
		for (const auto& ps : Edif::SDK->EdittimePropertySets)
		{
			if (ps.deleteButtonIdx == PropID)
			{
				const RuntimePropSet* const rs = (RuntimePropSet *)elevProps.Internal_DataAt(ps.nameListIdx, true)->ReadPropValue();
				assert(rs->setIndicator == 'S');
				return rs->numRepeats > 1;
			}
		}

		DebugProp_OutputString(_T("Detected property %s should be enabled.\n"),
			UTF8ToTString(propName).c_str());
		return TRUE;
	}

	// else res == 0
	DebugProp_OutputString(_T("Detected property %s should be disabled.\n"),
		UTF8ToTString(propName).c_str());
	return FALSE;
}

int FusionAPI EnumElts(mv* mV, EDITDATA* edPtr, ENUMELTPROC enumProc, ENUMELTPROC undoProc, LPARAM p1, LPARAM p2)
{
#pragma DllExportHint
	LOGV(_T("Call to %s with edPtr %p.\n"), _T(__FUNCTION__), edPtr);
	// Note from Yves that the textual font properties - that is, TEXT_OEFLAG_EXTENSION,
	// GetTextFont(), do not need their fonts enumed here, they are automatically grabbed during build.
	//
	// Direct3D 11 apps:	All LOGFONT used by TEXT_OEFLAG_EXTENSION automatically stored.
	//						Only TTF fonts are supported by D3D11.
	// Direct3D 8 + 9 apps: Will need to use Font Embed object, the one for Windows.
	//						It's manual: you embed the font yourself in Data Elements,
	//						then extract it at runtime, then pass the extracted path to its action.
	//						This supports all fonts.
	// Android apps:		Requires the Android Font Embed object to embed fonts.
	// iOS + Mac apps:		You have to manually add the font into your xcodeproj's Resources,
	//						and then add them to the plist as UIAppFonts. Don't just put them in
	//						the Resources directory, you must link them.
	//						You may also need to add the font to Build Phases.
	//						https://community.clickteam.com/forum/thread/67739-custom-fonts/

	// Note that enumProc may change the image or font number passed to it.
	int error = 0;
	std::vector<std::pair<std::uint16_t*, int>> IdAndType;

	const auto& realProps = Elevate(edPtr->Props);
	if (realProps.propVersion != 'DAR2')
	{
		DarkEdif::MsgBox::Error(_T("DarkEdif property error"), _T("Properties aren't as expected inside EnumElts()."));
		return 0;
	}

	const DarkEdif::Properties::Data* d = realProps.Internal_FirstData();
	for (std::size_t i = 0, j = realProps.numProps; i < j; ++i)
	{
		if (d->propTypeID == Edif::Properties::IDs::PROPTYPE_FONT)
		{
			LOGV(_T("Adding font of property %s from font bank ID %hu.\n"),
				UTF8ToTString(d->ReadPropName()).c_str(), *(std::uint16_t*)d->ReadPropValue());
			if ((error = enumProc((std::uint16_t*)d->ReadPropValue(), FONT_TAB, p1, p2)) != 0)
			{
				MsgBox::Error(_T("EnumElts"), _T("Adding font property %s, font ID %hu failed! Error %d.\n"),
					UTF8ToTString(d->ReadPropName()).c_str(), *(std::uint16_t *)d->ReadPropValue(), error);
				goto postLoop;
			}
			IdAndType.emplace_back(std::make_pair((std::uint16_t*)d->ReadPropValue(), FONT_TAB));
		}
		else if (d->propTypeID == Edif::Properties::IDs::PROPTYPE_IMAGELIST)
		{
			ImgListProperty* imgProp = (ImgListProperty*)d->ReadPropValue();
			LOGV(_T("Adding images of property %s, num IDs %hu...\n"),
				UTF8ToTString(d->ReadPropName()).c_str(), imgProp->numImages);
			for (std::size_t i = 0; i < imgProp->numImages; ++i)
			{
				if ((error = enumProc(&imgProp->imageIDs[i], IMG_TAB, p1, p2)) != 0)
				{
					MsgBox::Error(_T("EnumElts"), _T("Adding image of property %s, ID %hu failed! Error %d.\n"),
						UTF8ToTString(d->ReadPropName()).c_str(), imgProp->imageIDs[i], error);
					goto postLoop;
				}
				IdAndType.emplace_back(std::make_pair(&imgProp->imageIDs[i], IMG_TAB));
			}
		}
		d = d->Next();
	}
postLoop:
	// In case of error, undo it
	if (error != 0)
	{
		for (int i = IdAndType.size() - 1; i >= 0; --i)
			undoProc(std::get<0>(IdAndType[i]), std::get<1>(IdAndType[i]), p1, p2);
	}
	return error;
}

struct Properties::PreSmartPropertyReader : Properties::PropertyReader
{
	//int numProps;
	const std::uint8_t * at = nullptr;
	size_t atID = 0;
	const std::uint8_t * endPos = nullptr;
	const Properties::ConverterState * convState = nullptr;
	size_t maxChkboxID = 0;
	const std::uint8_t * chkBoxAt = nullptr;

	// Start the reader. Return ConverterUnsuitable if the converter isn't necessary.
	void Initialize(ConverterState &convState, ConverterReturn * const convRet)
	{
		this->convState = &convState;
		if (!convState.oldEdPtr || convState.oldEdPtr->eHeader.extPrivateData != 0)
		{
			atID = 0;
			endPos = nullptr;
			if (!convState.oldEdPtr)
				DebugProp_OutputString(_T("PreSmartPropertyReader: No old edPtr, not initing.\n"));
			else
				DebugProp_OutputString(_T("PreSmartPropertyReader: Private ext data is non-null, assuming smart properties.\n"));
			return Abort(convRet);
		}

		int sizeOfProps = *(int *)(&convState.oldEdPtr->Props);
		if (sizeOfProps < 0 || sizeOfProps < sizeof(EDITDATA::eHeader))
		{
			DebugProp_OutputString(_T("PreSmartPropertyReader: size in bytes of props is %i, not %u, expecting wrong converter.\n"),
				sizeOfProps, convState.oldEdPtr->eHeader.extPrivateData);
			return Abort(convRet);
		}

		sizeOfProps -= sizeof(EDITDATA::eHeader);

		at = ((const std::uint8_t *)&convState.oldEdPtr->Props) + sizeof(sizeOfProps);

		// Add offset for skipping checkbox data
		// note this is based on current JSON's property count, not when data was stored,
		// so it's going to be invalid if user appended too many properties.
		// Since num properties isn't stored, best guess is all we can hope for anyway.

		size_t numChkBoxBytes = (size_t)ceil(convState.jsonProps.u.array.length / 8.0f);
		maxChkboxID = (numChkBoxBytes * 8U) - 1;
		chkBoxAt = at;
		at += numChkBoxBytes;

		atID = 0;
		endPos = ((const std::uint8_t *)&convState.oldEdPtr->Props) + sizeOfProps;

		DebugProp_OutputString(_T("PreSmartPropertyReader: Initialised OK. %i bytes in properties, Fusion reports %u bytes without eHeader.\n"),
			sizeOfProps, convState.oldEdPtr->eHeader.extSize - sizeof(EDITDATA::eHeader));
		return convRet->Return_OK(nullptr, 0);
	}

	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetProperty() as if IDs can be skipped.
	void GetProperty(size_t id, ConverterReturn * const convRet)
	{
		// Converter unavailable already
		if (at == nullptr)
			return Abort(convRet);

		// Some to skip - don't pass convRet, just null
		while (id > atID + 1)
		{
			if (Next(nullptr) == false)
			{
				maxChkboxID = atID - 1;
				return Abort(convRet);
			}

			++atID;
		}

		// convRet will be set regardless. False means no more data, which means this ID is inaccessible,
		// making last readable prop ID atID - 1, so chkbox max is atID - 1 too.
		if (Next(convRet) == false)
			maxChkboxID = atID - 1;

		++atID;
	}


	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetPropertyCheckbox() as if IDs can be skipped.
	void GetPropertyCheckbox(size_t id, ConverterReturn * const convRet)
	{
		if (id > maxChkboxID)
			return convRet->Return_ConverterUnsuitable();

		convRet->Return_OK(nullptr, 0U, nullptr, (chkBoxAt[id / CHAR_BIT] >> (id % CHAR_BIT)) & 1);
	}


	void Abort(ConverterReturn * const convRet)
	{
		at = nullptr;
		endPos = nullptr;
		convRet->Return_ConverterUnsuitable();
	}
	bool Next(ConverterReturn * const convRet)
	{
		//OutputString("Prop ID %i: read from %p [%u], offset %u.\n", atID, at, at[0],
		//	((unsigned long)at) - ((unsigned long)&convState->oldEdPtr->Props));
		bool b = Next2(convRet);
		//OutputString("Prop ID %i: read end at %p [%u], offset %u.\n", atID, at, at ? at[0] : 'X',
		//	at ? ((unsigned long)at) - ((unsigned long)& convState->oldEdPtr->Props) : 0UL);
		return b;
	}
	// Return false if converter unsuitable.
	bool Next2(ConverterReturn * const convRet)
	{
		using IDs = Edif::Properties::IDs;
		int propTypeID = Edif::SDK->EdittimeProperties[atID].Type_ID % 1000;

		// look up checkbox if we think it's in it
		int chkState = -1;
		if (convRet && atID <= maxChkboxID)
			chkState = (chkBoxAt[atID / CHAR_BIT] >> (atID % CHAR_BIT)) & 1;

		// Hit the end, exactly last property
		if (at >= endPos)
		{
			// Checkboxes are stored outside the queue, so we can still read those
			if (propTypeID == IDs::PROPTYPE_LEFTCHECKBOX)
			{
				if (convRet)
				{
					if (chkState != -1)
						convRet->Return_OK(nullptr, 0U, nullptr, chkState);
					else
					{
						Abort(convRet);
						return false;
					}
				}
				return true;
			}

			maxChkboxID = atID;
			if (convRet)
				Abort(convRet);
			return false;
		}

		switch (propTypeID)
		{
			case IDs::PROPTYPE_LEFTCHECKBOX:
			{
				// Checkbox have no data
				if (convRet)
					convRet->Return_OK(nullptr, 0U, nullptr, chkState);
				return true;
			}
			case IDs::PROPTYPE_STATIC:
			case IDs::PROPTYPE_FOLDER:
			case IDs::PROPTYPE_FOLDER_END:
			case IDs::PROPTYPE_EDITBUTTON:
			{
				// No data for these. Should not be asked for.
				DebugProp_OutputString(_T("PreSmartPropertyReader: Got static type %s (ID %i) for ID %zu. Delegating.\n"),
					UTF8ToTString(Edif::Properties::Names[propTypeID]).c_str(), propTypeID, atID);
				if (convRet)
					convRet->Return_Pass();
				return true;
			}
			case IDs::PROPTYPE_EDIT_STRING:
			case IDs::PROPTYPE_EDIT_MULTILINE:
			case IDs::PROPTYPE_FILENAME:
			case IDs::PROPTYPE_PICTUREFILENAME:
			case IDs::PROPTYPE_DIRECTORYNAME:
			case IDs::PROPTYPE_URLBUTTON:
			{
				std::size_t maxSize = endPos - at;
				if (maxSize <= 0)
				{
					// string went past end of properties.
					DebugProp_OutputString(_T("PreSmartPropertyReader: Was already reading too far for ID %zu. Killing use of converter.\n"), atID);
					return false;
				}
				std::size_t sizeOfStr = maxSize <= 0 ? ~0 : strnlen((const char*)at, maxSize);
				if (sizeOfStr == maxSize)
				{
					// string went past end of properties.
					DebugProp_OutputString(_T("PreSmartPropertyReader: Couldn't find end of string for ID %zu. Killing use of converter.\n"), atID);
					return false;
				}

				// If there is a return, send it back
				if (convRet)
					convRet->Return_OK(sizeOfStr ? at : nullptr, sizeOfStr, nullptr, chkState);
				at += sizeOfStr + 1; // add null byte
				return true;
			}
			// Stores index of item (smart properties store item as string, in case of Items reordering)
			case IDs::PROPTYPE_COMBOBOX:
			case IDs::PROPTYPE_COMBOBOXBTN:
			case IDs::PROPTYPE_ICONCOMBOBOX:
			{
				// ID stored, read applicable string from JSON
				const json_value& itemsJSON = convState->jsonProps[atID]["Items"sv];
				if (itemsJSON.type != json_array || atID >= itemsJSON.u.array.length)
				{
					DebugProp_OutputString(_T("PreSmartPropertyReader: Can't find stored ID %zu in JSON Items array. Delegating.\n"), atID);
					if (convRet)
						convRet->Return_Pass();
					return true; // Item ID no longer exists.
				}
				if (at + sizeof(int) > endPos)
				{
					DebugProp_OutputString(_T("PreSmartPropertyReader: Exceeded sizeof data %zu. Killing converter.\n"), atID);
					return false;
				}

				// If there is a return, send it back
				if (convRet)
				{
					std::string res(itemsJSON[*(int*)at]);
					if (res.size() > 0)
						convRet->Return_OK(_strdup(res.c_str()), res.size(), [](const void* v) { free((void *)v); }, chkState);
					else
						convRet->Return_OK(nullptr, 0, nullptr, chkState);
				}

				at += sizeof(int);
				return true;
			}
			case IDs::PROPTYPE_SIZE:
			{
				if (at + (sizeof(int) * 2) > endPos)
				{
					DebugProp_OutputString(_T("PreSmartPropertyReader: Exceeded sizeof data %zu.\n"), atID);
					return false;
				}

				// If there is a return, send it back
				if (convRet)
					convRet->Return_OK(at, sizeof(int) * 2, nullptr, chkState);

				at += sizeof(int) * 2;
				return true;
			}
			case IDs::PROPTYPE_EDIT_NUMBER:
			case IDs::PROPTYPE_COLOR:
			case IDs::PROPTYPE_SLIDEREDIT:
			case IDs::PROPTYPE_SPINEDIT:
			case IDs::PROPTYPE_DIRCTRL:
			case IDs::PROPTYPE_EDIT_FLOAT:
			case IDs::PROPTYPE_SPINEDITFLOAT:
			{
				// int and float have same size

				// ID stored, read applicable string from JSON
				if (at + sizeof(int) > endPos)
				{
					DebugProp_OutputString(_T("PreSmartPropertyReader: Exceeded sizeof data %zu. Killing converter.\n"), atID);
					return false;
				}

				// If there is a return, send it back
				if (convRet)
					convRet->Return_OK(at, sizeof(int), nullptr, chkState);

				at += sizeof(int);
				return true;
			}
			// Null property at end of SDK::Properties
			case 0:
			case IDs::PROPTYPE_CUSTOM:
			default:
			{
				// End position is important
				if (propTypeID == 0 || at >= endPos)
				{
					DebugProp_OutputString(_T("PreSmartPropertyReader: Hit end of property data (prop ID %zu, type ID %i). Killing converter.\n"),
						atID, propTypeID);
					if (convRet)
						Abort(convRet);
					return false;
				}

				const json_value & jsonTitle = CurLang["Properties"sv][atID]["Title"sv];
				std::tstring jsonTitleStr = jsonTitle.type == json_none ? _T("<not in JSON>"s) : UTF8ToTString(jsonTitle);
				DebugProp_OutputString(_T("PreSmartPropertyReader: Custom or unrecognised property name %s, ID %zu, language %s, type ID %i. Delegating.\n"),
					jsonTitleStr.c_str(), atID, JSON::LanguageName(), propTypeID);
				if (convRet)
					convRet->Return_Pass();
				return true;
			}
		}
	}
};
static Properties::PreSmartPropertyReader preSmartPropertyReader;

struct Properties::SmartPropertyReader : Properties::PropertyReader
{
	std::vector<const Properties::Data *> data;
	const Properties::ConverterState * convState = nullptr;
	bool isVersion1Reading = false;

	// Resets the reader for a new run. Return ConverterUnsuitable if the converter isn't necessary.
	void Initialize(ConverterState &convState, ConverterReturn * const convRet)
	{
		this->convState = &convState;
		data.clear();

		if (!convState.oldEdPtr)
		{
			DebugProp_OutputString(_T("SmartPropertyReader: No old edPtr, not initing.\n"));
			return Abort(convRet);
		}

		if (convState.oldEdPtr->eHeader.extPrivateData == 0 ||
			convState.oldEdPtr->eHeader.extPrivateData >= convState.oldEdPtr->eHeader.extSize)
		{
			DebugProp_OutputString(_T("SmartPropertyReader: offset to Properties is invalid.\n"));
			return Abort(convRet);
		}

		const DLL::PropAccesser& oldProps = Elevate(*convState.oldEdPtrProps);

		const std::uint32_t curVer = 'DAR2';
		isVersion1Reading = oldProps.propVersion == 'DAR1';
		if (!isVersion1Reading && oldProps.propVersion != curVer)
		{
			DebugProp_OutputString(_T("SmartPropertyReader: smart property is implemented with DarkEdif::Properties struct %.4hs, but this reader is %.4hs.\n"),
				(const char *)&oldProps.propVersion, (const char*)&curVer);
			return Abort(convRet);
		}

		// Old version of properties pre-smart properties won't have this.
		// (there's not much chance they have extPrivateData set, but JIC)
		if (oldProps.numProps < 0 || oldProps.sizeBytes > std::numeric_limits<std::uint32_t>::max())
		{
			DebugProp_OutputString(_T("SmartPropertyReader: num properties too low, or size of properties is too high.\n"));
			return Abort(convRet);
		}

		if (oldProps.numProps == 0)
		{
			DebugProp_OutputString(_T("SmartPropertyReader: num properties is zero. No Data reading.\n"));
			return convRet->Return_OK(nullptr, 0U);
		}

		const Data * d = oldProps.Internal_FirstData();
		// v1 does not contain the visibleEditorProps/ptrPad member; move back by 8 bytes
		static_assert(sizeof(Properties) == 28, "Current Properties is not v2?");
		if (isVersion1Reading)
			d = (Data *)(((char*)d) - sizeof(Properties::ptrPad));
		for (std::size_t i = 0; i < oldProps.numProps; ++i)
		{
			data.push_back(d);
			if (isVersion1Reading)
				d = (Data *)((V1Data*)d)->Next();
			else
				d = d->Next();

			// None or overread
			if (d == nullptr || (const std::uint8_t *)d > ((const std::uint8_t *)convState.oldEdPtr) + convState.oldEdPtr->eHeader.extSize)
			{
				return convRet->Return_Error(_T("SmartPropertyReader: failed to read Data for property %zu / %zu.\n"),
					i, oldProps.numProps);
			}
		}
		return convRet->Return_OK(nullptr, 0U);
	}
	// Returns true if the type of Data can be upgraded to the type of second parameter.
	bool IsSimilar(const std::uint16_t propTypeOld, const std::uint16_t propTypeNew)
	{
		// Same type
		if (propTypeOld == propTypeNew)
			return true;

		using IDs = Edif::Properties::IDs;
		// Text-based types are similar enough (we don't allow multiline -> single-line, but we do allow vice versa)
		if ((propTypeOld == IDs::PROPTYPE_DIRECTORYNAME || propTypeOld == IDs::PROPTYPE_EDIT_STRING || propTypeOld == IDs::PROPTYPE_FILENAME || propTypeOld == IDs::PROPTYPE_PICTUREFILENAME) &&
			(propTypeNew == IDs::PROPTYPE_EDIT_MULTILINE || propTypeNew == IDs::PROPTYPE_DIRECTORYNAME || propTypeNew == IDs::PROPTYPE_EDIT_STRING || propTypeNew == IDs::PROPTYPE_FILENAME || propTypeNew == IDs::PROPTYPE_PICTUREFILENAME))
		{
			return true;
		}
		// Combo box are the same - we'll only store the item text, so the presence of icons is irrelevant
		if ((propTypeOld == IDs::PROPTYPE_COMBOBOX || propTypeOld == IDs::PROPTYPE_COMBOBOXBTN || propTypeOld == IDs::PROPTYPE_ICONCOMBOBOX) &&
			(propTypeNew == IDs::PROPTYPE_COMBOBOX || propTypeNew == IDs::PROPTYPE_COMBOBOXBTN || propTypeNew == IDs::PROPTYPE_ICONCOMBOBOX))
		{
			return true;
		}
		// All int selection are swappable
		if ((propTypeOld == IDs::PROPTYPE_EDIT_NUMBER || propTypeOld == IDs::PROPTYPE_SPINEDIT || propTypeOld == IDs::PROPTYPE_SLIDEREDIT) &&
			(propTypeNew == IDs::PROPTYPE_EDIT_NUMBER || propTypeNew == IDs::PROPTYPE_SPINEDIT || propTypeNew == IDs::PROPTYPE_SLIDEREDIT))
		{
			return true;
		}
		// Allow int -> float, but not vice versa
		if ((propTypeOld == IDs::PROPTYPE_EDIT_FLOAT || propTypeOld == IDs::PROPTYPE_SPINEDITFLOAT ||
			propTypeOld == IDs::PROPTYPE_EDIT_NUMBER || propTypeOld == IDs::PROPTYPE_SPINEDIT || propTypeOld == IDs::PROPTYPE_SLIDEREDIT) &&
			(propTypeNew == IDs::PROPTYPE_EDIT_FLOAT || propTypeNew == IDs::PROPTYPE_SPINEDITFLOAT))
		{
			return true;
		}

		// Later, we can expand this function by checking lengths are within bounds and max/min, etc.

		return false;
	}

	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetProperty() as if IDs can be skipped.
	void GetProperty(std::size_t id, ConverterReturn * const convRet)
	{
		const json_value & propJ = convState->jsonProps[id];

		const std::string_view jsonPropName = propJ["Title"sv];

		// If a property has "OldTitle", we'll check for that first
		// Worth noting if you rename a -> b and b -> a, a will simply be put as a.
		const std::string_view jsonPropOldName = propJ["OldTitle"sv];

		int jsonPropTypeID = -1;

		// Loop through Parameter names and compareth them.
		for (std::size_t j = Edif::Properties::IDs::PROPTYPE_FIRST_ITEM; j < Edif::Properties::IDs::PROPTYPE_LAST_ITEM; ++j)
		{
			if (SVICompare(propJ["Type"sv], Edif::Properties::Names[j]))
			{
				jsonPropTypeID = j;
				break;
			}
		}

		if (jsonPropTypeID == -1)
		{
			return convRet->Return_Error(_T("SmartPropertyReader: failed to read Data for property %zu. Type ID in JSON %s could not be converted to ID.\n"),
				id, UTF8ToTString(propJ["Type"sv]).c_str());
		}

		auto f = data.cend();
		// Find property by old name
		if (!jsonPropOldName.empty())
		{
			f = std::find_if(data.cbegin(), data.cend(), [&](const Data* d) {
				return SVICompare(d->ReadPropName(), jsonPropOldName) && IsSimilar(d->propTypeID, jsonPropTypeID);
			});
			if (f != data.cend())
			{
				DebugProp_OutputString(_T("SmartPropertyReader: found property %zu %s by old name %s, at index %zu.\n"),
					id, UTF8ToTString(jsonPropName).c_str(), UTF8ToTString(jsonPropOldName).c_str(), std::distance(data.cbegin(), f));
			}
		}
		if (f == data.cend())
		{
			// Find property by name (the "smart" part)
			f = std::find_if(data.cbegin(), data.cend(), [&](const Data* d) {
				return SVICompare(ReadPropName(d), jsonPropName) && IsSimilar(d->propTypeID, jsonPropTypeID);
			});
			if (f != data.cend())
			{
				DebugProp_OutputString(_T("SmartPropertyReader: found property %zu, name %s at index %zu.\n"),
					id, UTF8ToTString(jsonPropName).c_str(), std::distance(data.cbegin(), f));
			}
		}

		if (f != data.cend())
		{
			const std::size_t foundIndex = std::distance(data.cbegin(), f);
			const auto& p = GetRealPropsAddress((EDITDATA*)convState->oldEdPtr);
			if (!isVersion1Reading)
			{
				const Data* const d = *f;
				return convRet->Return_OK(d->ReadPropValue(), d->ReadPropValueSize(),
					nullptr, p.IsPropChecked((int)foundIndex));
			}

			const V1Data* const v1data = (const V1Data*)*f;

			// Although the Data::propJSONIndex was added in v2, that's not handled here,
			// but in the caller stringstream writer.
			// Combo Box lists in Prop v1 -> v2 were given a L set indicator prefix,
			// other Data contents were not altered.
			if (!IsComboBoxType(v1data->propTypeID))
			{
				return convRet->Return_OK(v1data->ReadPropValue(), v1data->ReadPropValueSize(),
					nullptr, p.IsPropChecked((int)foundIndex));
			}

			char* const fakeStr = (char *)malloc(1 + v1data->ReadPropValueSize());
			fakeStr[0] = 'L';
			memcpy(fakeStr + 1, v1data->ReadPropValue(), v1data->ReadPropValueSize());
			return convRet->Return_OK(fakeStr, 1 + v1data->ReadPropValueSize(),
				[](const void* v) { free((void *)v); }, p.IsPropChecked((int)foundIndex));
		}

#if 0
		// Name not found? fall back on looking up by ID
		// I've disabled this because it would lead to returning any property that matched. I want it smarter than that.

		if (data.size() <= id)
		{
			return convRet->Return_Error(_T("SmartPropertyReader: failed to read Data for property %zu, only %zu Datas are stored.\n"),
				id, data.size());
		}

		if (data[id]->propTypeID != jsonPropTypeID)
		{
			DebugProp_OutputString(_T("SmartPropertyReader: failed to read Data for property %zu (json name %s). Name and ID was not found in old data.\n"),
				id, UTF8ToTString(jsonPropName).c_str());
			return convRet->Return_Pass();
		}

		convRet->Return_OK(data[id]->ReadPropValue(), data[id]->ReadPropValueSize(), nullptr, convState->oldEdPtrProps->IsPropChecked(id));
#else
		DebugProp_OutputString(_T("SmartPropertyReader: property ID %zu couldn't be found by title \"%s\" or old title \"%s\". Passing it on for reset.\n"),
			id, UTF8ToTString(jsonPropName).c_str(), UTF8ToTString(jsonPropOldName).c_str());
		convRet->Return_Pass();
#endif
	}

	std::string ReadPropName(const Data* d) const
	{
		if (isVersion1Reading)
			return ((const V1Data*)d)->ReadPropName();
		return d->ReadPropName();
	}

	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetPropertyCheckbox() as if IDs can be skipped.
	void GetPropertyCheckbox(size_t id, ConverterReturn * const convRet)
	{
		const auto &p = GetRealPropsAddress((EDITDATA *)convState->oldEdPtr);
		// Added new property since last smart ext version? Delegate to JSON
		if (id >= p.numProps)
			return convRet->Return_Pass();

		return convRet->Return_OK(NULL, 0, NULL, p.IsPropChecked(id));
	}

	void Abort(ConverterReturn * const convRet)
	{
		data.clear();
		return convRet->Return_ConverterUnsuitable();
	}
};
static Properties::SmartPropertyReader smartPropertyReader;

struct Properties::JSONPropertyReader : Properties::PropertyReader
{
	Properties::ConverterState * convState = nullptr;
	static const char bullet[];

	// Start the reader. Return ConverterUnsuitable if the converter isn't necessary.
	void Initialize(ConverterState &convState, ConverterReturn * const convRet)
	{
		this->convState = &convState;

		// Are JSON Properties item an array?
		if (convState.jsonProps.type != json_array || convState.jsonProps.u.array.length > 1000)
		{
			// There is no Properties item in the JSON file. Consider the JSON valid, but with no properties.
			if (convState.jsonProps.type == json_none)
				DebugProp_OutputString(_T("JSONPropertyReader: No Properties item. Going to initialize with 0 properties.\n"));
			else // Properties is not an array - nani?
			{
				MsgBox::Error(_T("Property error"), _T("JSONPropertyReader: Properties is the wrong type, expected an array.\n"));
				return Abort(convRet);
			}
		}

		DebugProp_OutputString(_T("JSONPropertyReader: Initialised OK. %u properties.\n"),
			convState.jsonProps.u.array.length);
		return convRet->Return_OK(nullptr, 0U);
	}

	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetProperty() as if IDs can be skipped.
	void GetProperty(size_t id, ConverterReturn * const convRet)
	{
		if (convState->jsonProps.u.array.length <= id)
		{
			return convRet->Return_Error(_T("JSONPropertyReader: no JSON item for property %zu, only %u items are stored"),
				id, convState->jsonProps.u.array.length);
		}

		const json_value &prop = *convState->jsonProps.u.array.values[id];
		if (prop.type != json_object)
			return convRet->Return_Error(_T("JSDNPropertyReader: Index %zu is not a json_object (but within Properties array)"), id);
		const std::string_view title = prop["Title"sv].type == json_string ? prop["Title"sv] : "<missing title>"sv;

		using IDs = Edif::Properties::IDs;
		int propTypeID = Edif::SDK->EdittimeProperties[id].Type_ID % 1000;
		switch (propTypeID)
		{
		case IDs::PROPTYPE_STATIC:
		case IDs::PROPTYPE_FOLDER:
		case IDs::PROPTYPE_FOLDER_END:
		case IDs::PROPTYPE_EDITBUTTON:
		case IDs::PROPTYPE_LEFTCHECKBOX:
		case IDs::PROPTYPE_URLBUTTON:
		case IDs::PROPTYPE_GROUP:
		{
			// No data for these. We store just metadata so getting property by index has consistent indexes
			DebugProp_OutputString(_T("JSDNPropertyReader: Got static type %i (%hs) for property title %s, ID %zu. Storing just metadata.\n"),
				propTypeID, Edif::Properties::Names[propTypeID], UTF8ToTString(title).c_str(), id);
			return convRet->Return_OK(nullptr, 0U);
		}
		case IDs::PROPTYPE_EDIT_STRING:
		case IDs::PROPTYPE_EDIT_MULTILINE:
		case IDs::PROPTYPE_FILENAME:
		case IDs::PROPTYPE_PICTUREFILENAME:
		case IDs::PROPTYPE_DIRECTORYNAME:
		{
			const std::size_t maxSize = 4096;
			if (((std::string_view)prop["DefaultState"sv]).size() >= maxSize)
			{
				// string went past end of properties.
				DebugProp_OutputString(_T("JSONPropertyReader: Couldn't find end of string for property %s, ID %zu (language %s). Will "
					"delegate this property and any further properties."), UTF8ToTString(title).c_str(), id, JSON::LanguageName());
				return Abort(convRet);
			}

			// convState->resetPropertiesStream << bullet << title << " = \"" << (const char *)prop["DefaultState"] << "\"\n";
			convState->resetPropertiesStream << bullet << title << '\n';
			++convState->numPropsReset;
			std::string data(prop["DefaultState"sv]); // copy
			if (SVICompare(prop["Case"sv], "Lower"sv))
				std::transform(data.begin(), data.end(), data.begin(), [](std::uint8_t c) { return std::tolower(c); });
			else if (SVICompare(prop["Case"sv], "Upper"sv))
				std::transform(data.begin(), data.end(), data.begin(), [](std::uint8_t c) { return std::toupper(c); });
			return convRet->Return_OK(_strdup(data.c_str()), data.size(), [](const void* v) { free((void *)v); });
		}
		// Stores text of item, but checks it's in Items
		case IDs::PROPTYPE_COMBOBOX:
		case IDs::PROPTYPE_COMBOBOXBTN:
		case IDs::PROPTYPE_ICONCOMBOBOX:
		{
			const json_value& itemsJSON = prop["Items"sv];
			// This either stores the text of the drop-down item selected, or a RuntimePropSet
			// They are prefaced with L or S inside the EDITDATA to indicate which,
			// as Properties is not in a minifed JSON
			auto edPropIt = std::find_if(Edif::SDK->EdittimePropertySets.cbegin(),
				Edif::SDK->EdittimePropertySets.cend(),
				[id](const EdittimePropSet& es) { return es.nameListIdx == id; });
			if (edPropIt != Edif::SDK->EdittimePropertySets.cend())
			{
				const auto& edProp = *edPropIt;
				if (itemsJSON.type != json_none)
				{
					MsgBox::Error(_T("Property error"), _T("JSONPropertyReader: combo box name list %s has Items, which should not be used for a property set name list."),
						UTF8ToTString(title).c_str(), UTF8ToTString(edProp.setName).c_str());
				}

				const std::size_t rpsMemSize = sizeof(RuntimePropSet) + edProp.setName.size();
				std::uint8_t * const rpsMem = (std::uint8_t *)malloc(rpsMemSize);
				assert(rpsMem);
				RuntimePropSet* const rps = (RuntimePropSet *)rpsMem;
				rps->setIndicator = 'S';
				rps->firstSetJSONPropIndex = edProp.startSetIdx;
				rps->setNameJSONPropIndex = edProp.nameEditboxIdx;
				rps->lastSetJSONPropIndex = edProp.endSetIdx;
				rps->numRepeats = 1;
				rps->setIndexSelected = 0;
				memcpy(rps->setName, edProp.setName.c_str(), edProp.setName.size());
				convState->resetPropertiesStream << bullet << title << '\n';
				++convState->numPropsReset;
				return convRet->Return_OK(rpsMem, rpsMemSize, [](const void* v) { free((void *)v); });
			}

			// Standard list combo box
			if (itemsJSON.type != json_array && itemsJSON.type != json_null)
			{
				DebugProp_OutputString(_T("JSONPropertyReader: Can't find Items for JSON item %s, ID %zu, language %s. Passing onto next converter."),
					UTF8ToTString(title).c_str(), id, JSON::LanguageName());
				return convRet->Return_Pass(); // Item ID no longer exists.
			}

			const size_t maxSize = 4096;
			const std::string_view str = prop["DefaultState"sv];
			if (str.size() >= maxSize)
			{
				// string went past end of properties.
				DebugProp_OutputString(_T("JSONPropertyReader: Couldn't find end of string for JSON item %s, ID %zu, language %s. Will "
					"pass on this property and any further properties."), UTF8ToTString(title).c_str(), id, JSON::LanguageName());
				return Abort(convRet);
			}

			// convState->resetPropertiesStream << bullet << title << " = \"" << (const char *)prop["DefaultState"] << "\"\n";
			convState->resetPropertiesStream << bullet << title << '\n';
			++convState->numPropsReset;

			char* strDup = (char *)malloc(1 + str.size());
			assert(strDup);
			strDup[0] = 'L';
			memcpy(strDup + 1, str.data(), str.size());
			return convRet->Return_OK(strDup, 1 + str.size(), [](const void* v) { free((void*)v); });
		}
		case IDs::PROPTYPE_SIZE:
		{
			if (prop["DefaultState"sv].type != json_array || prop["DefaultState"sv].u.array.length != 2)
			{
				DebugProp_OutputString(_T("JSONPropertyReader: Couldn't read default state for JSON item %s, ID %zu, language %s. Erroring."),
					UTF8ToTString(title).c_str(), id, JSON::LanguageName());
				return Abort(convRet);
			}

			static int both[2] = { 0, 0 };
			both[0] = (int)(json_int_t)prop["DefaultState"sv][0];
			both[1] = (int)(json_int_t)prop["DefaultState"sv][1];

			// convState->resetPropertiesStream << bullet << title << " = (" << both[0] << ", " << both[1] << ")\n";
			convState->resetPropertiesStream << bullet << title << '\n';
			++convState->numPropsReset;

			return convRet->Return_OK(both, sizeof(both));
		}
		case IDs::PROPTYPE_EDIT_NUMBER:
		case IDs::PROPTYPE_COLOR:
		case IDs::PROPTYPE_SLIDEREDIT:
		case IDs::PROPTYPE_SPINEDIT:
		case IDs::PROPTYPE_DIRCTRL:
		{
			if (prop["DefaultState"sv].type != json_integer)
			{
				// Wrong data type for this property.
				return convRet->Return_Error(_T("JSONPropertyReader: JSON item %s, ID %zu, language %s has no default value."),
					UTF8ToTString(title).c_str(), id, JSON::LanguageName());
			}

			// JSON stores integers as long long (64-bit signed), but Fusion only allows 32-bit (signed).
			const std::int64_t intDataAsLong = prop["DefaultState"sv].u.integer;
			if (intDataAsLong > INT32_MAX || intDataAsLong < INT32_MIN)
			{
				return convRet->Return_Error(_T("JSONPropertyReader: JSON item %s, ID %zu, language %s has default value that can't be stored in 32-bit int (value is %lld)."),
					UTF8ToTString(title).c_str(), id, JSON::LanguageName(), intDataAsLong);
			}

			// convState->resetPropertiesStream << bullet << title << " = " << intDataAsLong << '\n';
			convState->resetPropertiesStream << bullet << title << '\n';
			++convState->numPropsReset;

			static int intData2;
			intData2 = (int)intDataAsLong;

			return convRet->Return_OK(&intData2, sizeof(int));
		}
		case IDs::PROPTYPE_EDIT_FLOAT:
		case IDs::PROPTYPE_SPINEDITFLOAT:
		{
			if (prop["DefaultState"sv].type != json_double)
			{
				// Wrong data type for this property.
				return convRet->Return_Error(_T("JSONPropertyReader: JSON item %s, ID %zu, language %s has no default value."),
					UTF8ToTString(title).c_str(), id, JSON::LanguageName());
			}

			static float f;
			f = (float)prop["DefaultState"sv].u.dbl;

			// convState->resetPropertiesStream << bullet << title << " = " << std::setprecision(3) << f << '\n';
			convState->resetPropertiesStream << bullet << title << '\n';
			++convState->numPropsReset;

			return convRet->Return_OK(&f, sizeof(float));
		}
		case IDs::PROPTYPE_IMAGELIST:
		{
			// Image list should have no images
			if (prop["DefaultState"sv].type != json_none)
			{
				return convRet->Return_Error(_T("JSONPropertyReader: JSON item %s, ID %zu, language %s has a default value, and shouldn't have one."),
					UTF8ToTString(title).c_str(), id, JSON::LanguageName());
			}

			// TODO: Check options are valid
			std::uint16_t numImages = (std::uint16_t)(json_int_t)prop["NumImages"sv];
			assert(numImages > 0);
			// increment for numImages prefix
			ImgListProperty* prop = (ImgListProperty*)calloc(++numImages, 2);
			prop->numImages = numImages - 1;
			// convState->resetPropertiesStream << bullet << title << " = image count 0\n";
			convState->resetPropertiesStream << bullet << title << '\n';
			++convState->numPropsReset;

			return convRet->Return_OK(prop, numImages * 2, [](const void* f) { free((void *) f); });
		}

		case IDs::PROPTYPE_CUSTOM:
		default:
		{
			if (propTypeID >= IDs::PROPTYPE_CUSTOM && propTypeID <= IDs::PROPTYPE_CUSTOM + 9)
			{
				DebugProp_OutputString(_T("JSONPropertyReader: Custom property name %s, ID %zu, language %s, type ID %d. Passing on, hopefully to user converter.\n"),
					UTF8ToTString(title).c_str(), id, JSON::LanguageName(), propTypeID);
				return convRet->Return_Pass();
			}

			DebugProp_OutputString(_T("JSONPropertyReader: Property ID %zu, name %s, language %s, type ID %u. Delegating.\n"),
				id, UTF8ToTString(title).c_str(), JSON::LanguageName(), propTypeID);
			return convRet->Return_Pass();
		}
		}
	}

	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetPropertyCheckbox() as if IDs can be skipped.
	void GetPropertyCheckbox(size_t id, ConverterReturn * const convRet)
	{
		if (convState->jsonProps.u.array.length <= id)
		{
			return convRet->Return_Error(_T("JSONPropertyReader: no JSON item for property %zu, only %u items are stored.\n"),
				id, convState->jsonProps.u.array.length);
		}

		const json_value &prop = *convState->jsonProps.u.array.values[id];
		if (prop.type != json_object)
			return convRet->Return_Error(_T("JSDNPropertyReader: Index %zu is not a json_object (but within Properties array).\n"), id);
		const std::string_view title = prop["Title"sv].type == json_string ? prop["Title"sv] : "<missing title>"sv;

		using IDs = Edif::Properties::IDs;
		int propTypeID = Edif::SDK->EdittimeProperties[id].Type_ID % 1000;
		if (propTypeID == IDs::PROPTYPE_LEFTCHECKBOX)
		{
			if (prop["DefaultState"sv].type == json_boolean)
			{
				// convState->resetPropertiesStream << bullet << title << " = " << (prop["DefaultState"] ? "true" : "false") << '\n';
				convState->resetPropertiesStream << bullet << title << '\n';
				++convState->numPropsReset;
				return convRet->Return_OK(nullptr, 0U, nullptr, ((bool)prop["DefaultState"sv]) ? 1 : 0);
			}

			return convRet->Return_Error(_T("JSONPropertyReader: DefaultState is not set for checkbox property. Property %s, ID %zu (language %s). Erroring.\n"),
				UTF8ToTString(title).c_str(), id, JSON::LanguageName());
		}

		if (prop["CheckboxDefaultState"sv].type != json_none)
		{
			if (prop["CheckboxDefaultState"sv].type == json_boolean)
			{
				// convState->resetPropertiesStream << bullet << title << " = " << (prop["DefaultState"] ? "true" : "false") << '\n';
				convState->resetPropertiesStream << bullet << title << '\n';
				++convState->numPropsReset;
				return convRet->Return_OK(nullptr, 0U, nullptr, ((bool)prop["CheckboxDefaultState"sv]) ? 1 : 0);
			}

			return convRet->Return_Error(_T("JSONPropertyReader: the CheckboxDefaultState is not set for property with checkbox flag. Property %s, ID %u (language %s). Erroring.\n"),
				UTF8ToTString(title).c_str(), id, JSON::LanguageName());
		}

		DebugProp_OutputString(_T("JSONPropertyReader: Property %s, ID %u (language %s) is not a checkbox-related property. Delegating.\n"),
			UTF8ToTString(title).c_str(), id, JSON::LanguageName());
		return convRet->Return_Pass();
	}

	void Abort(ConverterReturn * const convRet)
	{
		return convRet->Return_ConverterUnsuitable();
	}
};
static Properties::JSONPropertyReader jsonPropertyReader;
// static definition - UTF-8 bullet point, or ASCII
#ifdef _UNICODE
const char Properties::JSONPropertyReader::bullet[5] = { (char)0xE2, (char)0x80, (char)0xA2, ' ', '\0' };
#else
const char Properties::JSONPropertyReader::bullet[3] = { '>', ' ', '\0' };
#endif

// Default fallback for test_has_UserConverter when EDITDATA can't be used because it has no matching function: false
template <class T, typename = void>
struct test_has_UserConverter : std::false_type { };

// test_has_UserConverter when EDITDATA has a matching function: true!
template <class T>
struct test_has_UserConverter <T, std::enable_if_t<std::is_invocable_r<DarkEdif::Properties::PropertyReader*, decltype(T::UserConverter)>::value> >
	: std::true_type { };

// Wrapper for calling UserConverter; if it doesn't exist, this stops a compile-time error
template <typename T>
	typename std::enable_if_t<!test_has_UserConverter<T>::value, DarkEdif::Properties::PropertyReader *>
	UserConverterWrap()
{
	// This function is a dummy so the compiler thinks UserConverterWrap always has a definition, but it shouldn't be run,
	// because in the scenario where UserConverterWrap is executed, the below override should be what is compiled into the code instead.
	LOGF(_T("Running dummy UserConverterWrap(). Should not happen.\n"));
	return nullptr;
};
// Wrapper when EDITDATA has a usable UserConverter function: just call it!
template <typename T>
	typename std::enable_if_t<test_has_UserConverter<T>::value, DarkEdif::Properties::PropertyReader*>
		UserConverterWrap() {
	return T::UserConverter();
}

// Default fallback for test_has_MigrateMiddle when EDITDATA can't be used because it has no matching function: false
template <class T, typename = void>
struct test_has_MigrateMiddle : std::false_type { };

// test_has_MigrateMiddle when EDITDATA has a matching function: true!
template <class T>
struct test_has_MigrateMiddle <T, std::enable_if_t<std::is_invocable_r<bool, decltype(T::MigrateMiddle), mv *, const EDITDATA* const, EDITDATA* const>::value> >
	: std::true_type { };

// Wrapper for calling UserConverter; if it doesn't exist, this stops a compile-time error
template <typename T>
	typename std::enable_if_t<!test_has_MigrateMiddle<T>::value, bool>
	MigrateMiddleWrap(mv* mV, const EDITDATA* const oldEdPtr, EDITDATA* newEdPtr)
{
	// This function is a dummy so the compiler thinks MigrateMiddleWrap always has a definition, but it shouldn't be run,
	// because in the scenario where MigrateMiddleWrap is executed, the below override should be what is compiled into the code instead.
	LOGF(_T("Running dummy MigrateMiddleWrap(). Should not happen.\n"));
	return false;
};
// Wrapper when EDITDATA has a usable MigrateMiddle function: just call it!
template <typename T>
	typename std::enable_if_t<test_has_MigrateMiddle<T>::value, bool>
		MigrateMiddleWrap(mv* mV, const EDITDATA * const oldEdPtr, EDITDATA* newEdPtr)
{
	bool ret = false;
#ifdef _DEBUG
	try
	{
		ret = T::MigrateMiddle(mV, oldEdPtr, newEdPtr);
	}
	catch (...)
	{
		MsgBox::Error(_T("Migration error"), _T("Couldn't %s: did you read the old EDITDATA properly?"),
			oldEdPtr ? _T("copy properties over") : _T("reset properties for new " PROJECT_NAME " object"));
	}
#else
	ret = T::MigrateMiddle(mV, oldEdPtr, newEdPtr);
#endif
	return ret;
}

HGLOBAL FusionAPI UpdateEditStructure(mv * mV, void * OldEdPtr)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_UpdateEditStructure(mV, (EDITDATA*)OldEdPtr);
}

// CRT uses a Vista+ function to threadsafe-init, which means static constructors don't run;
// if this bug will happen, ThreadSafeStaticInitIsSafe is not defined
#ifdef ThreadSafeStaticInitIsSafe
static bool staticPropsStructCtored = true;
#else
static bool staticPropsStructCtored = false;
#endif
// While we can't read CEditApp, we do know it is different for each loaded MFA.
// This lets us check if we're loading the same frame and prevent repetitive popups.
static void* lastCEditApp = nullptr;

HGLOBAL DarkEdif::DLL::DLL_UpdateEditStructure(mv* mV, EDITDATA* oldEdPtr)
{
	// Note: GetRunObjectInfos provides the current Extension::Version to Fusion,
	// which causes UpdateEditStructure to be called by Fusion if it doesn't match
	// eHeader.extVersion .

	std::uint32_t oldExtVersion = 0, oldExtVersionWithFlag = Properties::VersionFlags;
	if (oldEdPtr)
	{
		oldExtVersionWithFlag = oldEdPtr->eHeader.extVersion;
		oldExtVersion = oldEdPtr->eHeader.extVersion & ~Properties::VersionFlags;
	}

	std::uint16_t curExtVersionWithFlag = Properties::VersionFlags | Extension::Version;

	DebugProp_OutputString(_T("UpdateEditStructure called; MFA/old version %u, current/new version %i (with flags: %hu).\n"),
		oldExtVersion, Extension::Version, curExtVersionWithFlag);

	// Fusion shouldn't call it for this scenario.
	if (oldEdPtr && oldExtVersionWithFlag == curExtVersionWithFlag)
	{
		DebugProp_OutputString(_T("UpdateEditStructure aborting: versions are equivalent.\n"));

		return nullptr;
	}

	// Fusion will call to upgrade if the MFA is earlier than current ext version.
	// This means the MFA Ext > MFX Ext, so either Fusion user installed an old ext version,
	// or ext dev accidentally reverted their Extension::Version
	if (oldExtVersion > Extension::Version)
	{
		DebugProp_OutputString(_T("UpdateEditStructure warning: MFA version is greater than current version.\n"));

#ifdef _DEBUG
		// If a developer, let them pick what to do. They might've screwed up the MFA version number.
		int msgRet = MsgBox::WarningYesNoCancel(_T("Property update warning"),
			_T("UpdateEditStructure has current ext version %i and MFA has a later version of %i.\n"
			"Did you install an old ext version or revert Extension::Version? If so, cancel and close MFA immediately without saving.\n\n"
			"Otherwise, select an action to take:\n"
			"Yes = run prop updater. Convert MFA version %i to current ext version %i.\n"
			"No = just set the MFA's version number %i to earlier version %i, without changing properties.\n"
			"Cancel = do not change anything, ignore the difference - will likely fail."),
			Extension::Version, oldExtVersion,
			oldExtVersion, Extension::Version,
			oldExtVersion, Extension::Version);
		if (msgRet != IDYES)
		{
			if (msgRet == IDNO)
				oldEdPtr->eHeader.extVersion = curExtVersionWithFlag;
			return nullptr;
		}
#else
		MsgBox::WarningOK(_T("Property update warning"), _T("%s MFA extension version %u is greater than current %s ext version %d. Is the installed extension out of date?"),
			_T("" PROJECT_NAME), oldExtVersion, Extension::Version);
		return nullptr;
#endif
	}

	// oldEdPtr->eHeader.extVersion < Extension::Version at this point, or conversion was requested.

	// While our properties haven't changed, the EDITDATA has.
	long oldOffset = oldEdPtr ? (long)oldEdPtr->eHeader.extPrivateData : 0L;
	const long newOffset = offsetof(EDITDATA, Props);

	const static int PROPID_ITEM_NAME = 21; // user-defined object taken from a Props.h file
	Prop_Str* ps = (Prop_Str*)(oldEdPtr ? mvGetPropValue(mV, oldEdPtr, PROPID_ITEM_NAME) : NULL);
	std::tstring objName = ExtensionName;
	if (ps != nullptr)
	{
		objName = ps->String;
		if (objName != ExtensionName && objName[0] != _T('\0'))
			objName += _T(" (") + ExtensionName + _T(")");
		ps->Delete();
	}

	// Reading pre-smart failed. Only feasible option is to reset.
	// MsgBox::Info("Resetting properties", "Warning: Forced to reset properties for object \"%s\" (%s), "
	//	"due to extension upgrade. Will use default property settings.",
	//	objName.c_str(), ExtensionName.c_str());

	std::ostringstream dataToWriteStream;

	const size_t numPropsIncludingStatics = CurLang["Properties"sv].u.array.length;
	// 1 byte per 8 properties, rounded up
	auto chkboxes = std::make_unique<std::uint8_t[]>((std::size_t)std::ceil((double)numPropsIncludingStatics / 8.0));
	memset(chkboxes.get(), 0, (std::size_t)std::ceil((double)numPropsIncludingStatics / 8.0));

	std::tstring propVersionUpgrade;

	// smart properties
	if (oldOffset != 0L)
	{
		auto& oldProps = Elevate(GetRealPropsAddress(oldEdPtr));

		DebugProp_OutputString(_T("Note: oldOffset = %i, newOffset is %i.\n"), oldOffset, newOffset);

		std::size_t newEdPtrSize = 0;

		// Smart prop version is not DAR2; need a full upgrade
		if (oldProps.propVersion != 'DAR2')
		{
			propVersionUpgrade = _T(" DarkEdif v1 smart properties were upgraded to v2."s);
			DebugProp_OutputString(_T("Properties were upgraded from earlier smart properties.\n"));
		}
		// Names and types match, both old and new version have same properties.
		// The upgrade of Extension::Version did not involve a property change.
		else if (oldProps.hash == ~Edif::SDK->jsonPropsNameAndTypesHash)
		{
			// No changes to EDITDATA outside properties, so Extension::Version change had no change to EDITDATA at all.
			// UpdateEditStructure() thus has nothing to do.
			if (oldOffset == newOffset)
			{
				DebugProp_OutputString(_T("Prop offsets are same. Replacing extVersion as %i and ignoring difference."), Extension::Version);
				oldEdPtr->eHeader.extVersion = curExtVersionWithFlag;
				return NULL; // No change
			}

			// Prop offsets change: properties are same, but EDITDATA changed.
			// Bulk copy all the properties over
			dataToWriteStream.write(((const char*)oldEdPtr) + oldOffset, ((std::streamsize)oldProps.sizeBytes) - oldOffset);
			DebugProp_OutputString(_T("Prop offsets changed, but hashes are the same. Jumping to output.\n"));
			goto ReadyToOutput;
		}
		// Names changed, but types didn't. Perhaps a typo was fixed.
		// All the data we could copy was copied, so now add defaults.
		else if (oldProps.hashTypes == ~Edif::SDK->jsonPropsTypesHash)
			DebugProp_OutputString(_T("Note: Types the same arrangement, but names changed. Assuming cosmetic change.\n"));
		else
			DebugProp_OutputString(_T("Types and names are both different.\n"));
	}
	else // non-smart properties
	{
		// If extPrivateData is 0, the old version was stored without smart JSON properties.
		// Since extVersion is set based on Extension::Version, that's very possible.
		DebugProp_OutputString(_T("Non-smart properties: skipping hash check, since the upgrade to smart properties is needed.\n"));
		//oldOffset = newOffset;
	}


	// so goto is happy
	{
		Properties::ConverterState convState(oldEdPtr, CurLang["Properties"sv]);

		struct reader {
			Properties::PropertyReader* ptr;	// singleton only, no garbage collect
			const char* name;					// string literal only, no garbage collect
		};
		std::vector<reader> readers;

		{
			Properties::ConverterReturn retState;
			DLL::ConverterReturnAccessor& retStateAdmin = *(DLL::ConverterReturnAccessor*)&retState;

			// Detect if UserConverter function exists in EDITDATA at compile-time, and init if so
			// This uses template fallback magic so the code will compile into no-op, rather than being unable to compile,
			// if EDITDATA has no matching function
			if constexpr (test_has_UserConverter<EDITDATA>::value)
			{
				Properties::PropertyReader* userConv = UserConverterWrap<EDITDATA>();
				userConv->Initialize(convState, &retState);
				if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				{
					DebugProp_OutputString(_T("User converter init OK; will be used.\n"));
					readers.push_back({ userConv, "UserConverter" });
				}
				else
				{
					DebugProp_OutputString(_T("User converter init failed; will be excluded.\n"));
				}
			}
			else
				DebugProp_OutputString(_T("User converter does not exist; will be excluded.\n"));

			// If XP-compatible, these may be zero-inited but constructors not called (as the CRT uses a Vista+ function),
			// so zero init is disabled, now we run ctor ourselves
			if (!staticPropsStructCtored)
			{
				staticPropsStructCtored = true;
				preSmartPropertyReader.PreSmartPropertyReader::PreSmartPropertyReader();
				smartPropertyReader.SmartPropertyReader::SmartPropertyReader();
				jsonPropertyReader.JSONPropertyReader::JSONPropertyReader();
			}

			preSmartPropertyReader.Initialize(convState, &retState);
			if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				readers.push_back({ &preSmartPropertyReader, "PreSmartPropertyReader" });
			else
				DebugProp_OutputString(_T("Pre-smart property reader init failed; will be excluded.\n"));

			smartPropertyReader.Initialize(convState, &retState);
			if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				readers.push_back({ &smartPropertyReader, "SmartPropertyReader" });
			else
				DebugProp_OutputString(_T("Smart property reader init failed; will be excluded.\n"));

			jsonPropertyReader.Initialize(convState, &retState);
			if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				readers.push_back({ &jsonPropertyReader, "JSONPropertyReader" });
			else
				DebugProp_OutputString(_T("JSON property reader init failed; will be excluded.\n"));

			// EDITDATA makes no sense
			if (readers.empty())
			{
				if (DarkEdif::RunMode == MFXRunMode::SplashScreen)
					MsgBox::Error(_T("Property reading error"), _T("Setting up %s object properties from JSON failed."), _T("" PROJECT_NAME));
				else
					MsgBox::Error(_T("Property conversion error"), _T("All %s property readers have failed. Your property data is corrupt beyond recovery or reset. Please re-add the object to frame."), _T("" PROJECT_NAME));
				return NULL;
			}
			// Only JSON loads, but there's old data: something went wrong
			// with the EDITDATA-based readers.
			if (readers.size() < 2 && oldEdPtr)
			{
				MsgBox::Error(_T("Property reading error"), _T("Parsing existing MFA %s object properties failed."), _T("" PROJECT_NAME));
				return NULL;
			}
		}

		std::vector<int> chkboxIndexesToRead(numPropsIncludingStatics);
		for (std::size_t i = 0; i < numPropsIncludingStatics; ++i)
			chkboxIndexesToRead[i] = i;

		std::vector<reader> chkReaders = readers;
		std::string title;

		DebugProp_OutputString(_T("Starting to read data. Accum data size is now %zu.\n"), std::max(0u, (size_t)dataToWriteStream.tellp()));

		for (std::size_t i = 0; i < numPropsIncludingStatics; ++i)
		{
			Properties::ConverterReturn retState;
			DLL::ConverterReturnAccessor& retStateAdmin = *(DLL::ConverterReturnAccessor*)&retState;
			if (readers.empty())
				return NULL; // error should already be reported when reader failed and removed itself

			// Don't include title for static text types
			if (IsUnchangeablePropExclCheckbox(convState.jsonProps[i]["Type"sv]))
				title.clear();
			else
				title = convState.jsonProps[i]["Title"sv];

			// Note: Unchangeable props are included for IDs to be consistent.
			// So if there's a label at ID 0, then a editbox, and property ID 1 is requested, property ID 0 won't be ignored
			// The JSON property parser will OK the properties.
			// if (IsUnchangeableProp(convState.jsonProps[i]["Type"sv]))
			// 	continue;

			bool ok = false;
			for (std::size_t j = 0; j < readers.size(); ++j)
			{
				readers[j].ptr->GetProperty(i, &retState);

				// Converted OK, no reason for other readers
				if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				{
					ok = true;

					std::uint16_t propTypeID = Edif::SDK->EdittimeProperties[i].Type_ID % 1000;
					std::uint16_t propJSONIndex = (std::uint16_t)i;

					assert(title.size() <= UINT8_MAX - 1); // can't store in titleLen
					std::uint8_t titleLen = (std::uint8_t)title.size();

					DebugProp_OutputString(_T("Prop index %zu (%s) was read by reader %s (%s checkbox).\n"),
						i, UTF8ToTString(title).c_str(), UTF8ToTString(readers[j].name).c_str(),
						retStateAdmin.checkboxState == -1 ? _T("without") : _T("with"));

					std::uint32_t propSize2 = sizeof(Properties::Data) + titleLen + (std::uint16_t)retStateAdmin.dataSize;
					static_assert(sizeof(propTypeID) == sizeof(Properties::Data::propTypeID), "Properties::Data does not match the stream's sizes.");
					static_assert(sizeof(propSize2) == sizeof(Properties::Data::sizeBytes), "Properties::Data does not match the stream's sizes.");
					static_assert(sizeof(propJSONIndex) == sizeof(Properties::Data::propJSONIndex), "Properties::Data does not match the stream's sizes.");
					static_assert(sizeof(titleLen) == sizeof(Properties::Data::propNameSize), "Properties::Data does not match the stream's sizes.");

					// embedded nulls upset the << operator, so write() is preferred.
					dataToWriteStream.write((char*)&propSize2, sizeof(propSize2));
					dataToWriteStream.write((char*)&propTypeID, sizeof(propTypeID));
					dataToWriteStream.write((char*)&propJSONIndex, sizeof(propJSONIndex));
					dataToWriteStream.write((char*)&titleLen, sizeof(titleLen));
					dataToWriteStream.write(title.c_str(), titleLen);

					if (retStateAdmin.data != nullptr)
					{
						// assert(retStateAdmin.dataSize > 0);
						dataToWriteStream.write((const char*)retStateAdmin.data, retStateAdmin.dataSize);

						if (retStateAdmin.freeData != nullptr)
							retStateAdmin.freeData(retStateAdmin.data);
					}

					if (retStateAdmin.checkboxState != -1)
					{
						assert(retStateAdmin.checkboxState == 0 || retStateAdmin.checkboxState == 1);
						if (retStateAdmin.checkboxState == 1)
							chkboxes[i / CHAR_BIT] |= 1 << (i % CHAR_BIT);

						auto f = std::find(chkboxIndexesToRead.cbegin(), chkboxIndexesToRead.cend(), i);
						assert(f != chkboxIndexesToRead.cend());
						chkboxIndexesToRead.erase(f);
					}

					DebugProp_OutputString(_T("Added property %zu - accum data size is now %zu.\n"), i, dataToWriteStream.str().size());
					break;
				}

				// Conversion failed and converter is no longer useable - drop converter
				if (retStateAdmin.convRetType == Properties::ConvReturnType::ConverterUnsuitable)
				{
					readers.erase(readers.cbegin() + j--);
					continue;
				}
				// Converter should've worked, but failed?
				else if (retStateAdmin.convRetType == Properties::ConvReturnType::Error)
				{
					std::tstring err = retStateAdmin.data ? (const TCHAR*)retStateAdmin.data : _T("unknown error");
					if (_istpunct((int)err.back()))
						err.erase(err.size() - 1);
					MsgBox::WarningOK(_T("Property conversion error"), _T("%hs converter encountered a failure: %s. Delegating."),
						readers[j].name, err.c_str());
					if (retStateAdmin.freeData)
						retStateAdmin.freeData(retStateAdmin.data);
					continue;
				}
				//else if (retStateAdmin.convRetType == Properties::ConvReturnType::Delegate)
				//	continue;
			}

			// JSON errors are reported elsewhere, and aren't relevant for upgrading anyway
			if (!ok && RunMode != MFXRunMode::SplashScreen)
			{
				MsgBox::Error(_T("Property conversion error"), _T("All converters have failed for property %i (%s). Your property data is corrupt "
					"beyond recovery or reset. Please re-add the object to frame."), i, UTF8ToTString(convState.jsonProps[i]["Title"sv]).c_str());
			}
		}

		readers = chkReaders;
		for (std::size_t k = 0; k < chkboxIndexesToRead.size(); ++k)
		{
			Properties::ConverterReturn retState;
			DLL::ConverterReturnAccessor& retStateAdmin = *(DLL::ConverterReturnAccessor*)&retState;
			size_t i = chkboxIndexesToRead[k];
			title = convState.jsonProps[i]["Title"sv];

			for (size_t j = 0; j < readers.size(); ++j)
			{
				readers[j].ptr->GetPropertyCheckbox(i, &retState);

				// Converted OK, no reason for other readers
				if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				{
					assert(retStateAdmin.checkboxState == 0 || retStateAdmin.checkboxState == 1);

					if (retStateAdmin.checkboxState != -1)
					{
						DebugProp_OutputString(_T("Prop index %zu (%s) checkbox read by reader %s.\n"),
							i, UTF8ToTString(title).c_str(), UTF8ToTString(readers[j].name).c_str());

						if (retStateAdmin.checkboxState == 1)
							chkboxes[i / CHAR_BIT] |= 1 << (i % CHAR_BIT);

						auto f = std::find(chkboxIndexesToRead.cbegin(), chkboxIndexesToRead.cend(), i);
						assert(f != chkboxIndexesToRead.cend());
						chkboxIndexesToRead.erase(f);
						--k;
					}

					break;
				}

				// Conversion failed and converter is no longer useable - drop converter
				if (retStateAdmin.convRetType == Properties::ConvReturnType::ConverterUnsuitable)
				{
					readers.erase(readers.cbegin() + j--);
					continue;
				}
				else if (retStateAdmin.convRetType == Properties::ConvReturnType::Error)
				{
					MsgBox::Error(_T("Property conversion error"), _T("Couldn't read check box of some properties. Delegating."));
					continue;
				}
				//else if (retStateAdmin.convRetType == Properties::ConvReturnType::Delegate)
				//	continue;
			}
		}

		// If it's an upgrade, make a message box
		if (oldEdPtr != nullptr)
		{
			// Valid upgrade box properties:
			// always				- make popup regardless of whether all properties converted, repeat every frame with ext in
			// always, once			- as above, but once for a MFA, not every frame with ext
			// for reset only		- only make popup if properties had to be reverted to JSON values
			// for reset only, once - as above, but once for a MFA, not every frame with ext
			// never				- never makes popup
			// Default: for reset only, once
			//
			// Worth noting that property upgrade errors will always create an error popup box - this is not affected by this setting.

			std::string upgradeBox = DarkEdif::GetIniSetting("SmartPropertiesUpgradeBox"sv);
			if (upgradeBox.empty())
				upgradeBox = "for reset only, once"sv; // if you edit this default, edit the one below and the hasOnce manual set

			bool hasOnce = upgradeBox.size() > ", once"sv.size() && upgradeBox.substr(upgradeBox.size() - ", once"sv.size()) == ", once"sv;
			const std::string_view upgradeBoxNoOnce = std::string_view(upgradeBox).substr(0, upgradeBox.size() - (hasOnce ? ", once"sv.size() : 0));
			// Unrecognised INI setting
			if (upgradeBox != "never"sv && upgradeBoxNoOnce != "for reset only"sv && upgradeBoxNoOnce != "always"sv)
			{
				MsgBox::Error(_T("DarkEdif INI error"), _T(
					R"(Couldn't understand "SmartPropertiesUpgradeBox" setting of "%s".)"
					R"( Valid values are "always", "always, once", "for reset only", "for reset only, once", and "never".)"),
					UTF8ToTString(upgradeBox).c_str());

				// see above comment about editing default
				upgradeBox = "for reset only, once"sv;
				hasOnce = true;
			}

			// mv->EditApp contents is opaque, but the pointer changes when a new MFA is loaded
			// We run a popup only if this is the first time we're seeing this CEditApp address
			bool isDiffApp = lastCEditApp != mV->EditApp;
			lastCEditApp = mV->EditApp;

			// Set to do popup, and if once only, this is a new app and eligible for its one popup
			if (upgradeBox != "never"sv && (!hasOnce || isDiffApp))
			{
				std::string resetPropStr = convState.resetPropertiesStream.str();
				if (resetPropStr.empty())
				{
					if (upgradeBoxNoOnce != "for reset only"sv)
					{
						MsgBox::Info(_T("Upgraded properties"), _T("Successfully upgraded all %u of %s object properties from ext version %lu to version %i."),
							convState.jsonProps.u.array.length, objName.c_str(), oldExtVersion, Extension::Version);
					}
				}
				else
				{
					resetPropStr.resize(resetPropStr.size() - 1U); // remove last line's ending newline
					MsgBox::Info(_T("Upgraded properties"), _T("Successfully copied %u of %s object properties from ext version %lu to %i.%s\n\nAlso set %zu %s to their default settings, namely:\n%s"),
						convState.jsonProps.u.array.length - convState.numPropsReset, objName.c_str(),
						oldExtVersion, Extension::Version,
						propVersionUpgrade.c_str(),
						convState.numPropsReset, convState.numPropsReset == 1 ? _T("property") : _T("properties"), UTF8ToTString(resetPropStr).c_str());
				}
			}
		}
	}

ReadyToOutput:
	std::string dataToWriteString = std::string((const char*)chkboxes.get(), (size_t)ceil(numPropsIncludingStatics / 8.0f)) + dataToWriteStream.str();
	size_t newEdPtrSize = sizeof(EDITDATA) + dataToWriteString.size();

	// GetRunObjectInfos EDITDATA size is 16-bit, so we can't exceed for a new EDITDATA
	// Note that eHeader.extSize is 32-bit, so *editing* a EDITDATA beyond 16-bit is OK
	if (!oldEdPtr && newEdPtrSize >= UINT16_MAX)
	{
		MsgBox::Error(_T("Properties too big"), _T("New EDITDATA was too big to store all the initial properties; needed %zu bytes, but %d is the maximum possible."), newEdPtrSize, UINT16_MAX);
		return NULL;
	}

	// Allocate memory and zero-fill
	HGLOBAL globalPtr = GlobalAlloc(GPTR, newEdPtrSize);
	if (globalPtr == NULL)
	{
		MsgBox::Error(_T("GlobalAlloc failed"), _T("Failed to allocate %zu bytes for data during upgrade."), newEdPtrSize);
		return NULL;
	}

	EDITDATA* newEdPtr = (EDITDATA*)GlobalLock(globalPtr);
	if (newEdPtr == NULL)
		throw std::logic_error("shouldn't fail to lock");
	newEdPtr->eHeader.extPrivateData = newOffset;
	newEdPtr->eHeader.extVersion = curExtVersionWithFlag;
	newEdPtr->eHeader.extSize = newEdPtrSize;
	newEdPtr->eHeader.extID = oldEdPtr ? oldEdPtr->eHeader.extID : 0;
	newEdPtr->eHeader.extMaxSize = std::max(newEdPtrSize, oldEdPtr ? oldEdPtr->eHeader.extMaxSize : 0);

	DLL::PropAccesser& newEdPtrProps = Elevate(newEdPtr->Props);

	newEdPtrProps.propVersion = 'DAR2';
	newEdPtrProps.hash = ~Edif::SDK->jsonPropsNameAndTypesHash;
	newEdPtrProps.hashTypes = ~Edif::SDK->jsonPropsTypesHash;
	newEdPtrProps.sizeBytes = newEdPtrSize;
	newEdPtrProps.ptrPad = 0;
	newEdPtrProps.visibleEditorProps = new bool[numPropsIncludingStatics];
	for (std::size_t i = 0; i < numPropsIncludingStatics; ++i)
		newEdPtrProps.visibleEditorProps[i] = true;
	newEdPtrProps.numProps = (std::uint16_t)numPropsIncludingStatics;

	// Copy data between eHeader and Props, if any present
	// This is either the font for a text ext, or it's nothing.
	//
	// To reset new edPtr's in-between data to zeros:
	// memset(((char*)newEdPtr) + sizeof(EDITDATA::eHeader), 0, newOffset - oldOffset);
	// But the GPTR alloc zero-inits so we don't bother here.

	// If we're migrating, not just creating a new one to check our default EDITDATA size is valid,
	// then try to auto-migrate, then use MigrateMiddle
	if (oldEdPtr)
	{
		// Is there custom info between eHeader and Props, outside of objSize and font
		// Offset checks prevents a "clever" ext dev swapping them
		int expSize = offsetof(EDITDATA, Props) - sizeof(EDITDATA::eHeader);

#if DARKEDIF_DISPLAY_TYPE > DARKEDIF_DISPLAY_ANIMATIONS
		// objSize exists and immediately follows Props: we can handle that
		if constexpr (offsetof(EDITDATA, objSize) == offsetof(EDITDATA, Props))
		{
			expSize -= sizeof(EDITDATA::objSize);
#if TEXT_OEFLAG_EXTENSION
			// and text immediately follows objSize: we can handle that
			if constexpr (offsetof(EDITDATA, font) == offsetof(EDITDATA, Props) + sizeof(EDITDATA::objSize))
				expSize -= sizeof(EDITDATA::font);
#endif
		}
#elif TEXT_OEFLAG_EXTENSION
		// Text immediately follows Props, no objSize: we can handle that
		if constexpr (offsetof(EDITDATA, font) == offsetof(EDITDATA, Props) + sizeof(EDITDATA::Props))
			expSize -= sizeof(EDITDATA::font);
#endif

		// No custom dev data between eHeader and Props, just font/objSize, and layout is what we expect
		if (oldOffset == newOffset && expSize == 0)
		{
			(void)0;
#if DARKEDIF_DISPLAY_TYPE > DARKEDIF_DISPLAY_ANIMATIONS
			newEdPtr->objSize = oldEdPtr->objSize;
#endif
#if TEXT_OEFLAG_EXTENSION
			memcpy(&newEdPtr->font, &oldEdPtr->font, sizeof(EDITDATA::font));
#endif
		}
		else
		{
			// If any custom data is present, then we must migrate it by calling MigrateMiddle
			// If MigrateMiddle is not defined in EDITDATA, error out
			if constexpr (!test_has_MigrateMiddle<EDITDATA>())
			{
				MsgBox::Error(_T("EDITDATA upgrade failed"),
					_T("Failed to migrate %s extension data between MFA and current extension version.\n"
						"No MigrateMiddle function. Contact %s extension developer."),
					_T("" PROJECT_NAME), _T("" PROJECT_NAME));
				GlobalUnlock(globalPtr);
				GlobalFree(globalPtr);
				return NULL;
			}
			// If MigrateMiddle is defined, but doesn't return true, abort
			else if (!MigrateMiddleWrap<EDITDATA>(mV, oldEdPtr, newEdPtr))
			{
				// Assume error already reported
				GlobalUnlock(globalPtr);
				GlobalFree(globalPtr);
				return NULL;
			}
			// else success, migrated OK, fall through
		}
	} // oldEdPtr defined, data between eHeader/Props migration

	// Properties data
	if (memcpy_s(newEdPtrProps.dataForProps, dataToWriteString.size(), dataToWriteString.data(), dataToWriteString.size()))
	{
		MsgBox::Error(_T("memcpy failed"), _T("Failed to copy data during conversion. Error %u. Attempted to copy %u bytes to space for %u."),
			errno, dataToWriteString.size(), newEdPtrSize - newOffset);
		GlobalUnlock(globalPtr);
		GlobalFree(globalPtr);
		return NULL;
	}

	GlobalUnlock(globalPtr);
	return globalPtr;
}

DarkEdif::Properties::ConverterState::ConverterState(EDITDATA* edPtr, const json_value& json)
	: jsonProps(json), oldEdPtr(edPtr), numPropsReset(0U)
{
	if (edPtr && edPtr->eHeader.extPrivateData != 0)
		oldEdPtrProps = &GetRealPropsAddress(edPtr);
	else
		oldEdPtrProps = nullptr;
}

#endif // EditorBuild

// Returns property checked or unchecked from property name.
bool DarkEdif::Properties::IsPropChecked(std::string_view propName) const
{
	const std::uint16_t index = PropJSONIdxFromName(_T("IsPropChecked"), propName);
#if EditorBuild
	const json_value& prop = CurLang["Properties"sv][index];
	// Not checkbox prop, and not with checkbox additional
	if (!SVICompare(prop["Type"sv], Edif::Properties::Names[Edif::Properties::IDs::PROPTYPE_LEFTCHECKBOX]) &&
		prop["CheckboxDefaultState"sv].type == json_type::json_none)
	{
		MsgBox::Error(_T("DarkEdif property error"), _T("IsPropChecked() name = \"%s\" does not have a checkbox in the JSON."), UTF8ToTString(propName).c_str());
		return false;
	}
#endif
	return IsPropChecked(index);
}
// Returns property checked or unchecked from property ID.
bool DarkEdif::Properties::IsPropChecked(int propID) const
{
	const auto &p = Elevate(*this);
	if (propID >= p.numProps)
	{
		LOGF(_T("Can't read property checkbox for property ID %d, the valid ID range is 0 to %hu.\n"), propID, p.numProps);
		return false;
	}
	const std::uint8_t* chkBytes = p.dataForProps;
	const int propJSONIndex = propID;
	// Properties v1 doesn't include visibleEditorProps/ptrPad
	if (p.propVersion == 'DAR1')
		chkBytes -= sizeof(DarkEdif::Properties::ptrPad);
	else
	{
		propID = (int)p.PropIdxFromJSONIdx(propID);
		if (propID == -1)
		{
			MsgBox::Error(_T("DarkEdif property error"), _T("Can't read property checkbox for JSON ID %d, converting JSON ID to real ID failed."), propID);
			return false;
		}
	}

	// The dataForProps consists of a set of chars, whereby each bit in the char is the "checked"
	// value for the Prop ID specified. Thus each char supports 8 properties.
	int byteIndex = propID / CHAR_BIT, bitIndex = propID % CHAR_BIT;
	bool yes = ((chkBytes[byteIndex] >> bitIndex) & 1) != 0;

	DebugProp_OutputString(_T("Prop index %zu (%s) real ID %zu checkbox read by JSON reader: result is %s.\n"),
		propJSONIndex, UTF8ToTString(CurLang["Properties"sv][propJSONIndex]["Title"sv]).c_str(),
		propID, yes ? _T("YES") : _T("NO"));
	return yes;
}
std::tstring DarkEdif::Properties::Internal_GetPropStr(const Properties::Data* data) const
{
	if (!data)
		return std::tstring();
	if (IsStringPropType(data->propTypeID))
	{
		std::tstring ret = UTF8ToTString(std::string_view((const char*)data->ReadPropValue(), data->ReadPropValueSize()));
#ifndef _WIN32
		// On non-Windows, the multiline editbox is stored as CRLF, and we want LF newlines instead
		if (data->propTypeID == Edif::Properties::IDs::PROPTYPE_EDIT_MULTILINE)
			ret.erase(std::remove(ret.begin(), ret.end(), _T('\r')), ret.end());
#endif
		return ret;
	}
	if (IsComboBoxType(data->propTypeID))
	{
		const RuntimePropSet* rs = (RuntimePropSet*)data->ReadPropValue();
		// Plain list: has content with L prefix
		if (rs->setIndicator == 'L')
			return UTF8ToTString(std::string_view((const char*)&data->ReadPropValue()[1], data->ReadPropValueSize() - 1));
		// Contains a RuntimePropSet following an S prefix: get the current set name and return that
		if (rs->setIndicator == 'S')
		{
			const Data* data2 = Elevate(*this).Internal_DataAt(rs->setNameJSONPropIndex, true);
			assert(data2->propTypeID == Edif::Properties::IDs::PROPTYPE_EDIT_STRING);
			return UTF8ToTString(std::string_view((const char*)data2->ReadPropValue(), data2->ReadPropValueSize()));
		}
		LOGF(_T("Unexpected data in list property, expecting L or S prefix, but got '%c'.\n"), (char)rs->setIndicator);
	}
	LOGF(_T("GetPropertyStr() error; prop index %hu (name %s) is not a string property.\n"),
		data->propJSONIndex, UTF8ToTString(data->ReadPropName()).c_str());
	return std::tstring();
}
// Returns std::tstring property setting from property name.
std::tstring DarkEdif::Properties::GetPropertyStr(std::string_view propName) const
{
	return Internal_GetPropStr(Internal_DataAt(PropJSONIdxFromName(_T("GetPropertyStr"), propName), false));
}
// Returns std::tstring property string from property ID.
std::tstring DarkEdif::Properties::GetPropertyStr(int propID) const
{
	return Internal_GetPropStr(Internal_DataAt(propID, true));
}
float DarkEdif::Properties::Internal_GetPropNum(const Properties::Data* data) const
{
	if (!data)
		return 0.f;
	float ret;
	if (data->propTypeID == Edif::Properties::IDs::PROPTYPE_EDIT_NUMBER ||
		data->propTypeID == Edif::Properties::IDs::PROPTYPE_COLOR ||
		data->propTypeID == Edif::Properties::IDs::PROPTYPE_SLIDEREDIT ||
		data->propTypeID == Edif::Properties::IDs::PROPTYPE_SPINEDIT ||
		data->propTypeID == Edif::Properties::IDs::PROPTYPE_DIRCTRL)
	{
		assert(data->ReadPropValueSize() == 4);
		// Integer prop; but since they have a low precision, we'll return as float
		ret = (float)*(int*)data->ReadPropValue();
		return ret;
	}
	if (data->propTypeID == Edif::Properties::IDs::PROPTYPE_EDIT_FLOAT ||
		data->propTypeID == Edif::Properties::IDs::PROPTYPE_SPINEDITFLOAT)
	{
		assert(data->ReadPropValueSize() == 4);
		ret = *(float*)data->ReadPropValue();
		return ret;
	}

	LOGF(_T("GetPropertyNum() error; property name \"%s\", JSON index %hu, is not a numeric property.\n"),
		UTF8ToTString(data->ReadPropName()).c_str(), data->propJSONIndex);
	return 0.f;
}

// Returns a float property setting from property name.
float DarkEdif::Properties::GetPropertyNum(std::string_view propName) const
{
	return Internal_GetPropNum(Internal_DataAt(PropJSONIdxFromName(_T("GetPropertyNum"), propName), true));
}
// Returns float property setting from a property ID.
float DarkEdif::Properties::GetPropertyNum(int propID) const
{
	return Internal_GetPropNum(Internal_DataAt(propID, true));
}

std::uint16_t DarkEdif::Properties::Internal_GetPropertyImageID(const Properties::Data* const data, const std::uint16_t imgIndex) const
{
	if (!data)
		return UINT16_MAX;
	if (data->propTypeID != Edif::Properties::IDs::PROPTYPE_IMAGELIST)
	{
		LOGF(_T("GetPropertyImageID() error; property name \"%s\", JSON index %hu, is not an image list.\n"),
			UTF8ToTString(data->ReadPropName()).c_str(), data->propJSONIndex);
		return UINT16_MAX;
	}
	const ImgListProperty* imgProp = (ImgListProperty*)data->ReadPropValue();
	if (imgIndex >= imgProp->numImages)
	{
		LOGF(_T("GetPropertyImageID() error; property name \"%s\", JSON index %hu, has %hu images, but reading image index %hu.\n"),
			UTF8ToTString(data->ReadPropName()).c_str(), data->propJSONIndex, imgProp->numImages, imgIndex);
		return UINT16_MAX;
	}
	return imgProp->imageIDs[imgIndex];
}
std::uint16_t DarkEdif::Properties::GetPropertyImageID(int propID, std::uint16_t imgIndex) const
{
	return Internal_GetPropertyImageID(Internal_DataAt(propID, true), imgIndex);
}
std::uint16_t DarkEdif::Properties::GetPropertyImageID(std::string_view propName, std::uint16_t imgIndex) const
{
	return Internal_GetPropertyImageID(Internal_DataAt(PropJSONIdxFromName(_T("GetPropertyImageID"), propName), false), imgIndex);
}
std::uint16_t DarkEdif::Properties::Internal_GetPropertyNumImages(const Properties::Data* const data) const
{
	if (!data)
		return UINT16_MAX;
	if (data->propTypeID != Edif::Properties::IDs::PROPTYPE_IMAGELIST)
	{
		LOGF(_T("GetPropertyNumImages() error; property name \"%s\", JSON ID %d is not an image list.\n"),
			UTF8ToTString(data->ReadPropName()).c_str(), data->propJSONIndex);
		return UINT16_MAX;
	}
	assert(data->ReadPropValueSize() >= sizeof(ImgListProperty));
	return ((ImgListProperty*)data->ReadPropValue())->numImages;
}
std::uint16_t DarkEdif::Properties::GetPropertyNumImages(int propID) const
{
	return Internal_GetPropertyNumImages(Internal_DataAt(propID, true));
}
std::uint16_t DarkEdif::Properties::GetPropertyNumImages(std::string_view propName) const
{
	return Internal_GetPropertyNumImages(Internal_DataAt(PropJSONIdxFromName(_T("GetPropertyImageID"), propName), false));
}

DarkEdif::Size DarkEdif::Properties::Internal_GetSizeProperty(const Properties::Data* const data) const
{
	if (!data)
		return Size { -1, -1 };
	if (data->propTypeID != Edif::Properties::IDs::PROPTYPE_SIZE)
	{
		LOGF(_T("GetSizeProperty() error; property name \"%s\", JSON index %hu, is not a size property.\n"),
			UTF8ToTString(data->ReadPropName()).c_str(), data->propJSONIndex);
		return Size { -1, -1 };
	}
	assert(data->ReadPropValueSize() == sizeof(DarkEdif::Size));
	return *(Size*)data->ReadPropValue();
}
DarkEdif::Size DarkEdif::Properties::GetSizeProperty(std::string_view propName) const
{
	return Internal_GetSizeProperty(Internal_DataAt(PropJSONIdxFromName(_T("GetSizeProperty"), propName), false));
}
DarkEdif::Size DarkEdif::Properties::GetSizeProperty(int propID) const
{
	return Internal_GetSizeProperty(Internal_DataAt(propID, true));
}

DarkEdif::Properties::PropSetIterator::PropSetIterator(const std::size_t nameListJSONIdx, const std::size_t numSkippedSetsBefore,
	Properties::Data* runSetEntry, Properties* const props)
	: nameListJSONIdx(nameListJSONIdx), numSkippedSetsBefore(numSkippedSetsBefore), props(props), runSetEntry(runSetEntry)
{
	runPropSet = ((RuntimePropSet*)runSetEntry->ReadPropValue());
	// reset prop set to 0
	runPropSet->setIndexSelected = 0;
	LOGV(_T("Property set %s from %p has ended. Last index was %hu.\n"),
		UTF8ToTString(runPropSet->GetPropSetName(runSetEntry)).c_str(), runSetEntry, curEntryIdx);
}

DarkEdif::Properties::PropSetIterator::PropSetIterator(const std::size_t nameListJSONIdx, const std::size_t numSkippedSetsBefore,
	Properties::Data * runSetEntry, Properties* const props, bool)
	: nameListJSONIdx(nameListJSONIdx), numSkippedSetsBefore(numSkippedSetsBefore), props(props), curEntryIdx(-1), runSetEntry(runSetEntry) {
}

DarkEdif::Properties::PropSetIterator& DarkEdif::Properties::PropSetIterator::operator++()
{
	if (curEntryIdx == UINT16_MAX)
		return *this;
	auto& setIndexSelected = ((RuntimePropSet*)runSetEntry->ReadPropValue())->setIndexSelected;
	if (++curEntryIdx >= GetNumSetRepeats())
	{
		LOGV(_T("Property set %s from %p has ended. Last index was %hu.\n"),
			UTF8ToTString(runPropSet->GetPropSetName(runSetEntry)).c_str(), runSetEntry, curEntryIdx);
		curEntryIdx = UINT16_MAX;
		setIndexSelected = 0;
	}
	else
	{
		LOGV(_T("Incrementing property set %s from %p to index %hu.\n"),
			UTF8ToTString(runPropSet->GetPropSetName(runSetEntry)).c_str(), runSetEntry, curEntryIdx);
		setIndexSelected = curEntryIdx;
	}
	return *this;
}
// x++, instead of ++x
DarkEdif::Properties::PropSetIterator DarkEdif::Properties::PropSetIterator::operator++(int) {
	auto retval = *this; ++(*this); return retval;
}
bool DarkEdif::Properties::PropSetIterator::operator==(PropSetIterator other) const {
	return nameListJSONIdx == other.nameListJSONIdx && numSkippedSetsBefore == other.numSkippedSetsBefore &&
		curEntryIdx == other.curEntryIdx;
}
bool DarkEdif::Properties::PropSetIterator::operator!=(PropSetIterator other) const {
	return !(*this == other);
};
DarkEdif::Properties::PropSetIterator::reference DarkEdif::Properties::PropSetIterator::operator*() const {
	return (void *)(long)curEntryIdx;
}

DarkEdif::Properties::PropSetIterator DarkEdif::Properties::PropSetIterator::begin() const {
	return PropSetIterator(nameListJSONIdx, numSkippedSetsBefore, runSetEntry, props);
}
DarkEdif::Properties::PropSetIterator DarkEdif::Properties::PropSetIterator::end() const {
	return PropSetIterator(nameListJSONIdx, numSkippedSetsBefore, runSetEntry, props, false);
}

void DarkEdif::Properties::PropSetIterator::SetCurrentIndex(std::uint16_t idx) {
	if (idx >= GetNumSetRepeats())
	{
		LOGF(_T("Property set %s from %p can't be set to index %hu. Last possible index was %i.\n"),
			UTF8ToTString(runPropSet->GetPropSetName(runSetEntry)).c_str(), runSetEntry, idx, GetNumSetRepeats() - 1);
	}
	else
	{
		LOGV(_T("Manually setting property set %s from %p to index %hu.\n"),
			UTF8ToTString(runPropSet->GetPropSetName(runSetEntry)).c_str(), runSetEntry, idx);
		runPropSet->setIndexSelected = curEntryIdx = idx;
	}
}
std::uint16_t DarkEdif::Properties::PropSetIterator::GetNumSetRepeats() const {
	return ((RuntimePropSet*)runSetEntry->ReadPropValue())->numRepeats;
}

DarkEdif::Properties::PropSetIterator DarkEdif::Properties::LoopPropSet(std::string_view setName, std::size_t numSkips /* = 0 */) const
{
	Data* d = (Data *)Internal_FirstData();
	for (std::size_t i = 0, j = 0; i < numProps; ++i)
	{
		RuntimePropSet* rs = (RuntimePropSet*)d->ReadPropValue();
		if (IsComboBoxType(d->propTypeID) && rs->setIndicator == 'S' && rs->GetPropSetName(d) == setName && ++j > numSkips)
			return PropSetIterator(i, j - 1, d, const_cast<DarkEdif::Properties *>(this));
		d = d->Next();
	}
	LOGF(_T("No set found with name %s.\n"), DarkEdif::UTF8ToTString(setName).c_str());
	return PropSetIterator(-1, -1, nullptr, const_cast<DarkEdif::Properties*>(this));
}

const Properties::Data * DarkEdif::Properties::Internal_FirstData() const
{
	return (const Data *)(dataForProps + (int)std::ceil(((float)numProps) / 8.0f));
}
const Properties::Data * DarkEdif::Properties::Internal_DataAt(int ID, bool idIsJSON /* = true */) const
{
#if EditorBuild
	const json_value& propsJSON = CurLang["Properties"sv];
	if (propsJSON.type != json_type::json_array)
	{
		MsgBox::Error(_T("Premature function call"), _T("Internal_DataAt() const called for prop ID %u without JSON properties being valid."), ID);
		return nullptr;
	}

	// If it's an unchangable property, no data to return anyway
	// Note there is a Data made for unchangeable, but they have a name length of 0, and no data.
	if (IsUnchangeablePropInclCheckbox(propsJSON[ID]["Type"sv]))
		return nullptr;
#endif

	const Data* data;
	if (idIsJSON)
		ID = (int)PropIdxFromJSONIdx(ID, &data);
	else
	{
		data = Internal_FirstData();
		for (std::size_t i = 0; i < (std::size_t)ID; ++i)
		{
#if EditorBuild
			DebugProp_OutputString(_T("Locating ID %d, at %zu: type %s, title: %s.\n"),
				ID, i, UTF8ToTString(propsJSON[ID]["Type"sv]).c_str(), UTF8ToTString(data->ReadPropName()).c_str());
#endif
			data = data->Next();
		}
	}
#if EditorBuild
	DebugProp_OutputString(_T("DataAt ID %d type %s, title: %s.\n"), ID,
		UTF8ToTString(propsJSON[ID]["Type"sv]).c_str(), UTF8ToTString(data->ReadPropName()).c_str());
#endif
	return data;
}

std::uint16_t DarkEdif::Properties::PropJSONIdxFromName(const TCHAR * func, const std::string_view& propName) const
{
	const auto& p = Elevate(*this);
	const Properties::Data* data = p.Internal_FirstData();
	for (std::size_t index = 0; data && index < p.numProps; ++index)
	{
		if (data->ReadPropName() == propName)
			return data->propJSONIndex;
		data = data->Next();
	}

	LOGF(_T("%s() error; property name \"%s\" does not exist.\n"),
		func, UTF8ToTString(propName).c_str());
	return UINT16_MAX;
}

std::size_t DarkEdif::Properties::PropIdxFromJSONIdx(std::size_t ID, const Data** dataPtr /*= nullptr*/,
	const Data ** rsContainerRetPtr /*= nullptr*/) const
{
	const Data* data = Internal_FirstData();
	std::size_t i = 0;
	RuntimePropSet* rs = nullptr;
	const Data* rsContainer = nullptr;
	while (data->propJSONIndex != ID)
	{
		if (i >= numProps)
		{
			LOGF(_T("Couldn't find property of JSON ID %zu, hit property %zu of %hu stored.\n"), ID, i, numProps);
			return -1;
		}

#ifdef _WIN32
		DebugProp_OutputString(_T("Locating JSON ID %zu, at Prop Index %u: type %s, title: %s.\n"),
			ID, i,
			UTF8ToTString(CurLang["Properties"sv][data->propJSONIndex]["Type"sv]).c_str(),
			UTF8ToTString(data->ReadPropName()).c_str());
		//assert(data->propJSONIndex < ID && i < numProps);

		assert(data->propJSONIndex <= CurLang["Properties"sv].u.array.length);
#endif

		if (IsComboBoxType(data->propTypeID) && data->ReadPropValue()[0] == 'S')
		{
			DebugProp_OutputString(_T("Runtime props at %zu...\n"), i);

			rs = (RuntimePropSet*)data->ReadPropValue();
			rsContainer = data;
			// We're beyond all of this set's JSON range: skip past all repeats
			if (ID > rs->lastSetJSONPropIndex)
			{
				DebugProp_OutputString(_T("Runtime props ends at JSON index %zu, behind sought ID %zu. Moving past...\n"), rs->lastSetJSONPropIndex, ID);
				while (data->propJSONIndex != rs->lastSetJSONPropIndex)
				{
					data = data->Next();
					++i;
				}
				DebugProp_OutputString(_T("Runtime props passed. Now at JSON ID %zu (prop name %s).\n"), data->propJSONIndex, UTF8ToTString(data->ReadPropName()).c_str());
				rs = nullptr;
				rsContainer = nullptr;
			}
			// It's within this set's range
			else if (ID >= rs->firstSetJSONPropIndex && ID <= rs->lastSetJSONPropIndex)
			{
				if (rs->setIndexSelected > 0)
				{
					DebugProp_OutputString(_T("Runtime props ends at JSON %zu, "
						"currently at JSON ID %zu, looking to skip %zu iterations of %zu JSON ID. "
						"Should end on JSON ID %zu.\n"),
						rs->lastSetJSONPropIndex, data->propJSONIndex, rs->setIndexSelected, rs->lastSetJSONPropIndex, rs->firstSetJSONPropIndex);
					for (std::size_t j = 0; ;)
					{
						data = data->Next();
						++i;

						DebugProp_OutputString(_T("RS: ends at JSON %zu, "
							"now at JSON ID %zu, i %zu, j %zu.\n"),
							rs->lastSetJSONPropIndex, data->propJSONIndex, i, j);

						// Skip until end of this entry, then move to next prop
						if (data->propJSONIndex == rs->lastSetJSONPropIndex)
						{
							if (++j == rs->setIndexSelected)
							{
								DebugProp_OutputString(_T("RS: Ending, at JSON %zu, "
									"hitting end at JSON ID %zu, i %zu, j %zu; data text \"%s\".\n"),
									rs->lastSetJSONPropIndex, data->propJSONIndex, i, j,
									UTF8ToTString(std::string_view((char*)data->ReadPropValue(), data->ReadPropValueSize())).c_str());
								data = data->Next();
								++i;
								DebugProp_OutputString(_T("RS: Ending post, at JSON %zu, "
									"hitting end at JSON ID %zu, i %zu, j %zu; data text \"%s\".\n"),
									rs->lastSetJSONPropIndex, data->propJSONIndex, i, j,
									UTF8ToTString(std::string_view((char*)data->ReadPropValue(), data->ReadPropValueSize())).c_str());
								break;
							}
							DebugProp_OutputString(_T("RS: Skip end JSON %zu, "
								"passed an end at JSON ID %zu, i %zu, j %zu; data text \"%s\".\n"),
								rs->lastSetJSONPropIndex, data->propJSONIndex, i, j,
								UTF8ToTString(std::string_view((char*)data->ReadPropValue(), data->ReadPropValueSize())).c_str());
						}
					}
					DebugProp_OutputString(_T("Runtime props rotated to current index %zu. Now at JSON ID %zu (prop index %zu, prop name %s).\n"),
						rs->setIndexSelected, data->propJSONIndex, i, UTF8ToTString(data->ReadPropName()).c_str());
					continue;
				}
				else
				{
					DebugProp_OutputString(_T("Runtime props ends at JSON %zu, within sought JSON ID %zu."
						"Selected set index is 0; continuing...\n"), rs->lastSetJSONPropIndex, ID);
					data = data->Next();
					++i;
					continue;
				}
			}
			// else it's not in this set: continue to standard loop
			else
			{
				rs = nullptr;
				rsContainer = nullptr;
			}
		}
		DebugProp_OutputString(_T("Pre-incr at  JSON %zu, i %zu.\n"),
			data->propJSONIndex, i);

		data = data->Next();
		++i;

		DebugProp_OutputString(_T("Post-incr at JSON %zu, i %zu.\n"),
			data->propJSONIndex, i);
	}

	std::tstring name = _T("none"s);
	if (rs)
		name = UTF8ToTString(rs->GetPropSetName(rsContainer));
	DebugProp_OutputString(_T("Found JSON ID %zu, at Props position %zu; "
		"data name is \"%s\", set name is \"%s\".\n"), ID, i, UTF8ToTString(data->ReadPropName()).c_str(),
		name.c_str());
	if (rsContainerRetPtr)
		*rsContainerRetPtr = rsContainer;
	if (dataPtr)
		*dataPtr = data;

	return i;
}

#if EditorBuild
Properties::Data * DarkEdif::Properties::Internal_FirstData()
{
	return (Data *)(dataForProps + (int)ceil(((float)numProps) / 8.0f));
}
std::size_t DarkEdif::Properties::PropIdxFromJSONIdx(std::size_t ID, Data** dataPtr /*= nullptr*/,
	DarkEdif::Properties::Data** rsContainerRetPtr /* = nullptr*/)
{
	Data* data = Internal_FirstData();
	std::size_t i = 0;
	RuntimePropSet* rs = nullptr;
	Data* rsContainer = nullptr;
	while (data->propJSONIndex != ID)
	{
		DebugProp_OutputString(_T("Locating JSON ID %zu, at Prop Index %u: type %s, title: %s.\n"),
			ID, i,
			UTF8ToTString(CurLang["Properties"sv][data->propJSONIndex]["Type"sv]).c_str(), UTF8ToTString(data->ReadPropName()).c_str());

		// Went off the end
		assert(i < numProps);
		// We went past the JSON index we wanted (ID), and it wasn't from skipping entries deliberately.
		assert(data->propJSONIndex < ID);
		// Prop index is invalid - we did a goof
		assert(data->propJSONIndex <= CurLang["Properties"sv].u.array.length);

		if (IsComboBoxType(data->propTypeID) && data->ReadPropValue()[0] == 'S')
		{
			DebugProp_OutputString(_T("Runtime props at ID %zu, a set combo box...\n"), i);

			rs = (RuntimePropSet*)data->ReadPropValue();
			rsContainer = data;
			// Desired JSON ID is beyond all of this set's repeating JSON range: skip past whole set
			if (ID > rs->lastSetJSONPropIndex)
			{
				DebugProp_OutputString(_T("Runtime props ends at JSON index %zu, behind sought ID %zu. Moving past...\n"), rs->lastSetJSONPropIndex, ID);
				for (std::size_t repIt = 0; repIt < rs->numRepeats; ++repIt)
				{
					while (data->propJSONIndex != rs->lastSetJSONPropIndex)
					{
						data = data->Next();
						++i;
					}

					data = data->Next();
					++i;
				}
				DebugProp_OutputString(_T("Runtime props passed. Now at JSON ID %zu (prop name %s).\n"), data->propJSONIndex, UTF8ToTString(data->ReadPropName()).c_str());
				rs = nullptr;
				rsContainer = nullptr;
				continue;
			}
			// It's within this set's range
			else if (ID >= rs->firstSetJSONPropIndex && ID <= rs->lastSetJSONPropIndex)
			{
				if (rs->setIndexSelected > 0)
				{
					DebugProp_OutputString(_T("Runtime props ends at JSON %zu, "
						"currently at JSON ID %zu, looking to skip %zu iterations of %zu JSON ID. "
						"Should end on JSON ID %zu.\n"),
						rs->lastSetJSONPropIndex, data->propJSONIndex, rs->setIndexSelected, rs->lastSetJSONPropIndex, rs->firstSetJSONPropIndex);
					for (std::size_t j = 0; /*j < rs->setIndexSelected*/;)
					{
						data = data->Next();
						++i;

						DebugProp_OutputString(_T("RS: ends at JSON %zu, "
							"now at JSON ID %zu, i %zu, j %zu.\n"),
							rs->lastSetJSONPropIndex, data->propJSONIndex, i, j);

						// Skip until end of this entry, then move to next prop
						if (data->propJSONIndex == rs->lastSetJSONPropIndex)
						{
							if (++j == rs->setIndexSelected)
							{
								DebugProp_OutputString(_T("RS: Ending, at JSON %zu, "
									"hitting end at JSON ID %zu, i %zu, j %zu; data text \"%s\".\n"),
									rs->lastSetJSONPropIndex, data->propJSONIndex, i, j,
									UTF8ToTString(std::string_view((char*)data->ReadPropValue(), data->ReadPropValueSize())).c_str());
								data = data->Next();
								++i;
								DebugProp_OutputString(_T("RS: Ending post, at JSON %zu, "
									"hitting end at JSON ID %zu, i %zu, j %zu; data text \"%s\".\n"),
									rs->lastSetJSONPropIndex, data->propJSONIndex, i, j,
									UTF8ToTString(std::string_view((char*)data->ReadPropValue(), data->ReadPropValueSize())).c_str());
								break;
							}
							DebugProp_OutputString(_T("RS: Skip end JSON %zu, "
								"passed an end at JSON ID %zu, i %zu, j %zu; data text \"%s\".\n"),
								rs->lastSetJSONPropIndex, data->propJSONIndex, i, j,
								UTF8ToTString(std::string_view((char*)data->ReadPropValue(), data->ReadPropValueSize())).c_str());
						}
					}
					DebugProp_OutputString(_T("Runtime props rotated to current index %zu. Now at JSON ID %zu (prop index %zu, prop name %s).\n"),
						rs->setIndexSelected, data->propJSONIndex, i, UTF8ToTString(data->ReadPropName()).c_str());
					continue;
				}
				else
				{
					DebugProp_OutputString(_T("Runtime props ends at JSON %zu, within sought JSON ID %zu."
						"Selected set index is 0; continuing...\n"), rs->lastSetJSONPropIndex, ID);
					data = data->Next();
					++i;
					continue;
				}
			}
			// else it's not in this set: continue to standard loop
			else
			{
				rs = nullptr;
				rsContainer = nullptr;
			}
		}
		DebugProp_OutputString(_T("Pre-incr at  JSON %zu, i %zu.\n"),
			data->propJSONIndex, i);

		data = data->Next();
		++i;

		DebugProp_OutputString(_T("Post-incr at JSON %zu, i %zu.\n"),
			data->propJSONIndex, i);
	}

	std::tstring name = _T("none"s);
	if (rs)
		name = UTF8ToTString(rs->GetPropSetName(rsContainer));
	DebugProp_OutputString(_T("Found JSON ID %zu, at Props position %zu; "
		"data name is \"%s\", set name is \"%s\".\n"), ID, i, UTF8ToTString(data->ReadPropName()).c_str(),
		name.c_str());
	if (rsContainerRetPtr)
		*rsContainerRetPtr = rsContainer;
	if (dataPtr)
		*dataPtr = data;

	return i;
}

// This is passed Prop/Fusion ID, not JSON ID
Properties::Data * DarkEdif::Properties::Internal_DataAt(int ID, bool idIsJSON /* = true*/)
{
	if (ID >= numProps)
	{
		LOGF(_T("Looking up invalid prop ID %i (is JSON index = %s).\n"), ID, idIsJSON ? _T("yes") : _T("no"));
		DebugProp_OutputString(_T("DataAt JSON ID %u was not found.\n"), ID);
		return nullptr;
	}

	const json_value& propsJSON = CurLang["Properties"sv];
	if (propsJSON.type != json_type::json_array)
	{
		MsgBox::Error(_T("Premature function call"), _T("Internal_DataAt() called for prop ID %u without DarkEdif_Props for language %s being valid."),
			ID, JSON::LanguageName());
		return nullptr;
	}

	Data* data;
	if (idIsJSON)
		ID = PropIdxFromJSONIdx(ID, &data);
	else
	{
		data = Internal_FirstData();
		for (int i = 0; i < ID; ++i)
		{
			DebugProp_OutputString(_T("Locating ID %d, at %u: type %s, title: %s.\n"),
				ID, i, UTF8ToTString(propsJSON[i]["Type"sv]).c_str(), UTF8ToTString(data->ReadPropName()).c_str());
			data = data->Next();
		}
	}

	// If it's an unchangable property, no data to return anyway
	//if (IsUnchangeablePropInclCheckbox((const char*)j[data->propJSONIndex]["Type"sv]))
	//	return nullptr;

	DebugProp_OutputString(_T("DataAt ID %d type %s, title: %s.\n"), ID, UTF8ToTString(propsJSON[ID]["Type"sv]).c_str(), data ? UTF8ToTString(data->ReadPropName()).c_str() : _T("(null)"));
	return data;
}

BOOL DarkEdif::DLL::DLL_EditProp(mv* mV, EDITDATA*& edPtr, unsigned int PropID)
{
	LOGV(_T("Call to %s with edPtr %p.\n"), _T(__FUNCTION__), edPtr);

	if (!GetPropRealID(PropID))
		return FALSE;

	LOGV(_T("EditProp running for JSON ID %u.\n"), PropID);

	auto& Props = Elevate(edPtr->Props);
	DarkEdif::Properties::Data* data = Props.Internal_DataAt(PropID);
	const json_value& jsonProp = CurLang["Properties"sv][data->propJSONIndex];
	if (jsonProp.type != json_object)
	{
		MsgBox::Error(_T("Property error"), _T("Property ID %u (%s) is not correctly formatted."),
			PropID, UTF8ToTString(data->ReadPropName()).c_str());
		return FALSE;
	}
	if (data->propTypeID == Edif::Properties::IDs::PROPTYPE_IMAGELIST)
	{
		ImgListProperty* thisPropData = (ImgListProperty*)data->ReadPropValue();

		const std::tstring windowTitle = UTF8ToTString(jsonProp["WindowTitle"sv]);

		const std::uint32_t maxNumImages = std::max(1U, (std::uint32_t)std::max<json_int_t>(jsonProp["MaxNumImages"sv], jsonProp["NumImages"sv]));
		std::uint32_t imageSize[2] = { 32, 32 };
		if (jsonProp["ImageSize"sv].type == json_array)
		{
			imageSize[0] = std::max(1U, (std::uint32_t)(json_int_t)jsonProp["ImageSize"sv][0]);
			imageSize[1] = std::max(1U, (std::uint32_t)(json_int_t)jsonProp["ImageSize"sv][1]);
		}

		PictureEditOptions opts = PictureEditOptions::None;
		if (jsonProp["FixedImageSize"sv])
			opts |= PictureEditOptions::FixedImageSize;
		if (jsonProp["HotSpotAndActionPoint"sv])
			opts |= PictureEditOptions::EditableHotSpot | PictureEditOptions::EditableActionPoint;
		if (jsonProp["NoAlphaChannel"sv])
			opts |= PictureEditOptions::NoAlphaChannel;
		if (jsonProp["NoTransparentColor"sv])
			opts |= PictureEditOptions::NoTransparentColor;
		if (jsonProp["16Colors"sv])
			opts |= PictureEditOptions::SixteenColors;
		// Ignore FixedNumOfImages if only one image, as it'll enable the add image button but make it no-op
		if (jsonProp["FixedNumOfImages"sv] || maxNumImages == 1)
			opts |= PictureEditOptions::FixedNumOfImages;
		if (jsonProp["AllowEmpty"sv])
			opts |= PictureEditOptions::CanBeEmpty;

		BOOL output;
		// Only one image possible: edit it in solo
		if (maxNumImages == 1)
		{
			EditImageParams<TCHAR> eip;
			eip.size = sizeof(eip);
			eip.windowTitle = windowTitle.empty() ? nullptr : windowTitle.c_str();
			eip.pad = 0;
			eip.imageID = thisPropData->numImages == 0 ? 0 : thisPropData->imageIDs[0];
			eip.defaultImageWidth = imageSize[0];
			eip.defaultImageHeight = imageSize[1];
			eip.options = opts;

			output = mV->mvEditImage(edPtr, &eip, mV->HEditWin);
			if (output == TRUE)
			{
				// Save image count + image ID, same as we do for multiple images
				const std::uint16_t two[] = { 1, eip.imageID };
				Props.Internal_PropChange(mV, edPtr, PropID, two, sizeof(two));
			}
		}
		else
		{
			std::tstring imageTitlesBuffer;
			std::unique_ptr<TCHAR* []> titles;
			if (jsonProp["ImageTitles"sv].type == json_array)
			{
				const json_value& imgTitles = jsonProp["ImageTitles"sv];
				titles = std::make_unique<TCHAR* []>(imgTitles.u.array.length + 1); // + 1 to end with null ptr
				std::tstringstream str;
				std::tstring tstr;
				for (std::size_t i = 0, j = 0; i < imgTitles.u.array.length; ++i)
				{
					tstr = UTF8ToTString(imgTitles[i]);
					str.write(tstr.c_str(), tstr.size() + 1);
					titles[i] = (TCHAR*)j;
					j += tstr.size() + 1;
				}
				imageTitlesBuffer = str.str();
				for (std::size_t i = 0, j = 0; i < imgTitles.u.array.length; ++i)
					titles[i] = imageTitlesBuffer.data() + (std::size_t)titles[i];
			}

			EditAnimationParams<TCHAR> eap;
			eap.size = sizeof(eap);
			eap.windowTitle = windowTitle.empty() ? nullptr : windowTitle.c_str();
			eap.numImages = thisPropData->numImages;
			eap.maxNumImages = maxNumImages;
			eap.startIndexToEdit = 0;
			auto prop = std::make_unique<std::uint16_t[]>(1 + maxNumImages);
			if (!prop || memcpy_s(&prop[1], maxNumImages * sizeof(std::uint16_t),
				thisPropData->imageIDs, thisPropData->numImages * sizeof(std::uint16_t)) != 0)
			{
				return FALSE;
			}
			eap.imageIDs = &prop[1];
			eap.imageTitles = titles.get();
			eap.defaultImageWidth = imageSize[0];
			eap.defaultImageHeight = imageSize[1];
			eap.options = opts;

			output = mV->mvEditAnimation(edPtr, &eap, mV->HEditWin);
			if (output == TRUE)
			{
				prop[0] = eap.numImages;
				Props.Internal_PropChange(mV, edPtr, PropID,
					prop.get(), (1 + eap.numImages) * sizeof(std::uint16_t));
			}
		}
		//mvRefreshProp(mV, edPtr, PropID_, FALSE);
		return output;
	}

	if (data->propTypeID == Edif::Properties::IDs::PROPTYPE_EDITBUTTON)
	{
		const auto setIt = std::find_if(Edif::SDK->EdittimePropertySets.cbegin(),
			Edif::SDK->EdittimePropertySets.cend(), [&](const EdittimePropSet& e) {
				return e.addButtonIdx == PropID || e.deleteButtonIdx == PropID;
			}
		);
		if (setIt == Edif::SDK->EdittimePropertySets.cend())
			return FALSE; // Not a prop set

		const EdittimePropSet& ps = *setIt;

		if (PropID == ps.addButtonIdx)
		{
			Properties::Data* rsContainer = (Properties::Data*)Props.Internal_DataAt(ps.nameListIdx);
			RuntimePropSet* rs = (RuntimePropSet*)rsContainer->ReadPropValue();
			const std::size_t rsNegativeOffset = (char*)rs - (char*)rsContainer;
			assert(rs->setIndicator == 'S');

			// Repeat how we generated the JSON properties
			Properties::ConverterReturn retState;
			DLL::ConverterReturnAccessor& retStateAdmin = *(DLL::ConverterReturnAccessor*)&retState;
			Properties::ConverterState convState(edPtr, CurLang["Properties"sv]);
			jsonPropertyReader.convState = &convState;

			std::stringstream dataToWriteStream;

			// 1 byte per 8 properties, rounded up
			const std::uint16_t numNewProps = (ps.endSetIdx - ps.startSetIdx) + 1;

			std::size_t oldChkBytesSize = (std::size_t)ceil(((double)Props.numProps) / 8.0);
			std::size_t newChkBytesSize = (std::size_t)ceil((((double)Props.numProps) + numNewProps) / 8.0);
			auto chkboxes = std::make_unique<std::uint8_t[]>(newChkBytesSize);
			// Copy all checkboxes to left of newly inserted set; we're inserting past last set entry
			rs->setIndexSelected = rs->numRepeats - 1;
			std::size_t propsBeforeNewSet = Props.PropIdxFromJSONIdx(ps.endSetIdx);

			// Copy checkbox bits before this set, we'll have to copy their checkbox values
			for (std::size_t i = 0; i < propsBeforeNewSet; ++i)
			{
				if ((Props.dataForProps[i / CHAR_BIT] & (1 << (i % CHAR_BIT))) != 0)
					chkboxes[i / CHAR_BIT] |= (1 << (i % CHAR_BIT));
			}

			// There may be properties after the set, we'll have to shift their checkbox values over to the right
			// by adding numPropsInSet to their written index
			for (std::size_t i = propsBeforeNewSet, j = i + numNewProps; i < Props.numProps; ++i)
			{
				if ((Props.dataForProps[i / CHAR_BIT] & (1 << (i % CHAR_BIT))) != 0)
					chkboxes[j / CHAR_BIT] |= (1 << (j % CHAR_BIT));
			}

			std::vector<int> chkboxJSONIndexesToRead(numNewProps);
			for (std::size_t i = rs->firstSetJSONPropIndex, j = 0; j < numNewProps; ++i, ++j)
				chkboxJSONIndexesToRead[j] = i;

			std::string title;
			for (std::uint16_t propJSONIndex = rs->firstSetJSONPropIndex, offsetFromSetStart = 0;
				offsetFromSetStart < numNewProps;
				++propJSONIndex, ++offsetFromSetStart)
			{
				// Don't include title for static text types
				if (IsUnchangeablePropExclCheckbox(convState.jsonProps[propJSONIndex]["Type"sv]))
					title.clear();
				else
					title = convState.jsonProps[propJSONIndex]["Title"sv];

				std::uint16_t propTypeID = Edif::SDK->EdittimeProperties[propJSONIndex].Type_ID % 1000;

				jsonPropertyReader.GetProperty(propJSONIndex, &retState);
				// Converter failed
				if (retStateAdmin.convRetType != Properties::ConvReturnType::OK)
				{
					MsgBox::Error(_T("Property error"), _T("Couldn't read initial value of property set for JSON ID %zu, title %s."),
						propJSONIndex, UTF8ToTString(convState.jsonProps[propJSONIndex]["Title"sv]).c_str());
				}

				assert(title.size() <= UINT8_MAX - 1); // can't store in titleLen
				std::uint8_t titleLen = (std::uint8_t)title.size();

				std::uint32_t propSize2 = sizeof(Properties::Data) + titleLen + (std::uint16_t)retStateAdmin.dataSize;

				// If fails, local vars don't match Data
				static_assert(sizeof(propSize2) + sizeof(propTypeID) + sizeof(propJSONIndex) + sizeof(titleLen) == sizeof(Properties::Data),
					"Data size mismatch");

				dataToWriteStream.write((char*)&propSize2, sizeof(propSize2));
				dataToWriteStream.write((char*)&propTypeID, sizeof(propTypeID));
				dataToWriteStream.write((char*)&propJSONIndex, sizeof(propJSONIndex));
				dataToWriteStream.write((char*)&titleLen, sizeof(titleLen));
				dataToWriteStream.write(title.c_str(), titleLen);

				if (retStateAdmin.data != nullptr)
				{
					// assert(retStateAdmin.dataSize > 0);
					dataToWriteStream.write((const char*)retStateAdmin.data, retStateAdmin.dataSize);

					if (retStateAdmin.freeData != nullptr)
						retStateAdmin.freeData(retStateAdmin.data);
				}

				if (retStateAdmin.checkboxState != -1)
				{
					assert(retStateAdmin.checkboxState == 0 || retStateAdmin.checkboxState == 1);

					int propRealIdx = propsBeforeNewSet + offsetFromSetStart;
					int secondIdx = propsBeforeNewSet + (rs->firstSetJSONPropIndex - propJSONIndex);
					assert(propRealIdx == secondIdx);

					if (retStateAdmin.checkboxState == 1)
						chkboxes[propRealIdx / CHAR_BIT] |= 1 << (propRealIdx % CHAR_BIT);

					auto f = std::find(chkboxJSONIndexesToRead.cbegin(), chkboxJSONIndexesToRead.cend(), propJSONIndex);
					assert(f != chkboxJSONIndexesToRead.cend());
					chkboxJSONIndexesToRead.erase(f);
				}
			}

			// Anything didn't supply checkboxes with the item data? grab them
			for (std::size_t propJSONIndex, offsetFromVectorStart = 0;
				offsetFromVectorStart < chkboxJSONIndexesToRead.size();)
			{
				propJSONIndex = chkboxJSONIndexesToRead[offsetFromVectorStart];
				title = convState.jsonProps[propJSONIndex]["Title"sv];

				jsonPropertyReader.GetPropertyCheckbox(propJSONIndex, &retState);

				// Converted OK, no reason for other readers
				if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				{
					assert(retStateAdmin.checkboxState == 0 || retStateAdmin.checkboxState == 1);

					if (retStateAdmin.checkboxState != -1)
					{
						//int propRealIdx = propsBeforeNewSet + offsetFromVectorStart;
						int propRealIdx  = propsBeforeNewSet + (propJSONIndex - rs->firstSetJSONPropIndex) + 1;
						//assert(propRealIdx == secondIdx);
						DebugProp_OutputString(_T("Prop index %zu (%s) real ID %zu checkbox read by JSON reader.\n"),
							propJSONIndex, UTF8ToTString(title).c_str(), propRealIdx);

						if (retStateAdmin.checkboxState == 1)
							chkboxes[propRealIdx / CHAR_BIT] |= 1 << (propRealIdx % CHAR_BIT);

						auto f = std::find(chkboxJSONIndexesToRead.cbegin(), chkboxJSONIndexesToRead.cend(), propJSONIndex);
						assert(f != chkboxJSONIndexesToRead.cend());
						chkboxJSONIndexesToRead.erase(f);
					}

					continue;
				}
				// No checkbox for this type
				if (retStateAdmin.convRetType == Properties::ConvReturnType::Pass)
				{
					++offsetFromVectorStart;
					continue;
				}

				MsgBox::Error(_T("Property error"), _T("Couldn't read check box of some set properties."));
				continue;
			}

			// It is valid that chkboxJSONIndexesToRead is not empty - some properties cannot have checkboxes,
			// like static text. Perhaps in future, we'll check for those cases specifically.

			const std::string dataToAdd = dataToWriteStream.str();

			// Differences in checkbox bytes' sizes
			std::size_t amountToShuffleForCheckboxes = newChkBytesSize - oldChkBytesSize;
			std::size_t addedByteCount = dataToAdd.size() + amountToShuffleForCheckboxes;

			// Step 1: copy all the data in EDITDATA
			std::size_t oldEDITDATASize = edPtr->eHeader.extSize;

			DebugProp_OutputString(_T("*  Pre-realloc: old EdPtr range: %p to %p...\n"),
				edPtr, ((char*)edPtr) + oldEDITDATASize);
			_CrtCheckMemory();

			EDITDATA* newEdPtr = (EDITDATA*)mvReAllocEditData(mV, edPtr, edPtr->eHeader.extSize + addedByteCount);
			if (!newEdPtr)
			{
				MsgBox::Error(_T("Property error"), _T("Couldn't resize EDITDATA to %zu bytes."), edPtr->eHeader.extSize + dataToAdd.size() + amountToShuffleForCheckboxes);
				return FALSE;
			}
			_CrtCheckMemory();
			DebugProp_OutputString(_T("* Post-realloc: old EdPtr range: %p to %p. New range: %p to %p.\n"),
				edPtr, ((char*)edPtr) + oldEDITDATASize, newEdPtr, ((char*)newEdPtr) + newEdPtr->eHeader.extSize);

			// Pointers/refs have changed due to realloc
			auto& NewProps = Elevate(newEdPtr->Props);
			rs = (RuntimePropSet*)NewProps.Internal_DataAt(ps.nameListIdx)->ReadPropValue();
			// Step 2: move the data from following old checkbox end to new checkbox end
			std::uint8_t* oldPropDataAt = (std::uint8_t*)NewProps.Internal_FirstData();
			std::uint8_t* newPropDataAt = oldPropDataAt + amountToShuffleForCheckboxes;
			if (amountToShuffleForCheckboxes > 0)
			{
				// This is the number of bytes following the checkboxes, to the end of old EDITDATA
				// We move it rightwards by amountToShuffleForCheckboxes
				memmove(newPropDataAt, oldPropDataAt, oldEDITDATASize - ((char*)oldPropDataAt - (char*)newEdPtr));
				rs = (RuntimePropSet*)(((std::uint8_t*)rs) + amountToShuffleForCheckboxes);
			}
			rsContainer = (Properties::Data*)((char*)rs - rsNegativeOffset);

			// Step 3: Overwrite with new checkboxes
			memcpy(NewProps.dataForProps, chkboxes.get(), newChkBytesSize);
			std::size_t oldNumProps = NewProps.numProps;
			NewProps.numProps += numNewProps; // to fix calculation of new FirstData/Internal_DataAt

			// Step 3: move data following all previous sets to end of editdata
			// we've selected last of set earlier, this will take us just past end of last set entry
			std::uint8_t* startOfFurtherData = (std::uint8_t*)(NewProps.Internal_DataAt(ps.endSetIdx)->Next());
			std::uint8_t* endOfFurtherData = (std::uint8_t*)NewProps.Internal_DataAt(oldNumProps - 1, false)->Next();
			std::size_t amountToMove = endOfFurtherData - startOfFurtherData;
			DebugProp_OutputString(_T("Calculated amount to move past end of set as %zu bytes.\n"), amountToMove);
			if (amountToMove > 0)
				memmove(startOfFurtherData + dataToAdd.size(), startOfFurtherData, endOfFurtherData - startOfFurtherData);

			// Step 4: insert new set
			memcpy(startOfFurtherData, dataToAdd.data(), dataToAdd.size());

			// Step 5: rewrite old data to compensate for new data
			++rs->numRepeats;
			NewProps.sizeBytes += addedByteCount;

			DebugProp_OutputString(_T("Add button edits to EDITDATA done; reloading props.\n"));
			ReloadPropSet(mV, newEdPtr, rsContainer, rs->numRepeats - 1);
			ScanForDynamicPropChange(mV, newEdPtr, UINT32_MAX);

			DebugProp_OutputString(_T("Add button completed.\n"));
			edPtr = newEdPtr;
			// TODO: If there are more than one set, do we have to rewrite their firstpropindex?
			return TRUE;
		}

		if (PropID == ps.deleteButtonIdx)
		{
			Properties::Data* firstData, * rsContainer;
			std::size_t firstPropTrueIndex = Props.PropIdxFromJSONIdx(ps.startSetIdx, &firstData, &rsContainer);
			RuntimePropSet* rs = (RuntimePropSet*)rsContainer->ReadPropValue();
			if (rs->numRepeats <= 1)
				return FALSE;

			const std::size_t negativeRSOffset = (char*)rs - (char*)rsContainer;
			auto endData = Props.Internal_DataAt(rs->lastSetJSONPropIndex)->Next();
			std::size_t sizePropInBytes = (char*)endData - (char*)firstData;
			std::uint16_t numPropsInThisSet = rs->GetNumPropsInThisEntry(rsContainer);

			// Move all data from right of our removed Data leftwards to overwrite it
			memmove(firstData, endData,
				((char*)edPtr + Props.sizeBytes) - (char*)endData);

			// Properties beyond this removed set had checkbox entries, which now have
			// their bits partly too far right, so we set them further left
			for (std::size_t origChkIdx = firstPropTrueIndex + numPropsInThisSet,
				newChkIdx = firstPropTrueIndex;
				origChkIdx < (std::size_t)(Props.numProps - numPropsInThisSet);
				++origChkIdx, ++newChkIdx)
			{
				// While we're reading and writing from same data, the original bit
				// being read from is always to the right of anything written,
				// so there's no corruption
				if ((Props.dataForProps[origChkIdx / CHAR_BIT] & (1 << (origChkIdx % CHAR_BIT))) != 0)
					Props.dataForProps[newChkIdx / CHAR_BIT] |= 1 << (newChkIdx % CHAR_BIT);
				else
					Props.dataForProps[newChkIdx / CHAR_BIT] &= ~(1 << (newChkIdx % CHAR_BIT));
			}

			// Checkbox bitfield size differences
			std::size_t amountToShuffleForCheckboxes = (std::size_t)std::ceil(Props.numProps / 8.0) -
				(std::size_t)std::ceil((Props.numProps - numPropsInThisSet) / 8.0);
			assert(amountToShuffleForCheckboxes < (32 / 8)); // sanity check

			// Move all data leftwards from old checkbox end to new checkbox end leftwards
			memmove(Props.dataForProps + (std::size_t)std::ceil((Props.numProps - numPropsInThisSet) / 8.0),
				Props.dataForProps + (std::size_t)std::ceil(Props.numProps / 8.0),
				Props.sizeBytes - offsetof(EDITDATA, Props) - offsetof(DarkEdif::DLL::PropAccesser, dataForProps) - sizePropInBytes - amountToShuffleForCheckboxes);

			// rs is now invalidated, re-grab; since propset is always before, we don't need to subtract removed props' size
			rs = (RuntimePropSet*)(((char*)rs) - amountToShuffleForCheckboxes);
			Props.numProps -= numPropsInThisSet;
			Props.sizeBytes -= amountToShuffleForCheckboxes + sizePropInBytes;
			--rs->numRepeats;

			std::size_t oldEDITDATASize = edPtr->eHeader.extSize;
			DebugProp_OutputString(_T("*  Pre-realloc: old EdPtr range: %p to %p...\n"),
				edPtr, ((char*)edPtr) + oldEDITDATASize);
			DebugProp_OutputString(_T("*  Pre-realloc: EdittimeProperties range: %p to %p...\n"),
				Edif::SDK->EdittimeProperties.get(),
				&Edif::SDK->EdittimeProperties[CurLang["Properties"sv].u.array.length - 1]
			);

			// EDITDATA has been fully reconfigured with set removed, realloc it smaller
			std::uint16_t origIdx = rs->setIndexSelected;
			std::uint16_t newIdx = rs->setIndexSelected - (rs->setIndexSelected >= rs->numRepeats ? 1 : 0);

			EDITDATA* newEdPtr = (EDITDATA*)mvReAllocEditData(mV, edPtr, edPtr->eHeader.extSize - (amountToShuffleForCheckboxes + sizePropInBytes));
			if (newEdPtr == NULL)
				return MsgBox::Error(_T("Property error"), _T("Couldn't allocate smaller EDITDATA; the MFA is now inconsistent.")), TRUE;

			DebugProp_OutputString(_T("* Post-realloc: old EdPtr range: %p to %p. New range: %p to %p.\n"),
				edPtr, ((char*)edPtr) + oldEDITDATASize, newEdPtr, ((char*)newEdPtr) + newEdPtr->eHeader.extSize);

			// rs is invalidated again, do some offset math
			rs = (RuntimePropSet*)(((char*)newEdPtr) + ((char*)rs - (char*)edPtr));
			// Scroll to next entry (so, same index), or back if at last entry
			rsContainer = (Properties::Data*)(((char*)rs) - negativeRSOffset);
			ReloadPropSet(mV, newEdPtr, rsContainer, newIdx);
			ScanForDynamicPropChange(mV, newEdPtr, UINT32_MAX);
			DebugProp_OutputString(_T("Finished reallocating editdata OK; new edPtr is %p.\n"), newEdPtr);

			return TRUE;
		}
	} // Edit Button for sets

	return FALSE;
}

#endif // EditorBuild

#endif // NOPROPS

#if EditorBuild

void DarkEdif::DLL::GeneratePropDataFromJSON()
{
	std::vector<PropData> VariableProps;
#ifndef NOPROPS
	std::vector<EdittimePropSet>& propSets = Edif::SDK->EdittimePropertySets;
#endif
	PropData* CurrentProperty;
	const TCHAR* errorPrefix = _T("") PROJECT_NAME _T(" JSON property parser");

	const json_value& props = CurLang["Properties"sv];

#ifndef NOPROPS
	for (const auto propSetItPtr : CurLang["PropertySets"sv].u.array)
	{
		const auto& propSet = *propSetItPtr;
		EdittimePropSet es = {};

		const auto IdxFromName = [&propSet, &es, &props](const std::string_view & itemNeeded, std::uint16_t &writeTo) {
			if (propSet[itemNeeded].type != json_string)
			{
				MsgBox::Error(_T("Property set error"), _T("Missing property set item %s in property set name %s."),
					UTF8ToTString(itemNeeded).c_str(), es.setName);
				return false;
			}
			const std::string_view itemName = propSet[itemNeeded];
			const auto idxIt = std::find_if(props.u.array.begin(), props.u.array.end(),
				[&itemName](const json_value* prop) { return SVICompare((*prop)["Title"sv], itemName); });
			if (idxIt == props.u.array.end())
			{
				MsgBox::Error(_T("Property set error"), _T("Property set \"%s\" requests item name \"%s\", which was not found."),
					UTF8ToTString(itemNeeded).c_str(), es.setName);
				return false;
			}
			writeTo = std::distance(props.u.array.begin(), idxIt);
			return true;
		};

		es.setName = propSet["SetName"sv];
		if (es.setName.empty() ||
			std::any_of(propSets.cbegin(), propSets.cend(),
				[&es](auto es2) { return SVICompare(es2.setName, es.setName); }))
		{
			MsgBox::Error(_T("Property set error"), _T("Property set name \"%s\" is invalid."),
				UTF8ToTString(es.setName).c_str());
			continue;
		}

		if (!IdxFromName("AddButton"sv, es.addButtonIdx) ||
			!IdxFromName("DeleteButton"sv, es.deleteButtonIdx) ||
			!IdxFromName("EntryList"sv, es.nameListIdx) ||
			!IdxFromName("EntryName"sv, es.nameEditboxIdx) ||
			!IdxFromName("StartItem"sv, es.startSetIdx) ||
			!IdxFromName("EndItem"sv, es.endSetIdx))
		{
			continue; // error already reported in IdxFromName
		}

		const auto CheckIndexValidOrDie = [&](std::tstring s, std::size_t checkIdx, bool insideSet,
			const std::vector<int> & propTypesExpected)
		{
			if (checkIdx == SIZE_MAX)
			{
				MsgBox::Error(_T("JSON property error"), _T("Set %s does not have a property for %s."),
					UTF8ToTString(es.setName).c_str(), s.c_str());
				return -1;
			}
			// Either it must be within the start/end set, inclusive, or it must be outside
			if (insideSet != (checkIdx >= es.startSetIdx && checkIdx <= es.endSetIdx))
			{
				MsgBox::Error(_T("JSON property error"), _T("Set %s's %s action property \"%s\" is %swithin the set range, which is not allowed."),
					UTF8ToTString(es.setName).c_str(), s.c_str(),
					UTF8ToTString(CurLang["Properties"sv][checkIdx]["Title"sv]).c_str(),
					insideSet ? _T("not ") : _T(""));
				return -1;
			}
			if (propTypesExpected.empty())
				return 1;

			for (std::size_t i = 0; i < std::size(propTypesExpected); ++i)
				if (SVICompare(CurLang["Properties"sv][checkIdx]["Type"sv], Edif::Properties::Names[propTypesExpected[i]]))
					return 1;
			MsgBox::Error(_T("JSON property error"), _T("Set %s's %s action property uses the wrong property type \"%s\"."),
				UTF8ToTString(es.setName).c_str(), s.c_str(),
				UTF8ToTString(CurLang["Properties"sv][checkIdx]["Type"sv]).c_str());
			return -1;
		};
		if (!CheckIndexValidOrDie(_T("AddButton"s), es.addButtonIdx, false, {
			Edif::Properties::IDs::PROPTYPE_EDITBUTTON }))
		{
			continue;
		}

		if (!CheckIndexValidOrDie(_T("DeleteButton"s), es.deleteButtonIdx, false, {
			Edif::Properties::IDs::PROPTYPE_EDITBUTTON }))
		{
			continue;
		}
		if (!CheckIndexValidOrDie(_T("EntryList"s), es.nameListIdx, false, {
			Edif::Properties::IDs::PROPTYPE_COMBOBOX, Edif::Properties::IDs::PROPTYPE_COMBOBOXBTN,
			Edif::Properties::IDs::PROPTYPE_ICONCOMBOBOX }))
		{
			continue;
		}

		// These three can overlap in index, allowing a set of just the set names
		if (!CheckIndexValidOrDie(_T("SetName"s), es.nameEditboxIdx, true, { Edif::Properties::IDs::PROPTYPE_EDIT_STRING }))
			continue;
		if (!CheckIndexValidOrDie(_T("StartItem"s), es.startSetIdx, true, { /* any */ }))
			continue;
		if (!CheckIndexValidOrDie(_T("EndItem"s), es.endSetIdx, true, { /* any */ }))
			continue;
		if (es.addButtonIdx == es.deleteButtonIdx)
		{
			MsgBox::Error(_T("JSON property error"), _T("Set %s has expected-independent actions are combined on the same property %zu or %zu, which is not allowed. (independent actions: add set, delete set, set name list)"),
				UTF8ToTString(es.setName).c_str(), es.addButtonIdx, es.deleteButtonIdx);
			continue;
		}
		if (es.endSetIdx < es.startSetIdx)
		{
			MsgBox::Error(_T("JSON property error"), _T("Set %s has an end index before its start index."),
				UTF8ToTString(es.setName).c_str());
			continue;
		}

		// Confirm this set does not use indexes from previous sets
		// Sets can overlap, in form of subsets only, but names, name lists etc can't
		for (auto& otherPropSet : propSets)
		{
			// This is not a subset, but an overlap.
			// Set 1 starts, Set 2 starts, Set 1 ends, Set 2 ends,
			// i.e. Set 1 starts inside Set 2, but ends past Set 2
			if (otherPropSet.startSetIdx > es.startSetIdx && otherPropSet.endSetIdx > es.endSetIdx)
			{
				MsgBox::Error(_T("JSON property error"), _T("Set \"%s\" begins inside set \"%s\", but does not end within the set."),
					UTF8ToTString(otherPropSet.setName).c_str(), UTF8ToTString(es.setName).c_str());
				break;
			}
			if (otherPropSet.addButtonIdx == es.addButtonIdx ||
				otherPropSet.deleteButtonIdx == es.addButtonIdx ||
				otherPropSet.addButtonIdx == es.deleteButtonIdx ||
				otherPropSet.deleteButtonIdx == es.deleteButtonIdx)
			{
				MsgBox::Error(_T("JSON property error"), _T("Set \"%s\" button index overlaps with set \"%s\" button indices."),
					UTF8ToTString(otherPropSet.setName).c_str(), UTF8ToTString(es.setName).c_str());
				break;
			}
			if (otherPropSet.nameListIdx == es.nameListIdx)
			{
				MsgBox::Error(_T("JSON property error"), _T("Set \"%s\" name list property is the same as set \"%s\" name list property."),
					UTF8ToTString(otherPropSet.setName).c_str(), UTF8ToTString(es.setName).c_str());
				break;
			}
			if (otherPropSet.nameEditboxIdx == es.nameEditboxIdx)
			{
				MsgBox::Error(_T("JSON property error"), _T("Set \"%s\" name editbox property is the same as set \"%s\" name editbox property."),
					UTF8ToTString(otherPropSet.setName).c_str(), UTF8ToTString(es.setName).c_str());
				break;
			}
		}

		// All checks good
		Edif::SDK->EdittimePropertySets.push_back(es);
		DebugProp_OutputString(_T("Successfully created object set \"%s\".\n"), UTF8ToTString(es.setName).c_str());
	}
#else
	if (CurLang["PropertySets"sv].type != json_none)
		MsgBox::Error(_T("JSON property error"), _T("PropertySets exists in JSON but is not supported in NOPROPS builds."));
#endif // NOPROPS

	std::vector<const char*> openFolderList;
	for (std::size_t i = 0; i < props.u.array.length; ++i)
	{
		const json_value& Property = props[i];
		CurrentProperty = nullptr;

		// Category shift. Currently not supported.
		// When implemented, the JSON will look like
		// "Properties": {
		//   "General",
		//   { ... }, // general props
		//   "Display",
		//   { ... }, // display props
		// }
		if (Property.type == json_type::json_string)
		{
			MsgBox::Error(errorPrefix, _T("Properties contains a string \"%s\" instead of an object. ")
				_T("Multiple categories of properties not currently implemented. (%s)"),
				UTF8ToTString(Property).c_str(), JSON::LanguageName());
			continue;
		}
		const std::string_view PropertyType = Property["Type"sv];

		// Reserved/invalid property types are marked with ! at the start.
		if (PropertyType.empty() || PropertyType[0] == '!')
		{
			MsgBox::Error(errorPrefix,
				_T("You have specified an invalid Parameter type \"%s\". These types are reserved. (%s)"),
				UTF8ToTString(PropertyType).c_str(), JSON::LanguageName());
			continue;
		}

#define SetAllProps(opt,lParams) CurrentProperty->SetAllProperties(Options|(opt), (LPARAM)(lParams)); break
		using namespace Edif::Properties;

		// Custom Parameter: Read the number CustomXXX and use that.
		if (SVIComparePrefix(PropertyType, "Custom"sv))
		{
			MsgBox::Info(errorPrefix, _T("Detected a custom property."));
			CurrentProperty = new PropData(VariableProps.size(), i + PROPTYPE_LAST_ITEM);
		}
		else if (SVICompare(PropertyType, Names[PROPTYPE_FOLDER_END]))
		{
			CurrentProperty = new PropData(-1, PROPTYPE_FOLDER_END);
			if (openFolderList.empty())
				MsgBox::Error(errorPrefix, _T("Too many FolderEnd properties! Extra one found at index %i."), i);
			else
				openFolderList.erase(openFolderList.cend() - 1);
		}
		else // Regular Parameter
		{
			// Empty or missing property name is not allowed - apart from folder end
			if (Property["Title"sv].type != json_type::json_string)
			{
				MsgBox::Error(errorPrefix,
					_T("Invalid or no title specified for property index %zu. (%s)"),
					i, JSON::LanguageName());
				continue;
			}
			// Reused property name is not allowed
			else
			{
				bool ok = true;
				for (std::size_t j = 0; j < i; ++j)
				{
					if (SVICompare(props[j]["Title"sv], Property["Title"sv]))
					{
						MsgBox::Error(errorPrefix,
							_T("Property title \"%s\" is reused between JSON index %zu and index %zu. (%s)"),
							Property["Title"sv].c_str(), j, i, JSON::LanguageName());
						ok = false;
						break;
					}
				}
				if (!ok)
					continue;
			}


			// Loop through Parameter names and compareth them.
			for (std::size_t j = PROPTYPE_FIRST_ITEM; j < PROPTYPE_LAST_ITEM; ++j)
			{
				if (SVICompare(PropertyType, Names[j]))
				{
					// Unicode Properties have IDs 1000 greater than their ANSI equivalents.
#ifdef _UNICODE
					CurrentProperty = new PropData(VariableProps.size(), j + 1000);
#else
					CurrentProperty = new PropData(VariableProps.size(), j);
#endif
					break;
				}
			}

			if (!CurrentProperty)
			{
				MsgBox::Error(errorPrefix, _T("Property index %zu \"%s\" has an unrecognised property type \"%s\" in the JSON (under %s language).\n"
					"Check your spelling of the \"Type\" Parameter."), i, UTF8ToTString(Property["Title"sv]).c_str(), UTF8ToTString(PropertyType).c_str(), JSON::LanguageName());
				continue;
			}
			// If checkbox is enabled, pass that as flags as well.
			unsigned int Options =
				(Property["CheckboxDefaultState"sv].type != json_type::json_none ? PROPOPT_CHECKBOX : 0)		// Checkbox enabled by property option in JSON
				| (bool(Property["Bold"sv]) ? PROPOPT_BOLD : 0)				// Bold enabled by property option in JSON
				| (bool(Property["Removable"sv]) ? PROPOPT_REMOVABLE : 0)		// Removable enabled by property option in JSON
				| (bool(Property["Renameable"sv]) ? PROPOPT_RENAMEABLE : 0)	// Renamable enabled by property option in JSON
				| (bool(Property["Moveable"sv]) ? PROPOPT_MOVABLE : 0)		// Movable enabled by property option in JSON
				| (bool(Property["List"sv]) ? PROPOPT_LIST : 0)				// List enabled by property option in JSON
				| (bool(Property["SingleSelect"sv]) ? PROPOPT_SINGLESEL : 0);	// Single-select enabled by property option in JSON
			bool EnableLParams = false;

			CurrentProperty->Title = Edif::ConvertString(Property["Title"sv]);
			CurrentProperty->Info = Edif::ConvertString(Property["Info"sv]);

			switch (CurrentProperty->Type_ID % 1000)
			{
			// Simple static text
			case PROPTYPE_STATIC:
				SetAllProps(0, NULL);

			// Folder
			case PROPTYPE_FOLDER:
			{
				openFolderList.push_back(Property["Title"sv].c_str());
				SetAllProps(0, NULL);
			}
			// FolderEnd handled outside of this switch, in the if() above

			// Edit button, Params1 = button text, or nullptr if Edit
			case PROPTYPE_EDITBUTTON:
			{
				if (((std::string_view)Property["DefaultState"sv]).empty()) {
					SetAllProps(0, NULL);
				}
				else {
					SetAllProps(PROPOPT_PARAMREQUIRED, Edif::ConvertString(Property["DefaultState"sv]));
				}
			}

			// Edit box for strings, Parameter = max length
			case PROPTYPE_EDIT_STRING:
			case PROPTYPE_EDIT_MULTILINE:
			case PROPTYPE_DIRECTORYNAME:
			{
				if (CurrentProperty->Type_ID % 1000 == PROPTYPE_EDIT_STRING)
				{
					Options |= ((SVICompare(Property["Case"sv], "Lower"sv)) ? PROPOPT_EDIT_LOWERCASE : 0)
						| ((SVICompare(Property["Case"sv], "Upper"sv)) ? PROPOPT_EDIT_UPPERCASE : 0)
						| ((Property["Password"sv]) ? PROPOPT_EDIT_PASSWORD : 0);
				}
				else
				{
					if (Property["Case"sv].type != json_type::json_none)
					{
						MsgBox::WarningOK(_T("DarkEdif JSON property"), _T(R"(Property "%s" is set to "Case"="%hs", but that won't work with property type %hs.)"),
							UTF8ToTString(Property["Title"sv]).c_str(), Property["Case"sv].c_str(), Names[CurrentProperty->Type_ID % 1000]);
					}
					if (Property["Password"sv].type != json_type::json_none)
					{
						MsgBox::WarningOK(_T("DarkEdif JSON property"), _T("Property \"%s\" is set to password mask, but that won't work with property type %hs."),
							UTF8ToTString(Property["Title"sv]).c_str(), Names[CurrentProperty->Type_ID % 1000]);
					}
				}
				if (Property["MaxLength"sv].type != json_type::json_integer) {
					SetAllProps(0, NULL);
				}
				else
				{
					int* textLength = new int;
					*textLength = (int)(json_int_t)Property["MaxLength"sv];
					SetAllProps(PROPOPT_PARAMREQUIRED, textLength);
				}
			}

			// Edit box for numbers (does not support min/max)
			case PROPTYPE_EDIT_NUMBER:
			case PROPTYPE_COLOR:
				SetAllProps(0, NULL);

			// Edit box for slider or spin control, Parameters = min value, max value
			case PROPTYPE_SPINEDIT:
			case PROPTYPE_SLIDEREDIT:
			{
				int* temp = new int[2];
				temp[0] = ((json_int_t)Property["Minimum"sv]) & 0xFFFFFFFF;
				temp[1] = ((json_int_t)Property["Maximum"sv]) & 0xFFFFFFFF;
				SetAllProps(PROPOPT_PARAMREQUIRED, temp);
			}

			// Combo box, Parameters = list of strings, options (sorted, etc)
			case PROPTYPE_COMBOBOX:
			case PROPTYPE_COMBOBOXBTN:
			{
				if (Property["Items"sv].type == json_type::json_none)
				{
					SetAllProps(PROPOPT_PARAMREQUIRED, NULL);
					// expects GetPropCreateParam() to be implemented
					break;
				}

#ifndef NOPROPS
				if (std::any_of(Edif::SDK->EdittimePropertySets.cbegin(), Edif::SDK->EdittimePropertySets.cend(),
					[i](const auto& es) { return es.nameListIdx == i; }))
				{
					MsgBox::Error(_T("DarkEdif JSON property"), _T("Items detected in a property set list box %s."),
						UTF8ToTString(Property["Title"sv]).c_str());
					SetAllProps(PROPOPT_PARAMREQUIRED, NULL);
					break;
				}
#endif // NOPROPS
				const json_value& itemsJSON = Property["Items"sv];
				if (itemsJSON.type != json_type::json_array || itemsJSON.u.array.length == 0)
				{
					MsgBox::Error(_T("DarkEdif JSON property"), _T("No Items detected in combobox property %s."),
						UTF8ToTString(Property["Title"sv]).c_str());
				}

				const TCHAR** Fixed = new const TCHAR * [itemsJSON.u.array.length + 2];

				// NULL is required at start and end of array
				Fixed[0] = Fixed[itemsJSON.u.array.length + 1] = nullptr;

				// Use incrementation and copy to fixed list.
				for (unsigned int index = 1; index < itemsJSON.u.array.length + 1; ++index)
					Fixed[index] = Edif::ConvertString(itemsJSON[index - 1]);

				// Pass fixed list as Parameter
				SetAllProps(PROPOPT_PARAMREQUIRED, (LPARAM)Fixed);
			}

			// Size
			case PROPTYPE_SIZE:
			{
				const json_value& presetList = Property["PresetSizes"sv];
				if (presetList.type == json_type::json_array && presetList.u.array.length > 0)
				{
					int* const predefSizes = new int[(presetList.u.array.length + 1) * 2];

					for (unsigned int index = 0; index < presetList.u.array.length; ++index)
					{
						const json_value& sizeEntry = presetList[index];

						if (sizeEntry.type != json_type::json_array || sizeEntry.u.array.length != 2 ||
							sizeEntry[0].type != json_type::json_integer || sizeEntry[1].type != json_type::json_integer ||
							(json_int_t)sizeEntry[0] < 0 || (json_int_t)sizeEntry[1] < 0)
						{
							DarkEdif::MsgBox::Error(_T("DarkEdif JSON property"), _T("Invalid preset size index %d in size property %s."),
								index, UTF8ToTString(Property["Title"sv]).c_str());
						}

						predefSizes[index * 2] = (int)(json_int_t)sizeEntry[0];
						predefSizes[index * 2 + 1] = (int)(json_int_t)sizeEntry[1];
					}

					// Array must end with a 0 x 0 entry
					predefSizes[presetList.u.array.length * 2] = 0;
					predefSizes[presetList.u.array.length * 2 + 1] = 0;

					SetAllProps(PROPOPT_PARAMREQUIRED, predefSizes);
				}
				if (presetList.type != json_type::json_none)
				{
					DarkEdif::MsgBox::Error(_T("DarkEdif JSON property"), _T("Invalid PresetSizes array in size property %s."),
						UTF8ToTString(Property["Title"sv]).c_str());
				}

				SetAllProps(0, NULL);
			}

			// Checkbox
			case PROPTYPE_LEFTCHECKBOX:
				// Enforce option to show it is a checkbox
				SetAllProps(PROPOPT_CHECKBOX, NULL);

			// Direction Selector
			case PROPTYPE_DIRCTRL:
				SetAllProps(0, NULL);

			// Group
			case PROPTYPE_GROUP:
				SetAllProps(0, NULL);

			// Edit box + browse file button, Parameter = FilenameCreateParams
			case PROPTYPE_FILENAME:
			case PROPTYPE_PICTUREFILENAME:
				SetAllProps(0, NULL);

			// Font dialog box
			case PROPTYPE_FONT:
				SetAllProps(0, NULL);

			// Edit box for floating point numbers (does not support min/max)
			case PROPTYPE_EDIT_FLOAT:
				SetAllProps(0, NULL);

			// Image list
			case PROPTYPE_IMAGELIST:
			{
				SetAllProps(0, NULL);
				/*
				int* textLength = new int;
				*textLength = 0;
				SetAllProps(PROPOPT_PARAMREQUIRED, textLength);*/
			}

			// Combo box with icons
			case PROPTYPE_ICONCOMBOBOX:
				SetAllProps(0, NULL);

			// URL button
			case PROPTYPE_URLBUTTON:
				SetAllProps(0, NULL);

			// Edit + Spin, value = floating point number, Parameters = min value, max value
			case PROPTYPE_SPINEDITFLOAT:
				SetAllProps(0, NULL);

			// Unrecognised
			default:
				MsgBox::Error(errorPrefix, _T("The Parameter type \"%hs\" was unrecognised."),
					Property["Type"sv].c_str());
				SetAllProps(0, NULL);
			}
		}

		// Add to properties
		VariableProps.push_back(*CurrentProperty);
	}

	Edif::SDK->EdittimeProperties = std::make_unique<PropData[]>(VariableProps.size() + 1);
	if (!VariableProps.empty())
		memcpy(Edif::SDK->EdittimeProperties.get(), &VariableProps[0], sizeof(PropData)* VariableProps.size());

	// End with completely null PropData
	memset(&Edif::SDK->EdittimeProperties[VariableProps.size()], 0, sizeof(PropData));

	if (!openFolderList.empty())
	{
		MsgBox::Error(_T("JSON verification error"), _T("There was a Folder [%hs] that does not have a FolderEnd property."),
			openFolderList.back());
	}

	// Generate hash of JSON props and IDs, in case JSON is changed and
	// EDITDATA properties are now invalid.
	std::stringstream hashNamesAndTypes, hashTypes;
	for (std::size_t i = 0; i < props.u.array.length; ++i)
	{
		const json_value& p = *props.u.array.values[i];

		// Don't include unchangeable props in the hash. Note there are still Data for those.
		if (IsUnchangeablePropExclCheckbox(p["Type"sv]))
			continue;

		// Lowercase the name in case there's a simple typo.
		std::string propName(p["Title"sv]);
		std::transform(propName.begin(), propName.end(), propName.begin(), ::tolower);

		std::uint32_t propTypeID = Edif::SDK->EdittimeProperties[i].Type_ID % 1000;

		hashNamesAndTypes << propName << '|' << propTypeID << " | "sv;
		hashTypes << propTypeID << '|';
	}

	Edif::SDK->jsonPropsNameAndTypesHash = fnv1a(hashNamesAndTypes.str());
	Edif::SDK->jsonPropsTypesHash = fnv1a(hashTypes.str());
}

#endif // EditorBuild

#if defined (_WIN32) && TEXT_OEFLAG_EXTENSION
std::unique_ptr<LOGFONT> DarkEdif::EditDataFont::GetWindowsLogFont() const {
	// In LOGFONT, these members are separate bytes; check the bools here aren't optimized into one byte
	static_assert(offsetof(EditDataFont, italic) != offsetof(EditDataFont, strikeOut),
		"Font member padding was too strong");

	std::unique_ptr<LOGFONT> ret = std::make_unique<LOGFONT>();
	if (memcpy_s(&ret->lfHeight, offsetof(EditDataFont, fontNameU8), &height, sizeof(LOGFONT) - sizeof(LOGFONT::lfFaceName)))
		LOGE(_T("Couldn't copy font content from EditDataFont -> LOGFONT, error %d."), errno);
	const std::tstring fntNameTStr = UTF8ToTString(fontNameU8);
	if (memcpy_s(&ret->lfFaceName, sizeof(ret->lfFaceName), fntNameTStr.c_str(), fntNameTStr.size() * sizeof(TCHAR)))
		LOGE(_T("Couldn't copy font name from EditDataFont -> LOGFONT, error %d."), errno);
	return ret;
}
std::unique_ptr<LOGFONTA> DarkEdif::EditDataFont::GetWindowsLogFontA() const {
	std::unique_ptr<LOGFONTA> ret = std::make_unique<LOGFONTA>();
	if (memcpy_s(&ret->lfHeight, offsetof(EditDataFont, fontNameU8), &height, sizeof(LOGFONT) - sizeof(LOGFONT::lfFaceName)))
		LOGE(_T("Couldn't copy font content from EditDataFont -> LOGFONTA, error %d."), errno);
	const std::string fntNameAStr = UTF8ToANSI(fontNameU8);
	if (memcpy_s(&ret->lfFaceName, sizeof(ret->lfFaceName), fntNameAStr.c_str(), fntNameAStr.size()))
		LOGE(_T("Couldn't copy font name from EditDataFont -> LOGFONTA, error %d."), errno);
	return ret;
}
std::unique_ptr<LOGFONTW> DarkEdif::EditDataFont::GetWindowsLogFontW() const {
	std::unique_ptr<LOGFONTW> ret = std::make_unique<LOGFONTW>();
	if (memcpy_s(&ret->lfHeight, offsetof(EditDataFont, fontNameU8), &height, sizeof(LOGFONT) - sizeof(LOGFONT::lfFaceName)))
		LOGE(_T("Couldn't copy font content from EditDataFont -> LOGFONTW, error %d."), errno);
	const std::wstring fntNameWStr = DarkEdif::UTF8ToWide(fontNameU8);
	if (memcpy_s(&ret->lfFaceName, sizeof(ret->lfFaceName), fntNameWStr.c_str(), fntNameWStr.size() * sizeof(wchar_t)))
		LOGE(_T("Couldn't copy font name from EditDataFont -> LOGFONTW, error %d."), errno);
	return ret;
}

void DarkEdif::EditDataFont::SetWindowsLogFont(const LOGFONT * const fnt) {
	if (!fnt)
		return LOGF(_T("Font cannot be null.\n"));
	if (memcpy_s(&height, offsetof(DarkEdif::EditDataFont, fontNameU8), &fnt->lfHeight, sizeof(LOGFONT) - sizeof(LOGFONT::lfFaceName)))
		LOGE(_T("Couldn't copy LOGFONT data -> EditDataFont, error %d.\n"), errno);
	this->SetFontName(fnt->lfFaceName); // calls UpdateLogFont
}
void DarkEdif::EditDataFont::SetWindowsLogFontA(const LOGFONTA * const fnt) {
	if (!fnt)
		return LOGF(_T("Font cannot be null.\n"));
	if (memcpy_s(&height, offsetof(DarkEdif::EditDataFont, fontNameU8), &fnt->lfHeight, sizeof(LOGFONT) - sizeof(LOGFONT::lfFaceName)))
		LOGE(_T("Couldn't copy LOGFONTA data -> EditDataFont, error %d.\n"), errno);
	this->SetFontName(ANSIToTString(fnt->lfFaceName));
}
void DarkEdif::EditDataFont::SetWindowsLogFontW(const LOGFONTW * const fnt) {
	if (!fnt)
		return LOGF(_T("Font cannot be null.\n"));
	if (memcpy_s(&height, offsetof(DarkEdif::EditDataFont, fontNameU8), &fnt->lfHeight, sizeof(LOGFONT) - sizeof(LOGFONT::lfFaceName)))
		LOGE(_T("Couldn't copy LOGFONTW data -> EditDataFont, error %d.\n"), errno);
	this->SetFontName(WideToTString(fnt->lfFaceName));
}

// Initializes from frame editor e.g. for CreateObject. Uses the frame default font,
// falls back on Arial, default size.
void DarkEdif::EditDataFont::Initialize(mv * mV)
{
	memset(this, 0, sizeof(*this));
	LOGFONT lg;
	if (mV->mvGetDefaultFont &&
		mV->mvGetDefaultFont(&lg, NULL, 0))
	{
		SetWindowsLogFont(&lg);
	}
	else
		SetFontName(_T("Arial"sv));
}

// Gets the DT_XX align flags converted to TEXT_ALIGN_XX flags.
std::uint32_t DarkEdif::EditDataFont::GetFusionTextAlignment() const
{
	// If TextCapacity is not defined in Extension, refer to DarkEdif Template
	constexpr TextCapacity textCaps = Extension::TextCapacity & (TextCapacity::HorizontalAlign | TextCapacity::VerticalAlign | TextCapacity::RightToLeft);
	TextCapacity ret = TextCapacity::None;
	if ((dtFlags | DT_RIGHT) != 0)
		ret = TextCapacity::Right;
	else if ((dtFlags | DT_CENTER) != 0)
		ret = TextCapacity::HCenter;
	else
		ret = TextCapacity::Left;
	if ((dtFlags | DT_BOTTOM) != 0)
		ret |= TextCapacity::Bottom;
	else if ((dtFlags | DT_VCENTER) != 0)
		ret |= TextCapacity::VCenter;
	else
		ret |= TextCapacity::Top;
	if ((dtFlags | DT_RTLREADING) != 0)
		ret |= TextCapacity::RightToLeft;
	return (std::uint32_t)(ret & Extension::TextCapacity);
}

// Converts the Fusion editor TextCapacity to DrawText DT_XX enum,
// optionally adding extra passed flags.
void DarkEdif::EditDataFont::SetFusionTextAlignment(TextCapacity textAlign, bool setSingleLine /* = true */)
{
	constexpr std::uint32_t allFlags = (std::uint32_t)(TextCapacity::HorizontalAlign | TextCapacity::VerticalAlign | TextCapacity::RightToLeft);
	constexpr TextCapacity textCaps = Extension::TextCapacity;
	if ((((std::uint32_t)textAlign) & (~allFlags)) != 0)
	{
		return MsgBox::Error(_T("Invalid align flags"), _T("Invalid text align flags passed: %u (0x%x)."),
			textAlign, textAlign);
	}
	if ((textAlign & textCaps) != TextCapacity::None)
	{
		return MsgBox::Error(_T("Invalid align flags"), _T("Uncapable text align flags passed: %u (0x%x)."),
			(std::uint32_t)(textAlign & textCaps), (std::uint32_t)(textAlign & Extension::TextCapacity));
	}
	if ((textCaps & TextCapacity::HorizontalAlign) != TextCapacity::None &&
		(textAlign & TextCapacity::HorizontalAlign) == TextCapacity::None)
		textAlign |= TextCapacity::Left;
	if ((textCaps & TextCapacity::HorizontalAlign) != TextCapacity::None &&
		(textAlign & TextCapacity::VerticalAlign) == TextCapacity::None)
		textAlign |= TextCapacity::Top;

	// SINGLELINE is required for VCENTER or BOTTOM, see MSDN DrawText docs
	// We'll only clear it if okayed to
	const std::uint32_t singleLine = setSingleLine ? DT_SINGLELINE : 0;
	dtFlags &= ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM | singleLine);

	// dtFlags |= (textAlign & TextCapacity::Left) != TextCapacity::None ? DT_LEFT : 0; // no-op
	dtFlags |= (textAlign & TextCapacity::Right) != TextCapacity::None ? DT_RIGHT : 0;
	dtFlags |= (textAlign & TextCapacity::HCenter) != TextCapacity::None ? DT_CENTER : 0;

	// dtFlags |= (textAlign & TextCapacity::Top) != TextCapacity::None ? DT_TOP : 0; // no-op
	dtFlags |= (textAlign & TextCapacity::VCenter) != TextCapacity::None ? DT_VCENTER | singleLine : 0;
	dtFlags |= (textAlign & TextCapacity::Bottom) != TextCapacity::None ? DT_BOTTOM | singleLine : 0;

	dtFlags |= (textAlign & TextCapacity::RightToLeft) != TextCapacity::None ? DT_RTLREADING : 0;
}

std::tstring DarkEdif::EditDataFont::GetFontName() const {
	// This must be consistent between platforms/configs
	static_assert(offsetof(DarkEdif::EditDataFont, fontNameU8) == 28, "Unexpected offset");
	return UTF8ToTString(fontNameU8);
}
void DarkEdif::EditDataFont::SetFontName(std::tstring_view fontNameTStr) {
	if (strcpy_s(fontNameU8, std::size(fontNameU8), TStringToUTF8(fontNameTStr).c_str()))
		LOGE(_T("Couldn't copy font name from LOGFONT -> EditDataFont, error %d."), errno);
}

#endif // _WIN32 and TEXT_OEFLAG_EXTENSION

#if TEXT_OEFLAG_EXTENSION
void FontInfoMultiPlat::CopyFromEditFont(Extension* const ext, const DarkEdif::EditDataFont &df)
{
	height = df.height;
	width = df.width;
	escapement = df.escapement;
	orientation = df.orientation;
	weight = df.weight;
	italic = df.italic;
	underline = df.underline;
	strikeOut = df.strikeOut;
	charSet = df.charSet;
	outPrecision = df.outPrecision;
	clipPrecision = df.clipPrecision;
	quality = df.quality;
	pitchAndFamily = df.pitchAndFamily;
	fontNameDesired = UTF8ToTString(df.fontNameU8);
	fontColor = df.fontColor;
	drawTextFlags = df.dtFlags;

#ifdef _WIN32
	logFont = df.GetWindowsLogFont();
	fontHandle = CreateFontIndirect(&*logFont);
#elif defined(__ANDROID__)
	threadEnv->SetIntField(cfontinfo, lfHeight, df.height);
	threadEnv->SetIntField(cfontinfo, lfWeight, df.weight);
	threadEnv->SetByteField(cfontinfo, lfItalic, df.italic);
	threadEnv->SetByteField(cfontinfo, lfUnderline, df.underline);
	threadEnv->SetByteField(cfontinfo, lfStrikeOut, df.strikeOut);

	jstring str = nullptr;
	if (df.fontNameU8[0] != '\0')
		str = CStrToJStr(df.fontNameU8);
	threadEnv->SetObjectField(cfontinfo, lfFaceName, str);
#else // Apple
	// Based on CFile::readLogFont
	cfontinfo = [[CFontInfo alloc] init];
	cfontinfo->lfHeight = df.height;
	if (cfontinfo->lfHeight < 0)
		cfontinfo->lfHeight = -cfontinfo->lfHeight; // Phi note: bad code
	// [self skipBytes:12];	// skip width, escapement, orientation
	cfontinfo->lfWeight = df.weight;
	cfontinfo->lfItalic = df.italic;
	cfontinfo->lfUnderline = df.underline;
	cfontinfo->lfStrikeOut = df.strikeOut;
	// [self skipBytes:5]; // skip charset, precision, quality
	cfontinfo->lfFaceName = [NSString stringWithUTF8String: df.fontNameU8];
#endif

	// Tie this font to ext, so SetRunObjectFont etc can refer to it
	if (ext->Runtime.extFont)
		LOGE(_T("Runtime extFont for text-based property was already inited."));
	ext->Runtime.extFont = this;
}
#endif // TEXT_OEFLAG_EXTENSION

#ifdef _WIN32
DarkEdif::FontInfoMultiPlat::FontInfoMultiPlat(HFONT ptr) : FontInfoMultiPlat() {
	SetFont(ptr);
}
void DarkEdif::FontInfoMultiPlat::SetFont(HFONT ptr) {
	fontHandle = ptr;
	if (ptr != NULL)
	{
		if (!logFont)
			logFont = std::make_unique<LOGFONT>();
		GetObject(ptr, sizeof(LOGFONT), logFont.get());
	}
	else
		logFont.reset();
	SetFont(logFont.get());
}
void DarkEdif::FontInfoMultiPlat::SetFont(const LOGFONT* ptr) {
	memcpy(&height, ptr, sizeof(LOGFONT) - sizeof(LOGFONT::lfFaceName));
	fontNameDesired = ptr->lfFaceName;
}

#elif defined(__ANDROID__)

extern thread_local JNIEnv* threadEnv;

JavaAndCString::JavaAndCString(jstring javaLocalRefStr, bool promoteToGlobal /* = false */)
{
	init(javaLocalRefStr, promoteToGlobal);
}
void JavaAndCString::init(jstring javaLocalRefStr, bool promoteToGlobal /* = false */)
{
	if (javaRef)
		LOGF(_T("init is for init only\n"));

	global = (promoteToGlobal && javaLocalRefStr);
	javaRef = global ? (jstring)threadEnv->NewGlobalRef(javaLocalRefStr) : javaLocalRefStr;
	JNIExceptionCheck();
	// null string from Java
	if (!javaLocalRefStr)
		return;

	memModUTF8 = threadEnv->GetStringUTFChars(javaRef, NULL);
	JNIExceptionCheck();

	// Check for Java's Modified-UTF-8 spins and convert to standard UTF-8
	// I'm not sure this is needed at all, as Java shouldn't have embedded nulls in its strings,
	// and the surrogate pair encoding is valid Unicode, just a bit longer form than necessary.
	// As it was a PITA to find decoding for it, I'm keeping it in.
	bool needsConvert = false;
	// Embedded null
	if (memModUTF8.find("\xC0\x80"sv) != std::tstring_view::npos)
		needsConvert = true;
	else if (memModUTF8.size() > 5)
	{
		for (std::size_t i = 0, j = memModUTF8.size() - 5; i < j; ++i)
		{
			if ((memModUTF8[i] & 0xF0) == 0xE0 && (memModUTF8[i + 1] & 0xC0) == 0x80 &&
				(memModUTF8[i + 2] & 0xC0) == 0x80 && (memModUTF8[i + 3] & 0xF0) == 0xE0 &&
				(memModUTF8[i + 4] & 0xC0) == 0x80 && (memModUTF8[i + 5] & 0xC0) == 0x80)
			{
				needsConvert = true;
				break;
			}
		}
	}
	if (needsConvert)
	{
		// Modified UTF-8 stores null as two bytes, and 4-byte UTF-8 as 2x3 byte MUTF-8,
		// so for the most part it's smaller, but we'll be aggressive anyway cos RAM is cheap.
		memUTF8.resize(memModUTF8.size() * 2, '\xDD');
		std::size_t j = 0;
		for (std::size_t i = 0; i < memModUTF8.size(); ++i)
		{
			// 0xE0 + 0x80 => embedded null
			if (memModUTF8.size() - i > 2 &&
				(memModUTF8[i] & 0xE0) == (char)0xC0 && memModUTF8[i + 1] == (char)0x80)
			{
				memUTF8[j++] = 0;
				++i; // skip i + 1
				continue;
			}
			// UTF-8 4-byte sequences are encoded as two 3-byte sequences
			if (memModUTF8.size() - i > 5 &&
				(memModUTF8[i] & 0xF0) == 0xE0 && (memModUTF8[i + 1] & 0xC0) == 0x80 &&
				(memModUTF8[i + 2] & 0xC0) == 0x80 && (memModUTF8[i + 3] & 0xF0) == 0xE0 &&
				(memModUTF8[i + 4] & 0xC0) == 0x80 && (memModUTF8[i + 5] & 0xC0) == 0x80)
			{
				// We read as bytes u-z, so i-(i+5), discarding u and x.
				// https://docs.oracle.com/javase/specs/jvms/se21/html/jvms-4.html#jvms-4.4.7:~:text=The%20six%20bytes%20represent
				unsigned int codePoint =
					0x10000 + ((memModUTF8[i + 1] & 0x0f) << 16) + ((memModUTF8[i + 2] & 0x3f) << 10) +
					((memModUTF8[i + 4] & 0x0f) << 6) + (memModUTF8[i + 5] & 0x3f);

				// Convert the code point to standard 4-byte UTF-8
				memUTF8[j++] = (char)(0xF0 | (codePoint >> 18));
				memUTF8[j++] = (char)(0x80 | ((codePoint >> 12) & 0x3F));
				memUTF8[j++] = (char)(0x80 | ((codePoint >> 6) & 0x3F));
				memUTF8[j++] = (char)(0x80 | (codePoint & 0x3F));
				i += 6;
				continue;
			}
			memUTF8[j] = memModUTF8[i];
		}
		memUTF8.resize(j);
	}
}
JavaAndCString::~JavaAndCString()
{
	if (memModUTF8.data())
		threadEnv->ReleaseStringUTFChars(javaRef, memModUTF8.data());
	if (javaRef)
	{
		if (global)
			threadEnv->DeleteGlobalRef(javaRef);
		else
			threadEnv->DeleteLocalRef(javaRef);
	}
}
std::tstring_view JavaAndCString::str() const {
	return memUTF8.empty() ? memModUTF8 : memUTF8;
}

// static definition with default ctor
jfieldID DarkEdif::FontInfoMultiPlat::lfHeight, DarkEdif::FontInfoMultiPlat::lfWeight,
	DarkEdif::FontInfoMultiPlat::lfItalic, DarkEdif::FontInfoMultiPlat::lfUnderline,
	DarkEdif::FontInfoMultiPlat::lfStrikeOut, DarkEdif::FontInfoMultiPlat::lfFaceName;

DarkEdif::FontInfoMultiPlat::FontInfoMultiPlat(jobject ptr) : FontInfoMultiPlat() {
	SetFont(ptr);
}
void DarkEdif::FontInfoMultiPlat::SetFont(const jobject ptr) {
	if (ptr == NULL)
	{
		height = weight = charSet = outPrecision = clipPrecision =
			quality = pitchAndFamily = fontColor = orientation = escapement = 0;
		italic = underline = strikeOut = false;
		fontNameDesired.clear();
		// keep dtflags same
		return;
	}

	height = threadEnv->GetIntField(ptr, lfHeight);
	weight = threadEnv->GetIntField(ptr, lfWeight);
	italic = threadEnv->GetByteField(ptr, lfItalic);
	underline = threadEnv->GetByteField(ptr, lfUnderline);
	strikeOut = threadEnv->GetByteField(ptr, lfStrikeOut);
	JavaAndCString str((jstring)threadEnv->GetObjectField(ptr, lfFaceName));
	fontNameDesired = str.str();
}
#else // apple
DarkEdif::FontInfoMultiPlat::FontInfoMultiPlat(CFontInfo* ptr) {
	cfontinfo = ptr;
	SetFont(cfontinfo);
}
void DarkEdif::FontInfoMultiPlat::SetFont(const void * ptr2) {
	const CFontInfo* ptr = (const CFontInfo*)ptr2;
	height = ptr->lfHeight;
	weight = ptr->lfWeight;
	italic = ptr->lfItalic != 0;
	underline = ptr->lfUnderline != 0;
	strikeOut = ptr->lfStrikeOut != 0;
	fontNameDesired = [ptr->lfFaceName UTF8String];
}

#endif

DarkEdif::FontInfoMultiPlat::FontInfoMultiPlat() {
#ifdef __ANDROID__
	cfontinfoClass = global(threadEnv->FindClass("Services/CFontInfo"), "CFontInfo class from EditDataFont");
	JNIExceptionCheck();
	cfontinfo = global(threadEnv->AllocObject(cfontinfoClass), "CFontInfo from EditDataFont");
	JNIExceptionCheck();
	if (lfHeight == NULL)
	{
		lfHeight = threadEnv->GetFieldID(cfontinfoClass, "lfHeight", "I");
		JNIExceptionCheck();
		lfWeight = threadEnv->GetFieldID(cfontinfoClass, "lfWeight", "I");
		JNIExceptionCheck();
		lfItalic = threadEnv->GetFieldID(cfontinfoClass, "lfItalic", "B");
		JNIExceptionCheck();
		lfUnderline = threadEnv->GetFieldID(cfontinfoClass, "lfUnderline", "B");
		JNIExceptionCheck();
		lfStrikeOut = threadEnv->GetFieldID(cfontinfoClass, "lfStrikeOut", "B");
		JNIExceptionCheck();
		lfFaceName = threadEnv->GetFieldID(cfontinfoClass, "lfFaceName", "Ljava/lang/String;");
		JNIExceptionCheck();
	}
#elif defined(__APPLE__)
	cfontinfo = [[CFontInfo alloc] init];
#endif
}
DarkEdif::FontInfoMultiPlat::~FontInfoMultiPlat() {
#ifdef _WIN32
	if (fontHandle)
	{
		DeleteObject(fontHandle);
		fontHandle = NULL;
	}
#elif defined (__APPLE__)
	// TODO: Will this be auto-dealloc'd?
	if (cfontinfo)
	{
		[cfontinfo dealloc];
		cfontinfo = NULL;
	}
#endif
}
std::tstring DarkEdif::FontInfoMultiPlat::GetActualFontName() {
#ifndef _WIN32
	return fontNameDesired;
#else // _WIN32
	if (!fontName.empty())
		return fontName;
	fontName.resize(64);
	// Font names may be localised, particularly for CJK and English.
	// Hat tip: https://stackoverflow.com/a/7193439
	// See a workaround: https://chromium.googlesource.com/chromium/blink/+/e2b472488d4a23f4eb7acdf2d590d513b802820e/Source/platform/fonts/win/FontCacheWin.cpp#64
	HWND ourWin = Edif::SDK->mV->RunApp->hEditWin;
	HDC ourDC = GetDC(ourWin);
	if (ourDC == NULL)
		return LOGE(_T("Couldn't get original DC, error %u."), GetLastError()), _T("<error>"s);
	HFONT origFont = (HFONT)SelectObject(ourDC, fontHandle);
	if (origFont == NULL)
	{
		LOGE(_T("Couldn't get original DC font, error %u."), GetLastError());
		ReleaseDC(ourWin, ourDC);
		return _T("<error>"s);
	}

	// TODO: It is not obvious whether GetTextFace ignores thread locale in favour of user default locale.
	//
	// PRIMARYLANGID(GetUserDefaultLangID()) != LANG_ENGLISH
	LCID origLCID = GetThreadLocale();
	SetThreadLocale(MAKELCID(LGRPID_WESTERN_EUROPE, SORT_DEFAULT));
	int res = GetTextFace(ourDC, fontName.size(), fontName.data());
	if (res <= 0)
		LOGE(_T("Couldn't get true font name, error %u."), GetLastError()), _T("<error>"s);
	else
	{
		fontName.resize(res - 1); // res includes null
		// TEXTMETRIC metrics;
		// GetTextMetrics(ourDC, &metrics);
	}
	SetThreadLocale(origLCID);
	origFont = (HFONT)SelectObject(ourDC, origFont);
	if (origFont == NULL)
		LOGE(_T("Couldn't restore original DC font, error %u."), GetLastError());
	ReleaseDC(ourWin, ourDC);
	return fontName;
#endif // _WIN32
}


// Returns size of EDITDATA and all properties if they were using their default values from JSON
std::uint16_t DarkEdif::DLL::Internal_GetEDITDATASizeFromJSON()
{
	const json_value& JSON = CurLang["Properties"sv];
	size_t fullSize = sizeof(EDITDATA);
	// Store one bit per property, for checkboxes
	fullSize += (int)std::ceil(JSON.u.array.length / 8.0f);

	DebugProp_OutputString(_T("Calulation: started with EDITDATA size %zu, checkbox size %d, accumulative size is now %zu.\n"), sizeof(EDITDATA), (int)std::ceil(JSON.u.array.length / 8.0f), fullSize);
	for (std::size_t i = 0, j = 0; j < JSON.u.array.length; ++i, ++j)
	{
		const json_value& propjson = *JSON.u.array.values[i];
		const std::string_view curPropType = propjson["Type"sv];

		// Metadata for all properties
		fullSize += sizeof(Properties::Data);

		// No title for unchangeable props, we won't need to smart rearrange those properties
		// by title, so storing the title is unnecessary.
		if (IsUnchangeablePropExclCheckbox(curPropType))
		{
			DebugProp_OutputString(_T("Adding property %hs (type %hs) accumulative size is now %zu.\n"),
				propjson["Title"sv].c_str(), curPropType, fullSize);
			continue;
		}

		fullSize += strnlen(propjson["Title"sv].c_str(), 254);
		// Don't bother checking title length. Other funcs will do that.

		// Checkboxes store title, but not data
		if (IsUnchangeablePropInclCheckbox(curPropType))
		{
			DebugProp_OutputString(_T("Adding property %hs (type %hs) accumulative size is now %zu.\n"),
				propjson["Title"sv].c_str(), curPropType, fullSize);
			continue;
		}

		// Stores text
		if (IsStringPropType(curPropType, false))
		{
			const std::string_view defaultText = propjson["DefaultState"sv];
			fullSize += defaultText.size(); // UTF-8
		}
		// Combo box: stores either the item text selected, or a RuntimePropSet
		else if (SVIComparePrefix(curPropType, "Combo Box"sv))
		{
#if !defined(NOPROPS) && EditorBuild
			const auto esIt = std::find_if(Edif::SDK->EdittimePropertySets.cbegin(), Edif::SDK->EdittimePropertySets.cend(),
				[i](const auto& es) { return es.nameListIdx == i; });
			if (esIt != Edif::SDK->EdittimePropertySets.cend())
			{
				DebugProp_OutputString(_T("Property %hs is a combo box set list. Adding size of RuntimePropSet + name.\n"),
					propjson["Title"sv].c_str());
				fullSize += sizeof(RuntimePropSet) + esIt->setName.size();
			}
			else
#endif // NOPROPS
			{
				const std::string_view defaultText = propjson["DefaultState"sv];
				fullSize += 1 + defaultText.size(); // UTF-8, with an L char prefix
			}
		}
		else if (SVICompare(curPropType, "Editbox Number"sv) || SVICompare(curPropType, "Edit spin"sv) || SVICompare(curPropType, "Edit slider"sv) ||
			SVICompare(curPropType, "Color"sv) || SVICompare(curPropType, "Edit direction"sv) || SVICompare(curPropType, "Editbox Float"sv) ||
			SVICompare(curPropType, "Edit spin float"sv))
		{
			fullSize += sizeof(int); // Floats are same size as int
		}
		// Stores two numbers
		else if (SVICompare(curPropType, "Size"sv))
			fullSize += sizeof(int) * 2;
		else if (SVICompare(curPropType, "Image list"))
		{
			// TODO: Check options are valid
			std::uint16_t numImages = (std::uint16_t)(json_int_t)propjson["NumImages"sv];
			assert(numImages > 0);
			// increment for numImages prefix
			fullSize += ++numImages * 2;
			// Required that there is one image added
			//if (((bool)propjson["FixedNumOfImages"]) && ((json_int_t)propjson["MaxNumImages"] == 1))
			//	fullSize += sizeof(std::uint16_t);
		}
		else
		{
			MsgBox::Error(_T("GetEDITDATASizeFromJSON failed"), _T("Calculation of edittime property size can't understand property type \"%s\". (%s)"),
				UTF8ToTString(curPropType).c_str(), JSON::LanguageName());
		}
		DebugProp_OutputString(_T("Adding property %hs (type %s) accumulative size is now %zu.\n"),
			propjson["Title"sv].c_str(), UTF8ToTString(curPropType).c_str(), fullSize);
	}
	DebugProp_OutputString(_T("Accumulative size finalized at %zu.\n"), fullSize);

#if !defined(NOPROPS) && EditorBuild
	// This function estimates the size of a fresh edPtr without generating one.
	// To check it's working, generate an actual fresh edPtr and see if it's different
	if constexpr (DarkEdif::Properties::DebugProperties)
	{
		HGLOBAL data = DLL_UpdateEditStructure(Edif::SDK->mV, nullptr);
		if (!data)
			return std::numeric_limits<std::uint16_t>::max();
		EDITDATA* edPtr = (EDITDATA*)GlobalLock(data);
		if (!edPtr)
		{
			GlobalFree(data);
			return std::numeric_limits<std::uint16_t>::max();
		}
		std::uint32_t actualResult = edPtr->eHeader.extSize;
		GlobalUnlock(data);
		GlobalFree(data);

		if (fullSize != actualResult)
		{
			MsgBox::Error(_T("GetEDITDATASizeFromJSON failed"), _T("Mismatch of calculation and actual: calculated %zu bytes, but the actual new EDITDATA was %zu bytes."),
				actualResult, fullSize);
		}
	}
#endif
	if (fullSize >= UINT16_MAX)
	{
		MsgBox::Error(_T("EDITDATA too large!"), _T("The JSON extension properties use more than %u bytes to store, and are too large for Fusion. (%s)"),
			UINT16_MAX, JSON::LanguageName());
	}

	DebugProp_OutputString(_T("GetEDITDATASizeFromJSON: result is %zu bytes.\n"), fullSize);

	return (std::uint16_t)fullSize;
}

// =====
// Get event number (CF2.5+ feature)
// =====


// Static definition; set during SDK::SDK()
#ifdef _WIN32
// True if Fusion 2.5. False if Fusion 2.0. Set during SDK ctor.
bool DarkEdif::IsFusion25;
// True if angle variables are degrees as floats, false if they are ints.
// Ints are used in MMF2 non-Direct3D display modes.
bool DarkEdif::IsHWAFloatAngles;
// True if running under Wine, false otherwise.
bool DarkEdif::IsRunningUnderWine;
#endif

// Returns the Fusion event number for this group. Works in CF2.5 and MMF2.0
std::uint16_t DarkEdif::GetEventNumber(EventGroupMP * evg) {
	if (!evg)
		return -1;
	// Windows may be 2.0 or 2.5; if 2.5, the local SDK's evgInhibit is where the identifier is.
	// Android/iOS is assumed to be 2.5 and should work directly.
#ifdef _WIN32
	if (DarkEdif::IsFusion25) {
		return evg->get_evgInhibit();
	}
#endif
	return evg->get_evgIdentifier();
}

/// <summary> If error, -1 is returned. </summary>
int DarkEdif::GetCurrentFusionEventNum(const Extension * const ext)
{
	// Reading Fusion's internals requires the main runtime to not be editing them
	if (MainThreadID != std::this_thread::get_id()) {
		LOGE(_T("Read GetCurrentFusionEventNum from non-main thread. Returning -1.\n"));
		return -1;
	}

#ifdef _WIN32
	// Can we read current event?
	if (!ext->rhPtr->get_EventGroup())
		return -1;

	int eventNum = GetEventNumber(ext->rhPtr->get_EventGroup());
	if (eventNum != 0)
		return eventNum;
	return -1;
#elif defined(__ANDROID__)
	// Call `int darkedif_jni_getCurrentFusionEventNum()` Java function
	static jmethodID getEventIDMethod;
	if (getEventIDMethod == nullptr)
	{
		jclass javaExtClass = threadEnv->GetObjectClass(ext->javaExtPtr);
		getEventIDMethod = threadEnv->GetMethodID(javaExtClass, "darkedif_jni_getCurrentFusionEventNum", "()I");

		// This is a Java wrapper implementation failure and so its absence should be considered fatal
		if (getEventIDMethod == nullptr)
			LOGF("Failed to find CRun" PROJECT_TARGET_NAME_UNDERSCORES "'s darkedif_jni_getCurrentFusionEventNum method in Java wrapper file.\n");
	}

	return threadEnv->CallIntMethod(ext->javaExtPtr, getEventIDMethod);
#else // iOS
	return DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, getCurrentFusionEventNum)(ext->objCExtPtr);
#endif
}

// Returns path as-is if valid, an extracted binary file's path, or an error; error indicated by returning '>' at start of text
std::tstring DarkEdif::MakePathUnembeddedIfNeeded(const Extension * ext, const std::tstring_view filePath)
{
	if (filePath.empty())
		return _T(">File path was blank"s);
#if _WIN32
	std::tstring truePath(MAX_PATH + 1, _T(' '));
#ifdef _UNICODE
	if (Edif::SDK->mV->GetFileW(std::tstring(filePath).c_str(), truePath.data(), 0) == FALSE)
#else
	if (Edif::SDK->mV->GetFileA(std::tstring(filePath).c_str(), truePath.data(), 0) == FALSE)
#endif
		return _T(">mvGetFile returned false"s);
	else // trim extra space
		truePath.resize(_tcslen(truePath.c_str()));
#elif defined(__ANDROID__)
	// Call `String darkedif_jni_makePathUnembeddedIfNeeded(String)` Java function
	static jmethodID getEventIDMethod;
	if (getEventIDMethod == nullptr)
	{
		jclass javaExtClass = threadEnv->GetObjectClass(ext->javaExtPtr);
		getEventIDMethod = threadEnv->GetMethodID(javaExtClass, "darkedif_jni_makePathUnembeddedIfNeeded", "(Ljava/lang/String;)Ljava/lang/String;");

		// This is a Java wrapper implementation failure and so its absence should be considered fatal
		if (getEventIDMethod == nullptr)
			LOGF("Failed to find CRun" PROJECT_TARGET_NAME_UNDERSCORES "'s darkedif_jni_makePathUnembeddedIfNeeded method in Java wrapper file.\n");
	}

	// First, copy the C++ string to Java memory
	JavaAndCString origPath(CStrToJStr(std::string(filePath).c_str()));
	// Pass origPath as param to string func, read back func return into newPath
	JavaAndCString newPath((jstring)threadEnv->CallObjectMethod(ext->javaExtPtr, getEventIDMethod, origPath.javaRef));
	// Copy the C++ memory out into its own variable
	const std::string truePath(newPath.str());
#else
	const std::string truePath = DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, makePathUnembeddedIfNeeded)(ext->objCExtPtr, std::string(filePath).c_str());
#endif
	if (filePath != truePath)
		LOGV(_T("File path extracted from \"%s\" to \"%s\".\n"), std::tstring(filePath).c_str(), truePath.c_str());
	else
		LOGV(_T("File path extraction of \"%s\" produced no change.\n"), truePath.c_str());
	return truePath;
}

// =====
// Text conversion - definitions
// =====

#include <assert.h>

#ifdef _WIN32
// For Windows, TString can be Wide or ANSI.
// ANSI function calls are internally converted to Wide by Windows.
// ANSI is not UTF-8, the earliest OS version that *can* use UTF-8 for OS function calls is
// Windows 10 Insider Preview Build 17035, and even that is non-default and in beta.

std::tstring DarkEdif::ANSIToTString(const std::string_view input) {
	return WideToTString(ANSIToWide(input));
}
std::string DarkEdif::ANSIToUTF8(const std::string_view input) {
	return WideToUTF8(ANSIToWide(input));
}
std::wstring DarkEdif::ANSIToWide(const std::string_view input) {
	if (input.empty())
		return std::wstring();

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = MultiByteToWideChar(CP_ACP, 0, input.data(), input.size(), NULL, 0);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");
	std::wstring outputStr(length, L'\0');

	// Actually convert
	length = MultiByteToWideChar(CP_ACP, 0, input.data(), input.size(), outputStr.data(), outputStr.size());
	assert(length > 0 && "Failed to convert between string encodings.");
	assert(input.back() != '\0' && "Input ends with null.");
	assert(outputStr.back() != L'\0' && "Output ends with null.");

	return outputStr;
}
std::string DarkEdif::UTF8ToANSI(const std::string_view input, bool * const allValidChars /* = nullptr */) {
	return WideToANSI(UTF8ToWide(input), allValidChars);
}
std::tstring DarkEdif::UTF8ToTString(const std::string_view input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	if (allValidChars)
		*allValidChars = true; // UTF-8 and UTF-16 share all chars
	return UTF8ToWide(input);
#else
	return UTF8ToANSI(input, allValidChars);
#endif
}
std::wstring DarkEdif::UTF8ToWide(const std::string_view input)
{
	if (input.empty())
		return std::wstring();

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = MultiByteToWideChar(CP_UTF8, 0, input.data(), input.size(), NULL, 0);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");
	std::wstring outputStr(length, L'\0');

	// Actually convert
	length = MultiByteToWideChar(CP_UTF8, 0, input.data(), input.size(), outputStr.data(), outputStr.size());
	assert(length > 0 && "Failed to convert between string encodings.");
	assert(input.back() != '\0' && "Input ends with null.");
	assert(outputStr.back() != L'\0' && "Output ends with null.");
	return outputStr;
}
std::string DarkEdif::WideToANSI(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
	if (input.empty())
	{
		if (allValidChars)
			*allValidChars = true;
		return std::string();
	}

	BOOL someFailed = FALSE;

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = WideCharToMultiByte(CP_ACP, 0, input.data(), input.size(), NULL, 0, 0, allValidChars ? &someFailed : NULL);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");

	if (allValidChars)
		*allValidChars = (someFailed == FALSE);

	std::string outputStr(length, '\0');

	// Actually convert
	length = WideCharToMultiByte(CP_ACP, 0, input.data(), input.size(), outputStr.data(), outputStr.size(), 0, NULL);
	assert(length > 0 && "Failed to convert between string encodings.");
	assert(input.back() != L'\0' && "Input ends with null.");
	assert(outputStr.back() != '\0' && "Output ends with null.");
	return outputStr;
}
std::tstring DarkEdif::WideToTString(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	if (allValidChars)
		*allValidChars = true;
	return std::wstring(input);
#else
	return WideToANSI(input, allValidChars);
#endif
}
std::string DarkEdif::WideToUTF8(const std::wstring_view input)
{
	if (input.empty())
		return std::string();

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = WideCharToMultiByte(CP_UTF8, 0, input.data(), input.size(), NULL, 0, 0, 0);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");
	std::string outputStr(length, '\0');

	// Actually convert
	length = WideCharToMultiByte(CP_UTF8, 0, input.data(), input.size(), outputStr.data(), outputStr.size(), 0, 0);
	assert(length > 0 && "Failed to convert between string encodings.");
	assert(input.back() != L'\0' && "Input ends with null.");
	assert(outputStr.back() != '\0' && "Output ends with null.");
	return outputStr;
}
std::string DarkEdif::TStringToANSI(const std::tstring_view input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	return WideToANSI(input, allValidChars);
#else
	if (allValidChars)
		*allValidChars = true;
	return std::string(input);
#endif
}
std::string DarkEdif::TStringToUTF8(const std::tstring_view input) {
#ifdef _UNICODE
	return WideToUTF8(input);
#else
	return ANSIToUTF8(input);
#endif
}
std::wstring DarkEdif::TStringToWide(const std::tstring_view input) {
#ifdef _UNICODE
	return std::wstring(input);
#else
	return ANSIToWide(input);
#endif
}

#else // !_WIN32

// Linux-based OSes including Android uses UTF-8 by default.
// ANSI and UTF-8 can be considered equivalent.
// Wide-char is barely used at all in Linux, but when it is, it's UTF-32.
// iconv() would be needed, and it's beyond the scope of a regular extension.
// Instead, this code merely returns back.

std::tstring DarkEdif::ANSIToTString(const std::string_view input) {
	return UTF8ToTString(input);
}
std::string DarkEdif::ANSIToUTF8(const std::string_view input) {
	return std::string(input);
}
std::wstring DarkEdif::ANSIToWide(const std::string_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::wstring();
}
std::string DarkEdif::UTF8ToANSI(const std::string_view input, bool * const allValidChars /* = nullptr */) {
	return std::string(input);
}
std::tstring DarkEdif::UTF8ToTString(const std::string_view input, bool * const allValidChars /* = nullptr */) {
	return std::tstring(input);
}
std::wstring DarkEdif::UTF8ToWide(const std::string_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::wstring();
}
std::string DarkEdif::WideToANSI(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::string();
}
std::tstring DarkEdif::WideToTString(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::tstring();
}
std::string DarkEdif::WideToUTF8(const std::wstring_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::string();
}
std::string DarkEdif::TStringToANSI(const std::tstring_view input, bool * const allValidChars /* = nullptr */) {
	return TStringToUTF8(input);
}
std::string DarkEdif::TStringToUTF8(const std::tstring_view input) {
	return std::string(input);
}
std::wstring DarkEdif::TStringToWide(const std::tstring_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::wstring();
}

#endif

#ifdef __ANDROID__
extern thread_local JNIEnv * threadEnv;
static jobject getGlobalContext()
{
	jclass activityThread = threadEnv->FindClass("android/app/ActivityThread");
	jmethodID currentActivityThread = threadEnv->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
	jobject at = threadEnv->CallStaticObjectMethod(activityThread, currentActivityThread);

	jmethodID getApplication = threadEnv->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
	jobject context = threadEnv->CallObjectMethod(at, getApplication);
	return context;
}
int DarkEdif::MessageBoxA(WindowHandleType hwnd, const TCHAR * text, const TCHAR * caption, int iconAndButtons)
{
	jclass toast = threadEnv->FindClass("android/widget/Toast");
	jobject globalContext = getGlobalContext();
	jmethodID methodMakeText = threadEnv->GetStaticMethodID(toast, "makeText", "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
	if (methodMakeText == NULL) {
		LOGE("toast.makeText not Found\n");
		return 0;
	}

	std::string toastText = caption + std::string(" -  ") + text;
	jstring toastTextJStr = CStrToJStr(toastText.c_str());

	jobject toastobj = threadEnv->CallStaticObjectMethod(toast, methodMakeText, globalContext, toastTextJStr, 1 /* toast length long, 0 for short*/);

	// Java: toastobj.show();
	jmethodID methodShow = threadEnv->GetMethodID(toast, "show", "()V");
	threadEnv->CallVoidMethod(toastobj, methodShow);

	__android_log_print(iconAndButtons, PROJECT_TARGET_NAME_UNDERSCORES, "Msg Box swallowed: \"%s\", %s.", caption, text);
	if (!strncmp(caption, "DarkEdif", sizeof("DarkEdif") - 1) && (iconAndButtons & MB_ICONERROR) != 0)
		DarkEdif::BreakIfDebuggerAttached();
	return 0;
}

void DarkEdif::BreakIfDebuggerAttached()
{
	raise(SIGINT);
}

void DarkEdif::SetDataBreakpoint(const void * memory, std::size_t size, DataBreakpointType dbt /* = DataBreakpointType::Write */)
{
	// Too Java-ridden.
	LOGE(_T("SetDataBreakpoint not possible on Android.\n"));
	(void)memory;
	(void)size;
	(void)dbt;
	return;
}

#elif defined(_WIN32)

void DarkEdif::BreakIfDebuggerAttached()
{
	if (IsDebuggerPresent())
		DebugBreak();
}

void DarkEdif::SetDataBreakpoint(const void* memory, std::size_t size, DataBreakpointType dbt /* = DataBreakpointType::Write */)
{
	if (!memory || (unsigned long)memory > 0x80000000UL)
		LOGF(_T("Invalid data breakpoint set: ptr is 0x%p.\n"), memory);

	// I saw a note that negative addresses can only be set from a driver? Might be folklore.
#ifndef _WIN64
	if ((unsigned long)memory > 0x80000000UL)
		LOGW(_T("Invalid data breakpoint set: ptr is 0x%p.\n"), memory);
#endif
	// Sizes allowed: 1, 2, 4, 8
	if (size == 0 || size == 3 || (size >= 5 && size <= 7) || size > 8)
		LOGF(_T("Invalid data breakpoint set: size is %zu. Sizes allowed: 1, 2, 4, 8. Combine breakpoints if needed.\n"), size);

	// Data breakpoints are sometimes used for anti-DRM or cheating, like most debugger-stuff.
	// Using it in release builds is probably unintended by ext dev and an accidental overlap from their debug code.
#ifndef _DEBUG
	LOGW(_T("It is not recommended to use data breakpoints in Release builds.\n"));
#endif

	// Originally inspired from hwbrk set on CodeProject.
	// Now adapted from https://github.com/rad9800/hwbp4mw/blob/main/HWBPP.cpp
	// We don't monitor access from every thread here; this may be possible in hwbpp.cpp.
	// The original, less hackerman code spawned a thread to set the current thread's debug registers;
	// that may be a requirement or might be over-engineering.
	// Also note https://stackoverflow.com/a/40820763 has useful info
	static thread_local std::uint8_t numBreakpoints = 0;

	if (numBreakpoints > 4)
		return LOGF(_T("Setting up a data breakpoint failed: max of 4 breakpoints.\n"));

	DWORD curThreadID = GetCurrentThreadId();
	bool good = false;

	// Context may be clobbered between instructions if we read  it within same thread, so spawn another one.
	std::thread setter([&good, memory, curThreadID, size, dbt](int pos)
	{
		CONTEXT context = {};
		context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		HANDLE callerThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, curThreadID);
		if (!callerThread)
			return LOGF(_T("Setting up a data breakpoint failed: couldn't get thread access, error %u.\n"), GetLastError());

		if (SuspendThread(callerThread) == -1)
			return LOGF(_T("Setting up a data breakpoint failed: couldn't suspend original thread, error %u.\n"), GetLastError());

		if (!GetThreadContext(callerThread, &context))
		{
			ResumeThread(callerThread);
			return LOGF(_T("Setting up a data breakpoint failed: couldn't get thread context, error %u.\n"), GetLastError());
		}

		// This weird size conversion is correct, I've tested. Odd, however.
		const int siz = size == 1 ? 0 : size == 2 ? 1 : size == 4 ? 3 : /* size == 8 */ 2;

		//if constexpr (create) {
		(&context.Dr0)[pos] = (DWORD)(long)memory;

		const auto SetBits = [](DWORD_PTR* dw, int lowBit, int bits, int newValue) {
			DWORD_PTR mask = (1 << bits) - 1;
			*dw = (*dw & ~(mask << lowBit)) | (newValue << lowBit);
		};
		SetBits(&context.Dr7, 16 + pos * 4, 2, (int)dbt);
		SetBits(&context.Dr7, 18 + pos * 4, 2, siz);
		SetBits(&context.Dr7, pos * 2, 1, 1);
		/* else remove:
			if ((&context.Dr0)[pos] == address) {

			}
		}*/
		BOOL ret = SetThreadContext(callerThread, &context);
		ResumeThread(callerThread);
		CloseHandle(callerThread);
		if (!ret)
			return LOGF(_T("Setting up a data breakpoint failed: couldn't set thread context, error %u.\n"), GetLastError());
		good = true;
	}, numBreakpoints);
	setter.join();

	if (good)
		++numBreakpoints;
}

[[noreturn]]
void DarkEdif::LOGFInternal(PrintFHintInside const TCHAR * x, ...)
{
	TCHAR buf[2048];
	va_list va;
	va_start(va, x);
	_vstprintf_s(buf, std::size(buf), x, va);
	va_end(va);
	DarkEdif::Log(DARKEDIF_LOG_FATAL, _T("%s"), buf);
	DarkEdif::MsgBox::Error(_T("Fatal error"), _T("%s"), buf);
	std::abort();
}
#else // APPLE

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/sysctl.h>

void DarkEdif::BreakIfDebuggerAttached()
{
	// Taken from https://stackoverflow.com/a/27254064
	static bool debuggerIsAttached = false;

	static dispatch_once_t debuggerPredicate;
	dispatch_once(&debuggerPredicate, ^{
		struct kinfo_proc info;
		size_t info_size = sizeof(info);
		int name[4] { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() }; // from unistd.h, included by Foundation

		if (sysctl(name, 4, &info, &info_size, NULL, 0) == -1)
		{
			LOGE("DarkEdif::BreakIfDebuggerAttached() couldn't call sysctl(): %s. Assuming true.", strerror(errno));
			debuggerIsAttached = true;
		}

		if (!debuggerIsAttached && (info.kp_proc.p_flag & P_TRACED) != 0)
			debuggerIsAttached = true;
	});

	if (debuggerIsAttached)
		__builtin_trap();
}

void DarkEdif::SetDataBreakpoint(const void* memory, std::size_t size, DataBreakpointType dbt /* = DataBreakpointType::Write */)
{
	// Too locked down.
	LOGE(_T("SetDataBreakpoint not possible on iOS/Mac.\n"));
	(void)memory;
	(void)size;
	(void)dbt;
	return;
}

int DarkEdif::MessageBoxA(WindowHandleType hwnd, const TCHAR * text, const TCHAR * caption, int iconAndButtons)
{
	NSString * firstButton = NULL, *secondButton = NULL, *thirdButton = NULL;
	__block int ret = -1;
	int ids[3];
	const int buttons = iconAndButtons & MB_TYPEMASK;
	if (buttons == MB_OK || buttons == MB_OKCANCEL)
	{
		firstButton = @"OK";
		ids[0] = IDOK;
		if (buttons == MB_OKCANCEL)
		{
			secondButton = @"Cancel";
			ids[1] = IDCANCEL;
		}
	}
	else if (buttons == MB_YESNO || buttons ==  MB_YESNOCANCEL)
	{
		firstButton = @"Yes";
		secondButton = @"No";
		ids[0] = IDYES;
		ids[1] = IDNO;
		if (buttons == MB_YESNOCANCEL)
		{
			thirdButton = @"Cancel";
			ids[2] = IDCANCEL;
		}
	}
	else
	{
		// Can't use LOGF, it'll be guided back into here
		LOGE("DarkEdif::MsgBox: Unrecognised button config in faux MessageBoxA\n");
		std::abort();
	}

#if MacBuild == 0
	UIAlertController* alert = [UIAlertController alertControllerWithTitle: [NSString stringWithUTF8String: caption]
		message:[NSString stringWithUTF8String: text]
		preferredStyle:UIAlertControllerStyleAlert];
	UIAlertAction* secondAction = NULL, * thirdAction = NULL;
	UIAlertAction* firstAction = [UIAlertAction actionWithTitle : firstButton
		style : UIAlertActionStyleDefault
		handler : ^ (UIAlertAction * _Nonnull action) { ret = 0; }];
	[alert addAction : firstAction];
	if (secondButton != NULL)
	{
		secondAction = [UIAlertAction actionWithTitle : secondButton
			style : UIAlertActionStyleDefault
			handler : ^ (UIAlertAction * _Nonnull action) {
			ret = 1; }];
		[alert addAction : secondAction];
	}
	if (thirdButton != NULL)
	{
		thirdAction = [UIAlertAction actionWithTitle : thirdButton
			style : UIAlertActionStyleDefault
			handler : ^ (UIAlertAction * _Nonnull action) { ret = 2; }];
		[alert addAction : thirdAction];
	}

	[(id)hwnd presentViewController: alert animated: NO completion: nil];

	// ids index to IDXX
	if (ret != -1)
		ret = ids[ret];
#else // Mac
	NSAlert* alert = [[NSAlert alloc] init];
	[alert setMessageText: [NSString stringWithUTF8String: caption]];
	[alert setInformativeText: [NSString stringWithUTF8String: text]];
	[alert addButtonWithTitle: firstButton];
	if (secondButton != NULL)
		[alert addButtonWithTitle: secondButton];
	if (thirdButton != NULL)
		[alert addButtonWithTitle: thirdButton];

	if ((iconAndButtons & MB_ICONINFORMATION) != 0)
		[alert setAlertStyle: NSAlertStyleInformational];
	else if ((iconAndButtons & MB_ICONERROR) != 0)
		[alert setAlertStyle: NSAlertStyleCritical];
	// else default: warning level

	@try {
		// Convert from NSModalResponse to int
		ret = (int)([alert runModal] - NSAlertFirstButtonReturn);
	}
	@catch (NSException * ex)
	{
		NSString* str =
			@"-[NSAlert runModal] may not be invoked inside of transaction begin/commit "
			@"pair, or inside of transaction commit";
		if ([ex.reason hasPrefix: str])
		{
			LOGE("DarkEdif::MsgBox: Can't do message box sync due to exception; probably inside a drawRect call.\n"
				"Msg Box caption: %s\nMsg Box Text: %s\n"
				"Returning default button, and creating message box async instead.\n", caption, text);
			dispatch_async(dispatch_get_main_queue(), ^{
				[alert runModal];
			});
			ret = 0;
		}
		else // Re-raise
			[ex raise];
	}
	LOGV("modal sheet completed, finishing MessageBoxA()\n");
#endif

	return ret;
}

[[noreturn]]
void DarkEdif::LOGFInternal(PrintFHintInside const TCHAR * x, ...)
{
	char buf[2048];
	va_list va;
	va_start(va, x);
	vsprintf(buf, x, va);
	DarkEdif::MsgBox::Error(_T("Fatal error"), _T("LOGF:\n%s"), buf);
	va_end(va);
	exit(EXIT_FAILURE);
}
#endif


// ============================================================================
//
// DEBUGGER (Interaction with Fusion debugger)
//
// ============================================================================

#if USE_DARKEDIF_FUSION_DEBUGGER

namespace DarkEdif
{
#if RuntimeBuild
	// Collection of no-op dummies. We'll let the user code it in, but since Fusion
	// debugger doesn't exist at edittime, no reaction is possible.

	struct FusionDebuggerAdmin { };
	void FusionDebugger::AddItemToDebugger(
		void (*getLatestFromExt)(Extension *const, std::tstring &),
		bool (*saveUserInputToExt)(Extension *const, std::tstring &),
		size_t, const char *
	) { /* no op in runtime */ }
	void FusionDebugger::AddItemToDebugger(
		int (*)(Extension *const),
		bool (*)(Extension *const, int),
		size_t, const char *) { /* no op in runtime */ }
	void FusionDebugger::UpdateItemInDebugger(
		const char *, int) { /* no op in runtime */ }
	void FusionDebugger::UpdateItemInDebugger(
		const char *, const TCHAR *) { /* no op in runtime */ }
	FusionDebugger::FusionDebugger(Extension *const) { /* runtime debugger not used */ }

#else // It's editor build
	// Prevent the ext dev from messing with internal DarkEdif functions
	struct FusionDebuggerAdmin
	{
		inline std::uint16_t * GetDebugTree(Extension *const ext) {
			return ext->FusionDebugger.GetDebugTree();
		}
		inline void StartEditForItemID(Extension *const ext, int debugItemID) {
			ext->FusionDebugger.StartEditForItemID(debugItemID);
		}
		inline void GetDebugItem(Extension *const ext, TCHAR *writeTo, int debugItemID) {
			ext->FusionDebugger.GetDebugItemFromCacheOrExt(writeTo, debugItemID);
		}
	};

	std::uint16_t *FusionDebugger::GetDebugTree() {
		return debugItemIDs.data();
	}
	void FusionDebugger::StartEditForItemID(int debugItemID)
	{
		if (debugItemID < 0 || (std::uint16_t)debugItems.size() < debugItemID)
			throw std::exception("Couldn't find debug ID in Fusion debugger list.");
		auto &di = debugItems[debugItemID];

		EditDebugInfo edi = {};
		if (di.isInt)
		{
			if (!di.intStoreDataToExt)
				throw std::exception("Item not editable.");
			edi.value = di.cachedInt;
			long ret = ext->Runtime.EditInteger(&edi);
			if (ret == IDOK)
			{
				int oldInteger = di.cachedInt;
				di.cachedInt = edi.value;

				if (!di.intStoreDataToExt(ext, edi.value))
				{
					di.cachedInt = oldInteger;
					di.nextRefreshTime = GetTickCount64();
				}
			}

		}
		else
		{
			if (!di.textStoreDataToExt)
				throw std::exception("Item not editable.");
			edi.text = di.cachedText.data();
			di.cachedText.resize(_tcslen(edi.text));
			edi.lText = di.cachedText.size();
			long ret = ext->Runtime.EditText(&edi);
			if (ret == IDOK)
			{
				std::tstring oldText = di.cachedText;
				di.cachedText = edi.text;
				if (!di.textStoreDataToExt(ext, di.cachedText))
				{
					di.cachedText = oldText;
					di.nextRefreshTime = GetTickCount64();
				}
			}
		}
	}
	void FusionDebugger::GetDebugItemFromCacheOrExt(TCHAR *writeTo, int debugItemID)
	{
		if (debugItemID < 0 || debugItemID >= (std::uint16_t)debugItems.size())
			throw std::exception("Couldn't find debug ID in Fusion debugger list.");

		// Reader function exists, and timer for refreshing (if it exists) has expired
		auto &di = debugItems[debugItemID];
		if ((di.isInt ? di.intReadFromExt != NULL : di.textReadFromExt != NULL) &&
			(!di.refreshMS || GetTickCount64() >= di.nextRefreshTime))
		{
			if (di.isInt)
				di.cachedInt = di.intReadFromExt(ext);
			else
			{
				di.textReadFromExt(ext, di.cachedText);
				if (di.cachedText.size() > 255)
				{
#ifndef _UNICODE
					const std::tstring ellipse("...");
#else
					const std::tstring ellipse(L"…");
#endif
					di.cachedText.resize(255 - ellipse.size());
					di.cachedText += ellipse;
				}
			}
			if (di.refreshMS)
				di.nextRefreshTime = GetTickCount64() + di.refreshMS;
		}
		_tcscpy_s(writeTo, 256U, di.cachedText.c_str());
	}

	void FusionDebugger::AddItemToDebugger(
		// Supply NULL if it will not ever change.
		void (*getLatestFromExt)(Extension *const ext, std::tstring &writeTo),
		// Supply NULL if not editable. In function, return true if cache should be updated, false if edit attempt was not accepted.
		bool (*saveUserInputToExt)(Extension *const ext, std::tstring &newValue),
		// Supply 0 if no caching should be used, otherwise will re-call reader().
		size_t refreshMS,
		// Supply NULL if not removable. Case-sensitive name, used for removing from Fusion debugger if needed.
		const char *userSuppliedName
	) {
		if (debugItems.size() == 127)
			throw std::exception("Too many items added to Fusion debugger.");

		debugItems.push_back(DebugItem(getLatestFromExt, saveUserInputToExt, refreshMS, userSuppliedName));
		// End it with DB_END, and second-to-last item is the new debug item ID
		debugItemIDs.push_back(DB_END);
		debugItemIDs[debugItemIDs.size() - 2] = (((std::uint16_t)debugItems.size()) - 1) | (saveUserInputToExt != NULL ? DB_EDITABLE : 0);
	}

	void FusionDebugger::AddItemToDebugger(
		// Supply NULL if it will not ever change.
		int (*getLatestFromExt)(Extension *const ext),
		// Supply NULL if not editable. In function, return true if cache should be updated, false if edit attempt was not
		bool (*saveUserInputToExt)(Extension *const ext, int newValue),
		// Supply 0 if no caching should be used, otherwise will re-call reader() every time Fusion requests.
		size_t refreshMS,
		// Supply NULL if not removable. Case-sensitive name, used for removing from Fusion debugger if needed.
		const char *userSuppliedName
	) {
		if (debugItems.size() == 127)
			throw std::exception("too many items added to Fusion debugger");

		if (userSuppliedName && std::any_of(debugItems.cbegin(), debugItems.cend(), [=](const DebugItem &d) { return d.DoesUserSuppliedNameMatch(userSuppliedName); }))
			throw std::exception("name already in use. Must be unique");

		debugItems.push_back(DebugItem(getLatestFromExt, saveUserInputToExt, refreshMS, userSuppliedName));
		// End it with DB_END, and second-to-last item is the new debug item ID
		debugItemIDs.push_back(DB_END);
		debugItemIDs[debugItemIDs.size() - 2] = (((std::uint16_t)debugItems.size()) - 1) | (saveUserInputToExt != NULL ? DB_EDITABLE : 0);
	}

	void FusionDebugger::UpdateItemInDebugger(
		const char *userSuppliedName, int newValue
	) {
		for (std::size_t i = 0; i < debugItems.size(); ++i)
		{
			if (debugItems[i].DoesUserSuppliedNameMatch(userSuppliedName))
			{
				if (debugItems[i].isInt)
					debugItems[i].cachedInt = newValue;
				else
					throw std::exception("Fusion debugger item is text, not int type");
				return;
			}
		}
	}

	void FusionDebugger::UpdateItemInDebugger(
		const char *userSuppliedName, const TCHAR *newText
	) {
		if (!newText)
			throw std::exception("null not allowed");

		for (std::size_t i = 0; i < debugItems.size(); ++i)
		{
			if (debugItems[i].DoesUserSuppliedNameMatch(userSuppliedName))
			{
				if (debugItems[i].isInt)
					throw std::runtime_error("Fusion debugger item is text, not int type");
				else
					debugItems[i].cachedText = newText;
				return;
			}
		}
	}

	FusionDebugger::FusionDebugger(Extension *const ext) : ext(ext) {
		// due to DB_EDITABLE flag being 0x80 in db value IDs, or 128, we can't have more than 128 editable properties.
		// DB IDs are 16-bit, so it might be possible to skip all IDs with 0x80's and use IDs 0-127 then 256-383, etc,
		// leaving 0x80 bit untouched, but haven't tested that.
		debugItems.reserve(128);
		debugItemIDs.reserve(129);
		debugItemIDs.push_back(DB_END);
	}
#endif // EditorBuild
}

#if EditorBuild

static DarkEdif::FusionDebuggerAdmin FusionDebugAdmin;

// ============================================================================
//
// DEBUGGER ROUTINES
//
// ============================================================================

// This routine returns the address of the debugger tree
std::uint16_t * FusionAPI GetDebugTree(RUNDATA *rdPtr)
{
#pragma DllExportHint
	return FusionDebugAdmin.GetDebugTree(((RunObject*)rdPtr)->GetExtension());
}

// This routine returns the text of a given item.
void FusionAPI GetDebugItem(TCHAR *pBuffer, RUNDATA *rdPtr, int id)
{
#pragma DllExportHint
	FusionDebugAdmin.GetDebugItem(((RunObject *)rdPtr)->GetExtension(), pBuffer, id);
}

// This routine allows the user to edit the debugger's editable items.
void FusionAPI EditDebugItem(RUNDATA *rdPtr, int id)
{
#pragma DllExportHint
	FusionDebugAdmin.StartEditForItemID(((RunObject*)rdPtr)->GetExtension(), id);
}
#endif // EditorBuild

#endif // USE_DARKEDIF_FUSION_DEBUGGER



// Removes the ending text if it exists, and returns true. If it doesn't exist, changes nothing and returns false.
bool DarkEdif::RemoveSuffixIfExists(std::tstring_view &tstr, const std::tstring_view suffix, bool caseInsensitive /*= true */)
{
	assert(suffix.size() != 0);

	// tstr is smaller than suffix, obviously can't match
	if (tstr.size() < suffix.size())
		return false;
	if (EndsWith(tstr, suffix, caseInsensitive))
	{
		tstr.remove_suffix(suffix.size());
		return true;
	}
	return false;
}

// Checks if first parameter ends with second parameter, returns true if so.
bool DarkEdif::EndsWith(const std::tstring_view tstr, const std::tstring_view suffix, bool caseInsensitive /* = true */)
{
	if (tstr.size() < suffix.size())
		return false;

	if (caseInsensitive)
		return 0 == _tcsnicmp(tstr.data() + tstr.size() - suffix.size(), suffix.data(), suffix.size());
	return 0 == tstr.compare(tstr.size() - suffix.size(), suffix.size(), suffix);
}

bool DarkEdif::FileExists(const std::tstring_view path)
{
#if _WIN32
	const std::tstring pathSafe(path);
	const DWORD fileAttr = GetFileAttributes(pathSafe.c_str());
	return fileAttr != INVALID_FILE_ATTRIBUTES && (fileAttr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY;
#else
	throw std::runtime_error("Function not implemented in non-Windows");
#endif
}


#if _WIN32

static std::tstring appPath, mfxPath, stdrtPath;
static std::tstring GetModulePath(HMODULE hModule)
{
	std::tstring path(MAX_PATH, _T('\0'));
	DWORD numCharsWrittenExcludingNull = GetModuleFileName(hModule, path.data(), path.size());

	// Something broke in our call
	if (numCharsWrittenExcludingNull == 0)
		std::abort();

	// Successfully looked up the path!
	if (numCharsWrittenExcludingNull != MAX_PATH &&
		// Convert the path to long path, check it's not too long or error'd
		(numCharsWrittenExcludingNull = GetLongPathName(path.data(), path.data(), path.size())) < MAX_PATH && numCharsWrittenExcludingNull > 0)
	{
		return path.substr(0, numCharsWrittenExcludingNull);
	}

	// else path is too long.

	// Only Unicode L"\\?\" prefix allows using a bigger path.
	// There's no point calling the wide version and then WideToANSI, because the result would be unusable as it would
	// be bigger than MAX_PATH in any file call, and would potentially have replacement characters if the codepage
	// couldn't represent all of it.
	// WideToUTF8 works, but if you want to support Unicode on Windows, do that in the Unicode builds of your extension,
	// don't try and force it in an ANSI build.
	// The only possible workarounds is using an 8.3 path, and that can be disabled, and also might not be long enough.
	//
	// MAX_PATH can be extended in Windows 10 b1603+, but the app/mfx requires a manifest showing it's big path aware.
	// https://docs.microsoft.com/en-gb/windows/win32/fileio/maximum-file-path-limitation#enable-long-paths-in-windows-10-version-1607-and-later

#ifndef _UNICODE
	DarkEdif::MsgBox::Error(_T("Fatal error - Path too long"),
		_T("Your application path is too long, several Fusion extensions may have difficulty loading.\n"
			"Build with the Unicode " PROJECT_NAME " to remove the limit."));
	throw std::runtime_error("Path reading error"); // Don't bother returning anything to user code.
#else
	// Max limit on path is 32,767 characters... ish. The documentation says it may go further.
	// If we assume surrogate pairs are in play, this limit can nearly double, although the folder names in the path are still capped to MAX_PATH each.
	path.resize(64 * 1024);
	numCharsWrittenExcludingNull = GetModuleFileName(hModule, path.data(), path.size());

	// We have no error handling for when this fails; there's no workaround, so we might as well give up.
	if (numCharsWrittenExcludingNull == 0 || numCharsWrittenExcludingNull == MAX_PATH)
		std::abort();

	return path.substr(0, numCharsWrittenExcludingNull);
#endif
}
std::tstring_view DarkEdif::GetRunningApplicationPath(GetRunningApplicationPathType type)
{
	// Singleton; load paths if we don't have 'em
	if (appPath.empty())
	{
		appPath = GetModulePath(NULL);

		// The user is using "compress the runtime", so we're running inside temp, not app folder
		if (EndsWith(appPath, _T(".tmp\\stdrt.exe")))
		{
			stdrtPath = appPath;

			// Compress the runtime runs stdrt.exe with commandline
			// "%tmp%\mrtXXX.tmp\stdrt.exe" /SF "full path to original.exe" ...
			const std::tstring_view cmdLine = GetCommandLine();

			size_t sfAt = cmdLine.find(_T(" /SF \""));
			if (sfAt == std::tstring_view::npos)
				std::abort(); // Commandline is in unrecognised format
			sfAt += sizeof(" /SF \"") - 1;

			const size_t sfEnd = cmdLine.find(_T('"'), sfAt);
			if (sfEnd == std::tstring_view::npos)
				std::abort(); // Commandline is in unrecognised format

			appPath = cmdLine.substr(sfAt, sfEnd - sfAt);
		}
	}

	std::tstring_view path = appPath;
	if (!stdrtPath.empty() && (type & GetRunningApplicationPathType::GetSTDRTNotApp) == GetRunningApplicationPathType::GetSTDRTNotApp)
		path = stdrtPath;

	if ((type & GetRunningApplicationPathType::AppFolderOnly) == GetRunningApplicationPathType::AppFolderOnly)
	{
		size_t lastSlash = std::tstring::npos;
		if ((lastSlash = path.rfind(_T('\\'))) == std::tstring::npos)
			lastSlash = path.rfind(_T('/'));
		if (lastSlash == std::tstring::npos)
			std::abort();
		return path.substr(0, lastSlash);
	}

	return path;
}
//
std::tstring_view DarkEdif::GetMFXRelativeFolder(GetFusionFolderType type)
{
	if (mfxPath.empty())
		mfxPath = GetModulePath(hInstLib);

	// Start by getting the MFX parent folder
	std::tstring_view curPath(mfxPath.c_str(), mfxPath.rfind(_T('\\')) + 1);

	// If in Extensions\Unicode, hop out to Extensions level; likewise for Data\Runtime\Unicode
	if (type != GetFusionFolderType::MFXLocation && !_tcsnicmp(curPath.data() + curPath.size() - (sizeof("Unicode\\") - 1), _T("Unicode\\"), sizeof("Unicode\\") - 1))
		curPath.remove_suffix(sizeof("Unicode\\") - 1);

	// Go straight to root
	if (type == GetFusionFolderType::FusionRoot)
	{
		if (!_tcsnicmp(curPath.data() + curPath.size() - (sizeof("Extensions\\") - 1), _T("Extensions\\"), sizeof("Extensions\\") - 1))
			curPath.remove_suffix(sizeof("Extensions\\") - 1);
		else if (!_tcsnicmp(curPath.data() + curPath.size() - (sizeof("Data\\Runtime\\") - 1), _T("Data\\Runtime\\"), sizeof("Data\\Runtime\\") - 1))
			curPath.remove_suffix(sizeof("Data\\Runtime") - 1);
		//else // we're not in Extensions or Data\Runtime... where are we?
		//	DarkEdif::MsgBox::Error(_T("Folder location failure"), _T("Couldn't calculate the Fusion root folder from \"%s\"; can't look for settings!"), FileToLookup);
	}

	return curPath;
}
#else
std::tstring_view DarkEdif::GetRunningApplicationPath(DarkEdif::GetRunningApplicationPathType type)
{
	throw std::runtime_error("GetRunningApplicationPath function not implemented on non-Windows.");
}
std::tstring_view DarkEdif::GetMFXRelativeFolder(GetFusionFolderType type)
{
	throw std::runtime_error("GetMFXRelativeFolder function not implemented on non-Windows.");
}
#endif // _WIN32

#if EditorBuild

// =====
// Get DarkEdif INIs and lines
// =====

static std::string sdkSettingsFileContent;
static std::atomic<bool> fileLock;
static bool fileOpened;

std::string DarkEdif::GetIniSetting(const std::string_view key)
{
	assert(DarkEdif::RunMode != DarkEdif::MFXRunMode::RunApplication && DarkEdif::RunMode != DarkEdif::MFXRunMode::BuiltEXE);

	// File locked already; wait
	while (fileLock.exchange(true))
		/* wait */;

	if (!fileOpened)
	{
		fileOpened = true; // Prevent repeatedly loading it into memory

		std::tstring iniPath(GetMFXRelativeFolder(GetFusionFolderType::FusionExtensions));
		iniPath += _T("DarkEdif.ini"sv);

		// Is the file in the directory of the MFX? (should be, languages are only needed in edittime)
		if (!FileExists(iniPath))
		{
			// Ignore DarkEdif.ini being non-existent; all other errors we report
			if (GetLastError() != ERROR_FILE_NOT_FOUND)
				DarkEdif::MsgBox::Error(_T("DarkEdif SDK error"), _T("Error %u opening Extensions\\DarkEdif.ini."), GetLastError());

			fileLock = false;
			return std::string();
		}

		// Open DarkEdif.ini settings file in read binary, and deny other apps writing permissions.
		FILE * fileHandle = _tfsopen(iniPath.c_str(), _T("rb"), _SH_DENYWR);

		// Could not open; abort (should report error)
		if (!fileHandle)
		{
			DarkEdif::MsgBox::Error(_T("DarkEdif SDK error"), _T("Couldn't open Extensions\\DarkEdif.ini, error %d."), errno);
			fileLock = false;
			return std::string();
		}

		fseek(fileHandle, 0, SEEK_END);
		long fileSize = ftell(fileHandle);
		fseek(fileHandle, 0, SEEK_SET);

		sdkSettingsFileContent.resize(fileSize + 2);
		// Could not read all of the file properly
		if (fileSize != fread_s(&sdkSettingsFileContent.data()[1], sdkSettingsFileContent.size() - 2, 1, fileSize, fileHandle))
		{
			DarkEdif::MsgBox::Error(_T("DarkEdif SDK error"), _T("Couldn't read Extensions\\DarkEdif.ini file, error %i."), errno);
			fclose(fileHandle);
			sdkSettingsFileContent.clear();
			fileLock = false;
			return std::string();
		}
		// Start and end with newline for easy line detection
		sdkSettingsFileContent.back() = sdkSettingsFileContent.front() = '\n';

		// Destroy any spaces to left or right of "=" - will only handle one space on the sides
		for (size_t s = sdkSettingsFileContent.find('='); s != std::string::npos; s = sdkSettingsFileContent.find('=', s))
		{
			if (sdkSettingsFileContent[s - 1] == ' ')
				sdkSettingsFileContent.erase(--s, 1);
			if (sdkSettingsFileContent[s + 1] == ' ')
				sdkSettingsFileContent.erase(++s, 1);
			if (++s > sdkSettingsFileContent.size())
			{
				DarkEdif::MsgBox::Error(_T("DarkEdif SDK error"), _T("SDK settings file is incorrectly formatted (ends with '=')."));
				break;
			}
		}

		fclose(fileHandle);
		// fall thru into unlock
	}
	// else other thread has finished loading the file, so we don't need to lock it
	fileLock = false;

	// Look for key (spaces around '=' are stripped in file open)
	std::string keyFind1;
	keyFind1 = '\n';
	keyFind1 += key;
	keyFind1 += '=';

	size_t Reading;
	if (sdkSettingsFileContent.find(keyFind1) != std::string::npos)
		Reading = sdkSettingsFileContent.find(keyFind1) + keyFind1.size();
	else // key not found in settings file
		return std::string();

	size_t lineEnd = sdkSettingsFileContent.find_first_of("\r\n", Reading);
	// Line hits end of file
	if (lineEnd == std::string::npos)
		return sdkSettingsFileContent.substr(Reading);

	return sdkSettingsFileContent.substr(Reading, lineEnd - Reading);
}

// =====
// Fusion SDK updater
// =====

#if USE_DARKEDIF_UPDATE_CHECKER

#include <WinSock2.h> // for network sockets
#include <shellapi.h> // for ShellExecuteW, opening the new ext version URL in preferred browser

static std::atomic_bool updateLock(false);
HANDLE updateThread;
static std::stringstream updateLog = std::stringstream();
static DarkEdif::SDKUpdater::ExtUpdateType pendingUpdateType;
static std::wstring pendingUpdateURL = std::wstring();
static std::wstring pendingUpdateDetails = std::wstring();

DWORD WINAPI DarkEdifUpdateThread(void * data);

void DarkEdif::SDKUpdater::StartUpdateCheck()
{
	// Can run in background, but not during startup screen. During the startup screen process, the MFX is loaded, read from, unloaded.
	// The update checker thread will try to write back to variables that have been unloaded from memory, crashing.
	// In Run Application/Built EXEs, it's useless, and shouldn't be running.
	if (DarkEdif::RunMode != DarkEdif::MFXRunMode::Editor)
		return DarkEdif::MsgBox::Error(_T("Critial error"), _T("The update checker is running during the wrong mode."));

	// Shouldn't run twice.
	if (updateThread != NULL)
		throw std::runtime_error("Using multiple update threads");

	updateThread = CreateThread(NULL, NULL, DarkEdifUpdateThread, NULL, 0, NULL);
	if (updateThread == NULL)
	{
		DarkEdif::MsgBox::Error(_T("Critial error"), _T("The update checker failed to start, error %u."), GetLastError());
		DarkEdifUpdateThread(Edif::SDK);
	}
}

DarkEdif::SDKUpdater::ExtUpdateType DarkEdif::SDKUpdater::ReadUpdateStatus(std::string * logData)
{
	// Lock for safety
	while (updateLock.exchange(true))
		/* when holder releases, exchange() will return false */;

	if (logData)
		*logData = updateLog.str();
	ExtUpdateType extUpdateType = pendingUpdateType;

	updateLock = false; // unlock
	return extUpdateType;
}

static bool handledUpdate;
void DarkEdif::SDKUpdater::RunUpdateNotifs(mv * mV, EDITDATA * edPtr)
{
	if (handledUpdate)
		return;

	// Lock for safety
	while (updateLock.exchange(true))
		/* when holder releases, exchange() will return false */;

	ExtUpdateType extUpdateType = pendingUpdateType;
	if (extUpdateType == ExtUpdateType::CheckInProgress) {
		updateLock = false; // unlock
		return;
	}
	// Any other status indicates update thread isn't running, so no reason to watch the lock.
	updateLock = false; // unlock

	// Prevent this function running again
	handledUpdate = true;

	// Clean up the thread object
	CloseHandle(updateThread);
	updateThread = NULL;

	// Connection errors are relevant to all users
	if (extUpdateType == ExtUpdateType::ConnectionError) {
		DarkEdif::MsgBox::Error(_T("Update check error"), _T("Error occurred while checking for extension updates:\n%hs"), updateLog.str().c_str());
		return;
	}

	// Ext dev errors can only be fixed by ext developer.
	if (extUpdateType == ExtUpdateType::ExtDevError) {
		DarkEdif::MsgBox::Error(_T("Update check error"), _T("Extension developer error occurred while checking for extension updates:\n%ls"), pendingUpdateDetails.c_str());
		return;
	}

	// SDK updates can only be done by ext developer.
	if (extUpdateType == ExtUpdateType::SDKUpdate) {
		DarkEdif::MsgBox::WarningOK(_T("SDK update notice"), _T("SDK update for " PROJECT_NAME ":\n%ls"), pendingUpdateDetails.c_str());
		return;
	}

	if (extUpdateType != ExtUpdateType::Major && extUpdateType != ExtUpdateType::Minor && extUpdateType != ExtUpdateType::ReinstallNeeded)
		return;

	// Lots of magic numbers created by a lot of trial and error. Do not recommend.
	if (Edif::SDK->Icon->GetWidth() != 32)
		return DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("" PROJECT_NAME "'s icon width is not 32. Contact the extension developer."));

	// If font creation fails, it's not that important; a null HFONT is replaced with a system default.
	HFONT font = CreateFontA(
		8 /* Height: 8px */,
		0 /* Width: 0; use closest match */,
		0 /* Escapement: Rotate 0 degrees */,
		0 /* Orientation:  Rotate 0 degrees */,
		FW_NORMAL /* Weight: Normal */,
		FALSE /* Italic */, FALSE /* Underline */, FALSE /* Strikeout */,
		ANSI_CHARSET /* Charset */,
		OUT_DEFAULT_PRECIS /* OUT_RASTER_PRECIS /* Out precision: pick raster font */, CLIP_DEFAULT_PRECIS /* Clip precision: default */,
		ANTIALIASED_QUALITY  /* Quality: Antialiased */,
		FF_MODERN /* Pitch and family: use fonts with constant stroke width */,
		"Small Fonts" /* Font face name */);

	auto FillBackground = [](const RECT rect, COLORREF fillColor) {
		// This is the grey background rectangle, which we'll need to both de-alpha and colour.
		if (Edif::SDK->Icon->HasAlpha())
		{
			LPBYTE alpha = Edif::SDK->Icon->LockAlpha();
			if (alpha != nullptr)
			{
				for (int i = rect.top; i < rect.bottom; ++i)
					memset(&alpha[rect.left + (i * 32)], 0xFF, rect.right - rect.left);
				Edif::SDK->Icon->UnlockAlpha();
			}
		}

		Edif::SDK->Icon->Fill(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, fillColor);
	};

	if (extUpdateType == ExtUpdateType::Major)
	{
		const RECT greyBkgdRect { 1, 8, 30, 27 };
		FillBackground(greyBkgdRect, RGB(80, 80, 80));

		// For some reason there are margins added in by the font drawing technique;
		// we have to counter it by positioning each line manually.
		RECT textDrawRect = { greyBkgdRect.left + 1, greyBkgdRect.top - 1, 32, 32 };
		COLORREF textColor = RGB(240, 0, 0);
		Edif::SDK->Icon->DrawTextA("MAJOR", sizeof("MAJOR") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left -= 1;
		textDrawRect.top += 6;
		Edif::SDK->Icon->DrawTextA("UPDATE", sizeof("UPDATE") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left += 1;
		textDrawRect.top += 6;
		Edif::SDK->Icon->DrawTextA("NEEDED", sizeof("NEEDED") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		// It's possible to do this so the icon in the sidebar is updated too, but that causes Fusion to register that the properties have changed,
		// and causes Fusion to save the "update needed" icon into the MFA, which is no good as it'll never be restored to normal.
		// We'll attempt to avoid that, although copying the frame with an altered icon causes it to have the altered icon in sidebar too,
		// making our avoidance only an attempt.
		// mvInvalidateObject(mV, edPtr);

		if (DarkEdif::GetIniSetting("MsgBoxForMajorUpdate"sv) != "false")
		{
			// No URL? Open a dialog to report it.
			if (pendingUpdateURL.empty())
				MsgBox::Info(_T("Update notice"), _T("Major update for " PROJECT_NAME ":\n%ls"), pendingUpdateDetails.c_str());
			else // URL? Request to open it. Let user say no.
			{
				int ret = MessageBoxW(NULL, (L"Major update for " PROJECT_NAME ":\n" + pendingUpdateDetails).c_str(), L"" PROJECT_NAME " update notice", MB_ICONINFORMATION | MB_YESNO | MB_DEFBUTTON2);
				if (ret == IDYES)
					ShellExecuteW(NULL, L"open", pendingUpdateURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}
	else if (extUpdateType == ExtUpdateType::Minor)
	{
		const RECT greyBkgdRect{ 1, 18, 31, 31 };
		FillBackground(greyBkgdRect, RGB(60, 60, 60));

		// For some reason there are margins added in by the font drawing technique;
		// we have to counter it by positioning each line manually.
		RECT textDrawRect = { greyBkgdRect.left, greyBkgdRect.top - 1, 32, 32 };
		COLORREF textColor = RGB(0, 180, 180);

		Edif::SDK->Icon->DrawTextA("UPDATE", sizeof("UPDATE") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left += 1;
		textDrawRect.top += 6;
		Edif::SDK->Icon->DrawTextA("NEEDED", sizeof("NEEDED") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		if (DarkEdif::GetIniSetting("MsgBoxForMinorUpdate"sv) == "true")
		{
			// No URL? Open a dialog to report it.
			if (pendingUpdateURL.empty())
				MsgBox::Info(_T("update notice"), _T("Minor update for " PROJECT_NAME ":\n%ls"), pendingUpdateDetails.c_str());
			else // URL? Request to open it. Let user say no.
			{
				int ret = MessageBoxW(NULL, (L"Minor update for " PROJECT_NAME ":\n" + pendingUpdateDetails).c_str(), L"" PROJECT_NAME " update notice ", MB_ICONINFORMATION | MB_YESNO | MB_DEFBUTTON2);
				if (ret == IDYES)
					ShellExecuteW(NULL, L"open", pendingUpdateURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}
	else if (extUpdateType == ExtUpdateType::ReinstallNeeded)
	{
		const RECT greyBkgdRect{ 1, 18, 31, 31 };
		FillBackground(greyBkgdRect, RGB(60, 60, 60));

		// For some reason there are margins added in by the font drawing technique;
		// we have to counter it by positioning each line manually.
		RECT textDrawRect = { greyBkgdRect.left, greyBkgdRect.top - 1, 32, 32 };
		COLORREF textColor = RGB(180, 0, 0);

		Edif::SDK->Icon->DrawTextA("REINSTL", sizeof("REINSTL") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left += 1;
		textDrawRect.top += 6;
		Edif::SDK->Icon->DrawTextA("NEEDED", sizeof("NEEDED") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		if (pendingUpdateDetails.size() > 3)
		{
			// No URL? Open a dialog to report it.
			if (pendingUpdateURL.empty())
				MsgBox::Info(_T("Reinstall needed"), _T("Reinstalled needed for " PROJECT_NAME ":\n%ls"), pendingUpdateDetails.c_str());
			else // URL? Request to open it. Let user say no, but default to yes.
			{
				int ret = MessageBoxW(NULL, (L"Reinstall needed for " PROJECT_NAME ":\n" + pendingUpdateDetails).c_str(), L"" PROJECT_NAME " error", MB_ICONERROR | MB_YESNO);
				if (ret == IDYES)
					ShellExecuteW(NULL, L"open", pendingUpdateURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}

	if (font)
		DeleteObject(font);
}

#pragma comment(lib,"ws2_32.lib")
#include <iomanip>
#include <functional>

std::string url_encode(const std::string & value) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		std::string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << int((unsigned char)c);
		escaped << std::nouppercase;
	}

	return escaped.str();
}

DWORD WINAPI DarkEdifUpdateThread(void *)
{
	// In order to detect it regardless of whether it as the start or end of the list,
	// we make sure the line content is wrapped in semicolons
	std::string ini = ";" + DarkEdif::GetIniSetting("DisableUpdateCheckFor"sv) + ";";

	// Remove spaces around the ';'s. We can't just remove all spaces, as some ext names have them.
	size_t semiSpace = 0;
	while ((semiSpace = ini.find("; "sv, semiSpace)) != std::string::npos)
		ini = ini.replace(semiSpace--, 2, ";"sv);

	semiSpace = 0;
	while ((semiSpace = ini.find(" ;"sv, semiSpace)) != std::string::npos)
		ini = ini.replace(semiSpace--, 2, ";"sv);

	// Acquire the rudimentary lock, do op, and release
#define GetLockAnd(x) while (updateLock.exchange(true)) /* retry */; x; updateLock = false
#define GetLockSetConnectErrorAnd(x) while (updateLock.exchange(true)) /* retry */; x; pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ConnectionError; updateLock = false

	// If the ext name is found, or the wildcard *
	bool updateCheckDisablingWasIgnored = false;
	if (ini.find(";" PROJECT_NAME ";"sv) != std::string::npos || ini.find(";*;"sv) != std::string::npos)
	{
#if USE_DARKEDIF_UC_TAGGING
		GetLockAnd(updateLog << "Update check was disabled, but UC tagging is on."sv);
		updateCheckDisablingWasIgnored = true;
#else
		GetLockAnd(updateLog << "Update check was disabled."sv;
		pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::CheckDisabled);
		return 0;
#endif
	}

	std::string projConfig = STRIFY(CONFIG);
	while ((semiSpace = projConfig.find(' ')) != std::string::npos)
		projConfig.replace(semiSpace, 1, "%20"sv);

	// Opt-in feature for tagging built EXEs with a unique key.
	// This key is unique per Fusion install, but is otherwise meaningless; it's not a hash of anything.

#if USE_DARKEDIF_UC_TAGGING
	// Due to code in the update checker caller, this MFX can be safely assumed to be in Extensions or Extensions\Unicode
	assert(DarkEdif::RunMode == DarkEdif::MFXRunMode::Editor);

	// Find Data\Runtime MFX that will be used in built apps.
	// If ANSI runtime, it will only use ANSI MFX.
	// If Unicode runtime, it will always prefer a Data\Runtime\Unicode MFX, even when the MFA uses an ANSI Extensions MFX.
	// So in the case of a Unicode runtime, always look for Unicode MFX.

	std::tstring drMFXPath(DarkEdif::GetMFXRelativeFolder(DarkEdif::GetFusionFolderType::FusionRoot));
	drMFXPath += _T("Data\\Runtime\\"sv);
	if (mvIsUnicodeVersion(Edif::SDK->mV))
	{
		std::tstring uniPath = drMFXPath;
		uniPath += _T("Unicode\\") PROJECT_TARGET_NAME ".mfx"sv;

		if (!DarkEdif::FileExists(uniPath))
		{
			drMFXPath += _T("" PROJECT_TARGET_NAME ".mfx"sv);

			// Couldn't find either; roll back to Uni for error messages
			if (!DarkEdif::FileExists(drMFXPath))
				drMFXPath = uniPath;

			// else roll with ANSI
		}
		else // else roll with Uni
			drMFXPath = uniPath;
	}
	else // ANSI runtime will only use ANSI MFX
		drMFXPath += _T("" PROJECT_TARGET_NAME ".mfx"sv);

	// Stores UC tags in resources or registry.
	std::wstring resKey, regKey;
	bool resFileExists = true;
	const std::wstring_view UC_TAG_NEW_SETUP = L"New setup."sv;

	// If used, indicates the resource file didn't have it
	HKEY mainKey = NULL;

	// Use RAII to auto-close the opened registry key, when this update thread exits
	auto CloseRegHandle = [](HKEY *h) { if (*h != NULL) { RegCloseKey(*h); *h = NULL; } return; };
	std::unique_ptr<HKEY, std::function<void(HKEY*)>> registryHandleHolder(&mainKey, CloseRegHandle);

	// if Data\Runtime(\Unicode) MFX is missing.
	HMODULE readHandle = LoadLibraryEx(drMFXPath.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (readHandle == NULL)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			resKey = L"Couldn't load MFX; not found"sv;
			resFileExists = false;
		}
		else if (GetLastError() == ERROR_ACCESS_DENIED)
		{
			DarkEdif::MsgBox::Error(_T("Resource loading"), _T("Failed to set up ") PROJECT_NAME _T(" - access denied writing to Data\\Runtime MFX. Try running the UCT Fix Tool, or run Fusion as admin.\n\nUCT fix tool:\nhttps://dark-wire.com/storage/UCT%%20Fix%%20Tool.exe"));
			std::abort();
		}
		else // Some other error loading; we'll consider it fatal.
		{
			DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while reading Data\\Runtime MFX.\n%s"), GetLastError(), drMFXPath.c_str());
			std::abort();
		}
	}
	else
	{
		// Read key back from Data\\Runtime MFX
		HRSRC hsrcForRes = FindResourceEx(readHandle, RT_STRING, _T("UCTAG"), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
		if (hsrcForRes == NULL)
			resKey = UC_TAG_NEW_SETUP;
		else
		{
			// Not actually a HGLOBAL! If you use GlobalXXX on it, you'll get an invalid handle error.
			// Use the Resource-related functions like LockResource instead.
			HGLOBAL data2 = LoadResource(readHandle, hsrcForRes);
			if (data2 == NULL)
			{
				DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while loading resource for reading."), GetLastError());
				FreeLibrary(readHandle);
				std::abort();
			}
			wchar_t * dataSrc = (wchar_t *)LockResource(data2);
			if (dataSrc == NULL)
			{
				DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while locking resource for reading."), GetLastError());
				FreeResource(data2);
				FreeLibrary(readHandle);
				std::abort();
			}

			unsigned short strLength = dataSrc[0];
			// We're setting to a view, but passing it to a copying std::wstring
			resKey = std::wstring_view(&dataSrc[1], strLength);
			UnlockResource(data2);
			FreeResource(data2);
		}

		FreeLibrary(readHandle);
	}

	// Attempt to open for writing
	HANDLE resHandle = NULL;
	if (resFileExists)
	{
		resHandle = BeginUpdateResource(drMFXPath.c_str(), FALSE);
		if (resHandle == NULL)
		{
			DWORD err = GetLastError();
			DarkEdif::MsgBox::Error(_T("Resource missing"), _T("UC tagging temporary failure %u. Try running your Fusion as admin."), err);
			wchar_t keyError[64];
			swprintf_s(keyError, std::size(keyError), L"Error %u tagging.", err);
			resKey = keyError;
		}
	}

	// Use RAII to auto-close the opened resource writing when this update thread exits
	auto CloseHandle = [](HANDLE *h) { EndUpdateResource(*h, TRUE); *h = NULL; return; };
	std::unique_ptr<HANDLE, std::function<void(HANDLE*)>> resHandleHolder(&resHandle, CloseHandle);

	// Users have access to registry of local machine, albeit read-only, so this should not fail.
	// Note: we always use the wide version of registry as it's always stored as UTF-16, and we
	// don't want half-baked ANSI conversions.
	DWORD ret;
	if ((ret = RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Clickteam", &mainKey)) != ERROR_SUCCESS)
	{
		DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while loading registration resource for reading."), ret);
		FreeLibrary(readHandle);
		std::abort();
	}

	// Users have access to registry of local machine, albeit read-only, so this should not fail.
	regKey.resize(256);
	DWORD keySize = regKey.size() * sizeof(wchar_t), type;
	ret = RegQueryValueExW(mainKey, L"UCTag", NULL, &type, (LPBYTE)regKey.data(), &keySize);
	if (ret == ERROR_SUCCESS)
	{
		regKey.resize(keySize / sizeof(wchar_t), L'?');
		// Null terminator is usually included in the value, remove it if so
		if (regKey[regKey.size() - 1] == L'\0')
			regKey.resize(regKey.size() - 1);
	}
	else if (ret != ERROR_FILE_NOT_FOUND)
	{
		DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while loading registration resource for reading."), GetLastError());
		RegCloseKey(mainKey);
		FreeLibrary(readHandle);
		std::abort();
	}
	else // reg key is missing
		regKey = UC_TAG_NEW_SETUP;

	// If UC tag is used, the ext MUST connect if there's any issues with the tagging,
	// even if update check was disabled
	if (updateCheckDisablingWasIgnored && regKey != UC_TAG_NEW_SETUP && resKey == regKey)
	{
		GetLockAnd(updateLog << "Update check was disabled, UC tagging already done, closing with disabled."sv;
		pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::CheckDisabled);
		return 0;
	}

#else // !USE_DARKEDIF_UC_TAGGING
	std::wstring resKey = L"disabled"s, regKey = L"disabled"s;
#endif

	try {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			GetLockSetConnectErrorAnd(
				updateLog << "WSAStartup failed.\n"sv);
			return 1;
		}
		SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (Socket == INVALID_SOCKET)
		{
			GetLockSetConnectErrorAnd(
				updateLog << "socket() failed. Error "sv << WSAGetLastError() << ".\n"sv);
			WSACleanup();
			return 1;
		}

		// If this returns true, updateLock is still held.
		const auto handleWSAError = [&](int lastWSAError) {
			// Get lock; caller will release it at end of update check
			while (updateLock.exchange(true))
				;

			// Overloading with details is unnecessary - empty the updateLog
			updateLog.str(std::string());
			updateLog.clear();

			bool reportError = true;
			// This machine doesn't have internet, so DNS lookup of nossl.dark-wire.com failed
			// Only occurs in gethostbyname()
			if (lastWSAError == WSANO_DATA) {
				reportError = false;
				updateLog << "The DNS provider couldn't find the update server; is your computer offline?\n"sv;
			}
			// Server machine is online, but refusing connection - e.g. http service is offline
			else if (lastWSAError == WSAECONNREFUSED) {
				reportError = false;
				updateLog << "The Darkwire update server is online, but not servicing HTTP requests.\n"sv;
			}
			// Server machine is offline
			else if (lastWSAError == WSAETIMEDOUT) {
				reportError = false;
				updateLog << "The Darkwire update server is offline.\n"sv;
			}
			// lastWSA is only set to this if the page content has an error
			else if (lastWSAError == EINVAL) {
				reportError = false;
				updateLog << "Page's response was unreadable.\n"sv;
			}

			// If it's not one of those errors, or user has set to report all errors, we report.
			reportError |= DarkEdif::GetIniSetting("ReportAllUpdateCheckErrors"sv) == "true"sv;

			// We also have to report if we can't read the resTag on a UC tagged file, or there's inconsistency.
			// We NEED internet for resolving that issue.
			#if USE_DARKEDIF_UC_TAGGING
				if (resFileExists && (resKey == UC_TAG_NEW_SETUP || regKey != resKey))
				{
					reportError = true;

					// Page content errors are none of their concern
					if (lastWSAError != EINVAL)
						updateLog << "This extension needs internet access for its initial setup.\n"sv;
				}
			#endif

			if (!reportError)
			{
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::None;
				updateLog << "Not reporting, as it's an error the user is unlikely to fix.\n"sv;
				updateLock = false;
				return false;
			}

			pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ConnectionError;
			// Let caller fill updateLog with the error information
			return true;
		};

		// Used in IP lookup and the HTTP request content
		const char domain[] = "nossl.dark-wire.com";

		// Suppress warning C4996 about using getaddrinfo instead; we don't need a newer function
		// for Darkwire's simple DNS, and it'll be a lot more code for no obvious benefit.
#pragma warning (push)
#pragma warning (disable: 4996)
		struct hostent * host = gethostbyname(domain);
#pragma warning (pop)

		if (host == NULL)
		{
			if (handleWSAError(WSAGetLastError()))
			{
				updateLog << "DNS lookup of \""sv << domain << "\" failed, error "sv << WSAGetLastError() << '.';
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ConnectionError;
				updateLock = false;
			}

			closesocket(Socket);
			WSACleanup();
			return 1;
		}
		SOCKADDR_IN SockAddr = {};
		SockAddr.sin_port = htons(80);
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_addr.s_addr = *((unsigned long *)host->h_addr);
		GetLockAnd(
			updateLog << "Connecting...\n"sv);
		if (connect(Socket, (SOCKADDR *)(&SockAddr), sizeof(SockAddr)) != 0) {
			if (handleWSAError(WSAGetLastError()))
			{
				updateLog << "Connect failed, error number "sv << WSAGetLastError() << '.';
				updateLock = false;
			}
			closesocket(Socket);
			WSACleanup();
			return 1;
		}
		GetLockAnd(
			updateLog << "Connected to update server.\n"sv);
		// Host necessary so servers serving multiple domains know what domain is requested.
		// Connection: close indicates server should close connection after transfer.
		std::stringstream requestStream;
		requestStream << "GET /storage/darkedif_vercheck.php?ext="sv << url_encode(PROJECT_NAME)
			<< "&build="sv << Extension::Version << "&sdkBuild="sv << DarkEdif::SDKVersion
			<< "&projConfig="sv << projConfig
			<< "&tagRes="sv << url_encode(WideToUTF8(resKey)) << "&tagReg="sv << url_encode(WideToUTF8(regKey))
			<< " HTTP/1.1\r\nHost: "sv << domain << "\r\nConnection: close\r\n\r\n"sv;
		std::string request = requestStream.str();

		GetLockAnd(
			updateLog << "Sent update request for ext \"" PROJECT_NAME "\", encoded as \""sv << url_encode(PROJECT_NAME)
				<< "\", build "sv << Extension::Version << ", SDK build "sv << DarkEdif::SDKVersion << ", config "sv << projConfig
				<< ", tagging res \""sv << url_encode(WideToUTF8(resKey)) << "\", reg \""sv << url_encode(WideToUTF8(regKey)) << "\".\n"sv);
#ifdef _DEBUG
		GetLockAnd(
			updateLog << request.substr(0, request.find(' ', 4)) << '\n');
#endif

		if (send(Socket, request.c_str(), request.size() + 1, 0) == SOCKET_ERROR)
		{
			GetLockSetConnectErrorAnd(
				updateLog << "Send failed, error "sv << WSAGetLastError() << "."sv);
			closesocket(Socket);
			WSACleanup();
			return 1;
		}

		std::string fullPage;
		{
			// Used as buffer
			std::string pagePart(1024, '\0');
			std::stringstream page;
			int nDataLength;

			GetLockAnd(
				updateLog << "Result follows:\n"sv);
			while ((nDataLength = recv(Socket, pagePart.data(), pagePart.size(), 0)) > 0) {
				page << std::string_view(pagePart).substr(0, nDataLength);
				memset(pagePart.data(), 0, nDataLength);
			}
			if (nDataLength < 0)
			{
				if (handleWSAError(WSAGetLastError()))
				{
					updateLog << "Error with recv()."sv;
					updateLock = false;
				}
				closesocket(Socket);
				WSACleanup();
				return 1;
			}
			#ifdef _DEBUG
				GetLockAnd(
					updateLog << page.str() << "\nResult concluded.\n"sv;
					OutputDebugStringA(updateLog.str().c_str());
				);
			#endif
			closesocket(Socket);
			WSACleanup();

			// the c_str() ensures no null or beyond in string, by using a different constructor
			fullPage = page.str().c_str();
		}

		// Socket was closed without any content or error - can happen if Darkwire is down from power cut
		// We only allow ignoring errors of server being down and just report no update
		if (fullPage.empty())
		{
			if (handleWSAError(WSAETIMEDOUT))
			{
				updateLog << "Page result was empty."sv;
				updateLock = false;
			}
			return 1;
		}

		// In case there's an automatic error page with CRLF, we'll check for CRs after.
		size_t endIndex = fullPage.find_first_of("\r\n");

		if (endIndex == std::string::npos)
		{
			GetLockSetConnectErrorAnd(
				updateLog << "End of first line not found. Full raw (non-http) response:\n"sv << fullPage);
			return 1;
		}

		const char expHttpHeader[] = "HTTP/1.1", expHttpOKHeader[] = "HTTP/1.1 200";
		std::string statusLine = endIndex == std::string::npos ? fullPage : fullPage.substr(0, endIndex);
		// Not a HTTP response
		if (endIndex == std::string::npos || strncmp(statusLine.c_str(), expHttpHeader, sizeof(expHttpHeader) - 1))
		{
			GetLockSetConnectErrorAnd(
				updateLog << "Unexpected non-http response:\n"sv << statusLine);
			return 1;
		}

		// HTTP response, but it's not an OK
		if (strncmp(statusLine.c_str(), expHttpOKHeader, sizeof(expHttpOKHeader) - 1))
		{
			GetLockSetConnectErrorAnd(
				updateLog << "HTTP error "sv << statusLine.substr(sizeof(expHttpHeader) - 1));
			return 1;
		}

		// HTTP response has header, two CRLF, then result
		size_t headerStart = fullPage.find("\r\n\r\n");
		if ((headerStart = fullPage.find("\r\n\r\n")) == std::string::npos)
		{
			GetLockSetConnectErrorAnd(
				updateLog << "Malformed HTTP response; end of HTTP header not found."sv);
			return 1;
		}

		// result should be LF line breaks only, ideally in UTF-8.
		std::string pageBody = fullPage.substr(headerStart + 4);
		if (pageBody.find('\r') != std::string::npos)
		{
			GetLockSetConnectErrorAnd(
				updateLog << "CR not permitted in update page response.\n"sv << pageBody);
			return 1;
		}

		if (!_strnicmp(pageBody.c_str(), "https://", sizeof("https://") - 1) || !_strnicmp(pageBody.c_str(), "http://", sizeof("http://") - 1))
		{
			pendingUpdateURL = DarkEdif::UTF8ToWide(pageBody.substr(0, pageBody.find('\n')));
			pageBody = pageBody.substr(pendingUpdateURL.size() + 1);
		}

		GetLockAnd(
			updateLog << "Completed OK. Response:\n"sv << pageBody;
		);

		const char noUpdate[] = "None";
		const char sdkUpdate[] = "SDK Update:\n";
		const char majorUpdate[] = "Major Update:\n";
		const char minorUpdate[] = "Minor Update:\n";

		// introduced in DarkEdif SDK v4
		const char extDevUpdate[] = "Ext Dev Error:\n";
		// introduced in DarkEdif SDK v12
		const char noUpdateWithTag[] = "None:\nTag=";
		const char reinstallNeeded[] = "Reinstall Needed:\n";

		if (!_strnicmp(pageBody.c_str(), reinstallNeeded, sizeof(reinstallNeeded) - 1))
		{
			pageBody = pageBody.substr(sizeof(reinstallNeeded) - 1);
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ReinstallNeeded;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody));
			return 0;
		}

		if (!_strnicmp(pageBody.c_str(), noUpdate, sizeof(noUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::None;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody));

			#if USE_DARKEDIF_UC_TAGGING
			// Pure "None" response, no tag
			if (!_stricmp(pageBody.c_str(), noUpdate))
				return 0; // Tagging disabled

			// None, but not recognised as plain None or None with tag.
			if (_strnicmp(pageBody.c_str(), noUpdateWithTag, sizeof(noUpdateWithTag) - 1))
				return DarkEdif::MsgBox::Error(_T("Update checker failure"), _T("Update checker failed to return a valid response.")), 0;

			const std::wstring providedKey = UTF8ToWide(pageBody.substr(sizeof(noUpdateWithTag) - 1));
			if (resKey == providedKey && regKey == providedKey)
				return 0; // our tag is up to date! woot!

			// No point trying to write resource key if there is no resource file
			if (resKey != providedKey && resFileExists)
			{
				if (providedKey.back() == L'\0')
					DarkEdif::MsgBox::Error(_T("?"), _T("Provided key is invalid."));

				if (resHandle == NULL)
				{
					if (GetLastError() == ERROR_ACCESS_DENIED)
						DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure %u. Try running the UCT Fix Tool, or run Fusion as admin.\n\nUCT fix tool:\nhttps://dark-wire.com/storage/UCT%%20Fix%%20Tool.exe"), GetLastError());
				}
				else
				{
					// String resources are always UTF-16, stored with preceding uint16 size, no null terminator, and always in groups of 16
					const int groupSize = 16;
					std::wstring keyOut(1 + providedKey.size() + (groupSize - 1), L'\0');

					// Copy string size then string content in
					*(std::uint16_t *)&keyOut[0] = (std::uint16_t)providedKey.size();
					wmemcpy_s(&keyOut[1], keyOut.size() - 1, providedKey.data(), providedKey.size());

					if (!UpdateResource(resHandle, RT_STRING, _T("UCTAG"), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), keyOut.data(), keyOut.size() * sizeof(wchar_t)))
						DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure; updating tag returned %u."), GetLastError());

					if (!EndUpdateResource(resHandle, FALSE))
					{
						if (GetLastError() == ERROR_ACCESS_DENIED)
							DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure. Try running the UCT Fix Tool, or run Fusion as admin.\n\nUCT fix tool:\nhttps://dark-wire.com/storage/UCT%%20Fix%%20Tool.exe"));
						else
							DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure; saving new tag returned %u."), GetLastError());
					}
					// Release the unique_ptr, we no longer need it to auto-run EndUpdateResource for us
					resHandleHolder.release();
				}
			}

			if (regKey != providedKey)
			{
				DWORD err = 0;

				// Users have access to registry of local machine, albeit read-only, so this should not fail.
				if (mainKey == NULL)
					err = RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Clickteam", &mainKey);

				// We can try writing to the registry - we'll include null, as RegEdit uses it
				err = err ? err : RegSetValueExW(mainKey, L"UCTag", 0, REG_SZ, (LPBYTE)providedKey.c_str(), (providedKey.size() + 1) * sizeof(wchar_t));
				if (err != 0)
				{
					DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure; saving new tag returned %u.%s"), err,
						err == ERROR_ACCESS_DENIED ? _T(" Try running the UCT Fix Tool, or running Fusion as admin.\n\nUCT fix tool:\nhttps://dark-wire.com/storage/UCT%20Fix%20Tool.exe") : _T(""));
				}
			}

			#endif

			return 0;
		};

		if (!_strnicmp(pageBody.c_str(), extDevUpdate, sizeof(extDevUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ExtDevError;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody.substr(sizeof(extDevUpdate) - 1)));
			return 0;
		}
		if (!_strnicmp(pageBody.c_str(), sdkUpdate, sizeof(sdkUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::SDKUpdate;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody.substr(sizeof(sdkUpdate) - 1)));
			return 0;
		}
		if (!_strnicmp(pageBody.c_str(), majorUpdate, sizeof(majorUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::Major;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody.substr(sizeof(majorUpdate) - 1)));
			return 0;
		}

		if (!_strnicmp(pageBody.c_str(), minorUpdate, sizeof(minorUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::Minor;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody.substr(sizeof(minorUpdate) - 1)));
			return 0;
		}

		if (handleWSAError(EINVAL))
		{
			#ifdef _DEBUG
				updateLog << "Can't interpret type of page:\n"sv << pageBody;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody);
			#else
				updateLog << "You should report this to Phi on the Clickteam Discord."sv;
				pendingUpdateDetails.clear();
			#endif
			updateLock = false;
		}
		return 0;
	}
	catch (...)
	{
		GetLockAnd(
			updateLog << "Caught a crash. Aborting update."sv);
		OutputDebugStringA(updateLog.str().c_str());
		return 0;
	}
#undef GetLockAnd
#undef GetLockSetConnectErrorAnd
}

#endif // USE_DARKEDIF_UPDATE_CHECKER


#endif // EditorBuild

// Define it
std::tstring DarkEdif::ExtensionName(_T("" PROJECT_NAME ""s));
std::thread::id DarkEdif::MainThreadID;
WindowHandleType DarkEdif::Internal_WindowHandle;
DarkEdif::MFXRunMode DarkEdif::RunMode = DarkEdif::MFXRunMode::Unset;

// =====
// Message boxes that mostly work on all platforms
// On Android, this makes a small popup on bottom of the screen, called a toast notification
// =====
static int Internal_MessageBox(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, va_list v, int flags)
{
	assert(titlePrefix != NULL && msgFormat != NULL);

	// This doesn't work on Windows XP in some scenarios; for an explanation, see the README file.
#ifdef ThreadSafeStaticInitIsSafe
	const static std::tstring titleSuffix = _T(" - " PROJECT_NAME ""s);
#else
	const static TCHAR projNameStatic[] = _T(" - " PROJECT_NAME);
	const std::tstring titleSuffix = projNameStatic;
#endif

	// Without this modal setting, the user will usually crash Fusion.
	// With non-modal, they can interact with the editor window of Fusion.
	// If you then switch editor from frame to event, then back to frame, then close the message box, the MB_OK gets sent back to Fusion's main
	// window unexpectedly, making it crash.
	//
	// Note: You can also fix the crash by passing HWND of NULL to MessageBox(), but this means it isn't put in front of Fusion, meaning that
	// users will often click too fast and hide the message behind the editor window... meaning they interact.. meaning when they close it, it crashes.
#if EditorBuild
	if (DarkEdif::RunMode == DarkEdif::MFXRunMode::Editor && DarkEdif::MainThreadID == std::this_thread::get_id() && (flags & (MB_SYSTEMMODAL | MB_TASKMODAL)) == 0)
		flags |= (MB_TASKMODAL | MB_SETFOREGROUND);
#endif

	std::tstring title = titlePrefix + titleSuffix;
	TCHAR msgData[4000];
	int numChars = _vstprintf_s(msgData, std::size(msgData), msgFormat, v);
	if (numChars <= 0)
	{
		MessageBox(DarkEdif::Internal_WindowHandle, _T("Failed to format a message box. Contact extension developer."), title.c_str(), MB_OK | MB_ICONERROR);
		DarkEdif::BreakIfDebuggerAttached();
		return IDCANCEL;
	}
	return MessageBox(DarkEdif::Internal_WindowHandle, msgData, title.c_str(), flags);
}
void DarkEdif::MsgBox::WarningOK(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	Internal_MessageBox(titlePrefix, msgFormat, v, MB_OK | MB_ICONWARNING);
	va_end(v);
}
int DarkEdif::MsgBox::WarningYesNo(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	int ret = Internal_MessageBox(titlePrefix, msgFormat, v, MB_YESNO | MB_ICONWARNING);
	va_end(v);
	return ret;
}
int DarkEdif::MsgBox::WarningYesNoCancel(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	int ret = Internal_MessageBox(titlePrefix, msgFormat, v, MB_YESNOCANCEL | MB_ICONWARNING);
	va_end(v);
	return ret;
}
void DarkEdif::MsgBox::Error(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	Internal_MessageBox(titlePrefix, msgFormat, v, MB_OK | MB_ICONERROR);
	va_end(v);
}
void DarkEdif::MsgBox::Info(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	Internal_MessageBox(titlePrefix, msgFormat, v, MB_OK | MB_ICONINFORMATION);
	va_end(v);
}
int DarkEdif::MsgBox::Custom(const int flags, const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	int ret = Internal_MessageBox(titlePrefix, msgFormat, v, flags);
	va_end(v);
	return ret;
}
extern int aceIndex;
void DarkEdif::Log(int logLevel, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	DarkEdif::LogV(logLevel, msgFormat, v);
	va_end(v);
}
void DarkEdif::LogV(int logLevel, PrintFHintInside const TCHAR* msgFormat, va_list v)
{
	// Listens to and optionally intercepts the log calls
	// On Android, some logs are done before Edif::SDK is set
	if (Edif::SDK && Edif::SDK->LogIntercept)
	{
		va_list dV;
		va_copy(dV, v);

		const bool wasKilled = Edif::SDK->LogIntercept(PROJECT_TARGET_NAME, logLevel, msgFormat, dV);
		va_end(dV);
		if (wasKilled)
			return;
	}

#ifdef _WIN32
	static TCHAR outputBuff[8192];
	int didTrunc = _vsntprintf_s(outputBuff, std::size(outputBuff), _TRUNCATE, msgFormat, v);

	// Truncated the log; trim it
	if (didTrunc == -1)
	{
		_tcscpy(&outputBuff[std::size(outputBuff) - std::size(_T("... [truncated]")) - 2], _T("... [truncated]"));
		// add newline if format had it
		if (msgFormat[_tcslen(msgFormat) - 2] == _T('\r'))
			_tcscpy(&outputBuff[std::size(outputBuff) - std::size(_T("\r\n"))], _T("\r\n"));
		else if (msgFormat[_tcslen(msgFormat) - 1] == _T('\n'))
			_tcscpy(&outputBuff[std::size(outputBuff) - std::size(_T("\n"))], _T("\n"));
	}

	// Wine debugstr only logs OutputDebugStringA, not OutputDebugStringW
	#ifdef _UNICODE
		if (DarkEdif::IsRunningUnderWine)
			OutputDebugStringA(TStringToUTF8(outputBuff).c_str());
		else
	#endif // Unicode
			OutputDebugString(outputBuff);

#elif defined(__ANDROID__)
	std::string msgFormatT = std::string(aceIndex, '>');
	msgFormatT += ' ';
	msgFormatT += msgFormat;
	__android_log_vprint(logLevel, PROJECT_TARGET_NAME_UNDERSCORES, msgFormatT.c_str(), v);
#else // iOS
	static const char* logLevels[] = {
		"", "", "verbose", "debug", "info", "warn", "error", "fatal"
	};
	printf("%-9s", logLevels[logLevel]);
	vprintf(msgFormat, v);
#endif
}

#if (defined(__ANDROID__) || defined(__APPLE__)) && DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_INFO
void DarkEdif::OutputDebugStringAInternal(const char * debugString)
{
	assert(debugString != NULL);
	// We can't get the user to remove their newlines, as Windows doesn't automatically add them in OutputDebugStringA(),
	// but __android_log_print includes automatic newlines, so strip them.
	std::string debugStringSafe(debugString);
#ifdef __ANDROID__
	if (debugStringSafe.back() == '\n')
		debugStringSafe.resize(debugStringSafe.size() - 1U);
	if (debugStringSafe.back() == '\r')
		debugStringSafe.resize(debugStringSafe.size() - 1U);
	if (debugStringSafe.back() == '.')
		debugStringSafe.resize(debugStringSafe.size() - 1U);
#endif

	LOGI("OutputDebugStringA: %s.\n", debugStringSafe.c_str());
}
#endif // Android, iOS, and DarkEdif log level INFO or lower level

#ifndef _WIN32
// To get the Windows-like behaviour
void DarkEdif::Sleep(unsigned int milliseconds)
{
	if (milliseconds == 0)
		std::this_thread::yield();
	else
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

#endif



// =====
// Embed the minified JSON file in Android and iOS
// =====

// Causes the produced extension to include DarkExt.PostMinify.json.
// Hat tip: https://stackoverflow.com/a/4910421
// Also note https://github.com/graphitemaster/incbin/blob/master/incbin.h

#ifdef __ANDROID__
__asm__(".section .rodata				\n\
	.global darkExtJSON					\n\
	.type   darkExtJSON, %object		\n\
	.align  4							\n\
darkExtJSON:							\n\
	.incbin \"DarkExt.PostMinify.json\"	\n\
darkExtJSON_end:						\n\
	.global darkExtJSONSize				\n\
	.type   darkExtJSONSize, %object	\n\
	.align  4							\n\
darkExtJSONSize:						\n\
	.int	darkExtJSON_end - darkExtJSON");
#elif defined(__APPLE__)
/**
 * @file incbin.h
 * @author Dale Weiler
 * @brief Utility for including binary files
 *
 * Facilities for including binary files into the current translation unit and
 * making use from them externally in other translation units.
 */

INCBIN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _darkExtJSON, "DarkExt.PostMinify.json");

// See https://stackoverflow.com/a/19725269
// Note the file will NOT be transmitted to Mac unless it's set as a C/C++ header file.
/*__asm__(".const_data					\n\
	.global darkExtJSON					\n\
	.align  4							\n\
darkExtJSON:							\n\
	.incbin \"DarkExt.PostMinify.json\"	\n\
__asm__("darkExtJSON_end:						\n\
	.global darkExtJSONSize				\n\
	.align  4							\n\
darkExtJSONSize:						\n\
	.int	darkExtJSON_end - darkExtJSON");*/

#endif
// These are caused by the above ASM block. (these are also declared in the Android/iOS master header)
// char darkExtJSON[];
// unsigned darkExtJSONSize;
