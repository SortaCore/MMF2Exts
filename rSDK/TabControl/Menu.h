// --------------------------------
// Condition menu
// --------------------------------

#ifdef CONDITION_MENU

	SEPARATOR
	ITEM(0,_T("Tab changed ?"))
	ITEM(6,_T("Tab changed to (by caption) ?"))
	ITEM(7,_T("Tab changed to (by index) ?"))
	SEPARATOR	
	ITEM(5,_T("Tab changed by user ?"))
	SEPARATOR
	ITEM(1,_T("LMB clicked ?"))
	ITEM(2,_T("LMB double-clicked ?"))
	ITEM(3,_T("RMB clicked ?"))
	ITEM(4,_T("RMB double-clicked ?"))

#endif

// --------------------------------
// Action menu
// --------------------------------

#ifdef ACTION_MENU
	SEPARATOR
	SUB_START(_T("Window settings"))
		ITEM(5,_T("Set width"))
		ITEM(15,_T("Set height"))
		ITEM(6,_T("Set visibility"))
		ITEM(16,_T("Set style"))
		SEPARATOR
		ITEM(23,_T("Force redraw"))
	SUB_END
		SUB_START(_T("Tab settings"))
		ITEM(8, _T("Set minimal tab width"))
		ITEM(10, _T("Set tab size"))
		ITEM(9, _T("Set padding"))
	SUB_END
	SEPARATOR
	SUB_START(_T("Modify tabs"))
		ITEM(0, _T("Insert tab"))
		ITEM(14,_T("Insert tabs from list"))
		ITEM(26,_T("Insert tabs with icons"))
		SEPARATOR
		ITEM(1, _T("Delete tab"))
		ITEM(2, _T("Delete all tabs"))
		SEPARATOR
		ITEM(4, _T("Set tab caption"))
		ITEM(24, _T("Set tab icon"))
		ITEM(27, _T("Set tab parameter"))
		SEPARATOR
		ITEM(7, _T("Hightlight tab"))
	SUB_END
	SUB_START(_T("Modify icons"))
		ITEM(19,_T("Set icon from active"))
		ITEM(25,_T("Set icons from active (horizontal sequence)"))
		SEPARATOR
		ITEM(18,_T("Set icon from image"))
		ITEM(22,_T("Set icon from overlay"))
		SEPARATOR
		ITEM(20,_T("Remove icon"))
		ITEM(21,_T("Remove all icons"))
	SUB_END
	SEPARATOR
	ITEM(3,_T("Set current tab"))
	ITEM(11,_T("Deselect all"))
	SEPARATOR

#endif

// --------------------------------
// Expression menu
// --------------------------------


#ifdef EXPRESSION_MENU
	SUB_START(_T("Size"))
		ITEM(5,_T("Get width"))
		ITEM(6,_T("Get height"))
	SUB_END
	SEPARATOR
	SUB_START(_T("Styles"))
		ITEM(12,_T("Vertical tabs"))
		ITEM(13,_T("Vertical: right side"))
		ITEM(7,_T("Tabs at bottom"))
		SEPARATOR
		ITEM(15,_T("Tabs as tabs"))
		ITEM(8,_T("Tabs as buttons"))
		ITEM(10,_T("Flat buttons"))
		SEPARATOR
		ITEM(11,_T("Multi-line"))
		ITEM(14,_T("Single-line"))
		SEPARATOR
		ITEM(9,_T("Fixed tab width"))
		ITEM(16,_T("No tab stretching"))
	SUB_END
	SEPARATOR
	ITEM(0, _T("Get current tab"))
	ITEM(18,_T("Find tab from caption"))
	ITEM(2, _T("Get tab count"))
	ITEM(3, _T("Get row count"))
	SEPARATOR
	ITEM(1, _T("Get tab caption"))
	ITEM(17, _T("Get tab icon"))
	ITEM(23, _T("Get tab parameter"))
	SEPARATOR
	ITEM(19,_T("Get tab X"))
	ITEM(20,_T("Get tab Y"))
	ITEM(21,_T("Get tab width"))
	ITEM(22,_T("Get tab height"))
	SEPARATOR


#endif