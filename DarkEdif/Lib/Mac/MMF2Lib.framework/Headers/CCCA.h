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
// CCCA : Objet sub-application
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "CObject.h"
#import "IDrawable.h"

#define CCAF_SHARE_GLOBALVALUES 0x00000001
#define CCAF_SHARE_LIVES 0x00000002
#define CCAF_SHARE_SCORES 0x00000004
#define CCAF_SHARE_WINATTRIB 0x00000008
#define CCAF_STRETCH 0x00000010
#define CCAF_POPUP 0x00000020
#define CCAF_CAPTION 0x00000040
#define CCAF_TOOLCAPTION 0x00000080
#define CCAF_BORDER 0x00000100
#define CCAF_WINRESIZE 0x00000200
#define CCAF_SYSMENU 0x00000400
#define CCAF_DISABLECLOSE 0x00000800
#define CCAF_MODAL 0x00001000
#define CCAF_DIALOGFRAME 0x00002000
#define CCAF_INTERNAL 0x00004000
#define CCAF_HIDEONCLOSE 0x00008000
#define CCAF_CUSTOMSIZE 0x00010000
#define CCAF_INTERNALABOUTBOX 0x00020000
#define CCAF_CLIPSIBLINGS 0x00040000
#define CCAF_SHARE_PLAYERCTRLS 0x00080000
#define CCAF_MDICHILD 0x00100000
#define CCAF_DOCKED 0x00200000
#define CCAF_DOCKING_AREA 0x00C00000
#define CCAF_DOCKED_LEFT 0x00000000
#define CCAF_DOCKED_TOP 0x00400000
#define CCAF_DOCKED_RIGHT 0x00800000
#define CCAF_DOCKED_BOTTOM 0x00C00000
#define CCAF_REOPEN 0x01000000
#define CCAF_MDIRUNEVENIFNOTACTIVE 0x02000000
#define CCAF_HIDDENATSTART 0x04000000

@class CRunApp;
@class CSprite;
@class CValue;
@class CRenderToTexture;

@interface CCCA : CObject <IDrawable>
{
@public
	int flags;
    int odOptions;
    CRunApp* subApp;
    int oldX;
    int oldY;
    int oldWidth;
    int oldHeight;
    int level;
    int oldLevel;
	CSprite* sprite;
	BOOL bVisible;
	CRenderToTexture* rtt;
}
-(void)startCCA:(CObjectCommon*)ocPtr withStartFrame:(int)nStartFrame;
-(void)initObject:(CObjectCommon*)ocPtr withCOB:(CCreateObjectInfo*)cob;
-(BOOL)kill:(BOOL)bFast;
-(void)handle;
-(void)modif;
-(void)restartApp;
-(void)endApp;
-(void)hide;
-(void)show;
-(void)jumpFrame:(int)frame;
-(void)nextFrame;
-(void)previousFrame;
-(void)restartFrame;
-(void)pause;
-(void)resume;
-(void)setGlobalValue:(int)number withValue:(CValue*)value;
-(void)setGlobalString:(int)number withString:(NSString*)value;
-(BOOL)isPaused;
-(BOOL)appFinished;
-(BOOL)isVisible;
-(BOOL)frameChanged;
-(NSString*)getGlobalString:(int)num;
-(CValue*)getGlobalValue:(int)num;
-(int)getFrameNumber;
-(void)bringToFront;
-(void)spriteDraw:(CRenderer*)renderer withSprite:(CSprite*)spr andImageBank:(CImageBank*)bank andX:(int)x andY:(int)y;
-(void)spriteKill:(CSprite*)spr;
-(CMask*)spriteGetMask;
-(void)mouseMoved:(int)x withY:(int)y;
-(void)mouseClicked:(int)numTap withButton:(int)button;
-(void)leftMouseDown:(BOOL)bFlag;
-(void)rightMouseDown:(BOOL)bFlag;

@end
