
// ============================================================================
//
// The following routines are used internally by MMF, and should not need to
// be modified.
//
// 
// ============================================================================

// Common Include
#include	"common.h"

HINSTANCE hInstLib;

// ============================================================================
//
// LIBRARY ENTRY & QUIT POINTS
// 
// ============================================================================

// -----------------
// Entry points
// -----------------
// Usually you do not need to do any initialization here: you will prefer to 
// do them in "Initialize" found in Edittime.cpp
BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		// DLL is attaching to the address space of the current process.
		case DLL_PROCESS_ATTACH:
			
			hInstLib = hDLL; // Store HINSTANCE
			break;

		// A new thread is being created in the current process.
		case DLL_THREAD_ATTACH:
			break;

		// A thread is exiting cleanly.
		case DLL_THREAD_DETACH:
			break;

		// The calling process is detaching the DLL from its address space.
	    case DLL_PROCESS_DETACH:
			break;
	}
	
	return TRUE;
}


// ============================================================================
//
// ROUTINES USED UNDER FRAME EDITOR
// 
// ============================================================================

// -----------------
// Get Infos
// -----------------
// You should not change this routine.
// 
extern "C" 
{
	DWORD WINAPI DLLExport GetInfos(int info)
	{
		
		switch (info)
		{
			case KGI_VERSION:
				return EXT_VERSION2;
			case KGI_PLUGIN:
				return EXT_PLUGIN_VERSION1;
			default:
				return 0;
		}
	}
}

// ============================================================================
//
// ROUTINES USED UNDER EVENT / TIME / STEP-THROUGH EDITOR
// You should not need to change these routines
// 
// ============================================================================

// ----------------------------------------------------------
// GetRunObjectInfos
// ----------------------------------------------------------
// Fills an information structure that tells CC&C everything
// about the object, its actions, conditions and expressions
// 

short WINAPI DLLExport GetRunObjectInfos(mv _far *knpV, fpKpxRunInfos infoPtr)
{            

	infoPtr->conditions = (LPBYTE)ConditionJumps;
	infoPtr->actions = (LPBYTE)ActionJumps;
	infoPtr->expressions = (LPBYTE)ExpressionJumps;

	infoPtr->numOfConditions = CND_LAST;
	infoPtr->numOfActions = ACT_LAST;
	infoPtr->numOfExpressions = EXP_LAST;

	infoPtr->editDataSize = MAX_EDITSIZE;
	infoPtr->editFlags= OEFLAGS;

	infoPtr->windowProcPriority = WINDOWPROC_PRIORITY;

	// See doc
	infoPtr->editPrefs = OEPREFS;

	// Identifier, for run-time identification
	infoPtr->identifier = IDENTIFIER;
	
	// Current version
	infoPtr->version = KCX_CURRENT_VERSION;
	
	return TRUE;
}

#ifndef RUN_ONLY
// -----------------
// menucpy
// -----------------
// Internal routine used later, copy one menu onto another
// 
void menucpy(HMENU hTargetMenu, HMENU hSourceMenu)
{
	int			n, id, nMn;
	NPSTR		strBuf;
	HMENU		hSubMenu;

	nMn = GetMenuItemCount(hSourceMenu);
	strBuf = (NPSTR)LocalAlloc(LPTR, 80);
	for (n=0; n<nMn; n++)
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
	LocalFree((HLOCAL)strBuf);
}

// -----------------
// GetPopupMenu
// -----------------
// Internal routine used later. Returns the first popup from a menu
// 
HMENU GetPopupMenu(short mn)
{
	HMENU	hMn, hSubMenu, hPopup = NULL;

	if ((hMn = LoadMenu(hInstLib, MAKEINTRESOURCE(mn))) != NULL)
	{
		if ((hSubMenu = GetSubMenu(hMn, 0)) != NULL)
		{
			if ((hPopup = CreatePopupMenu()) != NULL)
				menucpy(hPopup, hSubMenu);
		}
		DestroyMenu(hMn);
	}
	return hPopup;
}

// --------------------
// GetEventInformations
// --------------------
// Internal routine used later. Look for one event in one of the eventInfos array...
// No protection to go faster: you must properly enter the conditions/actions!
// 
static LPEVENTINFOS2 GetEventInformations(LPEVENTINFOS2 eiPtr, short code)

{
	while(eiPtr->infos.code != code)
		eiPtr = EVINFO2_NEXT(eiPtr);
	
	return eiPtr;
}


// ----------------------------------------------------
// GetConditionMenu / GetActionMenu / GetExpressionMenu
// ----------------------------------------------------
// Load the condition/action/expression menu from the resource, eventually
// enable or disable some options, and returns it to CC&C.
//
HMENU WINAPI DLLExport GetConditionMenu(mv _far *knpV, fpObjInfo oiPtr, LPEDATA edPtr)
{
	// Check compatibility
	if ( !IS_COMPATIBLE(knpV) )
		return NULL;
	return GetPopupMenu(MN_CONDITIONS);
}

HMENU WINAPI DLLExport GetActionMenu(mv _far *knpV, fpObjInfo oiPtr, LPEDATA edPtr)
{
	// Check compatibility
	if ( !IS_COMPATIBLE(knpV) )
		return NULL;
	return GetPopupMenu(MN_ACTIONS);
}

HMENU WINAPI DLLExport GetExpressionMenu(mv _far *knpV, fpObjInfo oiPtr, LPEDATA edPtr)
{
	// Check compatibility
	if ( !IS_COMPATIBLE(knpV) )
		return NULL;
	return GetPopupMenu(MN_EXPRESSIONS);
}


// -------------------------------------------------------
// GetConditionTitle / GetActionTitle / GetExpressionTitle
// -------------------------------------------------------
// Returns the title of the dialog box displayed when entering
// parameters for the condition, action or expressions, if any.
// Here, we simply return the title of the menu option
//

void GetCodeTitle(LPEVENTINFOS2 eiPtr, short code, short param, short mn, LPSTR strBuf, WORD maxLen)
{
	HMENU		hMn;


	// Finds event in array
	eiPtr=GetEventInformations(eiPtr, code);

	// If a special string is to be returned
	short strID = EVINFO2_PARAMTITLE(eiPtr, param);

	if ( strID != 0 )
		LoadString(hInstLib, strID, strBuf, maxLen);
	else
	{
		// Otherwise, returns the menu option 
		if ((hMn = LoadMenu(hInstLib, MAKEINTRESOURCE(mn))) != NULL )
		{
			GetMenuString(hMn, eiPtr->menu, strBuf, maxLen, MF_BYCOMMAND);
			DestroyMenu(hMn);
		}
	}
}

void WINAPI DLLExport GetConditionTitle(mv _far *knpV, short code, short param, LPSTR strBuf, short maxLen)
{
	GetCodeTitle((LPEVENTINFOS2)conditionsInfos, code, param, MN_CONDITIONS, strBuf, maxLen);
}
void WINAPI DLLExport GetActionTitle(mv _far *knpV, short code, short param, LPSTR strBuf, short maxLen)
{
	GetCodeTitle((LPEVENTINFOS2)actionsInfos, code, param, MN_ACTIONS, strBuf, maxLen);
}
void WINAPI DLLExport GetExpressionTitle(mv _far *knpV, short code, LPSTR strBuf, short maxLen)
{
	GetCodeTitle((LPEVENTINFOS2)expressionsInfos, code, 0, MN_EXPRESSIONS, strBuf, maxLen);
}

// -------------------------------------------------------
// GetConditionTitle / GetActionTitle / GetExpressionTitle
// -------------------------------------------------------
// From a menu ID, these routines returns the code of the condition,
// action or expression, as defined in the .H file
//

short WINAPI DLLExport GetConditionCodeFromMenu(mv _far *knpV, short menuId)
{
	LPEVENTINFOS2	eiPtr;
	int				n;

	for (n=CND_LAST, eiPtr=(LPEVENTINFOS2)conditionsInfos; n>0 && eiPtr->menu!=menuId; n--)
		eiPtr = EVINFO2_NEXT(eiPtr);
	if (n>0) 
		return eiPtr->infos.code;
	return -1;
}

short WINAPI DLLExport GetActionCodeFromMenu(mv _far *knpV, short menuId)
{
	LPEVENTINFOS2	eiPtr;
	int				n;

	for (n=ACT_LAST, eiPtr=(LPEVENTINFOS2)actionsInfos; n>0 && eiPtr->menu!=menuId; n--)
		eiPtr = EVINFO2_NEXT(eiPtr);
	if (n>0) 
		return eiPtr->infos.code;
	return -1;
}

short WINAPI DLLExport GetExpressionCodeFromMenu(mv _far *knpV, short menuId)
{
	LPEVENTINFOS2	eiPtr;
	int				n;

	for (n=EXP_LAST, eiPtr=(LPEVENTINFOS2)expressionsInfos; n>0 && eiPtr->menu!=menuId; n--)
		eiPtr = EVINFO2_NEXT(eiPtr);
	if (n>0) 
		return eiPtr->infos.code;
	return -1;
}


// -------------------------------------------------------
// GetConditionInfos / GetActionInfos / GetExpressionInfos
// -------------------------------------------------------
// From a action / condition / expression code, returns 
// an infosEvents structure. 
//

LPINFOEVENTSV2 WINAPI DLLExport GetConditionInfos(mv _far *knpV, short code)
{
	return &GetEventInformations((LPEVENTINFOS2)conditionsInfos, code)->infos;
}

LPINFOEVENTSV2 WINAPI DLLExport GetActionInfos(mv _far *knpV, short code)
{
	return &GetEventInformations((LPEVENTINFOS2)actionsInfos, code)->infos;
}

LPINFOEVENTSV2 WINAPI DLLExport GetExpressionInfos(mv _far *knpV, short code)
{
	return &GetEventInformations((LPEVENTINFOS2)expressionsInfos, code)->infos;
}


// ----------------------------------------------------------
// GetConditionString / GetActionString / GetExpressionString
// ----------------------------------------------------------
// From a action / condition / expression code, returns 
// the string to use for displaying it under the event editor
//

void WINAPI DLLExport GetConditionString(mv _far *knpV, short code, LPSTR strPtr, short maxLen)
{
	// Check compatibility
	if ( !IS_COMPATIBLE(knpV) )
		return;
	LoadString(hInstLib, GetEventInformations((LPEVENTINFOS2)conditionsInfos, code)->string, strPtr, maxLen);
}

void WINAPI DLLExport GetActionString(mv _far *knpV, short code, LPSTR strPtr, short maxLen)
{
	// Check compatibility
	if ( !IS_COMPATIBLE(knpV) )
		return;
	LoadString(hInstLib, GetEventInformations((LPEVENTINFOS2)actionsInfos, code)->string, strPtr, maxLen);
}

void WINAPI DLLExport GetExpressionString(mv _far *knpV, short code, LPSTR strPtr, short maxLen)
{
	// Check compatibility
	if ( !IS_COMPATIBLE(knpV) )
		return;
	LoadString(hInstLib, GetEventInformations((LPEVENTINFOS2)expressionsInfos, code)->string, strPtr, maxLen);
}

// Return object info - called by MMF when you select Objects / Information / About
void WINAPI	DLLExport GetObjInfos (mv _far *knpV, LPEDATA edPtr, LPSTR ObjName, LPSTR ObjAuthor, LPSTR ObjCopyright, LPSTR ObjComment, LPSTR ObjHttp)
{
	// Name
	LoadString(hInstLib, IDST_OBJNAME,ObjName, 255);

	// Author
	LoadString(hInstLib, IDST_AUTHOR,ObjAuthor,255);

	// Copyright
	LoadString(hInstLib, IDST_COPYRIGHT,ObjCopyright,255);

	// Comment
	LoadString(hInstLib, IDST_COMMENT,ObjComment,255);

	// Internet address
	LoadString(hInstLib, IDST_HTTP,ObjHttp,255);
}
#endif //Not RUN_ONLY
