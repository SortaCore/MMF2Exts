#include "Common.hpp"
#include <sstream>

typedef HMENU (FusionAPI * funcFormat)(mv * mV, ObjectInfo * oiPtr, EDITDATA * edPtr);

struct DarkMenuItem
{
	int index;
	int id;
	std::tstring name;
	DarkMenuItem(int index_, int id_)
		: index(index_), id(id_)
	{

	}
};

bool MenuToString(HMENU& menu, std::tstring & output, std::tstring prefix = std::tstring(), int startIndex = 0)
{
	const int numItems = GetMenuItemCount(menu);
	if (numItems < 0)
	{
		output = _T("Num items smaller than 0");
		return false;
	}
	if (numItems == 0)
	{
		output = _T("(no items in menu)");
		return true;
	}
	std::vector<std::unique_ptr<DarkMenuItem>> menuItems;
	MENUITEMINFO itemData = { };

	for (int i = 0; i < numItems; ++i)
		menuItems.push_back(std::make_unique<DarkMenuItem>(i, GetMenuItemID(menu, i)));

	TCHAR * buffer = (TCHAR *)malloc(1024U * sizeof(TCHAR));
	bool happy = false;
	std::tstringstream outputStr;
	if (buffer == nullptr) {
		outputStr << _T("Failed to allocate initial name buffer.");
		goto end;
	}

	for (int i = 0; i < numItems; ++i)
	{
		menuItems[i]->id = GetMenuItemID(menu, menuItems[i]->index);

		itemData = { 0 };
		itemData.cbSize = sizeof(MENUITEMINFO);
		itemData.fMask = MIIM_TYPE | MIIM_SUBMENU;
		itemData.fType = MIIM_FTYPE;
		itemData.wID = menuItems[i]->id;
		if (GetMenuItemInfo(menu, i, true, &itemData) == FALSE)
		{
			outputStr.clear(); outputStr.str(std::tstring());
			outputStr << _T("Failed to read name buffer size for item ID ") <<
				menuItems[i]->id << _T(" (index ") << menuItems[i]->index << _T("). Error ")
				<< GetLastError() << _T(".\r\n");
			goto end;
		}

		if (itemData.fType == MFT_SEPARATOR)
		{
			outputStr << std::tstring(
				// Add spaces so the separator lines up with the submenu
				// Since we pad 1-digit to 2 digits, we only have 3 or 2 digits.
				// +2 for the ": " after the ID
				(startIndex + i >= 100 ? 3 : 2) + 2
				, _T(' ')) << prefix << _T("-----------\r\n");
			continue;
		}
		bool isSubmenu = itemData.hSubMenu != nullptr;
		if (itemData.fType != MFT_STRING && !isSubmenu)
		{
			outputStr << prefix << _T("Item ID ") << menuItems[i]->id << _T(" (index ") << menuItems[i]->index <<
				_T("). FTYPE ") << itemData.fType << _T(" not matching, skipping.\r\n");
			continue;
		}

		itemData.fMask = MIIM_STRING | MIIM_SUBMENU;
		itemData.fType = MIIM_STRING;
		itemData.wID = menuItems[i]->id;
		itemData.dwTypeData = nullptr;

		if (GetMenuItemInfo(menu, i, TRUE, &itemData) == FALSE)
		{
			outputStr.clear(); outputStr.str(std::tstring());
			outputStr << prefix << _T("Failed to read name for item ID ") <<
				menuItems[i]->id << _T(" (index ") << menuItems[i]->index << _T("). Error ")
				<< GetLastError() << _T(".\r\n");
			goto end;
		}

		if (itemData.cch < 1)
		{
			outputStr << prefix << _T("Length is <1. Skipping.\r\n");
			continue;
		}

		buffer = (TCHAR *)realloc(buffer, ++itemData.cch);
		if (buffer == nullptr)
		{
			outputStr.clear(); outputStr.str(std::tstring());
			outputStr << _T("Failed to allocate name buffer size ") << itemData.cch << _T(" for item ID ") <<
				menuItems[i]->id << _T(" (index ") << menuItems[i]->index << _T(")\r\n");
			goto end;
		}

		itemData.dwTypeData = buffer;
		memset(buffer, 0, itemData.cch);
		itemData.wID = menuItems[i]->id;

		if (GetMenuItemInfo(menu, i, TRUE, &itemData) == FALSE)
		{
			outputStr.clear(); outputStr.str(std::tstring());
			outputStr << _T("Failed to read name for item ID ") <<
				menuItems[i]->id << _T(" (index ") << menuItems[i]->index << _T("). Error ")
				<< GetLastError() << _T(".\r\n");
			goto end;
		}
		menuItems[i]->name = buffer;

		if (isSubmenu)
		{
			if (!MenuToString(itemData.hSubMenu, output, prefix + menuItems[i]->name + _T(" > "), startIndex + i))
			{
				outputStr << prefix << _T("Submenu at index ") << menuItems[i]->index <<
					_T(") couldn't load properly, error ") << output << _T(".\r\n");
				goto end;
			}
			outputStr << output;
			continue;
		}
		else
		{
			if (menuItems[i]->id % 1000 < 10)
				outputStr << _T("0");
			outputStr << (menuItems[i]->id % 1000) << _T(": ") << prefix << menuItems[i]->name << _T("\r\n");
		}
	}

	happy = true;
end:
	output = outputStr.str();
	menuItems.clear();

	DestroyMenu(menu);
	menu = nullptr;

	free(buffer);
	buffer = nullptr;

	return happy;
}

void Extension::SetMFXFile(const TCHAR * filename_)
{
	filename = filename_;

	HINSTANCE hGetProcIDDLL = LoadLibrary(filename.c_str());
	error.clear();
	error.str(std::tstring());
	menuStr[0].clear();
	menuStr[1].clear();
	menuStr[2].clear();

	if (!hGetProcIDDLL)
	{
		error << _T("Could not load the dynamic library a.k.a. MFX, error ") << GetLastError();
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
	for (std::size_t i = 0; i < 3; ++i)
	{
		funcs[i] = (funcFormat)GetProcAddress(hGetProcIDDLL, funcNames[i]);
		if (!funcs[i])
		{
			error << _T("Could not locate function ") << DarkEdif::ANSIToTString(funcNames[i]);
			return;
		}
	}
	for (std::size_t i = 0; i < 3; ++i)
	{
		menus[i] = funcs[i](Edif::SDK->mV, nullptr, nullptr);
		if (!menus[i])
		{
			error << DarkEdif::ANSIToTString(funcNames[i]) << _T(" returned nullptr");
			return;
		}
	}

	for (std::size_t i = 0; i < 3; ++i)
	{
		if (!MenuToString(menus[i], menuStr[i]))
		{
			error << _T("Error: ") << menuStr[i];
			return;
		}
	}

	if (FreeLibrary(hGetProcIDDLL) == FALSE)
		error << _T("Error: FreeLibrary failed.");
}
