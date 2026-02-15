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
// CRMVT : Donnees de base d'un mouvement
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

#define EF_GOESINPLAYFIELD 0x0001
#define EF_GOESOUTPLAYFIELD 0x0002
#define EF_WRAP 0x0004

@class CMove;
@class CObject;
@class CObjectCommon;
@class CCreateObjectInfo;
@class CMoveDefExtension;

@interface CRMvt : NSObject
{
@public
	int rmMvtNum;					// Number of the current movement
    CMove* rmMovement;
    CMove* rmMovementBackup;
    unsigned char rmWrapping;					// For CHECK POSITION
    BOOL rmMoveFlag;					// Messages/movements
    int rmReverse;					// Ahaid or reverse?
    BOOL rmBouncing;					// Bouncing?
    short rmEventFlags;				// To accelerate events
}
-(void)dealloc;
-(void)initMovement:(int)nMove withObject:(CObject*)hoPtr andOC:(CObjectCommon*)ocPtr andCOB:(CCreateObjectInfo*)cob andNum:(int)forcedType;
-(void)initSimple:(CObject*)hoPtr withType:(int)forcedType andFlag:(BOOL)bRestore;
-(void)kill:(BOOL)bFast;
-(void)move;
-(void)nextMovement:(CObject*)hoPtr;
-(void)previousMovement:(CObject*)hoPtr;
-(void)selectMovement:(CObject*)hoPtr withNumber:(int)mvt;
-(int)dirAtStart:(CObject*)hoPtr withDirAtStart:(int)dirAtStart andDir:(int)dir;
-(CMove*)loadMvtExtension:(CObject*)hoPtr withDef:(CMoveDefExtension*)mvDef;

@end
