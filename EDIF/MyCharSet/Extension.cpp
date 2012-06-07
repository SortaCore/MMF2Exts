#include "Common.h"


///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(LPRDATA _rdPtr, LPEDATA edPtr, fpcob cobPtr)
    : rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.hoAdRunHeader), Runtime(_rdPtr)
{
    /*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
    */

	LinkCondition(0, IsUnicode);

    LinkExpression(0, GetASCIIStringFromUnicodeString);
    LinkExpression(1, GetUnicodeStringFromASCIIString);
	LinkExpression(2, GetASCIIStringFromUnicodeMemory);
	LinkExpression(3, GetUnicodeStringFromASCIIMemory);

	LinkExpression(4, GetASCIIMemoryFromUnicodeString);
    LinkExpression(5, GetUnicodeMemoryFromASCIIString);
	LinkExpression(6, GetASCIIMemoryFromUnicodeMemory);
	LinkExpression(7, GetUnicodeMemoryFromASCIIMemory);

    /*
        This is where you'd do anything you'd do in CreateRunObject in the original SDK

        It's the only place you'll get access to edPtr at runtime, so you should transfer
        anything from edPtr to the extension class here.
    
    */    
}

Extension::~Extension()
{
    /*
        This is where you'd do anything you'd do in DestroyRunObject in the original SDK.
        (except calling destructors and other such atrocities, because that's automatic in Edif)
    */



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
	return REFLAG_ONESHOT;
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

void Extension::Action(int ID, LPRDATA rdPtr, long param1, long param2)
{
	char t [50];
	sprintf_s(t, 50, "Action not set up: ID %i.", ID);
	MessageBoxA(NULL, t, "Darknet Object Error", MB_OK);
}

long Extension::Condition(int ID, LPRDATA rdPtr, long param1, long param2)
{
	char t [50];
	sprintf_s(t, 50, "Condition not set up: ID %i.", ID);
	MessageBoxA(NULL, t, "Darknet Object Error", MB_OK);
    return false;
}

long Extension::Expression(int ID, LPRDATA rdPtr, long param)
{
	char t [50];
	sprintf_s(t, 50, "Expression not set up: ID %i.", ID);
	MessageBoxA(NULL, t, "Darknet Object Error", MB_OK);
    return 0;
}

