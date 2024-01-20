#include "Common.hpp"

#if EditorBuild

void FusionAPI GetObjInfos(mv * mV, EDITDATA * edPtr, TCHAR * ObjName, TCHAR * ObjAuthor,
	TCHAR * ObjCopyright, TCHAR * ObjComment, TCHAR * ObjHttp)
{
#pragma DllExportHint
	Edif::ConvertAndCopyString(ObjAuthor, CurLang["About"]["Author"], MAX_PATH);
	// Could make this auto-replace the year, but only the ext author should be doing that.
	Edif::ConvertAndCopyString(ObjCopyright, CurLang["About"]["Copyright"], MAX_PATH);
	Edif::ConvertAndCopyString(ObjHttp, CurLang["About"]["URL"], MAX_PATH);

	if (mV && mV->IdAppli)
		Edif::ConvertAndCopyString(ObjName, CurLang["About"]["Name"], MAX_PATH);
	else
		Edif::ConvertAndCopyString(ObjName, (*Edif::SDK->json.u.object.values[2].value)["About"]["Name"], MAX_PATH);

	// Allows user to specify a static variable in the object comment.
	// e.g. build number, liblacewing version, etc.
	// Use printf macros as depicted by
	// https://docs.microsoft.com/en-gb/cpp/c-runtime-library/format-specification-syntax-printf-and-wprintf-functions
#ifndef JSON_COMMENT_MACRO
	Edif::ConvertAndCopyString(ObjComment, CurLang["About"]["Comment"], MAX_PATH);
#else
	char buff[MAX_PATH];
	bool bad = false;
	// Prevent crashing. It's not nice code, but it'll stop ext devs getting stuck.
	try {
		if (sprintf_s(buff, MAX_PATH, CurLang["About"]["Comment"], JSON_COMMENT_MACRO) == -1)
			bad = true;
	}
	catch (...) { bad = true; }

	if (bad)
	{
		DarkEdif::MsgBox::Error(_T("JSON error"), _T("%s"), _T("Error in JSON comment macro. Ensure your %'s are escaped (with %%)."));
		strcpy_s(buff, MAX_PATH, CurLang["About"]["Comment"]);
	}

	Edif::ConvertAndCopyString(ObjComment, buff, MAX_PATH);
#endif
}

const TCHAR * FusionAPI GetHelpFileName()
{
#pragma DllExportHint
	static TCHAR TempString[MAX_PATH];
	return Edif::ConvertAndCopyString(TempString, CurLang["About"]["Help"], MAX_PATH);
}

void FusionAPI GetConditionTitle(mv *mV, short code, short param, TCHAR * strBuf, short maxLen)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strBuf, CurLang["Conditions"][code]["Parameters"][param][1], maxLen);
}

void FusionAPI GetActionTitle(mv *mV, short code, short param, TCHAR * strBuf, short maxLen)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strBuf, CurLang["Actions"][code]["Parameters"][param][1], maxLen);
}

#ifndef DARKSCRIPT_EXTENSION
void FusionAPI GetExpressionParam(mv *mV, short code, short param, TCHAR * strBuf, short maxLen)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strBuf, CurLang["Expressions"][code]["Parameters"][param][1], maxLen);
}

void FusionAPI GetExpressionTitle(mv *mV, short code, TCHAR * strBuf, short maxLen)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
	{
		std::string Return = (const char *)CurLang["Expressions"][code]["Title"];
		if (Return.back() != '(')
			Return.push_back('(');
		Edif::ConvertAndCopyString(strBuf, Return.c_str(), maxLen);
	}
}
#endif

void FusionAPI GetConditionString(mv *mV, short code, TCHAR * strPtr, short maxLen)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strPtr, CurLang["Conditions"][code]["Title"], maxLen);
}

void FusionAPI GetActionString(mv *mV, short code, TCHAR * strPtr, short maxLen)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strPtr, CurLang["Actions"][code]["Title"], maxLen);
}

#ifndef DARKSCRIPT_EXTENSION
void FusionAPI GetExpressionString(mv * mV, short code, TCHAR * strPtr, short maxLen)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strPtr, CurLang["Expressions"][code]["Title"], maxLen);
}
#endif

std::int16_t FusionAPI GetConditionCodeFromMenu(mv* mV, short menuId)
{
#pragma DllExportHint
	return menuId - Edif::ConditionID(0);
}

std::int16_t FusionAPI GetActionCodeFromMenu(mv* mV, short menuId)
{
#pragma DllExportHint
	return menuId - Edif::ActionID(0);
}

std::int16_t FusionAPI GetExpressionCodeFromMenu(mv* mV, short menuId)
{
#pragma DllExportHint
	return menuId - Edif::ExpressionID(0);
}

// Define them
HMENU Edif::ActionMenu, Edif::ConditionMenu, Edif::ExpressionMenu;

HMENU Edif::LoadMenuJSON(int BaseID, const json_value &Source, HMENU Parent)
{
	if (!Parent)
		Parent = CreateMenu();

	for (unsigned int i = 0; i < Source.u.object.length; ++ i)
	{
		const json_value &MenuItem = Source[i];

		if (MenuItem.type == json_string)
		{
			if (!_stricmp(MenuItem, "Separator") || !strcmp(MenuItem, "---"))
			{
				AppendMenu(Parent, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
				continue;
			}

			continue;
		}

		if (MenuItem[0].type == json_string && MenuItem[1].type == json_array)
		{
			HMENU SubMenu = CreatePopupMenu();
			LoadMenuJSON(BaseID, MenuItem, SubMenu);

			TCHAR* str = ConvertString(MenuItem[0]);
			AppendMenu(Parent, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT) SubMenu, str);
			FreeString(str);

			continue;
		}

		unsigned int ItemOffset = 0;

		int ID = BaseID + (int) MenuItem[ItemOffset].u.integer;
		TCHAR * Text = ConvertString(MenuItem[ItemOffset + 1]);
		bool Disabled = MenuItem.u.object.length > (ItemOffset + 2) ? ((bool) MenuItem[ItemOffset + 2]) != 0 : false;

		AppendMenu(Parent, (Disabled ? MF_GRAYED | MF_UNCHECKED : 0) | MF_BYPOSITION | MF_STRING, ID, Text);

		FreeString(Text);
	}

	return Parent;
}

void menucpy(HMENU hTargetMenu, HMENU hSourceMenu)
{
	int		NumMenuItems;
	HMENU	hSubMenu;
	TCHAR*	strBuf;

	NumMenuItems = GetMenuItemCount(hSourceMenu);
	strBuf = (TCHAR*)malloc(80 * sizeof(TCHAR));
	for (int n = 0, id; n < NumMenuItems; ++n)
	{
		if (!(id = GetMenuItemID(hSourceMenu, n)))
			AppendMenu(hTargetMenu, MF_SEPARATOR, 0, 0L);
		else
		{
			GetMenuString(hSourceMenu, n, strBuf, 80, MF_BYPOSITION);
			if (id != -1)
				AppendMenu(hTargetMenu, GetMenuState(hSourceMenu, n, MF_BYPOSITION), id, strBuf);
			else
			{
				hSubMenu = CreatePopupMenu();
				AppendMenu(hTargetMenu, MF_POPUP | MF_STRING, (unsigned int)hSubMenu, strBuf);
				menucpy(hSubMenu, GetSubMenu(hSourceMenu, n));
			}
		}
	}
	free(strBuf);
}

HMENU FusionAPI GetConditionMenu(mv* mV, ObjectInfo* oiPtr, EDITDATA* edPtr)
{
#pragma DllExportHint
	// Check compatibility
	if (Edif::IS_COMPATIBLE(mV))
	{
		HMENU Menu = CreatePopupMenu();
		menucpy(Menu, Edif::ConditionMenu);
		return Menu;
	}
	return NULL;
}

HMENU FusionAPI GetActionMenu(mv* mV, ObjectInfo* oiPtr, EDITDATA* edPtr)
{
#pragma DllExportHint
	// Check compatibility
	if (Edif::IS_COMPATIBLE(mV))
	{
		HMENU Menu = CreatePopupMenu();
		menucpy(Menu, Edif::ActionMenu);
		return Menu;
	}
	return NULL;
}

HMENU FusionAPI GetExpressionMenu(mv* mV, ObjectInfo* oiPtr, EDITDATA* edPtr)
{
#pragma DllExportHint
	// Check compatibility
	if (Edif::IS_COMPATIBLE(mV))
	{
		HMENU Menu = CreatePopupMenu();
		menucpy(Menu, Edif::ExpressionMenu);
		return Menu;
	}
	return NULL;
}

void * FusionAPI GetConditionInfos(mv *mV, short code)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
		return Edif::SDK->ConditionInfos[code]->FusionPtr();
	return NULL;
}

void * FusionAPI GetActionInfos(mv * mV, short code)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
		return Edif::SDK->ActionInfos[code]->FusionPtr();
	return NULL;
}

void * FusionAPI GetExpressionInfos(mv * mV, short code)
{
#pragma DllExportHint
	if (Edif::IS_COMPATIBLE(mV))
		return Edif::SDK->ExpressionInfos[code]->FusionPtr();
	return NULL;
}

#ifdef DARKEXT_JSON_FILE_EXTERNAL
void AddDirectory(std::tstring &From, std::tstring &To)
{
	HANDLE fileHandle;
	WIN32_FIND_DATA Filejson;

	fileHandle = FindFirstFile(std::tstring(From + _T("*.*")).c_str(), &Filejson);

	if (fileHandle == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (*Filejson.cFileName == '.')
			continue;

		if ((Filejson.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			std::tstring OldPath = From;
			OldPath += Filejson.cFileName;
			OldPath += _T("\\");

			std::tstring NewPath = To;
			NewPath += Filejson.cFileName;
			NewPath += _T("\\");

			CreateDirectory(NewPath.c_str(), 0);
			AddDirectory(OldPath, NewPath);

			continue;
		}

		CopyFile((From + Filejson.cFileName).c_str(), (To + Filejson.cFileName).c_str(), FALSE);

	} while (FindNextFile(fileHandle, &Filejson));

	FindClose(fileHandle);
}

void FusionAPI PrepareFlexBuild(mv * pMV, EDITDATA * edPtr, const wchar_t * wTempFolder)
{
#pragma DllExportHint
	TCHAR FlashFolderPath[MAX_PATH];
	Edif::GetSiblingPath(FlashFolderPath, _T("Flash"));

	if (!*FlashFolderPath)
		return;

	TCHAR * TempFolder;

	#ifdef _UNICODE
		TempFolder = (TCHAR *)wTempFolder;
	#else
		{	// First call WideCharToMultiByte() without output folder, so the size is returned
			size_t Length = WideCharToMultiByte(CP_ACP, 0, wTempFolder, -1, NULL, 0, 0, 0);
			TempFolder = (char *) alloca(Length + 1);

			// Next call WideCharToMultiByte() with output folder
			WideCharToMultiByte(CP_ACP, 0, wTempFolder, -1, TempFolder, Length, 0, 0);
		}
	#endif

	AddDirectory(std::tstring(FlashFolderPath) + _T("\\"), std::tstring(TempFolder));
}
#endif // DARKEXT_JSON_FILE_EXTERNAL

#endif // EditorBuild
