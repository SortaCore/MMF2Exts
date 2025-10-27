/* Copyright (c) 1996-2014 Clickteam
*
* This source code is part of the iOS exporter for Clickteam Multimedia Fusion 2
* and Clickteam Fusion 2.5.
*
* Permission is hereby granted to any person obtaining a legal copy
* of Clickteam Multimedia Fusion 2 or Clickteam Fusion 2.5 to use or modify this source
* code for debugging, optimizing, or customizing applications created with
* Clickteam Multimedia Fusion 2 and/or Clickteam Fusion 2.5.
* Any other use of this source code is prohibited.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/
//----------------------------------------------------------------------------------
//
// CLAYER : classe layer
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>
#import "CEffectEx.h"
#import "CoreMath.h"
#import "CRect.h"

@class CFile;
@class CArrayList;

#define FLOPT_XCOEF 0x0001
#define FLOPT_YCOEF 0x0002
#define FLOPT_NOSAVEBKD 0x0004
#define FLOPT_WRAP_OBSOLETE 0x0008
#define FLOPT_VISIBLE 0x0010
#define FLOPT_WRAP_HORZ 0x0020
#define FLOPT_WRAP_VERT 0x0040
#define FLOPT_REDRAW 0x000010000
#define FLOPT_TOHIDE 0x000020000
#define FLOPT_TOSHOW 0x000040000

@class CRun;
@class CRunFrame;

@interface CLayer : NSObject
{
@public
	NSString* pName;			/// Name

    // Offset
	int x;				/// Current offset
    int y;
    int dx;				/// Offset to apply to the next refresh
    int dy;

	int xOff;		//Cached scrolling value (coefficient applied - rounded to integer)
	int yOff;

    CArrayList* pBkd2;

    // Ladders
    CArrayList* pLadders;

    // Z-order max index for dynamic objects
    int nZOrderMax;

    // Permanent data (EditFrameLayer)
    int dwOptions;			/// Options
    double xCoef;
    double yCoef;
    int nBkdLOs;				/// Number of backdrop objects
    int nFirstLOIndex;			/// Index of first backdrop object in LO table

    // Backup for restart
    int backUp_dwOptions;
    double backUp_xCoef;
    double backUp_yCoef;
    int backUp_nBkdLOs;
    int backUp_nFirstLOIndex;

	CArrayList* m_loZones;
	CRunFrame* frame;


	float angle;
	float scale;
	float scaleX;
	float scaleY;
	float xDest;
	float yDest;
	float xSpot;
	float ySpot;

	CRect visibleRect;
	CRect handleRect;
	CRect killRect;

    int effect;
    int effectParam;
    int effectIndex;
    int effectNParams;
    int effectPOffset;
    int effectShader;
    int* effectData;
    CEffectEx* effectEx;
}

-(id)initWithFrame:(CRunFrame*)frame;
-(void)dealloc;
-(void)load:(CFile*)file;
-(Mat3f)getTransformMatrix;
-(void)updateVisibleRect;
-(void)scrollToX:(int)x andY:(int)y;

-(void)resetZones;

-(int)checkOrCreateEffectIfNeededByIndex:(int)index andEffectParam:(int)rgba;
-(int)checkOrCreateEffectIfNeededByName:(NSString*)name andEffectParam:(int)rgba;
-(int)checkOrCreateEffectIfNeeded:(CRunApp*)app;
-(int)checkOrCreateEffectIfNeeded:(CRunApp*)app andName:(NSString*)name;

@end
