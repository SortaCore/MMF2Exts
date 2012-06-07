// --------------------------------
// Condition menu
// --------------------------------

#ifdef CONDITION_MENU

	ITEM(0, "Download in progress?")
	ITEM(1, "Download completed?")
	ITEM(2, "Connection error?")
	ITEM(3, "Download aborted?")
	SEPARATOR
	ITEM(4, "On error")
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

	ITEM(0,"Download a file")
	ITEM(1,"Abort download")
	ITEM(2,"Pause download")
	ITEM(3,"Resume download")
#endif
	

// --------------------------------
// Expression menu
// --------------------------------

#ifdef EXPRESSION_MENU
	ITEM(0,"Get file size (without download)")
    SEPARATOR
    ITEM(1,"Current URL")
    ITEM(2,"Total file size")
    ITEM(3,"Current download position")
    ITEM(4,"Current download percentage")
    ITEM(5,"Current speed (bytes per second)")
    ITEM(6,"Estimated download time")
	SEPARATOR
	ITEM(7,"Last error text")
#endif