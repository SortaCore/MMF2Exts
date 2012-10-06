
#include "Common.h"

GlobalData * Data = NULL;
class Extension * GlobalExt = NULL;
///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr)
    : rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr)
{
	if (!GlobalExt)
		GlobalExt = this;
    /*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
    */

	// Link information
    LinkAction(0, Output);
	LinkAction(1, SetOutputFile);
	LinkAction(2, SetOutputTimeFormat);
	LinkAction(3, SetOutputOnOff);
	LinkAction(4, SetHandler);
	LinkAction(5, CauseCrash_ZeroDivisionInt);
	LinkAction(6, CauseCrash_ZeroDivisionFloat);
	LinkAction(7, CauseCrash_ReadAccessViolation);
	LinkAction(8, CauseCrash_WriteAccessViolation);
	LinkAction(9, CauseCrash_ArrayOutOfBoundsRead);
	LinkAction(10, CauseCrash_ArrayOutOfBoundsWrite);
	LinkAction(11, SetConsoleOnOff);

	LinkCondition(0, OnSpecificConsoleInput);
	LinkCondition(1, OnAnyConsoleInput);

	// Initialise from edittime
	SetOutputOnOff(edPtr->EnableAtStart);
	Data->DoMsgBoxIfPathNotSet = edPtr->DoMsgBoxIfPathNotSet;
	SetConsoleOnOff(edPtr->ConsoleEnabled);

	if (edPtr->InitialPath[0] != '\0') // ""
		SetOutputFile(edPtr->InitialPath, 0);
	
}


Extension::~Extension()
{
	if (GlobalExt == this)
		GlobalExt = NULL;
}


short Extension::Handle()
{
    /*
       If your extension will draw to the MMF window you should first 
       check if anything about its display has changed :

           if (rdPtr->roc.rcChanged)
              return REFLAG_DISPLAY;
           else
              return 0;

       You will also need to make sure you change this flag yourself 
       to 1 whenever you want to redraw your object
     
       If your extension won't draw to the window, but it still needs 
       to do something every MMF loop use :

            return 0;

       If you don't need to do something every loop, use :

            return REFLAG_ONESHOT;

       This doesn't mean this function can never run again. If you want MMF
       to handle your object again (causing this code to run) use this function:

            Runtime.Rehandle();

       At the end of the loop this code will run

    */

	// Will not be called next loop	
	return REFLAG::ONE_SHOT;
}


short Extension::Display()
{
    /*
       If you return REFLAG_DISPLAY in Handle() this routine will run.
    */

    // Ok
    return 0;
}

short Extension::Pause()
{

    // Ok
    return 0;
}

short Extension::Continue()
{

    // Ok
    return 0;
}

bool Extension::Save(HANDLE File)
{
	bool OK = false;

    #ifndef VITALIZE

	    // Save the object's data here

	    OK = true;

    #endif

	return OK;
}

bool Extension::Load(HANDLE File)
{
	bool OK = false;

    #ifndef VITALIZE

	    // Load the object's data here

	    OK = true;

    #endif

	return OK;
}


// These are called if there's no function linked to an ID

void Extension::Action(int ID, RUNDATA * rdPtr, long param1, long param2)
{

}

long Extension::Condition(int ID, RUNDATA * rdPtr, long param1, long param2)
{

    return false;
}

long Extension::Expression(int ID, RUNDATA * rdPtr, long param)
{

    return 0;
}

