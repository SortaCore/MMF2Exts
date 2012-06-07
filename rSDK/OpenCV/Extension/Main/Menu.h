// --------------------------------
// Condition menu
// --------------------------------

#ifdef CONDITION_MENU

	//ITEM(0, "Evaluated condition")
	//SEPARATOR

	SEPARATOR
	ITEM(0, "On error (triggered condition)")

#endif

// --------------------------------
// Action menu
// --------------------------------

#ifdef ACTION_MENU

	// You have access to edPtr and you can throw in normal C++
	// code wherever you want in any of these menus. If you want to
	// show certain conditions based on a property (a la OINC socket)
	// use the property's value stored in edPtr:

	// if (edPtr->ShowAdvancedOptions)
	// {
	//		SUB_START("Bake pie")
	//			ITEM(0, "Apple pie")
	//			ITEM(1, "Mince pie")
	//		SUB_END
	// }
	// else
	// {
	//		ITEM(0, "Bake apple pie")
	// }

	// You could do some pretty cool stuff with this, like loading a
	// menu from an external source (ActiveX object?)

	// for (int i=0;i<edPtr->ActionCount;i++)
	//		ITEM(i,edPtr->Actions[i])

	// This will run every time the menu is opened in MMF2, so don't
	// make it too slow or it could get annoying

	//SEPARATOR
	
	SUB_START("Upon error")
		ITEM(0,"Do nothing")
		ITEM(1,"Log to file")
		ITEM(2,"Show abort-retry-ignore GUI box")
		ITEM(3,"Trigger condition \"OpenCV - On error\"")
	SUB_END

#endif

// --------------------------------
// Expression menu
// --------------------------------

#ifdef EXPRESSION_MENU

	SEPARATOR
	ITEM(0, "On error: Get error code")
	ITEM(0, "On error: Get error message")
	ITEM(0, "On error: Get function name")
	ITEM(0, "On error: Get filename")
	ITEM(0, "On error: Get line number")
	//ITEM(0,"Get width")
	//ITEM(1,"Get height")
	//ITEM(1,"Return a string")

#endif