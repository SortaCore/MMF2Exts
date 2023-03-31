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
// CMOVEPATH : Mouvement enregistre
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "CMove.h"

@class CObject;
@class CMoveDef;
@class CMoveDefPath;

@interface CMovePath : CMove
{
@public
	int MT_Speed;
    int MT_Sinus;
    int MT_Cosinus;
    int MT_Longueur;
    int MT_XOrigin;
    int MT_YOrigin;
    int MT_XDest;
    int MT_YDest;
    int MT_MoveNumber;
    BOOL MT_Direction;
    CMoveDefPath* MT_Movement;
    int MT_Calculs;
    int MT_XStart;
    int MT_YStart;
    int MT_Pause;
    NSString* MT_GotoNode;
    BOOL MT_FlagBranch;	
}
-(void)initMovement:(CObject*)ho withMoveDef:(CMoveDef*)mvPtr;
-(void)kill;
-(void)move;
-(BOOL)mtMove:(int)step;
-(void)mtGoAvant:(int)number;
-(void)mtGoArriere:(int)number;
-(void)mtBranche;
-(void)mtMessages;
-(BOOL)mtTheEnd;
-(void)mtReposAtEnd;
-(void)mtBranchNode:(NSString*)pName;
-(void)freeMTNode;
-(void)mtGotoNode:(NSString*)pName;
-(void)stop;
-(void)start;
-(void)reverse;
-(void)setXPosition:(int)x;
-(void)setYPosition:(int)y;
-(void)setSpeed:(int)speed;
-(void)setMaxSpeed:(int)speed;

@end
