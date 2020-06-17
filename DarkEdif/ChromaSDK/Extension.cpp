#include "Common.h"
#include	"Public/ChromaAnimationAPI.h"

using namespace ChromaSDK;


///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr)
	: rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr), FusionDebugger(this)
{
	_mInitResult = -1;
	if (!ChromaAnimationAPI::IsInitializedAPI())
	{
		_mInitResult = ChromaAnimationAPI::InitAPI();
		if (_mInitResult == 0)
		{
			fprintf(stderr, "Loaded Chroma SDK Plugin!\r\n");
		}
		else
		{
			fprintf(stderr, "Failed to loaded Chroma SDK Plugin!\r\n");
		}
	}

	/*
		Link all your action/condition/expression functions to their IDs to match the
		IDs in the JSON here
	*/

	LinkAction(0, ActionExample);
	LinkAction(1, SecondActionExample);

	unsigned int conditionIndex = 0;
	LinkCondition(conditionIndex++, AreTwoNumbersEqual);
	LinkCondition(conditionIndex++, CondIsInitialized);
	LinkCondition(conditionIndex++, CondInit);

	unsigned int expressionIndex = 0;
	LinkExpression(expressionIndex++, Add);
	LinkExpression(expressionIndex++, HelloWorld);
	LinkExpression(expressionIndex++, ExpIsInitialized);
	LinkExpression(expressionIndex++, ExpInit);

	/*
		This is where you'd do anything you'd do in CreateRunObject in the original SDK

		It's the only place you'll get access to edPtr at runtime, so you should transfer
		anything from edPtr to the extension class here.

	*/

	// Don't use "this" inside these lambda functions, always ext.
	// There can be nothing in the [] section of the lambda.
	// If you're not sure about lambdas, you can remove this debugger stuff without any side effects;
	// it's just an example of how to use the debugger. You can view it in Fusion itself to see.
	FusionDebugger.AddItemToDebugger(
		// reader function for your debug item
		[](Extension *ext, std::tstring &writeTo) {
			writeTo = _T("My text is: ") + ext->exampleDebuggerTextItem;
		},
		// writer function (can be null if you don't want user to be able to edit it in debugger)
		[](Extension *ext, std::tstring &newText)
		{
			ext->exampleDebuggerTextItem = newText;
			return true; // accept the changes
		}, 500, NULL
	);
	
}

Extension::~Extension()
{

}


REFLAG Extension::Handle()
{
	/*
		If your extension will draw to the MMF window you should first 
		check if anything about its display has changed :

			if (rdPtr->roc.rcChanged)
			  return REFLAG::DISPLAY;
			else
			  return REFLAG::NONE;

		You will also need to make sure you change this flag yourself 
		to 1 whenever you want to redraw your object
	 
		If your extension won't draw to the window, but it still needs 
		to do something every MMF loop use :

			return REFLAG::NONE;

		If you don't need to do something every loop, use :

			return REFLAG::ONE_SHOT;

		This doesn't mean this function can never run again. If you want MMF
		to handle your object again (causing this code to run) use this function:

			Runtime.Rehandle();

		At the end of the loop this code will run

	*/

	// Will not be called next loop	
	return REFLAG::ONE_SHOT;
}


REFLAG Extension::Display()
{
	/*
		If you return REFLAG_DISPLAY in Handle() this routine will run.
	*/

	// Ok
	return REFLAG::DISPLAY;
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


