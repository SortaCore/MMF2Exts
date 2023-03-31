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
// CRSPR : Gestion des objets sprites
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

#define RSFLAG_HIDDEN 0x0001
#define RSFLAG_INACTIVE 0x0002
#define RSFLAG_SLEEPING 0x0004
#define RSFLAG_SCALE_RESAMPLE 0x0008
#define RSFLAG_ROTATE_ANTIA 0x0010
#define RSFLAG_VISIBLE 0x0020
#define SPRTYPE_TRUESPRITE 0
#define SPRTYPE_OWNERDRAW 1
#define SPRTYPE_QUICKDISPLAY 2

@class CObject;
@class CSpriteGen;
@class CObjectCommon;
@class CCreateObjectInfo;
@class CSprite;
@class CTrans;
@class CFadeSprite;

@interface CRSpr : NSObject 
{
@public
	CObject* hoPtr;
    CSpriteGen* spriteGen;
    int rsFlash;				/// Flash objets
    double rsFlashCpt;
    short rsLayer;				/// Layer
    int rsZOrder;			/// Z-order value
    int rsCreaFlags;			/// Creation flags
    int rsBackColor;			/// background saving color
    int rsEffect;			/// Sprite effects
    int rsEffectParam;
    int rsEffectShader;  
    short rsFlags;			/// Handling flags
    short rsFadeCreaFlags;		/// Saved during a fadein
    short rsSpriteType;
    CTrans* rsTrans;
	CFadeSprite* fadeSprite;
}
-(void)dealloc;
-(id)initWithHO:(CObject*)ho andOC:(CObjectCommon*)ocPtr andCOB:(CCreateObjectInfo*)cobPtr;
-(void)init2;
-(void)reInit_Spr:(BOOL)fast;
-(void)displayRoutine;
-(void)handle;
-(void)modifRoutine;
-(BOOL)createSprite:(CSprite*)pSprBefore;
-(BOOL)kill:(BOOL)fast;
-(void)objGetZoneInfos;
-(void)obHide;
-(void)obShow;
-(BOOL)createFadeSprite:(BOOL)bFadeOut;
-(BOOL)checkEndFadeIn;
-(BOOL)checkEndFadeOut;

@end
