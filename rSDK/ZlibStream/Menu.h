// --------------------------------
// Condition menu
// --------------------------------

#ifdef CONDITION_MENU
	SEPARATOR
	ITEM(0, "On success")
	ITEM(1, "On error")
	SEPARATOR
	DISABLED(2, "Darkwire Software")
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
	//Item 0 was old compress action; 1 was old decompress action
	SEPARATOR
	ITEM(3,"Compress a file")
	ITEM(4,"Decompress a file")
	SEPARATOR
	ITEM(2,"Set buffer size (bytes)")
	SEPARATOR
#endif
	

// --------------------------------
// Expression menu
// --------------------------------

#ifdef EXPRESSION_MENU
	SEPARATOR
	ITEM(1,"Last returned string")
	ITEM(3,"Last percentage difference")
	ITEM(4,"Last output filename")
	SEPARATOR
	ITEM(2,"Current buffer size (bytes)")
	SEPARATOR
#endif