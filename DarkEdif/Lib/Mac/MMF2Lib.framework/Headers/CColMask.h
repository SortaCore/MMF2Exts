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
// CCOLMASK : masque de collision
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

#define CM_TEST_OBSTACLE 0
#define CM_TEST_PLATFORM 1
#define CM_OBSTACLE 0x0001
#define CM_PLATFORM 0x0002
#define COLMASK_XMARGIN 64
#define COLMASK_YMARGIN 16
#define HEIGHT_PLATFORM 6

@class CMask;

@interface CColMask : NSObject 
{
@public
	short* obstacle;
    short* platform;
    int lineWidth;
    int width;
    int height;
    int mX1;
    int mX2;
    int mY1;
    int mY2;
    int mX1Clip;
    int mX2Clip;
    int mY1Clip;
    int mY2Clip;
    int mDxScroll;
    int mDyScroll;	
}
-(void)dealloc;
+(CColMask*)create:(int)xx1 withY1:(int)yy1 andX2:(int)xx2 andY2:(int)yy2 andFlags:(int)flags;
-(void)setOrigin:(int)dx withDY:(int)dy;
-(void)fill:(short)value;
-(void)fillRectangle:(int)x1 withY1:(int)y1 andX2:(int)x2 andY2:(int)y2 andValue:(int)val;
-(void)fillRect:(short*)mask withX1:(int)x1 andY1:(int)y1 andX2:(int)x2 andY2:(int)y2 andValue:(int)val;
-(void)orMask:(CMask*)mask withX:(int)xx andY:(int)yy andPlane:(int)plans andValue:(int)val;
-(void)orIt:(short*)dMask withMask:(CMask*)sMask andX:(int)xx andY:(int)yy andFlag:(BOOL)bOr;
-(void)orPlatformMask:(CMask*)sMask withX:(int)xx andY:(int)yy;
-(BOOL)testPoint:(int)x withY:(int)y andPlane:(int)plans;
-(BOOL)testPt:(short*)mask withX:(int)x andY:(int)y;
-(BOOL)testRect:(int)x withY:(int)y andWidth:(int)w andHeight:(int)h andPlane:(int)plans;
-(BOOL)testRc:(short*)mask withX:(int)xx andY:(int)yy andWidth:(int)sx andHeight:(int)sy;
-(BOOL)testMask:(CMask*)mask withYBase:(int)yBase andX:(int)xx andY:(int)yy andPlane:(int)plans;
-(BOOL)testIt:(short*)dMask withMask:(CMask*)sMask andYBase:(int)yBase andX:(int)xx andY:(int)yy;

@end
