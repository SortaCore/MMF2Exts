
#include "Common.h"


///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr)
    : rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr)
{
    /*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
    */

    LinkAction(0, ActionExample);
    LinkAction(1, SecondActionExample);

    LinkCondition(0, AreTwoNumbersEqual);

    LinkExpression(0, Add);
    LinkExpression(1, HelloWorld);



    /*
        This is where you'd do anything you'd do in CreateRunObject in the original SDK

        It's the only place you'll get access to edPtr at runtime, so you should transfer
        anything from edPtr to the extension class here.
    
    */


    
}

#ifdef MULTI_THREADING
SaveExtInfo &Extension::AddEvent(int Event, bool UseLastData /* = false */)
{
	/*
		Saves all variables returned by expressions in order to ensure two conditions, triggering simultaneously,
		do not cause reading of only the last condition's output. Useful for multi-threading.
		
		For example, if an error event changes an output variable, and immediately afterward a 
		success event changes the same variable, only the success event's output can be read.
	*/

	// Cause a new event
	if (UseLastData == false)
	{
		SaveExtInfo * NewEvent = new SaveExtInfo;
		
		// Initialise with one condition to be triggered
		NewEvent->NumEvents = 1;
		NewEvent->CondTrig = (unsigned short *)malloc(sizeof(unsigned short));
		
		// Failed to allocate memory
		if (!NewEvent->CondTrig)
			return ThreadData;
		
		NewEvent->CondTrig[0] = (unsigned short)Event;
		
		// Copy Extension's data to vector
		if (memcpy_s(((char *)NewEvent)+5, sizeof(SaveExtInfo)-5, ((char *)&ThreadData)+5, sizeof(SaveExtInfo)-5))
		{
			// Failed to copy memory (error is stored in "errno")
			return ThreadData;
		}
		Saved.push_back(NewEvent);
	}
	else // New event is part of the last saved data (good for optimisation)
	{
		// Add current condition to saved expressions
		SaveExtInfo & S = Saved.size() == 0 ? ThreadData : *Saved.back();
		++S.NumEvents;
		unsigned short * CurrentCond = (unsigned short *)realloc(&S.CondTrig[0], S.NumEvents * sizeof(short));
		
		if (!CurrentCond)
			return ThreadData;

		CurrentCond[S.NumEvents-1] = (unsigned short)Event;
		
		S.CondTrig = CurrentCond;
	}

	// Cause Handle() to be triggered, allowing Saved to be parsed
	Runtime.Rehandle();
	return *Saved.back();
}
#endif // MULTI_THREADING

Extension::~Extension()
{
	#ifdef MULTI_THREADING
		// The SavedExtInfo array itself will deconstruct at the end of ~Extension(); we just need to free memory inside.
		for (unsigned int i = 0; i < Saved.size(); ++i)
		{
			// Deletion of memory pointed to inside SaveExtInfo should be handled here.
			free(Saved[i].CondTrig);
		}
	#endif // MULTI_THREADING
}


short Extension::Handle()
{
	#ifdef MULTI_THREADING
		// AddEvent() was called and not yet handled
		while (Saved.size() > 0)
		{
			// Copy from saved list of events to current extension
			try {
				if (memcpy_s(&ThreadData, sizeof(SaveExtInfo), Saved.front(), sizeof(SaveExtInfo)))
					break; // Failed; leave until next Extension::Handle()
			

				// Trigger all stored events (more than one may be stored by calling AddEvent(***, true) )
				for (unsigned char u = 0; u < ThreadData.NumEvents; ++u)
					Runtime.GenerateEvent((int) ThreadData.CondTrig[u]);

				Saved.erase(Saved.begin());
			}
			catch (...)
			{
				break; // Failed; leave until next Extension::Handle()
			}
		}
	#endif // MULTI_THREADING

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


