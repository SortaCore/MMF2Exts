Extension.cpp:
In Extension::Extension(), we register the functions associated with the Action/Expression IDs.
In Extension::Handle(), which is called every frame, we modify the attached objects.

Actions.cpp:
Contains the attach action.

Expressions.cpp:
Contains the count expression.

Ext.json:
Contains the menu structure for the Actions etc.

Extension.h:
Contains the declarations of the action functions etc. and the AttachedObjects vector.