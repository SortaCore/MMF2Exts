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
// COC: classe abstraite d'objectsCommon
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "IEnum.h"
#import "IDrawable.h"

#define OBSTACLE_NONE 0
#define OBSTACLE_SOLID 1
#define OBSTACLE_PLATFORM 2
#define OBSTACLE_LADDER 3
#define OBSTACLE_TRANSPARENT4

@class CFile;
@class CMask;
@class CSprite;
@class CImageBank;
@class CRunApp;
@class CEffectEx;
class CRenderer;
@class COI;

@interface COC : NSObject <IDrawable>
{
@public
	short ocObstacleType;		// Obstacle type
    short ocColMode;			// Collision mode (0 = fine, 1 = box)
    int ocCx;				// Size
    int ocCy;
    
    int ocEffectShader;
    CEffectEx* ocEffect;
}
-(void)load:(CFile*)file withType:(short)type andCOI:(COI*)pOI;
-(void)enumElements:(id)enumImages withFont:(id)enumFonts;
-(void)spriteDraw:(CGContextRef)g withSprite:(CSprite*)spr andImageBank:(CImageBank*)bank andX:(int)x andY:(int)y;
-(void)spriteKill:(CSprite*)spr;
-(CMask*)spriteGetMask;
-(int)checkOrCreateEffectIfNeeded:(CRunApp*)app andCOI:(COI*)oiPtr;
-(void)fillEffectData:(COI*)oiPtr;

@end
