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
// CMOVEEXTENSIOn : Mouvement extension
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "CMove.h"

@class CRunMvtExtension;

@interface CMoveExtension : CMove 
{
@public
    CRunMvtExtension* movement;
    double callParam1;
    double callParam2;
}
-(id)initWithObject:(CRunMvtExtension*)m;
-(void)dealloc;
-(void)initMovement:(CObject*)ho withMoveDef:(CMoveDef*)mvPtr;
-(void)kill;
-(void)move;
-(void)stop;
-(void)start;
-(void)bounce;
-(void)setSpeed:(int)speed;
-(void)setMaxSpeed:(int)speed;
-(void)reverse;
-(void)setXPosition:(int)x;
-(void)setYPosition:(int)y;
-(void)setDir:(int)dir;
-(double)callMovement:(int)function param:(double)param;
-(double)callMovement2:(int)function param:(double)param param2:(double)param2;

@end
