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
// CIMAGE Une image
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>

#define GLES_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#define QC_SILENCE_GL_DEPRECATION

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import "IDrawable.h"
#import "CTexture.h"
#include <time.h>

@class CFile;
@class CMask;
@class CImage;
@class CBitmap;
@class CRunApp;
@class CArrayList;

#define MAX_ROTATEDMASKS 10

struct ImageInfo
{
	BOOL isFound;
	short width;
	short height;
	short xSpot;
	short ySpot;
	short xAP;
	short yAP;
};
typedef struct ImageInfo ImageInfo;

@interface CImage : CTexture
{
@public
	CRunApp* app;

    short xSpot;
    short ySpot;
    short xAP;
    short yAP;
    short useCount;
	int chunkSize;
	CArrayList* maskRotation;
	CArrayList* replacedColors;

    #define IMGFLAG_REPLACEDCOLORS  0x1000  // Volatil flag used by replaceColor action

	short format;
	short flags;
	short bytesPrPixel;
	int openGLmode;
	int openGLformat;

	unsigned int* data;
	NSUInteger dataLength;
    CMask* mask;
    CMask* maskPlatform;

	int lineWidth;
	int bLineWidth;
	BOOL bCanRelease;
	NSUInteger offset;
	CFile* file;
	BOOL isUploading;
	BOOL opaqueMask;
}
-(id)initWithApp:(CRunApp*)a;
-(id)initWithWidth:(int)sx andHeight:(int)sy;
-(void)dealloc;
-(void)loadHandle:(CFile*)file;
-(void)load:(CFile*)file;
-(void)preload:(CFile*)file;
-(CMask*)getMask:(int)nFlags withAngle:(float)angle andScaleX:(double)scaleX andScaleY:(double)scaleY;
-(CMask*)getMask:(int)nFlags;
-(void)setOpaqueMask:(BOOL)_opaqueMask;
-(void)copyImage:(CImage*)image;
-(CGImageRef)getCGImage;
-(UIImage*)getUIImage;
-(void)freeCGImage:(CGImageRef)cgImage;
-(void)loadBitmap:(CBitmap*)bitmap;
-(void)calculateTextureSize;

+(CImage*)createFullColorImage:(CImage*)image;
+(CImage*)loadUIImage:(UIImage*)image;
+(CImage*)loadBitmap:(CBitmap*)bitmap;
+(int)getFormatByteSize:(int)format;

-(CImage*)copy;

-(int)getPixel:(int)x withY:(int)y;
-(ColorRGBA)getPixelF:(float)x withY:(float)y;
-(void)setPixel:(int)pixel  atX:(int)x andY:(int)y;
-(void)setPixelF:(ColorRGBA)pixel  atX:(int)x andY:(int)y;
-(void)replaceColors;
-(bool)replaceColor:(ReplacedColor*)info;
-(bool)isReplacedColorFlag;
-(void)setReplacedColorFlag:(bool)flag;

-(void)unpremultiply;

//CImage extra to reupload it's image data attempting to reuse the current texture ID if possible
-(void)expectTilableImage;
-(NSUInteger)uploadTexture;
-(NSUInteger)reUploadTexture;
-(int)deleteTexture;
-(void)cleanMemory;
-(void)cleanPixelBuffer;
+(unsigned int)getReducedColorFromRed:(unsigned int)r andGreen:(unsigned int)g andBlue:(unsigned int)b fromFormat:(int)format;

//-(void)loadMask:(short*)mask withWidth:(int)maskWidth andHeight:(int)maskHeight;

@end

//Inlined function for determining if a given pixel is transparent
//Giving the format to the function allows the compiler to optimize the inlined function even more
static inline bool pixelIsSolid(CImage* img, int x, int y)
{
	int pixel4;
	short pixel2;
	static const int JPEG = 5;
	switch (img->format)
	{
		case RGBA8888:
        case JPEG:
			pixel4 = img->data[x+ y*img->width];
			return ((pixel4 & 0xFF000000) != 0);
		case RGBA4444:
			pixel2 = *(short*)((char*)img->data + img->bLineWidth*y + x*img->bytesPrPixel);
			return ((pixel2 & 0xF) != 0);
		case RGBA5551:
			pixel2 = *(short*)((char*)img->data + img->bLineWidth*y + x*img->bytesPrPixel);
			return ((pixel2 & 0x01) != 0);
		case RGB888:
		case RGB565:
		default:
			return true;
	}
}

