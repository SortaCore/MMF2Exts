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
// CMOVEPLATFORM : Mouvement plateforme
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "CMove.h"

#define MPJC_NOJUMP 0
#define MPJC_DIAGO 1
#define MPJC_BUTTON1 2
#define MPJC_BUTTON2 3
#define MPTYPE_WALK 0
#define MPTYPE_CLIMB 1
#define MPTYPE_JUMP 2
#define MPTYPE_FALL 3
#define MPTYPE_CROUCH 4
#define MPTYPE_UNCROUCH 5

@class CObject;
@class CMoveDef;

@interface CMovePlatform : CMove
{
@public
	int MP_Type;
    int MP_Bounce;
    int MP_BounceMu;
    int MP_XSpeed;
    int MP_Gravity;
    int MP_Jump;
    int MP_YSpeed;
    int MP_XMB;
    int MP_YMB;
    int MP_HTFOOT;
    int MP_JumpControl;
    int MP_JumpStopped;
    int MP_PreviousDir;
    CObject* MP_ObjectUnder;
    int MP_XObjectUnder;
    int MP_YObjectUnder;
    BOOL MP_NoJump;	
}
-(void)initMovement:(CObject*)ho withMoveDef:(CMoveDef*)mvPtr;
-(void)move;
-(void)mpStopIt;
-(void)stop;
-(void)bounce;
-(void)setXPosition:(int)x;
-(void)setYPosition:(int)y;
-(void)setSpeed:(int)speed;
-(void)setMaxSpeed:(int)speed;
-(void)setGravity:(int)gravity;
-(void)setDir:(int)dir;
-(void)calcMBFoot;
-(int)check_Ladder:(int)nLayer withX:(int)x andY:(int)y;
-(void)mpHandle_Background;


@end
