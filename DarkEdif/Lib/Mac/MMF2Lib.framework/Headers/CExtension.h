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
// CEXTENSION: Objets d'extension
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "CObject.h"
#import "IDrawable.h"

@class CBitmap;
class CRenderer;
@class CRunExtension;
@class CBitmap;
@class CCndExtension;
@class CActExtension;
@class CValue;
@class CImage;
@class CRunApp;

@interface CExtension : CObject <IDrawable>
{
@public
	CRunExtension* ext;
    BOOL noHandle;
    int privateData;
    int objectCount;
    int objectNumber;
}
-(void)dealloc;
-(id)initWithType:(int)type andRun:(CRun*)rhPtr;
-(void)initObject:(CObjectCommon*)ocPtr withCOB:(CCreateObjectInfo*)cob;
-(void)runtimeIsReady;
-(void)handle;
-(void)modif;
-(void)display;
-(BOOL)kill:(BOOL)bFast;
-(void)getZoneInfos;
-(void)draw:(CRenderer*)renderer;
-(CMask*)getCollisionMask:(int)flags;
-(void)spriteDraw:(CRenderer*)renderer withSprite:(CSprite*)spr andImageBank:(CImageBank*)bank andX:(int)x andY:(int)y;
-(void)spriteKill:(CSprite*)spr;
-(CMask*)spriteGetMask;
-(BOOL)condition:(int)num withCndExtension:(CCndExtension*)cnd;
-(void)action:(int)num withActExtension:(CActExtension*)act;
-(CValue*)expression:(int)num;
-(CRunApp*)getApplication;
-(void)loadImageList:(unsigned short*)list withLength:(int)length;
-(void)loadImageByHandle:(unsigned short)handle;
-(CImage*)getImage:(unsigned short)handle;
-(void)reHandle;
-(int)getExtUserData;
-(void)setExtUserData:(int)data;
-(void)addBackdrop:(CImage*)img withX:(int)x andY:(int)y andEffect:(int)dwEffect andEffectParam:(int)dwEffectParam andType:(int)typeObst andLayer:(int)nLayer;
-(int)getEventCount;
-(CValue*)getExpParam;
-(int)getEventParam;
-(double)callMovement:(CObject*)hoPtr withAction:(int)action andParam:(double)param;
-(CValue*)callExpression:(CObject*)hoPtrw withExpression:(int)action andParam:(int)param;
-(int)getExpressionParam;
-(CObject*)getFirstObject;
-(CObject*)getNextObject;

@end
