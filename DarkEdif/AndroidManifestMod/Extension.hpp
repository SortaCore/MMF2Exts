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

	static const int MinimumBuild = 254;
	static const int Version = 4;

	static constexpr OEFLAGS OEFLAGS = OEFLAGS::NONE;
	static constexpr OEPREFS OEPREFS = OEPREFS::NONE;

	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
	~Extension();

	// ======================================
	// Extension data
	// ======================================

	int DummyExpression();
	void DummyAction();
	bool DummyCondition();

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

	// These are called if there's no function linked to an ID
	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);
};
