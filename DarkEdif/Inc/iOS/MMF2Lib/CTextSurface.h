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
//  CTextSurface.h
//  RuntimeIPhone
//
//  Created by Anders Riggelsen on 6/18/10.
//  Copyright 2010 Clickteam. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CRect.h"

@class CBitmap;
@class CFont;
@class CImage;
class CRenderer;


@interface CTextSurface : NSObject
{
@public
	NSString* prevText;
	short prevFlags;
	int prevColor;
	CFont* prevFont;
	CImage* textTexture;
	
	CRect rect;
	int width;
	int height;
	int effect;
	int effectParam;
	CBitmap* textBitmap;
    
    // Let's keep this heavy burden here
    NSMutableParagraphStyle *cachedParagraphStyle;
    short cachedFlags;
    BOOL smoothfonts;
}

-(id)initWidthWidth:(int)w andHeight:(int)h;
-(void)setAntialiased:(BOOL)antialias;
-(void)setText:(NSString*)s withFlags:(short)flags andColor:(int)color andFont:(CFont*)font;
-(void)draw:(CRenderer*)renderer withX:(int)x andY:(int)y andShader:(int)effectShader andEffect:(int)inkEffect andEffectParam:(int)inkEffectParam;
-(void)draw:(CRenderer*)renderer withX:(int)x andY:(int)y andEffect:(int)inkEffect andEffectParam:(int)inkEffectParam;

-(BOOL)setSizeWithWidth:(int)w andHeight:(int)h;
-(void)manualDrawText:(NSString*)s withFlags:(short)flags andRect:(CRect)rectangle andColor:(int)color andFont:(CFont*)font;
-(void)manualUploadTexture;
-(void)manualClear:(int)color;

-(CGSize)calculateTextSizeWithString:(NSString*)s andFlags:(short)flags andRect:(CRect)rc andFont:(CFont*)font;
@end
