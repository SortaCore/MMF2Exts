#pragma once
class Extension final
{
public:
	// ======================================
	// Required variables + functions
	// Variables here must not be moved or swapped around or it can cause future issues
	// ======================================
	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr;

	Edif::Runtime Runtime;

	static const int MinimumBuild = 254;
	static const int Version = 7;

	static constexpr OEFLAGS OEFLAGS = OEFLAGS::NONE;
	static constexpr OEPREFS OEPREFS = OEPREFS::NONE;

	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
	~Extension();

	// ======================================
	// Extension data
	// ======================================

	bool IsBadMemoryAddress(const void * const address);

	/// Actions
	void UTF16StrToUTF16Mem(const wchar_t * utf16Str, int addr, int excludeNull);

	/// Conditions

	/// Expressions

	int UTF16CharToUTF16Int(const wchar_t * utf16Char);
	const wchar_t * UTF16IntToUTF16Char(unsigned int utf16ints);
	const wchar_t * UTF16StrFromUTF16Mem(int addr, int numChars);
	const wchar_t * UTF16StrFromUTF8Mem(int addr, int numChars);

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

	// These are called if there's no function linked to an ID

	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);
};
