// --------------------------------
// Condition menu
// --------------------------------

#ifdef CONDITION_MENU

	//ITEM(0, "Evaluated condition")
	//SEPARATOR

	ITEM(0, "On looping through points (once per point)")

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
	
	ITEM(0,"Clear")
	ITEM(1,"Add point")
	ITEM(2,"Loop through all points")
#endif


// --------------------------------
// Expression menu
// --------------------------------

#ifdef EXPRESSION_MENU

	ITEM(0,"Number of points")
	SEPARATOR
	ITEM(1,"On looping: Get X of current point")
	ITEM(2,"On looping: Get Y of current point")

#endif