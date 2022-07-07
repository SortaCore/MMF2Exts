#pragma once
#include "DarkEdif.h"

class Extension
{
public:

#ifdef _WIN32
	RUNDATA * rdPtr;
	RunHeader * rhPtr;
#elif defined(__ANDROID__)
	RuntimeFunctions & runFuncs;
	global<jobject> javaExtPtr;
#else
	RuntimeFunctions & runFuncs;
	void * objCExtPtr;
#endif

	Edif::Runtime Runtime;

	static const int MinimumBuild = 254;
	static const int Version = 2;

	// If you change OEFLAGS, make sure you modify RUNDATA so the data is available, or you'll get crashes!
	// For example, OEFLAGS::VALUES makes use of the AltVals rv struct.
	static const OEFLAGS OEFLAGS = OEFLAGS::NONE;
	static const OEPREFS OEPREFS = OEPREFS::NONE;

	static const int WindowProcPriority = 100;

#ifdef _WIN32
	Extension(RUNDATA * rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr);
#elif defined(__ANDROID__)
	Extension(RuntimeFunctions & runFuncs, EDITDATA * edPtr, jobject javaExtPtr);
#else
	Extension(RuntimeFunctions & runFuncs, EDITDATA * edPtr, void * objCExtPtr);
#endif
	~Extension();

	int DummyExpression();
	void DummyAction();
	bool DummyCondition();


	/* These are called if there's no function linked to an ID */

	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);




	/*  These replace the functions like HandleRunObject that used to be
		implemented in Runtime.cpp. They work exactly the same, but they're
		inside the extension class.
	*/

	REFLAG Handle();
	REFLAG Display();

	short FusionRuntimePaused();
	short FusionRuntimeContinued();
};
