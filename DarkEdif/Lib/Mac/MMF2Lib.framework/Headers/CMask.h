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
// CMASK : un masque
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

#define SCMF_FULL 0x0000
#define SCMF_PLATFORM 0x0001
#define GCMF_OBSTACLE 0x0000
#define GCMF_PLATFORM 0x0001

@class CImage;

@interface CMask : NSObject 
{
@public
	short* mask;
    int lineWidth;
    int height;
    int width;
    int xSpot;
    int ySpot;
	
}
-(void)dealloc;
-(void)createMask:(CImage*)img withFlags:(int)nFlags;
-(void)createOpaqueMask:(CImage*)img withFlags:(int)nFlags;
-(BOOL)testMask:(int)yBase1 withX1:(int)x1 andY1:(int)y1 andMask:(CMask*)pMask2 andYBase:(int)yBase2 andX2:(int)x2 andY2:(int)y2;
-(BOOL)testRect:(int)yBase1 withX:(int)xx andY:(int)yy andWidth:(int)w andHeight:(int)h;
-(BOOL)testPoint:(int)x1 withY:(int)y1;
-(void)rotateRect:(int*)pWidth withPHeight:(int*)pHeight andPHX:(int*)pHX andPHY:(int*)pHY andAngle:(float)fAngle;
-(BOOL)createRotatedMask:(CMask*)pMask withAngle:(float)fAngle andScaleX:(float)fScaleX andScaleY:(float)fScaleY;

@end

typedef struct tagRM
{
	CMask* mask;
	int angle;
	double scaleX;
	double scaleY;
	int tick;
} RotatedMask;

