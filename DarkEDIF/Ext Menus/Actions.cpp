
#include "Common.h"
#include <sstream>

typedef HMENU (DLLExport * funcFormat)(mv * mV, ObjectInfo * oiPtr, EDITDATA * edPtr);

struct DarkMenuItem
{
	int index;
	int id;
	std::string name;
	DarkMenuItem(int index_, int id_)
		: index(index_), id(id_)
	{

	}
};

bool MenuToString(HMENU& menu, std::string & output, std::string prefix = "", int startIndex = 0)
{
	const int numItems = GetMenuItemCount(menu);
	if (numItems < 0)
	{
		output = "Num items smaller than 0";
		return false;
	}
	if (numItems == 0)
	{
		output = "(no items in menu)";
		return false;
	}
	std::vector<DarkMenuItem *> menuItems;
	MENUITEMINFOA itemData = { 0 };
	
	for (size_t i = 0; i < numItems; i++)
		menuItems.push_back(new DarkMenuItem(i, GetMenuItemID(menu, i)));
	
	char * buffer = (char *)malloc(1024U);
	bool happy = false;
	std::stringstream outputStr;
	if (buffer == nullptr) {
		outputStr << "Failed to allocate initial name buffer.";
		goto end;
	}

	for (size_t i = 0; i < numItems; i++)
	{
		menuItems[i]->id = GetMenuItemID(menu, menuItems[i]->index);

		itemData = { 0 };
		itemData.cbSize = sizeof(MENUITEMINFOA);
		itemData.fMask = MIIM_TYPE | MIIM_SUBMENU;
		itemData.fType = MIIM_FTYPE;
		itemData.wID = menuItems[i]->id;
		if (GetMenuItemInfoA(menu, i, true, &itemData) == FALSE)
		{
			outputStr.clear(); outputStr.str("");
			outputStr << "Failed to read name buffer size for item ID " << 
				menuItems[i]->id << " (index " << menuItems[i]->index << "). Error "
				<< GetLastError() << ".\r\n";
			goto end;
		}

		if (itemData.fType == MFT_SEPARATOR)
		{
			outputStr << std::string(
				// Add spaces so the separator lines up with the submenu
				// Since we pad 1-digit to 2 digits, we only have 3 or 2 digits.
				// +2 for the ": " after the ID
				(startIndex + i >= 100 ? 3 : 2) + 2
				, ' ') << prefix << "-----------\r\n";
			continue;
		}
		bool isSubmenu = itemData.hSubMenu != nullptr;
		if (itemData.fType != MFT_STRING && !isSubmenu)
		{
			outputStr << prefix << "Item ID " << menuItems[i]->id << " (index " << menuItems[i]->index <<
				"). FTYPE " << itemData.fType << " not matching, skipping.\r\n";
			continue;
		}

		itemData.fMask = MIIM_STRING | MIIM_SUBMENU;
		itemData.fType = MIIM_STRING;
		itemData.wID = menuItems[i]->id;
		itemData.dwTypeData = nullptr;

		if (GetMenuItemInfoA(menu, i, TRUE, &itemData) == FALSE)
		{
			outputStr.clear(); outputStr.str("");
			outputStr << prefix << "Failed to read name for item ID " <<
				menuItems[i]->id << " (index " << menuItems[i]->index << "). Error "
				<< GetLastError() << ".\r\n";
			goto end;
		}

		if (itemData.cch < 1)
		{
			outputStr << prefix << "Length is <1. Skipping.\r\n";
			continue;
		}

		buffer = (char *)realloc(buffer, ++itemData.cch);
		if (buffer == nullptr)
		{
			outputStr.clear(); outputStr.str("");
			outputStr << "Failed to allocate name buffer size " << itemData.cch << " for item ID " <<
				menuItems[i]->id << " (index " << menuItems[i]->index << ")\r\n";
			goto end;
		}

		itemData.dwTypeData = buffer;
		memset(buffer, 0, itemData.cch);
		itemData.wID = menuItems[i]->id;

		if (GetMenuItemInfoA(menu, i, TRUE, &itemData) == FALSE)
		{
			outputStr.clear(); outputStr.str("");
			outputStr << "Failed to read name for item ID " <<
				menuItems[i]->id << " (index " << menuItems[i]->index << "). Error "
				<< GetLastError() << ".\r\n";
			goto end;
		}
		menuItems[i]->name = buffer;

		if (isSubmenu)
		{
			if (!MenuToString(itemData.hSubMenu, output, prefix + menuItems[i]->name + " > ", startIndex + i))
			{
				outputStr << prefix << "Submenu at index " << menuItems[i]->index <<
					") couldn't load properly, error " << output << ".\r\n";
				goto end;
			}
			outputStr << output;
			continue;
		}
		else
		{
			if (menuItems[i]->id % 1000 < 10)
				outputStr << "0";
			outputStr << (menuItems[i]->id % 1000) << ": " << prefix << menuItems[i]->name << "\r\n";
		}
	}
	
	happy = true;
end:
	output = outputStr.str();

	for (size_t i = 0; i < menuItems.size(); i++)
		delete menuItems[i];
	menuItems.clear();

	DestroyMenu(menu);
	menu = nullptr;

	free(buffer);
	buffer = nullptr;

	return happy;
}

void Extension::SetMFXFile(char * filename_)
{
	filename = filename_;

	HINSTANCE hGetProcIDDLL = LoadLibraryA(filename.c_str());
	error.clear();
	error.str("");
	menuStr[0] = "";
	menuStr[1] = "";
	menuStr[2] = "";

	if (!hGetProcIDDLL) {
		error << "could not load the dynamic library";
		return;
	}
	// GetActionMenu, GetConditionMenu, GetExpressionMenu
	// resolve function address here
	const char * funcNames[]
	{
		"GetActionMenu",
		"GetConditionMenu",
		"GetExpressionMenu"
	};
	funcFormat funcs[3];
	HMENU menus[3];
	for (size_t i = 0; i < 3; i++)
	{
		funcs[i] = (funcFormat)GetProcAddress(hGetProcIDDLL, funcNames[i]);
		if (!funcs[i]) {
			error << "Could not locate function " << funcNames[i];
			return;
		}
	}
	for (size_t i = 0; i < 3; i++)
	{
		menus[i] = funcs[i](SDK->mV, nullptr, nullptr);
		if (!menus[i]) {
			error << funcNames[i] << " returned nullptr";
			return;
		}
	}

	for (size_t i = 0; i < 3; i++)
	{
		if (!MenuToString(menus[i], menuStr[i])) {
			error << "Error: " << menuStr[i];
			return;
		}
	}

	if (FreeLibrary(hGetProcIDDLL) == FALSE)
		error << "Error: FreeLibrary failed.";
}
