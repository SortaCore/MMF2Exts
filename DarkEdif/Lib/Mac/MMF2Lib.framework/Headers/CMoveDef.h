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
// CMOVEDEF classe abstraite de definition d'un mouvement
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

// Definition of movement types
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define MVTYPE_STATIC 0
#define MVTYPE_MOUSE 1
#define MVTYPE_RACE 2
#define MVTYPE_GENERIC 3
#define MVTYPE_BALL 4
#define MVTYPE_TAPED 5
#define MVTYPE_PLATFORM 9
#define MVTYPE_DISAPPEAR 11
#define MVTYPE_APPEAR 12
#define MVTYPE_BULLET 13
#define MVTYPE_EXT 14

#define MVRTFLAG_PHYSICS 1

@class CFile;

@interface CMoveDef : NSObject
{
@public
	short mvType;
    short mvControl;
    unsigned char mvMoveAtStart;
    int mvDirAtStart;
	unsigned char mvOpt;
	unsigned char mvRuntimeFlags;
}
-(void)load:(CFile*)file withLength:(int)length;
-(void)setData:(short)t withControl:(short)c andMoveAtStart:(unsigned char)m andDirAtStart:(int)d andOptions:(unsigned char)mo;

@end
