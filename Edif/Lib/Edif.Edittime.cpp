
#include "Common.h"

void WINAPI	DLLExport GetObjInfos (mv _far *mV, void *, LPTSTR ObjName, LPTSTR ObjAuthor,
											LPTSTR ObjCopyright, LPTSTR ObjComment, LPTSTR ObjHttp)
{
#ifndef RUN_ONLY

	const json_value &Properties = SDK->json["About"];

	Edif::ConvertAndCopyString(ObjAuthor,		Properties["Author"],	MAX_PATH);
	Edif::ConvertAndCopyString(ObjCopyright,	Properties["Copyright"],MAX_PATH);
	Edif::ConvertAndCopyString(ObjComment,	  Properties["Comment"],	MAX_PATH);
	Edif::ConvertAndCopyString(ObjHttp,		 Properties["URL"],		MAX_PATH);
	Edif::ConvertAndCopyString(ObjName,		 Properties["Name"],		MAX_PATH);


#endif // !defined(RUN_ONLY)
}

LPCTSTR WINAPI GetHelpFileName()
{
#ifndef RUN_ONLY
	// Return a file without path if your help file can be loaded by the MMF help file.
//	return "MyExt.chm";

	// Or return the path of your file, relatively to the MMF directory
	// if your file is not loaded by the MMF help file.
	static TCHAR TempString[MAX_PATH];		// used by GetHelpFileName()
	return Edif::ConvertAndCopyString(TempString, SDK->json["About"]["Help"], MAX_PATH);
#else
	return NULL;
#endif // !defined(RUN_ONLY)
}

void WINAPI DLLExport GetConditionTitle(mv _far *mV, short code, short param, LPTSTR strBuf, short maxLen)
{
#ifndef RUN_ONLY

	if (!IS_COMPATIBLE(mV))
		return;

	const json_value &Parameter = SDK->json["Conditions"][code]["Parameters"][param];

	if (Parameter.type == json_object)
		Edif::ConvertAndCopyString(strBuf, Parameter["Title"], maxLen);
	else
		Edif::ConvertAndCopyString(strBuf, Parameter[1], maxLen);

#endif // !defined(RUN_ONLY)
}

void WINAPI DLLExport GetActionTitle(mv _far *mV, short code, short param, LPTSTR strBuf, short maxLen)
{
#ifndef RUN_ONLY

	if (!IS_COMPATIBLE(mV))
		return;

	const json_value &Parameter = SDK->json["Actions"][code]["Parameters"][param];

	if (Parameter.type == json_object)
		Edif::ConvertAndCopyString(strBuf, Parameter["Title"], maxLen);
	else
		Edif::ConvertAndCopyString(strBuf, Parameter[1], maxLen);

#endif // !defined(RUN_ONLY)
}


void WINAPI DLLExport GetExpressionParam(mv _far *mV, short code, short param, LPTSTR strBuf, short maxLen)
{
#ifndef RUN_ONLY

	if (!IS_COMPATIBLE(mV))
		return;

	const json_value &Parameter = SDK->json["Expressions"][code]["Parameters"][param];

	if (Parameter.type == json_object)
		Edif::ConvertAndCopyString(strBuf, Parameter["Title"], maxLen);
	else
		Edif::ConvertAndCopyString(strBuf, Parameter[1], maxLen);


#endif // !defined(RUN_ONLY)
}


void WINAPI DLLExport GetExpressionTitle(mv _far *mV, short code, LPTSTR strBuf, short maxLen)
{
#ifndef RUN_ONLY

	if (!IS_COMPATIBLE(mV))
		return;

	Edif::ConvertAndCopyString(strBuf, SDK->json["Expressions"][code]["Title"], maxLen);

#endif // !defined(RUN_ONLY)
}

void WINAPI DLLExport GetConditionString(mv _far *mV, short code, LPTSTR strPtr, short maxLen)
{
#ifndef RUN_ONLY

	if (!IS_COMPATIBLE(mV))
		return;

	Edif::ConvertAndCopyString(strPtr, SDK->json["Conditions"][code]["Title"], maxLen);

#endif // !defined(RUN_ONLY)
}

void WINAPI DLLExport GetActionString(mv _far *mV, short code, LPTSTR strPtr, short maxLen)
{
#ifndef RUN_ONLY

	if (!IS_COMPATIBLE(mV))
		return;

	Edif::ConvertAndCopyString(strPtr, SDK->json["Actions"][code]["Title"], maxLen);

#endif // !defined(RUN_ONLY)
}

void WINAPI DLLExport GetExpressionString(mv _far *mV, short code, LPTSTR strPtr, short maxLen)
{
#ifndef RUN_ONLY

	if (!IS_COMPATIBLE(mV))
		return;

	Edif::ConvertAndCopyString(strPtr, SDK->json["Expressions"][code]["Title"], maxLen);

#endif // !defined(RUN_ONLY)
}

LPINFOEVENTSV2 WINAPI DLLExport GetConditionInfos(mv _far *mV, short code)
{
#ifndef RUN_ONLY

	if (IS_COMPATIBLE(mV))
		return &GetEventInformations((LPEVENTINFOS2) &SDK->ConditionInfos[0], code)->infos;

#endif // !defined(RUN_ONLY)
	return NULL;
}

LPINFOEVENTSV2 WINAPI DLLExport GetActionInfos(mv _far *mV, short code)
{
#ifndef RUN_ONLY

	if (IS_COMPATIBLE(mV))
		return &GetEventInformations((LPEVENTINFOS2) &SDK->ActionInfos[0], code)->infos;

#endif // !defined(RUN_ONLY)
	return NULL;
}

LPINFOEVENTSV2 WINAPI DLLExport GetExpressionInfos(mv _far *mV, short code)
{
#ifndef RUN_ONLY

	if (IS_COMPATIBLE(mV))
		return &GetEventInformations((LPEVENTINFOS2) &SDK->ExpressionInfos[0], code)->infos;

#endif // !defined(RUN_ONLY)
	return NULL;
}

short WINAPI DLLExport GetConditionCodeFromMenu(mv _far *mV, short menuId)
{
#ifndef RUN_ONLY

	return menuId - Edif::ConditionID(0);

#endif // !defined(RUN_ONLY)
	return -1;
}

short WINAPI DLLExport GetActionCodeFromMenu(mv _far *mV, short menuId)
{
#ifndef RUN_ONLY

	return menuId - Edif::ActionID(0);

#endif // !defined(RUN_ONLY)
	return -1;
}

short WINAPI DLLExport GetExpressionCodeFromMenu(mv _far *mV, short menuId)
{
#ifndef RUN_ONLY

	return menuId - Edif::ExpressionID(0);

#endif // !defined(RUN_ONLY)
	return -1;
}

void menucpy(HMENU hTargetMenu, HMENU hSourceMenu)
{
#ifndef RUN_ONLY
	int			n, id, nMn;
	LPTSTR		strBuf;
	HMENU		hSubMenu;

	nMn = GetMenuItemCount(hSourceMenu);
	strBuf = (LPTSTR)calloc(80, sizeof(TCHAR));
	for (n = 0; n < nMn; n++)
	{
		if (0 == (id = GetMenuItemID(hSourceMenu, n)))
			AppendMenu(hTargetMenu, MF_SEPARATOR, 0, 0L);
		else
		{
			GetMenuString(hSourceMenu, n, strBuf, 80, MF_BYPOSITION);
			if (id != -1)
				AppendMenu(hTargetMenu, GetMenuState(hSourceMenu, n, MF_BYPOSITION), id, strBuf);
			else
			{
				hSubMenu = CreatePopupMenu();
				AppendMenu(hTargetMenu, MF_POPUP | MF_STRING, (uint)hSubMenu, strBuf);
				menucpy(hSubMenu, GetSubMenu(hSourceMenu, n));
			}
		}
	}
	free(strBuf);
#endif
}

HMENU WINAPI DLLExport GetConditionMenu(mv _far *mV, fpObjInfo oiPtr, LPEDATA edPtr)
{
#ifndef RUN_ONLY
	// Check compatibility
	if ( IS_COMPATIBLE(mV) )
	{
		HMENU Menu = CreatePopupMenu();
		menucpy(Menu, Edif::ConditionMenu);
		return Menu;
	}
#endif // !defined(RUN_ONLY)
	return NULL;
}

HMENU WINAPI DLLExport GetActionMenu(mv _far *mV, fpObjInfo oiPtr, LPEDATA edPtr)
{
#ifndef RUN_ONLY
	// Check compatibility
	if ( IS_COMPATIBLE(mV) )
	{
		HMENU Menu = CreatePopupMenu();
		menucpy(Menu, Edif::ActionMenu);
		return Menu;
	}
#endif // !defined(RUN_ONLY)
	return NULL;
}

HMENU WINAPI DLLExport GetExpressionMenu(mv _far *mV, fpObjInfo oiPtr, LPEDATA edPtr)
{
#ifndef RUN_ONLY
	// Check compatibility
	if ( IS_COMPATIBLE(mV) )
	{
		HMENU Menu = CreatePopupMenu();
		menucpy(Menu, Edif::ExpressionMenu);
		return Menu;
	}
#endif // !defined(RUN_ONLY)
	return NULL;
}

#ifdef _UNICODE
#define _tstring wstring
#else
#define _tstring string
#endif

void AddDirectory(_tstring &From, _tstring &To)
{
	HANDLE FileHandle;
	WIN32_FIND_DATA Filejson;

	_tstring SearchString;

	SearchString += From;
	SearchString += _T("*.*");

	FileHandle = FindFirstFile(SearchString.c_str(), &Filejson);

	if (FileHandle == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (*Filejson.cFileName == '.')
			continue;

		if ((Filejson.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			_tstring OldPath;

			OldPath += From;
			OldPath += Filejson.cFileName;
			OldPath += _T("\\");

			_tstring NewPath;

			NewPath += To;
			NewPath += Filejson.cFileName;
			NewPath += _T("\\");

			CreateDirectory(NewPath.c_str(), 0);
			AddDirectory(OldPath, NewPath);

			continue;
		}

		CopyFile((From + Filejson.cFileName).c_str(), (To + Filejson.cFileName).c_str(), FALSE);

	} while (FindNextFile(FileHandle, &Filejson));

	FindClose(FileHandle);
}

void WINAPI PrepareFlexBuild(LPMV pMV, LPEDATA edPtr, LPCWSTR wTempFolder)
{
#if !defined(RUN_ONLY)

	TCHAR FlashFolderPath[MAX_PATH];
	Edif::GetSiblingPath(FlashFolderPath, _T("Flash"));

	if (!*FlashFolderPath)
		return;

	LPTSTR TempFolder;

#ifdef _UNICODE
	TempFolder = (LPTSTR)wTempFolder;
#else
	{	size_t Length = WideCharToMultiByte(CP_ACP, 0, wTempFolder, -1, 0, 0, 0, 0);
		TempFolder = (LPSTR) alloca(Length + 1);

		WideCharToMultiByte(CP_ACP, 0, wTempFolder, -1, TempFolder, Length, 0, 0);
	}
#endif

	AddDirectory(((_tstring) FlashFolderPath + _T("\\")), (_tstring) TempFolder);

#endif // !defined(RUN_ONLY)
}
