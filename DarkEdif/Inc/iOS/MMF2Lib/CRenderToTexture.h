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
//  CRenderToTexture.h
//  RuntimeIPhone
//
//  Created by Anders Riggelsen on 8/10/10.
//  Copyright 2010 Clickteam. All rights reserved.
//

#pragma once
#import <Foundation/Foundation.h>

#define GLES_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#define QC_SILENCE_GL_DEPRECATION

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import "Classes/CRect.h"
#import "Classes/CTexture.h"

@class CBitmap;
@class CFont;
@class CImage;
class CRenderer;
@class CRunApp;

@interface CRenderToTexture : CTexture
{
@public
	GLuint framebuffer;
	CRunApp* app;
	CRenderer* renderer;
	GLint previousBuffer;
	GLint prevWidth;
	GLint prevHeight;
	BOOL isUploaded;
}

- (id)initWithWidth:(int)w andHeight:(int)h andRunApp:(CRunApp *)runApp andSwapMode:(bool)bSwap andPOTMode:(bool)potMode;
- (id)initWithWidth:(int)w andHeight:(int)h andRunApp:(CRunApp *)runApp andSwapMode:(bool)bSwap;
- (id)initWithWidth:(int)w andHeight:(int)h andRunApp:(CRunApp *)runApp andPOTMode:(bool)potMode;
- (id)initWithWidth:(int)w andHeight:(int)h andRunApp:(CRunApp *)runApp;

- (id)initWithWidth:(int)w andHeight:(int)h andRenderer:(CRenderer *)renderer andSwapMode:(bool)bSwap andPOTMode:(bool)potMode;
- (id)initWithWidth:(int)w andHeight:(int)h andRenderer:(CRenderer *)renderer andSwapMode:(bool)bSwap;
- (id)initWithWidth:(int)w andHeight:(int)h andRenderer:(CRenderer *)renderer andPOTMode:(bool)potMode;
- (id)initWithWidth:(int)w andHeight:(int)h andRenderer:(CRenderer *)renderer;

- (void)dealloc;

- (GLuint)newEmptyTextureWithWidth:(int)w andHeight:(int)h;
- (void)bindFrameBuffer;
- (void)bindFrameBufferWithOffset:(Vec2f)offset;
- (void)unbindFrameBuffer;

- (void)fillWithColor:(int)color;
- (void)fillWithColor:(int)color andAlpha:(unsigned char)alpha;

- (void)fillWithPixels:(unsigned int*)buffer withWidth:(int)w andHeight:(int)h;

- (void)clearColorChannelWithColor:(int)color;
- (void)clearWithAlpha:(float)alpha;
- (void)clearWithAlphaDontBind:(float)alpha;
- (void)clearAlphaChannel:(float)alpha;
- (void)copyAlphaFrom:(CRenderToTexture*)rtt;

-(int)uploadTexture;
-(int)deleteTexture;
-(void)cleanMemory;
-(void)setResampling:(BOOL)resample;

@end
