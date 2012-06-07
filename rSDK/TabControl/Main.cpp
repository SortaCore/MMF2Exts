// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================

#include "common.h"

// ============================================================================
//
// CONDITIONS
// 
// ============================================================================

struct OverlayStructure
{
	headerObject	rHo;					// Header
	rVal			rv;						// Values
	LPSURFACE		runSurface;
	long			X;
	long			Y;
	long			OldX;
	long			OldY;
	short			swidth;
	short			sheight;
	short			oldwidth;		//For stretching
	short			oldheight;		//For stretching
	char			rFilename[_MAX_PATH];
	BOOL			debugmode;		//Is debug mode on? (visibility)
	BOOL            stretching;		//Is stretching on?
	BOOL			AbsolutePos;	//drawing position absolute
	BOOL			Immediate;		//update display immediately
	BOOL			Automatic;		//update display automatically
	BOOL			AntiA;			//anti-aliasing
	BlitMode		Mode;			//display mode (opaque or trans)
	BlitOp			Effect;			//blit effect
	int				EffectParam;	//effect parameter
	POINT *			Points;			//points for "Plot Polygon"
	unsigned long	NumPoints;		//number of points to plot
	unsigned long	SizePoints;		//size of array of points
	BOOL			RemovePoints;	//remove points after plotting polygon
	BOOL			Fade;			//true if we should be fading
	float			FadeSpeed;		//speed at which to fade
	float			FadeInk;		//current ink effect param as a float
};
typedef OverlayStructure _far * OVERLAY;

CONDITION(
	/* ID */			0,
	/* Name */			_T("%o: On tab changed"),
	/* Flags */			0,
	/* Params */		(0)
) {
	return true;
}
CONDITION(
	/* ID */			1,
	/* Name */			_T("%o: On left mouse button clicked"),
	/* Flags */			0,
	/* Params */		(0)
) {
	return true;
}
CONDITION(
	/* ID */			2,
	/* Name */			_T("%o: On left mouse button double-clicked"),
	/* Flags */			0,
	/* Params */		(0)
) {
	return true;
}
CONDITION(
	/* ID */			3,
	/* Name */			_T("%o: On right mouse button clicked"),
	/* Flags */			0,
	/* Params */		(0)
) {
	return true;
}

CONDITION(
	/* ID */			4,
	/* Name */			_T("%o: On right mouse button double-clicked"),
	/* Flags */			0,
	/* Params */		(0)
) {
	return true;
}
CONDITION(
	/* ID */			5,
	/* Name */			_T("%o: On tab changed by user"),
	/* Flags */			0,
	/* Params */		(0)
) {
	return true;
}
CONDITION(
	/* ID */			6,
	/* Name */			_T("%o: On tab changed to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_STRING,_T("Caption"))
) {	
	TCHAR* caption = (TCHAR*)param1;
	TCHAR buff[64];
	TC_ITEM tie;
	tie.mask = TCIF_TEXT;
	tie.pszText = &buff[0];
	tie.cchTextMax = 64;
	TabCtrl_GetItem(rdPtr->hWnd,TabCtrl_GetCurSel(rdPtr->hWnd),&tie);
	return !_tcscmp(&buff[0],caption);
}
CONDITION(
	/* ID */			7,
	/* Name */			_T("%o: On tab index changed to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Index"))
) {
	return param1==TabCtrl_GetCurSel(rdPtr->hWnd);
}




// ============================================================================
//
// ACTIONS
// 
// ============================================================================


// -----------------
// Set Text
// -----------------
// 
ACTION(
	/* ID */			0,
	/* Name */			_T("Insert tab with caption %0, icon %1 at %2"),
	/* Flags */			0,
	/* Params */		(3,PARAM_STRING,_T("Caption"),PARAM_NUMBER,_T("Icon index (-1: None)"),PARAM_NUMBER,_T("Index (-1: Add to the end)"))
) {
	TCHAR* text = (TCHAR*)Param(TYPE_STRING);
	int img = Param(TYPE_INT);
	int index = Param(TYPE_INT);
	if (index == -1) index = TabCtrl_GetItemCount(rdPtr->hWnd);
	index = max(index,0);
	TCITEM tie; 
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
    tie.iImage = img;
    tie.pszText = text; 
	TabCtrl_InsertItem(rdPtr->hWnd,index,&tie);
}

ACTION(
	/* ID */			1,
	/* Name */			_T("Delete tab at %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Index"))
) {
	int index = param1;
	index = max(index,0);
	TabCtrl_DeleteItem(rdPtr->hWnd,index);
}

ACTION(
	/* ID */			2,
	/* Name */			_T("Delete all tabs"),
	/* Flags */			0,
	/* Params */		(0)
) {
	TabCtrl_DeleteAllItems(rdPtr->hWnd);
}

ACTION(
	/* ID */			3,
	/* Name */			_T("Set current tab to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Index"))
) {
	TabCtrl_SetCurSel(rdPtr->hWnd,param1);
	rdPtr->rRd->GenerateEvent(0);
}

ACTION(
	/* ID */			4,
	/* Name */			_T("Set caption of tab %0 to %1"),
	/* Flags */			0,
	/* Params */		(2,PARAM_NUMBER,_T("Index"),PARAM_STRING,_T("Caption"))
) {
	int index = param1;
	TCHAR* text = (TCHAR*)param2;
	TCITEM tie; 
	tie.mask = TCIF_TEXT;
    tie.iImage = -1; 
    tie.pszText = text; 
	TabCtrl_SetItem(rdPtr->hWnd,index,&tie);
}

ACTION(
	/* ID */			5,
	/* Name */			_T("Set width to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Width"))
) {
	rdPtr->rHo.hoImgWidth = max(0,param1);
}

ACTION(
	/* ID */			6,
	/* Name */			_T("Set visibility to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Visiblity (0 = Hide, 1 = Show)"))
) {
	bool visi = param1!=0;
	SetWindowPos(rdPtr->hWnd,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | (visi ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
}

ACTION(
	/* ID */			7,
	/* Name */			_T("Hightlight tab %0: %1"),
	/* Flags */			0,
	/* Params */		(2,PARAM_NUMBER,_T("Index"),PARAM_NUMBER,_T("Highlight (0 = False, 1 = True)"))
) {
	TabCtrl_HighlightItem(rdPtr->hWnd,param1,param2);
}

ACTION(
	/* ID */			8,
	/* Name */			_T("Set minimal tab width to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Width"))
) {
	TabCtrl_SetMinTabWidth(rdPtr->hWnd,param1);
}

ACTION(
	/* ID */			9,
	/* Name */			_T("Set padding to (%0,%1)"),
	/* Flags */			0,
	/* Params */		(2,PARAM_NUMBER,_T("Width"),PARAM_NUMBER,_T("Height"))
) {
	TabCtrl_SetPadding(rdPtr->hWnd,param1,param2);
}

ACTION(
	/* ID */			10,
	/* Name */			_T("Set tab size to (%0,%1)"),
	/* Flags */			0,
	/* Params */		(2,PARAM_NUMBER,_T("Width"),PARAM_NUMBER,_T("Height"))
) {
	TabCtrl_SetItemSize(rdPtr->hWnd,param1,param2);
}

ACTION(
	/* ID */			11,
	/* Name */			_T("Deselect all"),
	/* Flags */			0,
	/* Params */		(0)
) {
	TabCtrl_DeselectAll(rdPtr->hWnd,0);
}

ACTION(
	/* ID */			12,
	/* Name */			_T("Set focused tab to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Index"))
) {
	TabCtrl_SetCurFocus(rdPtr->hWnd,param1);
}

ACTION(
	/* ID */			13,
	/* Name */			_T("Set extended tab style to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Extended style (Merge with \"or\" operator)"))
) {
	TabCtrl_SetExtendedStyle(rdPtr->hWnd,param1);
}

ACTION(
	/* ID */			14,
	/* Name */			_T("Insert tabs from list %0 at %1"),
	/* Flags */			0,
	/* Params */		(2,PARAM_STRING,_T("Semicolon-separated list"),PARAM_NUMBER,_T("Index (-1: Add to the end)"))
) {
	TCITEM tie; 
	tie.mask = TCIF_TEXT | TCIF_IMAGE; 
	tie.iImage = -1; 
	int i = 0;
	TCHAR* text = (TCHAR*)Param(TYPE_STRING);
	int in = Param(TYPE_INT);
	if (in == -1) in = TabCtrl_GetItemCount(rdPtr->hWnd);
	TCHAR* t = _tcstok(text,_T(";"));
	while (t != NULL) {
		tie.pszText = t; 
		TabCtrl_InsertItem(rdPtr->hWnd,in+i,&tie);
		t = _tcstok(NULL,_T(";"));
		i++;
	}
}	

ACTION(
	/* ID */			15,
	/* Name */			_T("Set height to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Height"))
) {
	rdPtr->rHo.hoImgHeight = Param(TYPE_INT);
}

ACTION(
	/* ID */			16,
	/* Name */			_T("Set style to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Style (See \"Styles\", merge with \"or\" operator)"))
) {
	SetWindowLong(rdPtr->hWnd,GWL_STYLE,WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE | param1);
}

ACTION(
	/* ID */			17,
	/* Name */			_T("Set extended style to %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Extended style (See \"Extended styles\", merge with \"or\" operator)"))
) {
	SetWindowLong(rdPtr->hWnd,GWL_EXSTYLE,param1);
}

ACTION(
	/* ID */			18,
	/* Name */			_T("Set icon %0 from image %1, transparent color = %2"),
	/* Flags */			0,
	/* Params */		(3,PARAM_NUMBER,_T("Index (-1: Add to the end)"),PARAM_FILENAME2,_T("Image path"),PARAM_COLOUR,_T("Transparent color (-1: First pixel color)"))
) {
		LPRH rhPtr = rdPtr->rHo.hoAdRunHeader;
		int index = Param(TYPE_INT);
		char * file = (char *)Param(TYPE_STRING);
		int col = Param(TYPE_GENERIC);
		// Create a surface to load the image into
		cSurface* proto = 0, psf;
		GetSurfacePrototype(&proto, 24, ST_MEMORY, SD_DIB);
		psf.Clone(*proto);
		// Get MMF ImgFilterMgr
		CImageFilterMgr* pImgMgr = rhPtr->rh4.rh4Mv->mvImgFilterMgr;
		// Import the image
		if (!ImportImage(pImgMgr, file, &psf, NULL, IMPORT_IMAGE_USESURFACEDEPTH))
			return;
		if (col == -1) psf.GetPixel(0,0,col);
		HICON hicon = psf.CreateIcon(rdPtr->iconwidth,rdPtr->iconheight,col,NULL);
		ImageList_ReplaceIcon(rdPtr->imgl,index,hicon);
		DestroyIcon(hicon);
		psf.Delete();
}

ACTION(
	/* ID */			19,
	/* Name */			_T("Set icon %0 from active object %1"),
	/* Flags */			0,
	/* Params */		(2,PARAM_NUMBER,_T("Index (-1: Add to the end)"),PARAM_OBJECT,_T("Object"))
) {
		LPRH rhPtr = rdPtr->rHo.hoAdRunHeader;
		unsigned int index = Param(TYPE_INT);
		LPRO obj = (LPRO)Param(TYPE_GENERIC);
		LPSURFACE ps = WinGetSurface((int)rhPtr->rhIdEditWin);
		cSurface surface;
		surface.Create(rdPtr->iconwidth,rdPtr->iconheight,ps);
		rdPtr->rRd->GrabSurface(obj,surface);
		HICON hicon = surface.CreateIcon(rdPtr->iconwidth,rdPtr->iconheight,surface.GetTransparentColor(),NULL);
		ImageList_ReplaceIcon(rdPtr->imgl,index,hicon);
		DestroyIcon(hicon);
}		

ACTION(
	/* ID */			20,
	/* Name */			_T("Remove icon %0"),
	/* Flags */			0,
	/* Params */		(1,PARAM_NUMBER,_T("Index"))
) {
	int index = Param(TYPE_INT);
	ImageList_Remove(rdPtr->imgl,index);
}

ACTION(
	/* ID */			21,
	/* Name */			_T("Remove all icons"),
	/* Flags */			0,
	/* Params */		(0)
) {
	ImageList_RemoveAll(rdPtr->imgl);
}

ACTION(
	/* ID */			22,
	/* Name */			_T("Set icon %0 from overlay address %1"),
	/* Flags */			0,
	/* Params */		(2,PARAM_NUMBER,_T("Index (-1: Add to the end)"),PARAM_NUMBER,_T("Overlay address"))
) {
		int index = Param(TYPE_INT);
		OVERLAY ov = (OVERLAY)Param(TYPE_INT);
		HICON hicon = ov->runSurface->CreateIcon(rdPtr->iconwidth,rdPtr->iconheight,BLACK,NULL);
		ImageList_ReplaceIcon(rdPtr->imgl,index,hicon);
		DestroyIcon(hicon);
}

ACTION(
	/* ID */			23,
	/* Name */			_T("Force redraw"),
	/* Flags */			0,
	/* Params */		(0)
) {
	LPRH rhPtr = rdPtr->rHo.hoAdRunHeader;
	RedrawWindow(rhPtr->rhHEditWin,0,0,RDW_ALLCHILDREN|RDW_INVALIDATE);
}

ACTION(
	/* ID */			24,
	/* Name */			_T("Set icon of tab %0 to %1"),
	/* Flags */			0,
	/* Params */		(2,PARAM_NUMBER,_T("Index"),PARAM_NUMBER,_T("Icon index (-1: None)"))
) {
	int index = Param(TYPE_INT);
	int icon = Param(TYPE_INT);
	TCITEM tie; 
	tie.mask = TCIF_IMAGE; 
    tie.iImage = icon; 
	TabCtrl_SetItem(rdPtr->hWnd,index,&tie);
}
ACTION(
	/* ID */			25,
	/* Name */			_T("Set icons at %0 from active object %1"),
	/* Flags */			0,
	/* Params */		(2,PARAM_NUMBER,_T("Starting index (-1: Add to the end)"),PARAM_OBJECT,_T("Object"))
) {

		LPRH rhPtr = rdPtr->rHo.hoAdRunHeader;
		unsigned int index = Param(TYPE_INT);
		LPRO obj = (LPRO)Param(TYPE_GENERIC);
		LPSURFACE ps = WinGetSurface((int)rhPtr->rhIdEditWin);
		cSurface surface,blit;
		if (obj->roHo.hoImgWidth%rdPtr->iconwidth) return;
		surface.Create(obj->roHo.hoImgWidth,obj->roHo.hoImgHeight,ps);
		blit.Create(rdPtr->iconwidth,rdPtr->iconheight,ps);
		rdPtr->rRd->GrabSurface(obj,surface);
		for (int i=0;i<obj->roHo.hoImgWidth/rdPtr->iconwidth;i++) {
			surface.Blit(blit,0,0,i*rdPtr->iconwidth,0,rdPtr->iconwidth,rdPtr->iconheight,(BlitMode)BMODE_OPAQUE);
			HICON hicon = blit.CreateIcon(rdPtr->iconwidth,rdPtr->iconheight,blit.GetTransparentColor(),NULL);
			ImageList_ReplaceIcon(rdPtr->imgl,index+i*(index!=-1),hicon);
			DestroyIcon(hicon);
		}
}

ACTION(
	/* ID */			26,
	/* Name */			_T("Insert tabs with icons from list %0 at %1"),
	/* Flags */			0,
	/* Params */		(2,PARAM_STRING,_T("List (icon1;text1;icon2;...)"),PARAM_NUMBER,_T("Index (-1: Add to the end)"))
) {
	TCITEM tie; 
	tie.mask = TCIF_TEXT|TCIF_IMAGE; 
	tie.iImage = -1; 
	int i=0;
	TCHAR* text = (TCHAR*)param1;
	int in = param2;
	if (in == -1) in = TabCtrl_GetItemCount(rdPtr->hWnd);
	TCHAR* t = _tcstok((TCHAR*)text,_T(";"));
	while (t != NULL) {
		if (i%2) {
			tie.pszText = t; 
			TabCtrl_InsertItem(rdPtr->hWnd,in+i,&tie);
		} else tie.iImage = _tstoi(t);
		t = _tcstok(NULL,_T(";"));
		i++;
	}
}	

ACTION(
	/* ID */			27,
	/* Name */			_T("Set parameter of tab %0 to %1"),
	/* Flags */			0,
	/* Params */		(2,PARAM_NUMBER,_T("Index"),PARAM_NUMBER,_T("Parameter"))
) {
	TCITEM tie; 
	tie.mask = TCIF_PARAM; 
    tie.lParam = (LPARAM)param2;
	TabCtrl_SetItem(rdPtr->hWnd,param1,&tie);
}

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================


#define CONSTEXPR(id,ex,label) \
EXPRESSION( \
	/* ID */			id, \
	/* Name */			label _T("("), \
	/* Flags */			0, \
	/* Params */		(0) \
) { \
	return ex; \
}

EXPRESSION(
	/* ID */			0,
	/* Name */			_T("CurTab("),
	/* Flags */			0,
	/* Params */		(0)
) {
	return TabCtrl_GetCurSel(rdPtr->hWnd);
}

EXPRESSION(
	/* ID */			1,
	/* Name */			_T("TabCaption$("),
	/* Flags */			EXPFLAG_STRING,
	/* Params */		(1,EXPPARAM_NUMBER,_T("Index"))
) {	
	int index = ExParam(TYPE_INT);
	TCHAR buff[64] = {0};
	TC_ITEM tie;
	tie.mask = TCIF_TEXT;
	tie.pszText = &buff[0];
	tie.cchTextMax = 64;
	TabCtrl_GetItem(rdPtr->hWnd,index,&tie);
	ReturnStringSafe(buff);
}


EXPRESSION(
	/* ID */			2,
	/* Name */			_T("TabCount("),
	/* Flags */			0,
	/* Params */		(0)
) {
	return TabCtrl_GetItemCount(rdPtr->hWnd);
}

EXPRESSION(
	/* ID */			3,
	/* Name */			_T("RowCount("),
	/* Flags */			0,
	/* Params */		(0)
) {
	return TabCtrl_GetRowCount(rdPtr->hWnd);
}

EXPRESSION(
	/* ID */			4,
	/* Name */			_T("Focus("),
	/* Flags */			0,
	/* Params */		(0)
) {
	return TabCtrl_GetCurFocus(rdPtr->hWnd);
}

EXPRESSION(
	/* ID */			5,
	/* Name */			_T("W("),
	/* Flags */			0,
	/* Params */		(0)
) {
	return rdPtr->rHo.hoImgWidth;
}

EXPRESSION(
	/* ID */			6,
	/* Name */			_T("H("),
	/* Flags */			0,
	/* Params */		(0)
) {
	return rdPtr->rHo.hoImgHeight;
}

CONSTEXPR(7,TCS_BOTTOM,_T("StBottom"))
CONSTEXPR(8,TCS_BUTTONS,_T("StButtons"))
CONSTEXPR(9,TCS_FIXEDWIDTH,_T("StFixed"))
CONSTEXPR(10,TCS_FLATBUTTONS,_T("StFlat"))
CONSTEXPR(11,TCS_MULTILINE,_T("StMulti"))
CONSTEXPR(12,TCS_VERTICAL,_T("StVertical"))
CONSTEXPR(13,TCS_RIGHT,_T("StRight"))
CONSTEXPR(14,TCS_SINGLELINE,_T("StSingle"))
CONSTEXPR(15,TCS_TABS,_T("StTabs"))
CONSTEXPR(16,TCS_RAGGEDRIGHT,_T("StNoStretch"))


EXPRESSION(
	/* ID */			17,
	/* Name */			_T("TabIcon("),
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_NUMBER,_T("Index"))
) {
	int index = ExParam(TYPE_INT);
	if (index < 0 || index > TabCtrl_GetItemCount(rdPtr->hWnd)-1) return -1;
	TC_ITEM tie;
	tie.mask = TCIF_IMAGE;
	TabCtrl_GetItem(rdPtr->hWnd,index,&tie);
	return tie.iImage;
}

EXPRESSION(
	/* ID */			18,
	/* Name */			_T("FindTab("),
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_STRING,_T("Caption"))
) {	
	TCHAR* capt = (TCHAR*)ExParam(TYPE_STRING);
	TCHAR buff[64];
	int count = TabCtrl_GetItemCount(rdPtr->hWnd);
	TC_ITEM tie;
	tie.mask = TCIF_TEXT;
	tie.pszText = &buff[0];
	tie.cchTextMax = 64;
	for (int i=0;i<count;i++)
	{
		TabCtrl_GetItem(rdPtr->hWnd,i,&tie);
		if (!_tcscmp(buff,capt)) return i;
	}
	return -1;
}

EXPRESSION(
	/* ID */			19,
	/* Name */			_T("TabX("),
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_NUMBER,_T("Index (-1: Last)"))
) {	
	int i = ExParam(TYPE_INT);
	if (i == -1) i = TabCtrl_GetItemCount(rdPtr->hWnd)-1;
	RECT r;
	TabCtrl_GetItemRect(rdPtr->hWnd,i,&r);
	return r.left;
}

EXPRESSION(
	/* ID */			20,
	/* Name */			_T("TabY("),
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_NUMBER,_T("Index (-1: Last)"))
) {	
	int i = ExParam(TYPE_INT);
	if (i == -1) i = TabCtrl_GetItemCount(rdPtr->hWnd)-1;
	RECT r;
	TabCtrl_GetItemRect(rdPtr->hWnd,i,&r);
	return r.top;
}

EXPRESSION(
	/* ID */			21,
	/* Name */			_T("TabW("),
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_NUMBER,_T("Index (-1: Last)"))
) {	
	int i = ExParam(TYPE_INT);
	if (i == -1) i = TabCtrl_GetItemCount(rdPtr->hWnd)-1;
	RECT r;
	TabCtrl_GetItemRect(rdPtr->hWnd,i,&r);
	return r.right-r.left;
}

EXPRESSION(
	/* ID */			22,
	/* Name */			_T("TabH("),
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_NUMBER,_T("Index (-1: Last)"))
) {	
	int i = ExParam(TYPE_INT);
	if (i == -1) i = TabCtrl_GetItemCount(rdPtr->hWnd)-1;
	RECT r;
	TabCtrl_GetItemRect(rdPtr->hWnd,i,&r);
	return r.bottom-r.top;
}

EXPRESSION(
	/* ID */			23,
	/* Name */			_T("TabParam("),
	/* Flags */			0,
	/* Params */		(1,EXPPARAM_NUMBER,_T("Index"))
) {
	int index = ExParam(TYPE_INT);
	TC_ITEM tie;
	tie.mask = TCIF_PARAM;
	TabCtrl_GetItem(rdPtr->hWnd,index,&tie);
	return (long)tie.lParam;
}