// --------------------------------
// Condition menu
// --------------------------------

#ifdef CONDITION_MENU

	SEPARATOR
	ITEM(0,"Tab changed ?")
	ITEM(6,"Tab changed to (by caption) ?")
	ITEM(7,"Tab changed to (by index) ?")
	SEPARATOR	
	ITEM(5,"Tab changed by user ?")
	SEPARATOR
	ITEM(1,"LMB clicked ?")
	ITEM(2,"LMB double-clicked ?")
	ITEM(3,"RMB clicked ?")
	ITEM(4,"RMB double-clicked ?")

#endif

// --------------------------------
// Action menu
// --------------------------------

#ifdef ACTION_MENU
	SEPARATOR
	SUB_START("Window settings")
		ITEM(5,"Set width")
		ITEM(15,"Set height")
		ITEM(6,"Set visibility")
		ITEM(16,"Set style")
		SEPARATOR
		ITEM(23,"Force redraw")
	SUB_END
		SUB_START("Tab settings")
		ITEM(8, "Set minimal tab width")
		ITEM(10, "Set tab size")
		ITEM(9, "Set padding")
	SUB_END
	SEPARATOR
	SUB_START("Modify tabs")
		ITEM(0, "Insert tab")
		ITEM(14,"Insert tabs from list")
		ITEM(26,"Insert tabs with icons")
		SEPARATOR
		ITEM(1, "Delete tab")
		ITEM(2, "Delete all tabs")
		SEPARATOR
		ITEM(4, "Set tab caption")
		ITEM(24, "Set tab icon")
		ITEM(27, "Set tab parameter")
		SEPARATOR
		ITEM(7, "Hightlight tab")
	SUB_END
	SUB_START("Modify icons")
		ITEM(19,"Set icon from active")
		ITEM(25,"Set icons from active (horizontal sequence)")
		SEPARATOR
		ITEM(18,"Set icon from image")
		ITEM(22,"Set icon from overlay")
		SEPARATOR
		ITEM(20,"Remove icon")
		ITEM(21,"Remove all icons")
	SUB_END
	SEPARATOR
	ITEM(3,"Set current tab")
	ITEM(11,"Deselect all")
	SEPARATOR

#endif

// --------------------------------
// Expression menu
// --------------------------------


#ifdef EXPRESSION_MENU
	SUB_START("Size")
		ITEM(5,"Get width")
		ITEM(6,"Get height")
	SUB_END
	SEPARATOR
	SUB_START("Styles")
		ITEM(12,"Vertical tabs")
		ITEM(13,"Vertical: right side")
		ITEM(7,"Tabs at bottom")
		SEPARATOR
		ITEM(15,"Tabs as tabs")
		ITEM(8,"Tabs as buttons")
		ITEM(10,"Flat buttons")
		SEPARATOR
		ITEM(11,"Multi-line")
		ITEM(14,"Single-line")
		SEPARATOR
		ITEM(9,"Fixed tab width")
		ITEM(16,"No tab stretching")
	SUB_END
	SEPARATOR
	ITEM(0, "Get current tab")
	ITEM(18,"Find tab from caption")
	ITEM(2, "Get tab count")
	ITEM(3, "Get row count")
	SEPARATOR
	ITEM(1, "Get tab caption")
	ITEM(17, "Get tab icon")
	ITEM(23, "Get tab parameter")
	SEPARATOR
	ITEM(19,"Get tab X")
	ITEM(20,"Get tab Y")
	ITEM(21,"Get tab width")
	ITEM(22,"Get tab height")
	SEPARATOR


#endif