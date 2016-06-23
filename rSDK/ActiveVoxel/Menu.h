// --------------------------------
// Condition menu
// --------------------------------

#ifdef CONDITION_MENU

	ITEM(0, "Evaluated condition")
	SEPARATOR
	ITEM(1, "Triggered condition")

#endif

// --------------------------------
// Action menu
// --------------------------------

#ifdef ACTION_MENU

	// You have access to edPtr and you can throw in normal C++
	// code wherever you want in any of these menus. If you want to
	// show certain conditions based on a property (a la OINC socket)
	// use the property's value stored in edPtr:

	// if(edPtr->ShowAdvancedOptions)
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

	// for(int i=0;i<edPtr->ActionCount;i++)
	//		ITEM(i,edPtr->Actions[i])

	// This will run every time the menu is opened in MMF2, so don't
	// make it too slow or it could get annoying

	SEPARATOR
	//ITEM(6,"Load Palette (do first of all)")
	ITEM(0,"Load VXL (once per object)")
	SUB_START("Angle")
		ITEM(1,"Set X angle")
		ITEM(2,"Set Y angle")
		ITEM(3,"Set Z angle")
	SUB_END
	SUB_START("Effects")
		ITEM(10,"Enable shading")
		ITEM(11,"Disable shading (default)")

		ITEM(4,"Set specular")
		ITEM(5,"Set ambient")

		ITEM(7,"Set X of light")
		ITEM(8,"Set Y of light")
		ITEM(9,"Set Z of light")
	SUB_END

#endif


// --------------------------------
// Expression menu
// --------------------------------

#ifdef EXPRESSION_MENU

	ITEM(0,"Sum of")
	SEPARATOR
	ITEM(1,"Return a string")

#endif