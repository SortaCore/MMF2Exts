#pragma once
#include "Common.hpp"

class Extension final
{
public:
	// ======================================
	// Required variables
	// Variables here must not be moved or swapped around or it can cause future issues
	// ======================================
	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr;

	Edif::Runtime Runtime;
	static const int MinimumBuild = 256;
	static const int Version = 16;
	// b16: SDK update to v20...
	// b15: SDK update to v14, update checker + smart properties, and made the error message readable to a native debugger
	// b14: Added more details to crash information
	// b13: Fixed message box about properties failing to convert
	// b12: Fixed use of tcsdup in expressions, upgrade to SDK v5
	// b11: Fixed minidump handle not being passed
	// b10: Use of DarkEdif SDK properties, formatting changes
	// b9: Fixed lockup when disabling console and debug was already disabled
	// b8: Fixed non-closing console when console was disabled
	// b7: Fixed subapp "MFX Not Found" issue in DarkEdif
	// b6: Release for edittime property fix, added runtime property, new minidump events
	// b5: Release for bug fixes and global settings, and unhandled exception event
	// b4: First release with console
	// b3: Release due to bug fixes
	// b2: Release due to new features
	// b1: First release
	static constexpr OEFLAGS OEFLAGS = OEFLAGS::NEVER_KILL | OEFLAGS::NEVER_SLEEP | OEFLAGS::RUN_BEFORE_FADE_IN;
	static constexpr OEPREFS OEPREFS = OEPREFS::GLOBAL;

	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
	~Extension();

	// ======================================
	// Extension data
	// ======================================

	GlobalData * data;

	// Internal
	void OutputNow(int intensity, int line, std::string textToOutputU8);

	// Actions
	void SetOutputFile(const TCHAR * fileP, int describeAppI);
	void Output(int intensity, int line, const TCHAR * textToOutput);
	void SetOutputTimeFormat(TCHAR * format);
	void SetOutputOnOff(int onOff);
	void SetHandler(int reaction, int continuesRemaining);
	void CauseCrash_ZeroDivisionInt();
	void CauseCrash_ZeroDivisionFloat();
	void CauseCrash_WriteAccessViolation();
	void CauseCrash_ReadAccessViolation();
	void CauseCrash_ArrayOutOfBoundsRead();
	void CauseCrash_ArrayOutOfBoundsWrite();
	void SetConsoleOnOff(int onOff);
	void SetDumpFile(const TCHAR * path, int flags);

	// Conditions
	const bool AlwaysTrue() const;
	bool OnSpecificConsoleInput(TCHAR * command);

	// Expressions
	const TCHAR * FullCommand();
	const TCHAR * CommandMinusName();

	void LoadDataVariable();

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

	// These are called if there's no function linked to an ID

	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);
};
