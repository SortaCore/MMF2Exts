#include "Common.h"

void DLLExport GetObjInfos(mv * mV, EDITDATA * edPtr, TCHAR * ObjName, TCHAR * ObjAuthor,
	TCHAR * ObjCopyright, TCHAR * ObjComment, TCHAR * ObjHttp)
{
#ifndef RUN_ONLY

	Edif::ConvertAndCopyString(ObjAuthor, CurLang["About"]["Author"], MAX_PATH);
	Edif::ConvertAndCopyString(ObjCopyright, CurLang["About"]["Copyright"], MAX_PATH);
	Edif::ConvertAndCopyString(ObjHttp, CurLang["About"]["URL"], MAX_PATH);
	
	if (mV && mV->IdAppli)
		Edif::ConvertAndCopyString(ObjName, CurLang["About"]["Name"], MAX_PATH);
	else
		Edif::ConvertAndCopyString(ObjName, (*SDK->json.u.object.values[2].value)["About"]["Name"], MAX_PATH);

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
		MessageBoxA(NULL, "Error in JSON comment macro. Ensure your %s are escaped (%%).", "DarkEdif error", MB_OK);
		strcpy_s(buff, MAX_PATH, CurLang["About"]["Comment"]);
	}

	Edif::ConvertAndCopyString(ObjComment, buff, MAX_PATH);
#endif

	#endif // !defined(RUN_ONLY)
}

const TCHAR * WINAPI GetHelpFileName()
{
	#ifdef RUN_ONLY
		return NULL;
	#else // !RUN_ONLY
		static TCHAR TempString[MAX_PATH];
		return Edif::ConvertAndCopyString(TempString, CurLang["About"]["Help"], MAX_PATH);
	#endif
}

void DLLExport GetConditionTitle(mv *mV, short code, short param, TCHAR * strBuf, short maxLen)
{
	if (IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strBuf, CurLang["Conditions"][code]["Parameters"][param][1], maxLen);
}

void DLLExport GetActionTitle(mv *mV, short code, short param, TCHAR * strBuf, short maxLen)
{
	if (IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strBuf, CurLang["Actions"][code]["Parameters"][param][1], maxLen);
}

void DLLExport GetExpressionParam(mv *mV, short code, short param, TCHAR * strBuf, short maxLen)
{
	if (IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strBuf, CurLang["Expressions"][code]["Parameters"][param][1], maxLen);
}

void DLLExport GetExpressionTitle(mv *mV, short code, TCHAR * strBuf, short maxLen)
{
	if (IS_COMPATIBLE(mV))
	{
		std::string Return = CurLang["Expressions"][code]["Title"];
		if (Return.back() != '(')
			Return.push_back('(');
		Edif::ConvertAndCopyString(strBuf, Return.c_str(), maxLen);
	}
}

void DLLExport GetConditionString(mv *mV, short code, TCHAR * strPtr, short maxLen)
{
	if (IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strPtr, CurLang["Conditions"][code]["Title"], maxLen);
}

void DLLExport GetActionString(mv *mV, short code, TCHAR * strPtr, short maxLen)
{
	if (IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strPtr, CurLang["Actions"][code]["Title"], maxLen);
}

void DLLExport GetExpressionString(mv * mV, short code, TCHAR * strPtr, short maxLen)
{
	if (IS_COMPATIBLE(mV))
		Edif::ConvertAndCopyString(strPtr, CurLang["Expressions"][code]["Title"], maxLen);
}

void * DLLExport GetConditionInfos(mv *mV, short code)
{
	if (IS_COMPATIBLE(mV))
		return ::SDK->ConditionInfos[code]->MMFPtr();
	return NULL;
}

void * DLLExport GetActionInfos(mv * mV, short code)
{
	if (IS_COMPATIBLE(mV))
		return ::SDK->ActionInfos[code]->MMFPtr();
	return NULL;
}

void * DLLExport GetExpressionInfos(mv * mV, short code)
{
	if (IS_COMPATIBLE(mV))
		return ::SDK->ExpressionInfos[code]->MMFPtr();
	return NULL;
}

short DLLExport GetConditionCodeFromMenu(mv * mV, short menuId)
{
	#ifndef RUN_ONLY
		return menuId - Edif::ConditionID(0);
	#endif // !defined(RUN_ONLY)
	
	return -1;
}

short DLLExport GetActionCodeFromMenu(mv * mV, short menuId)
{
	#ifndef RUN_ONLY
		return menuId - Edif::ActionID(0);
	#endif // !defined(RUN_ONLY)

	return -1;
}

short DLLExport GetExpressionCodeFromMenu(mv *mV, short menuId)
{
	#ifndef RUN_ONLY
		return menuId - Edif::ExpressionID(0);
	#endif // !defined(RUN_ONLY)

	return -1;
}

void menucpy(HMENU hTargetMenu, HMENU hSourceMenu)
{
	#ifndef RUN_ONLY
		int			NumMenuItems;
		TCHAR *		strBuf;
		HMENU		hSubMenu;
	
		NumMenuItems = GetMenuItemCount(hSourceMenu);
		strBuf = (TCHAR *)calloc(80, sizeof(TCHAR));
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
	#endif
}

HMENU DLLExport GetConditionMenu(mv * mV, ObjectInfo * oiPtr, EDITDATA * edPtr)
{
	// Check compatibility
	if ( IS_COMPATIBLE(mV) )
	{
		HMENU Menu = CreatePopupMenu();
		menucpy(Menu, Edif::ConditionMenu);
		return Menu;
	}
	return NULL;
}

HMENU DLLExport GetActionMenu(mv * mV, ObjectInfo * oiPtr, EDITDATA * edPtr)
{
	// Check compatibility
	if ( IS_COMPATIBLE(mV) )
	{
		HMENU Menu = CreatePopupMenu();
		menucpy(Menu, Edif::ActionMenu);
		return Menu;
	}
	return NULL;
}

HMENU DLLExport GetExpressionMenu(mv * mV, ObjectInfo * oiPtr, EDITDATA * edPtr)
{
	// Check compatibility
	if ( IS_COMPATIBLE(mV) )
	{
		HMENU Menu = CreatePopupMenu();
		menucpy(Menu, Edif::ExpressionMenu);
		return Menu;
	}
	return NULL;
}

#ifdef _UNICODE
	#define _tstring wstring
#else
	#define _tstring string
#endif

void AddDirectory(std::_tstring &From, std::_tstring &To)
{
	HANDLE fileHandle;
	WIN32_FIND_DATA Filejson;

	fileHandle = FindFirstFile(std::_tstring(From + _T("*.*")).c_str(), &Filejson);

	if (fileHandle == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (*Filejson.cFileName == '.')
			continue;

		if ((Filejson.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			std::_tstring OldPath = From;
			OldPath += Filejson.cFileName;
			OldPath += _T("\\");
			
			std::_tstring NewPath = To;
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

void WINAPI PrepareFlexBuild(mv * pMV, EDITDATA * edPtr, const wchar_t * wTempFolder)
{
	#ifndef RUN_ONLY
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

		AddDirectory(std::_tstring(FlashFolderPath) + _T("\\"), std::_tstring(TempFolder));
	#endif // !RUN_ONLY
}
