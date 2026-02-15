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
// --------------------------------------------------------------------------
//
// VIRTUAL JOYSTICK
//
// --------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>
#import "ITouches.h"

#define KEY_JOYSTICK 0
#define KEY_FIRE1 1
#define KEY_FIRE2 2
#define KEY_NONE -1
#define MAX_TOUCHES 3

#define JFLAG_JOYSTICK		0x0001
#define JFLAG_FIRE1			0x0002
#define JFLAG_FIRE2			0x0004
#define JFLAG_LEFTHANDED	0x0008
#define JFLAG_DPAD          0x0010
#define PI 3.141592654
#define JPOS_NOTDEFINED 0x80000000


@class CRunApp;
@class CImage;
@interface CJoystick : NSObject <ITouches>
{
@public
	CRunApp* app;
	UIImage* joyBack;
	UIImage* joyFront;
    UIImage* joyUp;
    UIImage* joyUpD;
    UIImage* joyDown;
    UIImage* joyDownD;
    UIImage* joyLeft;
    UIImage* joyLeftD;
    UIImage* joyRight;
    UIImage* joyRightD;
	UIImage* fire1U;
	UIImage* fire2U;
	UIImage* fire1D;
	UIImage* fire2D;
	UITouch* touches[3];
	BOOL bLandScape;
	int imagesX[3];
	int imagesY[3];
	int joystickX;
	int joystickY;
	int joystick;
	int flags;
    double zoom;
    double joydeadzone;
    double joyanglezone;
    int joyradsize;
    int COMFORT_FACTOR;

	CImage* joyBackTex;
	CImage* joyFrontTex;
    CImage* joyUpTex;
    CImage* joyUpDTex;
    CImage* joyDownTex;
    CImage* joyDownDTex;
    CImage* joyLeftTex;
    CImage* joyLeftDTex;
    CImage* joyRightTex;
    CImage* joyRightDTex;
	CImage* fire1UTex;
	CImage* fire2UTex;
	CImage* fire1DTex;
	CImage* fire2DTex;
}
-(id)initWithApp:(CRunApp*)a;
-(void)setJoystickStateBasedOnXY;
-(void)dealloc;
-(BOOL)touchBegan:(UITouch*)touch;
-(void)touchMoved:(UITouch*)touch;
-(void)touchEnded:(UITouch*)touch;
-(void)touchCancelled:(UITouch*)touch;
-(void)draw;
-(int)getKey:(int)x withY:(int)y;
-(void)setPositions;
-(unsigned char)getJoystick;
-(void)reset:(int)flag;
-(void)setXPosition:(int)flags withPos:(int)p;
-(void)setYPosition:(int)flags withPos:(int)p;

-(BOOL)InsideZone:(double)angle withAngleRef:(double) angle_ref andGap:(double) gap;

@end
