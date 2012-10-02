// ============================================================================
//
// This file is used only when you are build a RUN_ONLY object.
//
// There should be no need to modify this file.
// 
// ============================================================================
#ifdef RUN_ONLY
// Common Include
#include	"common.h"

void WINAPI	DLLExport GetObjInfos (mv _far *knpV, LPEDATA edPtr, LPSTR ObjName, LPSTR ObjAuthor, LPSTR ObjCopyright, LPSTR ObjComment, LPSTR ObjHttp)
{
}

HMENU WINAPI DLLExport GetConditionMenu(mv _far *knpV, fpObjInfo oiPtr, LPEDATA edPtr)
{
	return NULL;
}

HMENU WINAPI DLLExport GetActionMenu(mv _far *knpV, fpObjInfo oiPtr, LPEDATA edPtr)
{
	return NULL;
}

HMENU WINAPI DLLExport GetExpressionMenu(mv _far *knpV, fpObjInfo oiPtr, LPEDATA edPtr)
{
	return NULL;
}

void WINAPI DLLExport GetConditionTitle(mv _far *knpV, short code, short param, LPSTR strBuf, short maxLen)
{
}

void WINAPI DLLExport GetActionTitle(mv _far *knpV, short code, short param, LPSTR strBuf, short maxLen)
{
}

void WINAPI DLLExport GetExpressionTitle(mv _far *knpV, short code, LPSTR strBuf, short maxLen)
{
}

short WINAPI DLLExport GetConditionCodeFromMenu(mv _far *knpV, short menuId)
{
	return -1;
}

short WINAPI DLLExport GetActionCodeFromMenu(mv _far *knpV, short menuId)
{
	return -1;
}

short WINAPI DLLExport GetExpressionCodeFromMenu(mv _far *knpV, short menuId)
{
	return -1;
}

LPINFOEVENTSV2 WINAPI DLLExport GetConditionInfos(mv _far *knpV, short code)
{
	return NULL;
}

LPINFOEVENTSV2 WINAPI DLLExport GetActionInfos(mv _far *knpV, short code)
{
	return NULL;
}

LPINFOEVENTSV2 WINAPI DLLExport GetExpressionInfos(mv _far *knpV, short code)
{
	return NULL;
}

void WINAPI DLLExport GetConditionString(mv _far *knpV, short code, LPSTR strPtr, short maxLen)
{
}

void WINAPI DLLExport GetActionString(mv _far *knpV, short code, LPSTR strPtr, short maxLen)
{
}

void WINAPI DLLExport GetExpressionString(mv _far *knpV, short code, LPSTR strPtr, short maxLen)
{
}


HGLOBAL WINAPI DLLExport UpdateEditStructure(mv __far *knpV, void __far * OldEdPtr)
{
	return NULL;
}

void WINAPI	DLLExport PutObject(mv _far *knpV, fpLevObj loPtr, LPEDATA edPtr, ushort cpt)
{
	MessageBoxA(NULL, "You are using PackX runtime MFX at edittime. Please reinstall PackX extension.", "PackX - Error!", MB_OK);
}

void WINAPI	DLLExport RemoveObject(mv _far *knpV, fpLevObj loPtr, LPEDATA edPtr, ushort cpt)
{
}

int WINAPI DLLExport MakeIcon ( mv _far *knpV, BITMAPINFO FAR *lpBitmap, LPSTR lpName, fpObjInfo oiPtr, LPEDATA edPtr )
{
	return 0;
}

void WINAPI DLLExport AppendPopup(mv _far *knpV, HMENU hPopup, fpLevObj loPtr, LPEDATA edPtr, int nbSel)
{
}

int WINAPI DLLExport CreateObject(mv _far *knpV, fpLevObj loPtr, LPEDATA edPtr)
{
	return 0;
}

int WINAPI DLLExport SelectPopup(mv _far *knpV, int modif, fpObjInfo oiPtr, fpLevObj loPtr, LPEDATA edPtr, fpushort lpParams, int maxParams)
{
	return 0;
}

int WINAPI DLLExport ModifyObject(mv _far *knpV, LPEDATA edPtr, fpObjInfo oiPtr, fpLevObj loPtr, int modif, fpushort lpParams)
{
	return 0;
}

int WINAPI DLLExport RebuildExt(mv _far *knpV, LPEDATA edPtr, LPBYTE oldExtPtr, fpObjInfo oiPtr, fpLevObj loPtr, fpushort lpParams)
{
	return FALSE;
}

void WINAPI DLLExport EndModifyObject(mv _far *knpV, int modif, fpushort lpParams)
{
}

void WINAPI DLLExport GetObjectRect(mv _far *knpV, RECT FAR *rc, fpLevObj loPtr, LPEDATA edPtr)
{
}

void WINAPI DLLExport EditorDisplay(mv _far *knpV, fpObjInfo oiPtr, fpLevObj loPtr, LPEDATA edPtr, RECT FAR *rc)
{
}

extern "C" {
	BOOL WINAPI DLLExport IsTransparent(mv _far *knpV, fpLevObj loPtr, LPEDATA edPtr, int dx, int dy)
	{
		return FALSE;
	}
}

void WINAPI	DLLExport PrepareToWriteObject(mv _far *knpV, LPEDATA edPtr, fpObjInfo adoi)
{
}

void WINAPI	DLLExport CreateFromFile (LPSTR fileName, LPEDATA edPtr)
{
}

void WINAPI DLLExport UpdateFileNames(mv _far *knpV, LPSTR gameName, LPEDATA edPtr, void (WINAPI * lpfnUpdate)(LPSTR, LPSTR))
{
}

BOOL WINAPI	DLLExport UsesFile (LPSTR fileName)
{
	return FALSE;
}

int WINAPI DLLExport EnumElts (mv __far *knpV, LPEDATA edPtr, ENUMELTPROC enumProc, ENUMELTPROC undoProc, LPARAM lp1, LPARAM lp2)
{ 
	return 0;
}

int WINAPI DLLExport Initialize(mv _far *knpV, int quiet)
{
	return 0;
}

int WINAPI DLLExport Free(mv _far *knpV)
{
	return 0;
}

int	WINAPI DLLExport LoadObject(mv _far *knpV, LPCSTR fileName, LPEDATA edPtr, int reserved)
{
	return 0;
}

void WINAPI DLLExport UnloadObject(mv _far *knpV, LPEDATA edPtr, int reserved)
{
}

#endif	// RUN_ONLY