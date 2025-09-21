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
//
//  CServices.h
//  RuntimeIPhone
//
//  Created by Francois Lionet on 19/10/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CRect.h"
#import "CoreMath.h"

#define DT_LEFT 0x0000
#define DT_TOP 0x0000
#define DT_CENTER 0x0001
#define DT_RIGHT 0x0002
#define DT_BOTTOM 0x0008
#define DT_VCENTER 0x0004
#define DT_SINGLELINE 0x0020
#define DT_CALCRECT 0x0400
#define DT_VALIGN 0x0800
#define DT_NOINTERLINESUB 0x1000
#define CPTDISPFLAG_INTNDIGITS 0x000F
#define CPTDISPFLAG_FLOATNDIGITS 0x00F0
#define CPTDISPFLAG_FLOATNDIGITS_SHIFT 4
#define CPTDISPFLAG_FLOATNDECIMALS 0xF000
#define CPTDISPFLAG_FLOATNDECIMALS_SHIFT 12
#define CPTDISPFLAG_FLOAT_FORMAT 0x0200
#define CPTDISPFLAG_FLOAT_USEDECIMALS 0x0400
#define CPTDISPFLAG_FLOAT_PADD 0x0800

@class CFont;
@class CBitmap;
@class CRenderToTexture;
class CRenderer;
@class CSprite;

extern int getR(int rgb);
extern int getG(int rgb);
extern int getB(int rgb);
extern int getRGB(int r, int g, int b);
extern int getRGBA(int r, int g, int b, int a);
extern int getABGR(int a, int b, int g, int r);
extern int getABGR(ColorRGBA color);
extern int getARGB(int a, int r, int g, int b);
extern int getARGB(ColorRGBA color);
extern int getABGRPremultiply(int a, int b, int g, int r);
extern int ABGRtoRGB(int abgr);
extern int ARGBtoBGR(int argb);
extern unsigned int BGRtoARGB(int color);

extern NSRect MakeNSRectFromCRect(CRect rc);

extern int clamp(int val, int a, int b);
extern double clampd(double val, double a, double b);
extern int nextPOT(int val, int max);
extern double maxd(double a, double b);
extern double mind(double a, double b);

double absDouble(double v);
extern void setRGBFillColor(CGContextRef ctx, int rgb);
extern void setRGBStrokeColor(CGContextRef ctx, int rgb);
extern void drawLine(CGContextRef context, int x1, int y1, int x2, int y2);
extern int HIWORD(int ul);
extern int LOWORD(int ul);
extern int POSX(int ul);
extern int POSY(int ul);
extern int MAKELONG(int lo, int hi);
extern int swapRGB(int rgb);
extern int SemiTranspToAlpha(int semi);
extern int AlphaToSemiTransp(int alpha);
extern int strUnicharLen(unichar* str);
extern NSColor* getNSColor(int rgb);

@interface CServices : NSObject 
{
}
+(NSColor*)getNSColor:(int)color;
+(int)indexOf:(NSString*)s withChar:(unichar)c startingAt:(int)start;
+(int)drawText:(CBitmap*)bitmap withString:(NSString*)s andFlags:(short)flags andRect:(CRect)rc andColor:(int)rgb andFont:(CFont*)font;
+(NSString*)intToString:(int)v withFlags:(int)flags;
+(NSString*)doubleToString:(double)v withFlags:(int)flags;
+(NSSize)textSizeForText:(NSString*)string andFont:(NSFont*)font andMaxWidth:(int)width;
+(CGRect)CGRectFromSprite:(CSprite*)sprite;

@end

