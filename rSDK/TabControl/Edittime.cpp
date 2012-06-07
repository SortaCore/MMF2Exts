// ============================================================================
//
// This file contains routines that are handled only during the Edittime,
// under the Frame and Event editors.
//
// Including creating, display, and setting up your object.
// 
// ============================================================================

#include "common.h"

// --------------------
// Properties
// --------------------
// If you defined USE_MAGIC_PROPS in information.h you should
// not use these; see Properties.h instead

PROPS_IDS_START()
	PROPID_TEXT,			// Text property
	PROPID_BACKCOLOR,		// Background color property
	PROPID_SYSTEMCOLORS,	// System Colors check box
	PROPID_BORDER,			// Border check box
	PROPID_ICONS,
	PROPID_DELICONS,
	//VISIBLE
	PROPID_ICONSIZE,
	PROPID_MAXICONS,
	PROPID_BUTTONSTYLE,
	PROPID_TABDIR,
	PROPID_LINEMODE,
	PROPID_TABMODE,
	//
	PROPID_GRP,
PROPS_IDS_END()

int iconSizes[] = {
	8,8,
	16,16,
	24,24,
	32,32
};
char* buttonStyle[] = {0,"Tabs","Buttons","Flat",0};
char* tabDir[] = {0,"Top","Bottom","Left","Right",0};
char* lineMode[] = {0,"Single-line","Multi-line",0};
char* tabMode[] = {0,"Normal","Fixed width",0};


PROPS_DATA_START()
PropData_EditMultiLine(PROPID_TEXT,	PSTR("Default tabs"),PSTR("Icon and name separated by a semicolon (optional), separated by line breaks.")),
PropData_ImageList(PROPID_ICONS,PSTR("Default icons"),PSTR("")),
PropData_Button(PROPID_DELICONS,PSTR(""),PSTR("This will delete all icons. They cannot be restored."),PSTR("Delete all icons")),
PropData_Size(PROPID_ICONSIZE,PSTR("Icon size"),PSTR(""),iconSizes),
PropData_EditNumber(PROPID_MAXICONS,PSTR("Max icon count"),PSTR("The maximum number of icons - At both edittime and runtime.")),
PropData_Group(PROPID_GRP,PSTR("Style"),PSTR("")),
PropData_ComboBox(PROPID_BUTTONSTYLE,PSTR("Tab style"),PSTR(""),buttonStyle),
PropData_ComboBox(PROPID_TABDIR,PSTR("Tab alignment"),PSTR(""),tabDir),
PropData_ComboBox(PROPID_LINEMODE,PSTR("Line mode"),PSTR(""),lineMode),
PropData_ComboBox(PROPID_TABMODE,PSTR("Tab behaviour"),PSTR(""),tabMode),
PROPS_DATA_END()

// --------------------
// Debugger
// --------------------

DEBUGGER_IDS_START()

	// DB_CURRENTSTRING,

DEBUGGER_IDS_END()

DEBUGGER_ITEMS_START()

	//  DB_CURRENTSTRING,
	//	DB_CURRENTSTRING|DB_EDITABLE,
	//	DB_CURRENTVALUE|DB_EDITABLE,
	//	DB_CURRENTCHECK,
	//	DB_CURRENTCOMBO,

DEBUGGER_ITEMS_END()

const TCHAR szDefaultText[] = _T("");
// --------------------
// GetProperties
// --------------------
// Inserts properties into the properties of the object.
//
BOOL WINAPI DLLExport GetProperties(LPMV mV, LPEDATA edPtr, BOOL bMasterItem)
{
#ifndef RUN_ONLY
	mvInsertProps(mV, edPtr, Properties, PROPID_TAB_GENERAL, TRUE);
#endif // !defined(RUN_ONLY)

	// OK
	return TRUE;
}

// --------------------
// ReleaseProperties
// --------------------
// Called when the properties are removed from the property window.
//
void WINAPI DLLExport ReleaseProperties(LPMV mV, LPEDATA edPtr, BOOL bMasterItem)
{
#ifndef RUN_ONLY
	// Write your code here
#endif // !defined(RUN_ONLY)
}

// --------------------
// GetPropCreateParam
// --------------------
// Called when a property is initialized and its creation parameter is NULL (in the PropData).
// Allows you, for example, to change the content of a combobox property according to specific settings in the EDITDATA structure.
//
LPARAM WINAPI DLLExport GetPropCreateParam(LPMV mV, LPEDATA edPtr, UINT nPropID)
{
#ifndef RUN_ONLY
	// Example
	// -------
//	if ( nPropID == PROPID_COMBO )
//	{
//		switch (edPtr->sType)
//		{
//		case TYPE1:
//			return (LPARAM)ComboList1;
//		case TYPE2:
//			return (LPARAM)ComboList2;
//		}
//	}

#endif // !defined(RUN_ONLY)
	return NULL;
}

// ----------------------
// ReleasePropCreateParam
// ----------------------
// Called after a property has been initialized.
// Allows you, for example, to free memory allocated in GetPropCreateParam.
//
void WINAPI DLLExport ReleasePropCreateParam(LPMV mV, LPEDATA edPtr, UINT nPropID, LPARAM lParam)
{
#ifndef RUN_ONLY
#endif // !defined(RUN_ONLY)
}

// --------------------
// GetPropValue
// --------------------
// Returns the value of properties that have a value.
// Note: see GetPropCheck for checkbox properties
//
LPVOID WINAPI DLLExport GetPropValue(LPMV mV, LPEDATA edPtr, UINT nPropID)
{
#ifndef RUN_ONLY
	switch (nPropID) {

		case PROPID_MAXICONS:
			return new CPropDWordValue(edPtr->maxIcon);
			break;
		case PROPID_BUTTONSTYLE:
			return new CPropDWordValue(edPtr->buttonStyle);
			break;
		case PROPID_TABDIR:
			return new CPropDWordValue(edPtr->tabDir);
			break;
		case PROPID_LINEMODE:
			return new CPropDWordValue(edPtr->lineMode);
			break;
		case PROPID_TABMODE:
			return new CPropDWordValue(edPtr->tabMode);
			break;
		case PROPID_ICONSIZE:
			return new CPropSizeValue(edPtr->iconW,edPtr->iconH);
			break;
		case PROPID_TEXT:
			return new CPropDataValue(edPtr->sText);
			break;
		case PROPID_ICONS:
            CPropDataValue* pv = new CPropDataValue((edPtr->nImages + 1) * sizeof(WORD), NULL);
            if ( pv != NULL )
            {
                if ( pv->m_pData != NULL )
                {
                    LPWORD pw = (LPWORD)pv->m_pData;
                    *pw++ = edPtr->nImages;
                    for (WORD w=0; w<edPtr->nImages; w++)
                        *pw++ = edPtr->wImages[w];
                    return pv;
                }
                pv->Delete();
            }
			break;
	}

	//case PROPID_ICONS:
	//		
	//	CPropDataValue* pv = new CPropDataValue((edPtr->nImages + 1) * sizeof(WORD), NULL);
	//	if ( pv != NULL )
	//	{
	//		if ( pv->m_pData != NULL )
	//		{
	//			LPWORD pw = (LPWORD)pv->m_pData;
	//			*pw++ = edPtr->nImages;
	//			for (WORD w=0; w<edPtr->nImages; w++)
	//				*pw++ = edPtr->wImages[w];
	//			return pv;
	//		}
	//		pv->Delete();
	//	}
	//break;

	//}
#endif // !defined(RUN_ONLY)
	return NULL;
}

// --------------------
// GetPropCheck
// --------------------
// Returns the checked state of properties that have a check box.
//
BOOL WINAPI DLLExport GetPropCheck(LPMV mV, LPEDATA edPtr, UINT nPropID)
{
#ifndef RUN_ONLY

	/*switch(nPropID) {
		case PROPID_VERTICAL:
			return edPtr->opts[0]?TRUE:FALSE;
		break;
		case PROPID_VERTRIGHT:
			return edPtr->opts[1]?TRUE:FALSE;
		break;
		case PROPID_BUTTONS:
			return edPtr->opts[2]?TRUE:FALSE;
		break;
		case PROPID_FLATBUTTONS:
			return edPtr->opts[3]?TRUE:FALSE;
		break;
	}*/

#endif // !defined(RUN_ONLY)
	return 0;		// Unchecked
}

// --------------------
// SetPropValue
// --------------------
// This routine is called by MMF after a property has been modified.
//
void WINAPI DLLExport SetPropValue(LPMV mV, LPEDATA edPtr, UINT nPropID, LPVOID lParam)
{
#ifndef RUN_ONLY
	// Gets the pointer to the CPropValue structure
	CPropValue* pValue = (CPropValue*)lParam;

	switch (nPropID) {

	case PROPID_ICONSIZE:
		edPtr->iconW = ((CPropSizeValue*)pValue)->m_cx;
		edPtr->iconH = ((CPropSizeValue*)pValue)->m_cy;
		break;
	case PROPID_MAXICONS:
		edPtr->maxIcon = ((CPropDWordValue*)pValue)->m_dwValue;
		break;
	case PROPID_BUTTONSTYLE:
		edPtr->buttonStyle = ((CPropDWordValue*)pValue)->m_dwValue;
		break;
	case PROPID_TABDIR:
		edPtr->tabDir = ((CPropDWordValue*)pValue)->m_dwValue;
		break;
	case PROPID_LINEMODE:
		edPtr->lineMode = ((CPropDWordValue*)pValue)->m_dwValue;
		break;
	case PROPID_TABMODE:
		edPtr->tabMode = ((CPropDWordValue*)pValue)->m_dwValue;
		break;
	case PROPID_TEXT:
		{
			LPTSTR pStr = (LPTSTR)((CPropDataValue*)pValue)->m_pData;
			if ( (_tcslen(pStr)*sizeof(TCHAR)) != (_tcslen(edPtr->sText)*sizeof(TCHAR)) ) {
				LPEDATA pNewPtr = (LPEDATA)mvReAllocEditData(mV, edPtr, sizeof(EDITDATA)+(_tcslen(pStr)*sizeof(TCHAR)));
				if (pNewPtr) {
					edPtr = pNewPtr;
					_tcscpy(edPtr->sText, pStr);
				}
			}
			else _tcscpy(edPtr->sText, pStr);
		}
		mvRefreshProp(mV, edPtr, PROPID_TEXT, TRUE);
	break;
	case PROPID_ICONS:
		if (((CPropDataValue*)pValue)->m_pData != NULL)
		{
			LPWORD pw = (LPWORD)((CPropDataValue*)pValue)->m_pData;
			edPtr->nImages = *pw++;
            for (WORD w=0; w<edPtr->nImages; w++)
                edPtr->wImages[w] = *pw++;
		}
		break;
	}
	mvInvalidateObject(mV, edPtr);

#endif // !defined(RUN_ONLY)
}

// --------------------
// SetPropCheck
// --------------------
// This routine is called by MMF when the user modifies a checkbox in the properties.
//
void WINAPI DLLExport SetPropCheck(LPMV mV, LPEDATA edPtr, UINT nPropID, BOOL nCheck)
{
#ifndef RUN_ONLY

/*	switch (nPropID) {

	// Check or Uncheck System Colors option
	case PROPID_VERTICAL:
		edPtr->opts[0] = nCheck==1;
		mvRefreshProp(mV, edPtr, PROPID_VERTICAL, FALSE);
	break;
	case PROPID_VERTRIGHT:
		edPtr->opts[1] = nCheck==1;
		mvRefreshProp(mV, edPtr, PROPID_VERTRIGHT, FALSE);
	break;
	case PROPID_BUTTONS:
		edPtr->opts[2] = nCheck==1;
		mvRefreshProp(mV, edPtr, PROPID_BUTTONS, FALSE);
	break;
	case PROPID_FLATBUTTONS:
		edPtr->opts[3] = nCheck==1;
		mvRefreshProp(mV, edPtr, PROPID_FLATBUTTONS, FALSE);
	break;
	}*/

#endif // !defined(RUN_ONLY)
}
// --------------------
// EditProp
// --------------------
// This routine is called when the user clicks the button of a Button or EditButton property.
//
BOOL WINAPI DLLExport EditProp(LPMV mV, LPEDATA edPtr, UINT nPropID)
{
#ifndef RUN_ONLY

	if (nPropID==PROPID_ICONS)
	{
		mvInvalidateObject(mV,edPtr);
		EditAnimationParams eip;
		eip.m_dwSize = sizeof(EditAnimationParams);
		eip.m_pWindowTitle = 0;
		eip.m_nImages = edPtr->nImages;
		eip.m_nMaxImages = 16;
		eip.m_nStartIndex = 0;
		eip.m_pImages = &edPtr->wImages[0];
		eip.m_pImageTitles = 0;
		eip.m_dwOptions = PICTEDOPT_NOALPHACHANNEL;
		eip.m_dwFixedWidth = edPtr->iconW;
		eip.m_dwFixedHeight = edPtr->iconH;
		BOOL output = mV->mvEditAnimation(edPtr,&eip,mV->mvHEditWin);
		//Update object count
		edPtr->nImages = eip.m_nImages;
		mvRefreshProp(mV,edPtr,PROPID_ICONS,true);
		return output;
	}
	else if (nPropID==PROPID_DELICONS)
	{
		int kill = MessageBox(0,_T("Do you really want to delete all icons?"),_T("Warning"),MB_YESNO|MB_ICONWARNING);
		if (kill==IDYES)
		{
			for (int i=0;i<16;i++)
				edPtr->wImages[i] = 0;
			edPtr->nImages = 0;
			mvInvalidateObject(mV,edPtr);
			mvRefreshProp(mV,edPtr,PROPID_ICONS,true);
		}
	}

#endif // !defined(RUN_ONLY)
	return false;
}

// --------------------
// IsPropEnabled
// --------------------
// This routine returns the enabled state of a property.
//
BOOL WINAPI IsPropEnabled(LPMV mV, LPEDATA edPtr, UINT nPropID)
{
#ifndef RUN_ONLY

	switch (nPropID) {

	// Enable background color property only if not system colors
	case PROPID_BACKCOLOR:
		return ((edPtr->dwFlags & SCTRL_SYSTEMCOLORS) == 0);
	}

#endif // !defined(RUN_ONLY)
	return TRUE;
}


// ============================================================================
//
// TEXT PROPERTIES
// 
// ============================================================================

// --------------------
// GetTextCaps
// --------------------
// Return the text capabilities of the object under the frame editor.
//
DWORD WINAPI DLLExport GetTextCaps(mv _far *mV, LPEDATA edPtr)
{
	return TEXT_FONT;
}

// --------------------
// GetTextFont
// --------------------
// Return the font used the object.
// Note: the pStyle and cbSize parameters are obsolete and passed for compatibility reasons only.
//
BOOL WINAPI DLLExport GetTextFont(mv _far *mV, LPEDATA edPtr, LPLOGFONT plf, LPTSTR pStyle, UINT cbSize)
{
#if !defined(RUN_ONLY)

	// Copy LOGFONT structure from EDITDATA
	memcpy(plf, &edPtr->textFont, sizeof(LOGFONT));

#endif // !defined(RUN_ONLY)

	return TRUE;
}

// --------------------
// SetTextFont
// --------------------
// Change the font used the object.
// Note: the pStyle parameter is obsolete and passed for compatibility reasons only.
//
BOOL WINAPI DLLExport SetTextFont(mv _far *mV, LPEDATA edPtr, LPLOGFONT plf, LPCSTR pStyle)
{
#if !defined(RUN_ONLY)

	// Copy LOGFONT structure to EDITDATA
	memcpy(&edPtr->textFont, plf, sizeof(LOGFONT));

#endif // !defined(RUN_ONLY)

	return TRUE;
}

// --------------------
// GetTextClr
// --------------------
// Get the text color of the object.
//
COLORREF WINAPI DLLExport GetTextClr(mv _far *mV, LPEDATA edPtr)
{
	return edPtr->textColor;
}

// --------------------
// SetTextClr
// --------------------
// Set the text color of the object.
//
void WINAPI DLLExport SetTextClr(mv _far *mV, LPEDATA edPtr, COLORREF color)
{
	edPtr->textColor = color;
}

// --------------------
// GetTextAlignment
// --------------------
// Get the text alignment of the object.
//
DWORD WINAPI DLLExport GetTextAlignment(mv _far *mV, LPEDATA edPtr)
{
	DWORD dw = 0;

#if !defined(RUN_ONLY)

	if ( (edPtr->dwAlignFlags & DT_CENTER) != 0 )
		dw |= TEXT_ALIGN_HCENTER;
	else if ( (edPtr->dwAlignFlags & DT_RIGHT) != 0 )
		dw |= TEXT_ALIGN_RIGHT;
	else
		dw |= TEXT_ALIGN_LEFT;

#endif // !defined(RUN_ONLY)

	return dw;
}

// --------------------
// SetTextAlignment
// --------------------
// Set the text alignment of the object.
//
void WINAPI DLLExport SetTextAlignment(mv _far *mV, LPEDATA edPtr, DWORD dwAlignFlags)
{
#if !defined(RUN_ONLY)

	edPtr->dwAlignFlags &= ~(DT_LEFT|DT_CENTER|DT_RIGHT);

	if ( (dwAlignFlags & TEXT_ALIGN_LEFT) != 0 )
		edPtr->dwAlignFlags |= DT_LEFT;
	if ( (dwAlignFlags & TEXT_ALIGN_HCENTER) != 0 )
		edPtr->dwAlignFlags |= DT_CENTER;
	if ( (dwAlignFlags & TEXT_ALIGN_RIGHT) != 0 )
		edPtr->dwAlignFlags |= DT_RIGHT;

#endif // !defined(RUN_ONLY)
}



// -----------------
// BmpToImg
// -----------------
// Converts an image from the resource to an image displayable under MMF2
// Not used in this template, but it is a good example on how to create
// an image.
//

/*
WORD BmpToImg(int bmID, npAppli idApp, short HotX = 0, short HotY = 0, short ActionX = 0, short ActionY = 0)
{
	Img					ifo;
	WORD				img;
	HRSRC				hs;
	HGLOBAL				hgBuf;
	LPBYTE				adBuf;
	LPBITMAPINFOHEADER	adBmi;

	img = 0;
	if ((hs = FindResource(hInstLib, MAKEINTRESOURCE(bmID), RT_BITMAP)) != NULL)
	{
		if ((hgBuf = LoadResource(hInstLib, hs)) != NULL)
		{
			if ((adBuf = (LPBYTE)LockResource(hgBuf)) != NULL)
			{
				adBmi = (LPBITMAPINFOHEADER)adBuf;
				ifo.imgXSpot = HotX;
				ifo.imgYSpot = HotY;
				ifo.imgXAction = ActionX;
				ifo.imgYAction = ActionY;
				if (adBmi->biBitCount > 4)
					RemapDib((LPBITMAPINFO)adBmi, idApp, NULL);
				img = (WORD)DibToImage(idApp, &ifo, adBmi);
				UnlockResource(hgBuf);
			}
			FreeResource(hgBuf);
		}
	}
	return img;
}
*/

// ============================================================================
//
// ROUTINES USED UNDER FRAME EDITOR
// 
// ============================================================================

// --------------------
// MakeIcon
// --------------------
// Called once object is created or modified, just after setup.
//
// Note: this function is optional. If it's not defined in your extension,
// MMF2 will load the EXO_ICON bitmap if it's defined in your resource file.
//
// If you need to draw the icon manually, remove the comments around this function and in the .def file.
//

/*
int WINAPI DLLExport MakeIconEx ( mv _far *mV, cSurface* pIconSf, LPTSTR lpName, fpObjInfo oiPtr, LPEDATA edPtr )
{
	int error = -1;
#ifndef RUN_ONLY
	if ( pIconSf->LoadImage(hInstLib, EXO_ICON) != 0 )
		error = 0;
#endif // !RUN_ONLY
	return error;
}
*/


// --------------------
// CreateObject
// --------------------
// Called when you choose "Create new object". It should display the setup box 
// and initialize everything in the datazone.

int WINAPI DLLExport CreateObject(mv _far *mV, fpLevObj loPtr, LPEDATA edPtr)
{
#ifndef RUN_ONLY

	// Do some rSDK stuff
	#include "rCreateObject.h"
	
		// Default size
		edPtr->nWidth = 300;
		edPtr->nHeight = 300;			
		edPtr->iconH = 16;
		edPtr->iconW = 16;
		edPtr->maxIcon = 256;
		edPtr->buttonStyle = 0;
		edPtr->tabDir = 0;
		edPtr->lineMode = 0;
		edPtr->tabMode = 0;

		for (int i=0;i<16;i++)
			edPtr->wImages[i] = 0;
		edPtr->nImages = 0;

		// Default font
		if ( mV->mvGetDefaultFont != NULL )
			mV->mvGetDefaultFont(&edPtr->textFont, NULL, 0);		// Get default font from frame editor preferences
		else
		{
			edPtr->textFont.lfWidth = 0;
			edPtr->textFont.lfHeight = 8;
			strcpy((LPSTR)edPtr->textFont.lfFaceName, "Arial");
		}

		// Re-allocate EDITDATA structure according to the size of the default text
		DWORD dwNewSize = sizeof(EDITDATA) + _tcslen(szDefaultText);
		LPEDATA pNewPtr = (LPEDATA)mvReAllocEditData(mV, edPtr, dwNewSize);
		if ( pNewPtr != NULL )
		{
			edPtr = pNewPtr;

			// Initialize default text
			_tcscpy(edPtr->sText, szDefaultText);
		}
	return 0;	// No error

#endif // !RUN_ONLY

	// Error
	return -1;
}

// --------------------
// EditObject
// --------------------
// One of the option from the menu has been selected, and not a default menu option
// automatically handled by MMF2: this routine is then called.
//

BOOL WINAPI EditObject (mv _far *mV, fpObjInfo oiPtr, fpLevObj loPtr, LPEDATA edPtr)
{
#ifndef RUN_ONLY
	
	return EditProp(mV,edPtr,PROPID_ICONS);

#endif // !RUN_ONLY

	// Error
	return FALSE;
}

// --------------------
// SetEditSize
// --------------------
// Called by MMF2 when the object has been resized
//
// Note: remove the comments if your object can be resized (and remove the comments in the .def file)


BOOL WINAPI SetEditSize(LPMV mv, LPEDATA edPtr, int cx, int cy)
{
#ifndef RUN_ONLY
	edPtr->nWidth = cx;
	edPtr->nHeight = cy;
#endif // !RUN_ONLY
	return TRUE;	// OK
}


// --------------------
// PutObject
// --------------------
// Called when each individual object is dropped in the frame.
//

void WINAPI	DLLExport PutObject(mv _far *mV, fpLevObj loPtr, LPEDATA edPtr, ushort cpt)
{
#ifndef RUN_ONLY
#endif // !RUN_ONLY
}

// --------------------
// RemoveObject
// --------------------
// Called when each individual object is removed from the frame.
//

void WINAPI	DLLExport RemoveObject(mv _far *mV, fpLevObj loPtr, LPEDATA edPtr, ushort cpt)
{
#ifndef RUN_ONLY
	// Is the last object removed?
    if (0 == cpt)
	{
		// Do whatever necessary to remove our data
	}
#endif // !RUN_ONLY
}

// --------------------
// DuplicateObject
// --------------------
// Called when an object is created from another one (note: should be called CloneObject instead...)
//

void WINAPI DLLExport DuplicateObject(mv __far *mV, fpObjInfo oiPtr, LPEDATA edPtr)
{
#ifndef RUN_ONLY
#endif // !RUN_ONLY
}

// --------------------
// GetObjectRect
// --------------------
// Returns the size of the rectangle of the object in the frame editor.
//

void WINAPI DLLExport GetObjectRect(mv _far *mV, RECT FAR *rc, fpLevObj loPtr, LPEDATA edPtr)
{
#ifndef RUN_ONLY
	rc->right = rc->left + edPtr->nWidth;
	rc->bottom = rc->top + edPtr->nHeight;
#endif // !RUN_ONLY
	return;
}


// --------------------
// EditorDisplay
// --------------------
// Displays the object under the frame editor
//
// Note: this function is optional. If it's not defined in your extension,
// MMF2 will load and display the EXO_IMAGE bitmap if it's defined in your resource file.
//
// If you need to draw the icon manually, remove the comments around this function and in the .def file.
//


void WINAPI DLLExport EditorDisplay(mv _far *mV, fpObjInfo oiPtr, fpLevObj loPtr, LPEDATA edPtr, RECT FAR *rc)
{
#ifndef RUN_ONLY

	// This is a simple case of drawing an image onto MMF's frame editor window
	// First, we must get a pointer to the surface used by the frame editor

	LPSURFACE ps = WinGetSurface((int)mV->mvIdEditWin);
	if ( ps != NULL )		// Do the following if this surface exists
	{
		
		COLORREF dwBackColor = GetSysColor(COLOR_BTNFACE);
		ps->Fill(rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top, dwBackColor);
		ps->Rectangle(rc->left, rc->top, rc->right, rc->bottom, 1, RGB(0,0,0));
		ps->Rectangle(rc->left+3,min(rc->top+24,rc->bottom), rc->right-3, max(rc->top,rc->bottom-3), 1, RGB(0,0,0));

	}

#endif // !RUN_ONLY
}


// --------------------
// IsTransparent
// --------------------
// This routine tells MMF2 if the mouse pointer is over a transparent zone of the object.
// 

extern "C" BOOL WINAPI DLLExport IsTransparent(mv _far *mV, fpLevObj loPtr, LPEDATA edPtr, int dx, int dy)
{
#ifndef RUN_ONLY
	// Write your code here
#endif // !RUN_ONLY
	return FALSE;
}

// --------------------
// PrepareToWriteObject
// --------------------
// Just before writing the datazone when saving the application, MMF2 calls this routine.
// 

void WINAPI	DLLExport PrepareToWriteObject(mv _far *mV, LPEDATA edPtr, fpObjInfo adoi)
{
#ifndef RUN_ONLY
	// Write your code here
#endif // !RUN_ONLY
}

// --------------------
// GetFilters
// --------------------
//

BOOL WINAPI GetFilters(LPMV mV, LPEDATA edPtr, DWORD dwFlags, LPVOID pReserved)
{
#ifndef RUN_ONLY
	// If your extension uses image filters
//	if ( (dwFlags & GETFILTERS_IMAGES) != 0 )
//		return TRUE;

	// If your extension uses sound filters
//	if ( (dwFlags & GETFILTERS_SOUNDS) != 0 )
//		return TRUE;
#endif // RUN_ONLY
	return FALSE;
}

// --------------------
// UsesFile
// --------------------
// Triggers when a file is dropped onto the frame
// Return TRUE if you can create an object from the given file
//

BOOL WINAPI	DLLExport UsesFile (LPMV mV, LPTSTR fileName)
{
	BOOL r = FALSE;
#ifndef RUN_ONLY

	// Example: return TRUE if file extension is ".txt"
/*	
	LPSTR	ext, npath;

	if ( fileName != NULL )
	{
		if ( (ext=(LPSTR)malloc(_MAX_EXT)) != NULL )
		{
			if ( (npath=(LPSTR)malloc(_MAX_PATH)) != NULL )
			{
				strcpy(npath, fileName);
				_splitpath(npath, NULL, NULL, NULL, ext);
				if ( _stricmp(ext, ".txt") == 0 )
					r = TRUE;
				free(npath);
			}
			free(ext);
		}
	} */
#endif // !RUN_ONLY
	return r;
}


// --------------------
// CreateFromFile
// --------------------
// Creates a new object from file
//

void WINAPI	DLLExport CreateFromFile (LPMV mV, LPTSTR fileName, LPEDATA edPtr)
{
#ifndef RUN_ONLY
	// Initialize your extension data from the given file
//	edPtr->swidth = 32;
//	edPtr->sheight = 32;

	// Example: store the filename
	// strcpy(edPtr->myFileName, fileName);
#endif // !RUN_ONLY
}



// ============================================================================
//
// ROUTINES USED UNDER EVENT / TIME / STEP-THROUGH EDITOR
// You should not need to change these routines
// 
// ============================================================================

// -----------------
// menucpy
// -----------------
// Internal routine used later, copy one menu onto another
// 

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
// -----------------
// GetPopupMenu
// -----------------
// Internal routine used later. Returns the first popup from a menu
// 
HMENU GetPopupMenu(LPEDATA edPtr,short mn)
{
#ifndef RUN_ONLY
	HMENU hPopup=CreatePopupMenu();
	
	if (mn == MN_CONDITIONS)
		menucpy(hPopup,ConditionMenu(edPtr));
	else if (mn == MN_ACTIONS)
		menucpy(hPopup,ActionMenu(edPtr));
	else if (mn == MN_EXPRESSIONS)
		menucpy(hPopup,ExpressionMenu(edPtr));
	return hPopup;
#else
	return NULL;
#endif
}

// --------------------
// GetEventInformations
// --------------------
// Internal routine used later. Look for one event in one of the eventInfos array...
// No protection to go faster: you must properly enter the conditions/actions!
//

static LPEVENTINFOS2 GetEventInformations(LPEVENTINFOS2 eiPtr, short code)
{
#ifndef RUN_ONLY
	while (eiPtr->infos.code != code)
		eiPtr = EVINFO2_NEXT(eiPtr);
	
	return eiPtr;
#endif // !RUN_ONLY
	return NULL;
}



// ----------------------------------------------------
// GetConditionMenu / GetActionMenu / GetExpressionMenu
// ----------------------------------------------------
// Load the condition/action/expression menu from the resource, eventually
// enable or disable some options, and returns it to MMF2.
//

HMENU WINAPI DLLExport GetConditionMenu(mv _far *mV, fpObjInfo oiPtr, LPEDATA edPtr)
{
#ifndef RUN_ONLY
	// Check compatibility
	return GetPopupMenu(edPtr,MN_CONDITIONS);
#endif // !RUN_ONLY
	return NULL;
}

HMENU WINAPI DLLExport GetActionMenu(mv _far *mV, fpObjInfo oiPtr, LPEDATA edPtr)
{
#ifndef RUN_ONLY
	// Check compatibility
	return GetPopupMenu(edPtr,MN_ACTIONS);
#endif // !RUN_ONLY
	return NULL;
}

HMENU WINAPI DLLExport GetExpressionMenu(mv _far *mV, fpObjInfo oiPtr, LPEDATA edPtr)
{
#ifndef RUN_ONLY
	// Check compatibility
	return GetPopupMenu(edPtr,MN_EXPRESSIONS);
#endif // !RUN_ONLY
	return NULL;
}


// -------------------------------------------------------
// GetConditionTitle / GetActionTitle / GetExpressionTitle
// -------------------------------------------------------
// Returns the title of the dialog box displayed when entering
// parameters for the condition, action or expressions, if any.
// Here, we simply return the title of the menu option
//

#ifndef RUN_ONLY
void GetCodeTitle(LPEVENTINFOS2 eiPtr, short code, short param, short mn, LPTSTR strBuf, WORD maxLen)
{
	HMENU		hMn;

	// Finds event in array
	eiPtr=GetEventInformations(eiPtr, code);

	// If a special string is to be returned
	short strID = EVINFO2_PARAMTITLE(eiPtr, param);

	if (strID) {
		switch(mn) {
		case MN_CONDITIONS:
			if (code>=0&&code<(short)Conditions.size()) {
				if (param>=0&&param<(short)Conditions[code]->getParamCount())
					_tcscpy(strBuf,(TCHAR *)Conditions[code]->getParamName(param));
			}
			break;
		case MN_ACTIONS:
			if (code>=0&&code<(short)Actions.size()) {
				if (param>=0&&param<(short)Actions[code]->getParamCount())
					_tcscpy(strBuf,(TCHAR *)Actions[code]->getParamName(param));
			}
			break;
		case MN_EXPRESSIONS:
			if (code>=0&&code<(short)Expressions.size()) {
				if (param>=0&&param<(short)Expressions[code]->getParamCount())
					_tcscpy(strBuf,(TCHAR *)Expressions[code]->getParamName(param));
			}
			break;
		}
	} else {
		if ((hMn=LoadMenu(hInstLib, MAKEINTRESOURCE(mn)))) {
			GetMenuString(hMn, eiPtr->menu, strBuf, maxLen, MF_BYCOMMAND);
			DestroyMenu(hMn);
		}
	}
}
#else
#define GetCodeTitle(a,b,c,d,e,f)
#endif // !RUN_ONLY

void WINAPI DLLExport GetConditionTitle(mv _far *mV, short code, short param, LPTSTR strBuf, short maxLen) {
	GetCodeTitle((LPEVENTINFOS2)conditionsInfos, code, param, MN_CONDITIONS, strBuf, maxLen);
}
void WINAPI DLLExport GetActionTitle(mv _far *mV, short code, short param, LPTSTR strBuf, short maxLen) {
	GetCodeTitle((LPEVENTINFOS2)actionsInfos, code, param, MN_ACTIONS, strBuf, maxLen);
}
void WINAPI DLLExport GetExpressionTitle(mv _far *mV, short code, LPTSTR strBuf, short maxLen) {
	GetCodeTitle((LPEVENTINFOS2)expressionsInfos, code, 0, MN_EXPRESSIONS, strBuf, maxLen);
}

// -------------------------------------------------------
// GetConditionTitle / GetActionTitle / GetExpressionTitle
// -------------------------------------------------------
// From a menu ID, these routines returns the code of the condition,
// action or expression, as defined in the .H file
//

short WINAPI DLLExport GetConditionCodeFromMenu(mv _far *mV, short menuId)
{
#ifndef RUN_ONLY
	LPEVENTINFOS2 eiPtr;
	int n;
	for (n=Conditions.size(),eiPtr=(LPEVENTINFOS2)conditionsInfos;n>0&&eiPtr->menu!=menuId;n--)
		eiPtr=EVINFO2_NEXT(eiPtr);
	if (n>0)
		return eiPtr->infos.code;
#endif // !RUN_ONLY
	return -1;
}

short WINAPI DLLExport GetActionCodeFromMenu(mv _far *mV, short menuId)
{
#ifndef RUN_ONLY
	LPEVENTINFOS2 eiPtr;
	int n;

	for (n=Actions.size(),eiPtr=(LPEVENTINFOS2)actionsInfos;n>0&&eiPtr->menu!=menuId;n--)
		eiPtr = EVINFO2_NEXT(eiPtr);
	if (n>0) 
		return eiPtr->infos.code;
#endif // !RUN_ONLY
	return -1;
}

short WINAPI DLLExport GetExpressionCodeFromMenu(mv _far *mV, short menuId)
{
#ifndef RUN_ONLY
	LPEVENTINFOS2 eiPtr;
	int	n;
	for (n=Expressions.size(),eiPtr=(LPEVENTINFOS2)expressionsInfos;n>0&&eiPtr->menu!=menuId;n--)
		eiPtr = EVINFO2_NEXT(eiPtr);
	if (n>0) 
		return eiPtr->infos.code;
#endif // !RUN_ONLY
	return -1;
}


// -------------------------------------------------------
// GetConditionInfos / GetActionInfos / GetExpressionInfos
// -------------------------------------------------------
// From a action / condition / expression code, returns 
// an infosEvents structure. 
//

LPINFOEVENTSV2 WINAPI DLLExport GetConditionInfos(mv _far *mV, short code)
{
#ifndef RUN_ONLY
	return &GetEventInformations((LPEVENTINFOS2)conditionsInfos, code)->infos;
#endif // !RUN_ONLY
	return NULL;
}

LPINFOEVENTSV2 WINAPI DLLExport GetActionInfos(mv _far *mV, short code)
{
#ifndef RUN_ONLY
	return &GetEventInformations((LPEVENTINFOS2)actionsInfos, code)->infos;
#endif // !RUN_ONLY
	return NULL;
}

LPINFOEVENTSV2 WINAPI DLLExport GetExpressionInfos(mv _far *mV, short code)
{
#ifndef RUN_ONLY
	return &GetEventInformations((LPEVENTINFOS2)expressionsInfos, code)->infos;
#endif // !RUN_ONLY
	return NULL;
}


// ----------------------------------------------------------
// GetConditionString / GetActionString / GetExpressionString
// ----------------------------------------------------------
// From a action / condition / expression code, returns 
// the string to use for displaying it under the event editor
//

void WINAPI DLLExport GetConditionString(mv _far *mV, short code, LPTSTR strPtr, short maxLen)
{
#ifndef RUN_ONLY
		if (code>=0&&code<(short)Conditions.size())
			_tcscpy(strPtr,(TCHAR *)Conditions[code]->getName());
#endif // !RUN_ONLY
}

void WINAPI DLLExport GetActionString(mv _far *mV, short code, LPTSTR strPtr, short maxLen)
{
#ifndef RUN_ONLY
		if (code>=0&&code<(short)Actions.size())
			_tcscpy(strPtr,(TCHAR *)Actions[code]->getName());
#endif // !RUN_ONLY
}

void WINAPI DLLExport GetExpressionString(mv _far *mV, short code, LPTSTR strPtr, short maxLen)
{
#ifndef RUN_ONLY
		if (code>=0&&code<(short)Expressions.size())
			_tcscpy(strPtr,(TCHAR *)Expressions[code]->getName());
#endif // !RUN_ONLY
}

// ----------------------------------------------------------
// GetExpressionParam
// ----------------------------------------------------------
// Returns the parameter name to display in the expression editor
//

void WINAPI DLLExport GetExpressionParam(mv _far *mV, short code, short param, LPTSTR strBuf, short maxLen)
{
#ifndef RUN_ONLY
	if (_tcslen(Expressions[code]->getParamName(param))*sizeof(TCHAR))
		_tcscpy(strBuf,(TCHAR *)Expressions[code]->getParamName(param));
	else
		*strBuf=0;
#endif
}

// ----------------------------------------------------------
// Custom Parameters
// ----------------------------------------------------------

// --------------------
// InitParameter
// --------------------
// Initialize the parameter.
//

void WINAPI InitParameter(mv _far *mV, short code, paramExt* pExt)
{
#ifndef RUN_ONLY
	// Example
	// -------
	// strcpy(&pExt->pextData[0], "Parameter Test");
	// pExt->pextSize = sizeof(paramExt) + strlen(pExt->pextData)+1;
#endif // !RUN_ONLY
}

// Example of custom parameter setup proc
// --------------------------------------
/*
#if !RUN_ONLY
BOOL CALLBACK DLLExport SetupProc(HWND hDlg, UINT msgType, WPARAM wParam, LPARAM lParam)
{
	paramExt*			pExt;

	switch (msgType)
	{
		case WM_INITDIALOG: // Init dialog

			// Save edptr
			SetWindowLong(hDlg, DWL_USER, lParam);
			pExt=(paramExt*)lParam;

			SetDlgItemText(hDlg, IDC_EDIT, pExt->pextData);
			return TRUE;

		case WM_COMMAND: // Command

			// Retrieve edptr
			pExt = (paramExt *)GetWindowLong(hDlg, DWL_USER);

			switch (wmCommandID)
			{
			case IDOK:	// Exit
				GetDlgItemText(hDlg, IDC_EDIT, pExt->pextData, 500);
				pExt->pextSize=sizeof(paramExt)+strlen(pExt->pextData)+1;
				EndDialog(hDlg, TRUE);
				return TRUE;

				default:
					break;
			}
			break;

		default:
			break;
	}
	return FALSE;
}
#endif // !RUN_ONLY
*/

// --------------------
// EditParameter
// --------------------
// Edit the parameter.
//

void WINAPI EditParameter(mv _far *mV, short code, paramExt* pExt)
{
#ifndef RUN_ONLY

	// Example
	// -------
	// DialogBoxParam(hInstLib, MAKEINTRESOURCE(DB_TRYPARAM), mV->mvHEditWin, SetupProc, (LPARAM)(LPBYTE)pExt);

#endif // !RUN_ONLY
}

// --------------------
// GetParameterString
// --------------------
// Initialize the parameter.
//

void WINAPI GetParameterString(mv _far *mV, short code, paramExt* pExt, LPTSTR pDest, short size)
{
#ifndef RUN_ONLY

	// Example
	// -------
	// wsprintf(pDest, "Super parameter %s", pExt->pextData);

#endif // !RUN_ONLY
}

// -----------------
// GetObjInfos
// -----------------
// Return object info
//
// Info displayed in the object's About properties
// Note: ObjComment is also displayed in the Quick Description box in the Insert Object dialog box
//

void WINAPI	DLLExport GetObjInfos (mv _far *mV, LPEDATA edPtr, LPTSTR ObjName, LPTSTR ObjAuthor, LPTSTR ObjCopyright, LPTSTR ObjComment, LPTSTR ObjHttp)
{
#ifndef RUN_ONLY
	_tcscpy(ObjName,(TCHAR *)ObjectName);
	_tcscpy(ObjAuthor,(TCHAR *)ObjectAuthor);
	_tcscpy(ObjCopyright,(TCHAR *)ObjectCopyright);
	_tcscpy(ObjComment,(TCHAR *)ObjectComment);
	_tcscpy(ObjHttp,(TCHAR *)ObjectURL);
#endif
}

// -----------------
// GetHelpFileName
// -----------------
// Returns the help filename of the object.
//

LPCTSTR WINAPI GetHelpFileName()
{
#ifndef RUN_ONLY
	return (TCHAR *)ObjectHelp;
#else
	return NULL;
#endif
}

// -----------------
// GetRegID
// -----------------
// Returns the registered ID of the object
//

int WINAPI GetRegID()
{
#ifndef RUN_ONLY
	return ObjectRegID;
#else
	return REGID_RUNTIME;
#endif
}
