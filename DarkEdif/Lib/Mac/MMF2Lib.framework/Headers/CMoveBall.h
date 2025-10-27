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
// CMOVEBALL : Mouvement balle
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "CMove.h"

@class CObject;
@class CMoveDef;

@interface CMoveBall : CMove
{
@public
	int MB_StartDir;
    int MB_Angles;
    int MB_Securite;
    int MB_SecuCpt;
    int MB_Bounce;
    int MB_Speed;
    int MB_MaskBounce;
    int MB_LastBounce;
    BOOL MB_Blocked;
}
-(void)initMovement:(CObject*)ho withMoveDef:(CMoveDef*)mvPtr;
-(void)move;
-(void)stop;
-(void)start;
-(void)bounce;
-(BOOL)mvb_Test:(int)dir;
-(void)setSpeed:(int)speed;
-(void)setMaxSpeed:(int)speed;
-(void)reverse;
-(void)setXPosition:(int)x;
-(void)setYPosition:(int)y;

@end
