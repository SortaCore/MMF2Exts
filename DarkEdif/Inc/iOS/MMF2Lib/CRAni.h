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
// CRANI gestion des animations
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>

@class CObject;
@class CAnim;
@class CAnimDir;

@interface CRAni : NSObject
{
@public
	CObject* hoPtr;
    int raAnimForced;				/// Flags if forced
    int raAnimDirForced;
    int raAnimSpeedForced;
    BOOL raAnimStopped;
    int raAnimOn;				/// Current animation
    CAnim* raAnimOffset;
    int raAnimDir;				/// Direction of current animation
    int raAnimPreviousDir;                       /// Previous OK direction
    CAnimDir* raAnimDirOffset;
    int raAnimSpeed;
    int raAnimMinSpeed;                          /// Minimum speed of movement
    int raAnimMaxSpeed;                          /// Maximum speed of movement
    int raAnimDeltaSpeed;
    int raAnimCounter;                           /// Animation speed counter
    int raAnimDelta;				/// Speed counter
    int raAnimRepeat;				/// Number of repeats
    int raAnimRepeatLoop;			/// Looping picture
    int raAnimFrame;				/// Current frame
    int raAnimNumberOfFrame;                     /// Number of frames
    int raAnimFrameForced;
    int raRoutineAnimation;
    float raOldAngle;
}
-(id)initWithHO:(CObject*)ho;
-(void)kill:(BOOL)bFast;
-(void)initRAni;
-(void)init_Animation:(int)anim;
-(void)check_Animate;
-(void)extAnimations:(int)anim;
-(BOOL)animate;
-(BOOL)animations;
-(BOOL)animIn:(int)vbl;
-(BOOL)anim_Exist:(int)animId;
-(void)animation_OneLoop;
-(void)animation_Force:(int)anim;
-(void)animation_Restore;
-(void)animDir_Force:(int)dir;
-(void)animDir_Restore;
-(void)animSpeed_Force:(int)speed;
-(void)animSpeed_Restore;
-(void)anim_Restart;
-(void)animFrame_Force:(int)frame;
-(void)animFrame_Restore;
-(void)anim_Appear;
-(void)anim_Disappear;

@end
