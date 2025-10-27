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
// CScore : Objet score
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "CObject.h"
#import "IDrawable.h"

@class CObjectCommon;
@class CCreateObjectInfo;
@class CImageBank;
@class CSprite;
@class CMask;
@class CValue;
@class CFontInfo;
@class CBitmap;
class CRenderer;
@class CTextSurface;

@interface CScore : CObject <IDrawable>
{
@public
	short rsPlayer;
    CValue* rsValue;
    int rsBoxCx;			// Dimensions box (for lives, counters, texts)
    int rsBoxCy;
    short rsFont;				// Temporary font for texts
    int rsColor1;			// Bar color
    int displayFlags;
	CTextSurface* textSurface;
	int oldScore;
	NSString* cachedString;
	NSUInteger cachedLength;
	int vInt;
}
-(void)dealloc;
-(void)initObject:(CObjectCommon*)ocPtr withCOB:(CCreateObjectInfo*)cob;
-(void)handle;
-(void)modif;
-(void)display;
-(void)getZoneInfos;
-(void)updateCachedData;
-(void)draw:(CRenderer*)renderer;
-(CMask*)getCollisionMask:(int)flags;
-(void)spriteDraw:(CRenderer*)renderer withSprite:(CSprite*)spr andImageBank:(CImageBank*)bank andX:(int)x andY:(int)y;
-(void)spriteKill:(CSprite*)spr;
-(CMask*)spriteGetMask;
-(CFontInfo*)getFont;
-(void)setFont:(CFontInfo*)info withRect:(CRect)pRc;
-(int)getFontColor;
-(void)setFontColor:(int)rgb;

@end
