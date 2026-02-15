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
// CBITMAP : ecran bitmap
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>

@class CImage;

enum
{
	BOP_COPY,
	BOP_BLEND,
	BOP_INVERT,
	BOP_XOR,
	BOP_AND,
	BOP_OR,
	BOP_BLEND_REPLEACETRANSP,
	BOP_DWROP,
	BOP_ANDNOT,
	BOP_ADD,
	BOP_MONO,
	BOP_SUB,
	BOP_BLEND_DONTREPLACECOLOR,
	BOP_EFFECTEX,
	BOP_MAX
};
#define BOP_MASK 0xFFF
#define BOP_RGBAFILTER 0x1000
#define EFFECTFLAG_TRANSPARENT 0x10000000L
#define EFFECTFLAG_ANTIALIAS   0x20000000L
#define EFFECT_MASK 0xFFFF

@interface CBitmap : NSObject
{
@public
	int width;
	int height;
	int clipX1;
	int clipY1;
	int clipX2;
	int clipY2;
	CGContextRef context;
	unsigned int* data;
}
-(id)initWithWidth:(int)sx andHeight:(int)sy;
-(id)initWithoutContext:(int)w withHeight:(int)h;
-(id)initWithImage:(CBitmap*)source;
-(id)initWithBitmap:(CBitmap*)source;
-(void)dealloc;
-(void)resizeWithWidth:(int)w andHeight:(int)h;
-(void)fillWithColor:(int)c;
-(void)fillRect:(int)x withY:(int)y andWidth:(int)w andHeight:(int)h andColor:(int)c;
-(void)drawImage:(int)x withY:(int)y andImage:(CImage*)image andInkEffect:(int)inkEffect andInkEffectParam:(int)inkEffectParam;
-(void)copyColorMask:(CImage*)img withColor:(int)couleur;
-(void)setClipWithX1:(int)x1 andY1:(int)y1 andX2:(int)x2 andY2:(int)y2;
-(void)resetClip;
-(void)copyImage:(CImage*)source;
-(void)screenCopy:(CBitmap*)source;
-(void)screenCopy:(CBitmap*)source withX:(int)xSource andY:(int)ySource andWidth:(int)wSource andHeight:(int)hSource;
-(void)drawPatternRect:(CImage*)image withX:(int)x andY:(int)y andWidth:(int)w andHeight:(int)h andInkEffect:(int)inkEffect andInkEffectParam:(int)inkEffectParam;
-(void)drawPatternEllipse:(CImage*)image withX1:(int)x1 andY1:(int)y1 andX2:(int)x2 andY2:(int)y2 andInkEffect:(int)inkEffect andInkEffectParam:(int)inkEffectParam;
+(void)stretchLineWithDest:(unsigned int*)dest andSource:(unsigned int*)src andNewWidth:(int)newWidth andSrcWidth:(int)srcWidth andCoefX:(int)CoefX;
-(void)stretchWithSource:(CBitmap*)pSource andDestX:(int)tX andDestY:(int)tY andSourceX:(int)sX andSourceY:(int)sY andNewWidth:(int)newWidth andNewHeight:(int)newHeight andSrcWidth:(int)srcWidth andSrcHeight:(int)srcHeight;
-(void)screenCopy:(CBitmap*)source withDestX:(int)xDest andDestY:(int)yDest andSourceX:(int)xSource andSourceY:(int)ySource andWidth:(int)wSource andHeight:(int)hSource;
-(void)rotateWithSource:(CBitmap*)source andAngle:(float)angle andAA:(BOOL)bAA andClrFill:(int)clrFill;
-(void)reverseXWithX:(int)xx andY:(int)yy andWidth:(int)ww andHeight:(int)hh;
-(void)reverseYWithX:(int)xx andY:(int)yy andWidth:(int)ww andHeight:(int)hh;
-(void)fade:(CBitmap*)source withCoef:(int)coef;

@end
