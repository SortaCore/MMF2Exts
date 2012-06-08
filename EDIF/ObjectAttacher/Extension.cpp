
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

	LinkAction(0, AttachObjects);

    LinkExpression(0, NumObjects);


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
	/* We loop through all objects and update them... */
	for(unsigned int i = 0; i < AttachedObjects.size(); ++i)
	{
		/* Store a pointer to the current loop's object */
		RunObject* Object = AttachedObjects[i];

		/* Check if this object was deleted */
		if(Object->roHo.hoFlags & HOF_DESTROYED)
		{
			/* If so, remove it from this array */
			AttachedObjects.erase(AttachedObjects.begin() + i);

			/* Now we need to process this loop index again. */
			i--;
			continue;
		}

		/* Modify it somehow */
		Object->roHo.hoX += (rand() % 5) - (rand() % 5);
		Object->roHo.hoY += (rand() % 5) - (rand() % 5);
		Object->roc.rcAngle += (rand() % 5) - (rand() % 5);

		/* Tell MMF to redraw the object */
		Object->roc.rcChanged = true;
	}

	/* Returning 0 will make sure Handle() gets called again... */
	return 0;
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

}

long Extension::Condition(int ID, LPRDATA rdPtr, long param1, long param2)
{

    return false;
}

long Extension::Expression(int ID, LPRDATA rdPtr, long param)
{

    return 0;
}

