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
// CSPRITEGEN : Generateur de sprites
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>
#import "CRect.h"

#define AS_DEACTIVATE 0x0000		// Desactive un sprite actif
#define AS_REDRAW 0x0001			// Reaffiche un sprite inactif
#define AS_ACTIVATE 0x0002			// Active un sprite inactif
#define AS_ENABLE 0x0004
#define AS_DISABLE 0x0008
#define AS_REDRAW_NOBKD 0x0011
#define AS_REDRAW_RECT 0x0020
#define GS_BACKGROUND 0x0001
#define GS_SAMELAYER 0x0002
#define CM_BOX 0
#define CM_BITMAP 1
#define PSCF_CURRENTSURFACE 0x0001
#define PSCF_TEMPSURFACE 0x0002
#define LAYER_ALL -1
#define EFFECT_NONE 0
#define EFFECT_SEMITRANSP 1
#define EFFECT_INVERTED 2
#define EFFECT_XOR 3
#define EFFECT_AND 4
#define EFFECT_OR 5
#define PSF_HOTSPOT 0x0001		// Take hot spot into account
#define PSF_NOTRANSP 0x0002		// Non transparent image... ignored in PasteSpriteEffect
#define SCF_OBSTACLE 1
#define SCF_PLATFORM 2
#define SCF_EVENNOCOL 4
#define SCF_BACKGROUND 8

@class CSprite;
@class CRunFrame;
@class CRunApp;
@class CImageBank;
@class CObject;
@class CMask;
@class CArrayList;
@class CBitmap;
@class CPSCM;
@class CRunFrame;
class CRenderer;

@interface CSpriteGen : NSObject
{
@public
	CSprite* firstSprite;
    CSprite* lastSprite;
    CRunApp* app;
	CRunFrame* frame;
    CImageBank* bank;
	CBitmap* spritesBack;
    short colMode;
}

-(id)initWithBank:(CImageBank*)b andApp:(CRunApp*)a;
-(void)dealloc;
-(CSprite*)addSprite:(int)xSpr withY:(int)ySpr andImage:(short)iSpr andLayer:(short)wLayer andZOrder:(int)nZOrder andBackColor:(int)backSpr andFlags:(int)sFlags andObject:(CObject*)extraInfo;
-(CSprite*)addOwnerDrawSprite:(int)x1 withY1:(int)y1 andX2:(int)x2 andY2:(int)y2 andLayer:(short)wLayer andZOrder:(int)nZOrder andBackColor:(int)backSpr andFlags:(int)sFlags andObject:(CObject*)extraInfo andDrawable:(id)sprProc;
-(CSprite*)modifSprite:(CSprite*)ptSpr withX:(int)xSpr andY:(int)ySpr andImage:(short)iSpr;
-(CSprite*)modifSpriteEx:(CSprite*)ptSpr withX:(int)xSpr andY:(int)ySpr andImage:(short)iSpr andScaleX:(float)fScaleX andScaleY:(float)fScaleY andScaleFlag:(BOOL)bResample andAngle:(float)nAngle andRotateFlag:(BOOL)bAntiA;
-(CSprite*)modifSpriteEffect:(CSprite*)ptSpr withInkEffect:(int)effect  andInkEffectParam:(int)effectParam andEffectShader:(int)effectShader ;
-(CSprite*)modifOwnerDrawSprite:(CSprite*)ptSprModif withX1:(int)x1 andY1:(int)y1 andX2:(int)x2 andY2:(int)y2;
-(void)setSpriteLayer:(CSprite*)ptSpr withLayer:(int)nLayer;
-(void)setSpriteScale:(CSprite*)ptSpr withScaleX:(float)fScaleX andScaleY:(float)fScaleY andFlag:(BOOL)bResample;
-(void)setSpriteAngle:(CSprite*)ptSpr withAngle:(float)nAngle andFlag:(BOOL)bAntiA;
-(void)sortLastSprite:(CSprite*)ptSprOrg;
-(void)swapSprites:(CSprite*)sp1 withSprite:(CSprite*)sp2;
-(void)moveSpriteToFront:(CSprite*)pSpr;
-(void)moveSpriteToBack:(CSprite*)pSpr;
-(void)moveSpriteBefore:(CSprite*)pSprToMove withSprite:(CSprite*)pSprDest;
-(void)moveSpriteAfter:(CSprite*)pSprToMove withSprite:(CSprite*)pSprDest;
-(BOOL)isSpriteBefore:(CSprite*)pSpr withSprite:(CSprite*)pSprDest;
-(BOOL)isSpriteAfter:(CSprite*)pSpr withSprite:(CSprite*)pSprDest;
-(CSprite*)getFirstSprite:(int)nLayer withFlags:(int)dwFlags;
-(CSprite*)getNextSprite:(CSprite*)pSpr withFlags:(int)dwFlags;
-(CSprite*)getPrevSprite:(CSprite*)pSpr withFlags:(int)dwFlags;
-(void)showSprite:(CSprite*)ptSpr withFlag:(BOOL)showFlag;
-(void)activeSprite:(CSprite*)ptSpr withFlags:(int)activeFlag andRect:(CRect)reafRect;
-(void)killSprite:(CSprite*)ptSprToKill withFast:(BOOL)bFast;
-(void)delSprite:(CSprite*)ptSprToDel;
-(void)delSpriteFast:(CSprite*)ptSpr;
-(CMask*)getSpriteMask:(CSprite*)ptSpr withImage:(short)newImg andFlags:(int)nFlags andAngle:(float)newAngle andScaleX:(double)newScaleX andScaleY:(double)newScaleY;
-(CMask*)getSpriteMask:(CSprite*)ptSpr withImage:(short)newImg andFlags:(int)nFlags;
-(void)spriteUpdate;
-(void)spriteClear;
-(void)pasteSpriteEffect:(CRenderer*)renderer withImage:(short)iNum andX:(int)iX andY:(int)iY andFlags:(int)flags andInkEffect:(int)effect andInkEffectParam:(int)effectParam andEffectShader:(int)effectShader;
-(void)winDrawSprites:(CRenderer*)renderer;
-(void)drawSprite:(CSprite*)sprite withRenderer:(CRenderer*)renderer;
-(CSprite*)getLastSprite:(int)nLayer withFlags:(int)dwFlags;
-(CSprite*)winAllocSprite;
-(void)winFreeSprite:(CSprite*)spr;
-(void)winSetColMode:(short)c;
-(CSprite*)spriteCol_TestPoint:(CSprite*)firstSpr withLayer:(short)nLayer andX:(int)xp andY:(int)yp andFlags:(int)dwFlags;
-(BOOL)spriteCol_TestPointOne:(CSprite*)firstSpr withLayer:(short)nLayer andX:(int)xp andY:(int)yp andFlags:(int)dwFlags;
-(CArrayList*)spriteCol_TestSprite_All:(CSprite*)ptSpr withImage:(short)newImg andX:(int)newX andY:(int)newY andAngle:(float)newAngle andScaleX:(float)newScaleX andScaleY:(float)newScaleY andFlags:(int)dwFlags;
-(CSprite*)spriteCol_TestSprite:(CSprite*)ptSpr withImage:(short)newImg andX:(int)newX andY:(int)newY andAngle:(float)newAngle andScaleX:(float)newScaleX andScaleY:(float)newScaleY andFoot:(int)subHt andFlags:(int)dwFlags;
-(CSprite*)spriteCol_TestRect:(CSprite*)firstSpr withLayer:(int)nLayer andX:(int)xp andY:(int)yp andWidth:(int)wp andHeight:(int)hp andFlags:(int)dwFlags;
-(void)recalcSpriteSurface:(CSprite*)ptSpr;
-(void)spriteDraw:(CRenderer*)renderer;
-(void)setFrame:(CRunFrame*)f;
-(int)getNSprites;

@end
