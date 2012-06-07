// ============================================================================
//
// This file contains routines that are handled during the Runtime
// 
// ============================================================================

// Common Include
#include	"common.h"
//#include <cv.hpp>
//#include "highgui.h"


//#include	"image.cpp"
//
//void  FillBitmapInfo( BITMAPINFO* bmi, int width, int height, int bpp, int origin )
//{
//    assert( bmi && width >= 0 && height >= 0 && (bpp == 8 || bpp == 24 || bpp == 32));
//
//    BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);
//
//    memset( bmih, 0, sizeof(*bmih));
//    bmih->biSize = sizeof(BITMAPINFOHEADER);
//    bmih->biWidth = width;
//    bmih->biHeight = origin ? abs(height) : -abs(height);
//    bmih->biPlanes = 1;
//    bmih->biBitCount = (unsigned short)bpp;
//    bmih->biCompression = BI_RGB;
//
//    if ( bpp == 8 )
//    {
//        RGBQUAD* palette = bmi->bmiColors;
//        int i;
//        for ( i = 0; i < 256; i++ )
//        {
//            palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
//            palette[i].rgbReserved = 0;
//        }
//    }
//}

short WINAPI DLLExport GetRunObjectDataSize(fprh rhPtr, LPEDATA edPtr) { return(sizeof(RUNDATA)); }

//cSurface * createSurface(int width, int height, LPRDATA rdPtr){
//	LPRH	rhPtr = rdPtr->rHo.hoAdRunHeader;
//	LPSURFACE wSurf = WinGetSurface((int)rhPtr->rhIdEditWin);
//	LPSURFACE proto;
//	GetSurfacePrototype(&proto, (wSurf != NULL) ? wSurf->GetDepth() : 24, ST_MEMORYWITHDC, SD_DIB);
//	if ( proto == NULL )
//		return NULL;
//
//	cSurface* psf = NewSurface();
//
//	if ( psf == NULL )
//		return NULL;
//
//	psf->Create(width, height, proto);
//	
//	psf->Fill(RGB(1,0,0));
//	psf->SetTransparentColor(RGB(0, 0, 0));
//	
//	if ( psf->GetDepth() == 8 )
//		psf->SetPalette (*wSurf);
//
//	return psf;
//}

// CreateRunObject
short WINAPI DLLExport CreateRunObject(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr) {
	// Do some rSDK stuff
	#include "rCreateRunObject.h"

	LPRH	rhPtr  = rdPtr->rHo.hoAdRunHeader;
	// List of object offsets


	rdPtr->rHo.hoX = cobPtr->cobX;
	rdPtr->rHo.hoY = cobPtr->cobY;
	rdPtr->rHo.hoImgWidth = edPtr->swidth;
	rdPtr->rHo.hoImgHeight = edPtr->sheight;
	
	rdPtr->firstPoint = NULL;
	rdPtr->lastPoint = NULL;
	rdPtr->currentPointOnLoop = NULL;

	//rdPtr->polygon_size = 50;
	//
	//int width = rdPtr->rHo.hoImgWidth;
	//int height = rdPtr->rHo.hoImgHeight;

	//rdPtr->pSf = createSurface(width, height, rdPtr);

	//rdPtr->myImage=cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	//cvFillImage(rdPtr->myImage, 0xFFFFFF);

	//cvRectangle(rdPtr->myImage, cvPoint(100,100), cvPoint(200,200), CV_RGB(255,0,0), 1);

	// No errors
	return 0;
}

// DestroyRunObject
short WINAPI DLLExport DestroyRunObject(LPRDATA rdPtr, long fast) {
	// Delete picture surface
	//if ( rdPtr->pSf != NULL )
	//{
	//	rdPtr->pSf->Delete();
	//	//rdPtr->pSf = NULL;
	//}
	//if ( rdPtr->lamp != NULL )
	//{
	//	rdPtr->lamp->Delete();
	//	//rdPtr->pSf = NULL;
	//}
	
	Linked2DPoint* currentPoint;

	do{
		currentPoint = rdPtr->firstPoint;

		if (currentPoint == NULL){
			break;
		}
		
		rdPtr->firstPoint = currentPoint->nextPoint;

		delete currentPoint;
	}while (TRUE);

	rdPtr->lastPoint = NULL;

	delete rdPtr->rRd;

	return 0;
}

// HandleRunObject
short WINAPI DLLExport HandleRunObject(LPRDATA rdPtr) {
	rdPtr->rc.rcChanged = TRUE;

	return rdPtr->rc.rcChanged?REFLAG_DISPLAY:0; 
//	return REFLAG_DISPLAY; 

}

// DisplayRunObject
short WINAPI DLLExport DisplayRunObject(LPRDATA rdPtr) {
//	HDC myImageDC = rdPtr->pSf->GetDC();
//
//RECT destRect;
//destRect.top=0;
//destRect.left=0;
//destRect.right=rdPtr->rHo.hoImgWidth;
//destRect.bottom=rdPtr->rHo.hoImgHeight;
//
//	CvvImage cvvImage;
//	cvvImage.CopyOf(rdPtr->myImage, -1);
//	cvvImage.DrawToHDC(myImageDC, &destRect);
//	cvvImage.Destroy();
//
//	rdPtr->pSf->ReleaseDC(myImageDC);
//
//	
//
//	fprh rhPtr = rdPtr->rHo.hoAdRunHeader;
//	LPSURFACE ps = WinGetSurface((int)rhPtr->rhIdEditWin);
//
//	rdPtr->pSf->Blit(*ps,
//		rdPtr->rHo.hoX, rdPtr->rHo.hoY,	//Dest XY
//		0, 0, //Src XY
//		rdPtr->rHo.hoImgWidth, rdPtr->rHo.hoImgHeight, // Width/height
//		(BlitMode)(rdPtr->rHo.hoOiList->oilInkEffect>>28), //BMODE_TRANSP,
//		(BlitOp)((rdPtr->rHo.hoOiList->oilInkEffect<<16)>>16), //BOP_COPY,
//		rdPtr->rHo.hoOiList->oilEffectParam,
//		0); //BLTF_COPYALPHA
//
//	WinAddZone(rhPtr->rhIdEditWin, &rdPtr->rHo.hoRect);

	return 0;
}

// -------------------
// GetRunObjectSurface
// -------------------
// Implement this function instead of DisplayRunObject if your extension
// supports ink effects and transitions. Note: you can support ink effects
// in DisplayRunObject too, but this is automatically done if you implement
// GetRunObjectSurface (MMF applies the ink effect to the transition).
//
// Note: do not forget to enable the function in the .def file 
// if you remove the comments below.

//cSurface* WINAPI DLLExport GetRunObjectSurface(LPRDATA rdPtr)
//{
//	//do{
//	//	int		i;
//	//	vec3_t	cam;
//	//	
//	//	rdPtr->frame++;
//	//	
//	//	cam[0] = rdPtr->xAngle;
//	//	cam[1] = rdPtr->yAngle;
//	//	cam[2] = rdPtr->zAngle + 90;
//
//	//	GenerateMatrix(cam, &rdPtr->lMatrix, rdPtr );
//
//	//	if (rdPtr->voxel.active){
//	//		memset(rdPtr->zstack, 100, VPATCH_SIZE * VPATCH_SIZE * 2);
//	//		memset(rdPtr->screenDot, 0, VPATCH_SIZE * VPATCH_SIZE * 3);
//
//	//		for (i = 0 ; i < rdPtr->voxel.partCount ; i ++ )
//	//			voxel_Set_part(i,rdPtr->frame % rdPtr->voxel.numFrames, rdPtr);
//	//		
//	//		rdPtr->pSf->Fill(RGB(0,0,0));
//	//		
//	//		voxel_Render_unit(rdPtr->pSf, rdPtr);
//	//	}
//	//}while (FALSE);
//
//
//	//for (int i = 0; i < voxelLimbHeaders[0].unknown2; i++){
//	//	voxelDot_t dot = voxelParts[i];
//	//
//	//	#ifdef VOXEL_DEBUG
//	//	fprintf(debug, "drawing dot: %8.2f, %8.2f, %d %02X = %08X\n", dot.position[0], dot.position[1], dot.color, dot.color, ts_palette[dot.color]);
//	//	fflush(debug);
//	//	#endif
//	//
//	//	if (ts_palette[dot.color] == 0){
//	//		rdPtr->pSf->SetPixel((int)dot.position[0], (int)dot.position[1], ts_palette[dot.color] + 1);
//	//	} else {
//	//		rdPtr->pSf->SetPixel((int)dot.position[0], (int)dot.position[1], ts_palette[dot.color]);
//	//	}
//	//}
//
//	return rdPtr->pSf;
//}


// -------------------------
// GetRunObjectCollisionMask
// -------------------------
// Implement this function if your extension supports fine collision mode (OEPREFS_FINECOLLISIONS),
// Or if it's a background object and you want Obstacle properties for this object.
//
// Should return NULL if the object is not transparent.
//
// Note: do not forget to enable the function in the .def file 
// if you remove the comments below.
//
/*
cSurface* WINAPI DLLExport GetRunObjectCollisionMask(LPRDATA rdPtr, LPARAM lParam)
{
	// Typical example for active objects
	// ----------------------------------
	// Opaque? collide with box
	if ( (rdPtr->rs.rsEffect & EFFECTFLAG_TRANSPARENT) == 0 )	// Note: only if your object has the OEPREFS_INKEFFECTS option
		return NULL;

	// Transparent? Create mask
	LPSMASK pMask = rdPtr->m_pColMask;
	if ( pMask == NULL )
	{
		if ( rdPtr->m_pSurface != NULL )
		{
			DWORD dwMaskSize = rdPtr->m_pSurface->CreateMask(NULL, lParam);
			if ( dwMaskSize != 0 )
			{
				pMask = (LPSMASK)calloc(dwMaskSize, 1);
				if ( pMask != NULL )
				{
					rdPtr->m_pSurface->CreateMask(pMask, lParam);
					rdPtr->m_pColMask = pMask;
				}
			}
		}
	}

	// Note: for active objects, lParam is always the same.
	// For background objects (OEFLAG_BACKGROUND), lParam maybe be different if the user uses your object
	// as obstacle and as platform. In this case, you should store 2 collision masks
	// in your data: one if lParam is 0 and another one if lParam is different from 0.

	return pMask;
}
*/

// ----------------
// PauseRunObject
// ----------------
// Enters the pause mode
// 
short WINAPI DLLExport PauseRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}


// -----------------
// ContinueRunObject
// -----------------
// Quits the pause mode
//
short WINAPI DLLExport ContinueRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}


// ============================================================================
//
// START APP / END APP / START FRAME / END FRAME routines
// 
// ============================================================================

// -------------------
// StartApp
// -------------------
// Called when the application starts or restarts.
// Useful for storing global data
// 
void WINAPI DLLExport StartApp(mv _far *mV, CRunApp* pApp)
{
	

	// Example
	// -------
	// Delete global data (if restarts application)
//	CMyData* pData = (CMyData*)mV->mvGetExtUserData(pApp, hInstLib);
//	if ( pData != NULL )
//	{
//		delete pData;
//		mV->mvSetExtUserData(pApp, hInstLib, NULL);
//	}
}

// -------------------
// EndApp
// -------------------
// Called when the application ends.
// 
void WINAPI DLLExport EndApp(mv _far *mV, CRunApp* pApp)
{
	// Example
	// -------
	// Delete global data
//	CMyData* pData = (CMyData*)mV->mvGetExtUserData(pApp, hInstLib);
//	if ( pData != NULL )
//	{
//		delete pData;
//		mV->mvSetExtUserData(pApp, hInstLib, NULL);
//	}
}

// -------------------
// StartFrame
// -------------------
// Called when the frame starts or restarts.
// 
void WINAPI DLLExport StartFrame(mv _far *mV, DWORD dwReserved, int nFrameIndex)
{

}

// -------------------
// EndFrame
// -------------------
// Called when the frame ends.
// 
void WINAPI DLLExport EndFrame(mv _far *mV, DWORD dwReserved, int nFrameIndex)
{

}

// ============================================================================
//
// TEXT ROUTINES (if OEFLAG_TEXT)
// 
// ============================================================================

// -------------------
// GetRunObjectFont
// -------------------
// Return the font used by the object.
// 
/*

  // Note: do not forget to enable the functions in the .def file 
  // if you remove the comments below.

void WINAPI GetRunObjectFont(LPRDATA rdPtr, LOGFONT* pLf)
{
	// Example
	// -------
	// GetObject(rdPtr->m_hFont, sizeof(LOGFONT), pLf);
}

// -------------------
// SetRunObjectFont
// -------------------
// Change the font used by the object.
// 
void WINAPI SetRunObjectFont(LPRDATA rdPtr, LOGFONT* pLf, RECT* pRc)
{
	// Example
	// -------
//	HFONT hFont = CreateFontIndirect(pLf);
//	if ( hFont != NULL )
//	{
//		if (rdPtr->m_hFont!=0)
//			DeleteObject(rdPtr->m_hFont);
//		rdPtr->m_hFont = hFont;
//		SendMessage(rdPtr->m_hWnd, WM_SETFONT, (WPARAM)rdPtr->m_hFont, FALSE);
//	}

}

// ---------------------
// GetRunObjectTextColor
// ---------------------
// Return the text color of the object.
// 
COLORREF WINAPI GetRunObjectTextColor(LPRDATA rdPtr)
{
	// Example
	// -------
	return 0;	// rdPtr->m_dwColor;
}

// ---------------------
// SetRunObjectTextColor
// ---------------------
// Change the text color of the object.
// 
void WINAPI SetRunObjectTextColor(LPRDATA rdPtr, COLORREF rgb)
{
	// Example
	// -------
	rdPtr->m_dwColor = rgb;
	InvalidateRect(rdPtr->m_hWnd, NULL, TRUE);
}
*/


// ============================================================================
//
// DEBUGGER ROUTINES
// 
// ============================================================================

// -----------------
// GetDebugTree
// -----------------
// This routine returns the address of the debugger tree
//
LPWORD WINAPI DLLExport GetDebugTree(LPRDATA rdPtr)
{
#if !defined(RUN_ONLY)
	return DebugTree;
#else
	return NULL;
#endif // !defined(RUN_ONLY)
}

// -----------------
// GetDebugItem
// -----------------
// This routine returns the text of a given item.
//
void WINAPI DLLExport GetDebugItem(LPSTR pBuffer, LPRDATA rdPtr, int id)
{
#if !defined(RUN_ONLY)

	// Example
	// -------
/*
	char temp[DB_BUFFERSIZE];

	switch (id)
	{
	case DB_CURRENTSTRING:
		LoadString(hInstLib, IDS_CURRENTSTRING, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->text);
		break;
	case DB_CURRENTVALUE:
		LoadString(hInstLib, IDS_CURRENTVALUE, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->value);
		break;
	case DB_CURRENTCHECK:
		LoadString(hInstLib, IDS_CURRENTCHECK, temp, DB_BUFFERSIZE);
		if (rdPtr->check)
			wsprintf(pBuffer, temp, "TRUE");
		else
			wsprintf(pBuffer, temp, "FALSE");
		break;
	case DB_CURRENTCOMBO:
		LoadString(hInstLib, IDS_CURRENTCOMBO, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->combo);
		break;
	}
*/

#endif // !defined(RUN_ONLY)
}

// -----------------
// EditDebugItem
// -----------------
// This routine allows to edit editable items.
//
void WINAPI DLLExport EditDebugItem(LPRDATA rdPtr, int id)
{
#if !defined(RUN_ONLY)

	// Example
	// -------
/*
	switch (id)
	{
	case DB_CURRENTSTRING:
		{
			EditDebugInfo dbi;
			char buffer[256];

			dbi.pText=buffer;
			dbi.lText=TEXT_MAX;
			dbi.pTitle=NULL;

			strcpy(buffer, rdPtr->text);
			long ret=callRunTimeFunction(rdPtr, RFUNCTION_EDITTEXT, 0, (LPARAM)&dbi);
			if (ret)
				strcpy(rdPtr->text, dbi.pText);
		}
		break;
	case DB_CURRENTVALUE:
		{
			EditDebugInfo dbi;

			dbi.value=rdPtr->value;
			dbi.pTitle=NULL;

			long ret=callRunTimeFunction(rdPtr, RFUNCTION_EDITINT, 0, (LPARAM)&dbi);
			if (ret)
				rdPtr->value=dbi.value;
		}
		break;
	}
*/
#endif // !defined(RUN_ONLY)
}


