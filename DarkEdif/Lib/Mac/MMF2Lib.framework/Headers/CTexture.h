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
//  CTexture.h
//  RuntimeIPhone
//
//  Created by Anders Riggelsen on 9/28/13.
//  Copyright (c) 2013 Clickteam. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CoreMath.h"

// Uncomment this if you want to use POT power of two images
// but shader must be recalculated by resolution
//
// #define POT

@interface CTexture : NSObject
{
@public
    unsigned short handle;
	GLuint textureId;
	int width;
	int height;
	int originalWidth;
	int originalHeight;
	int textureWidth;
	int textureHeight;
	int usageCount;
    short format;
	BOOL resample;		//MUST be set with 'setResampling' so it can update the filters
	BOOL hasMipMaps;
    BOOL isPrunable;

	GLuint wrapS;
	GLuint wrapT;

	Mat3f textureMatrix;
	BOOL coordsAreSwapped;
}

-(size_t)uploadTexture;
-(void)createGLTextureWidth:(int)width withHeight:(int)height andResampling:(int)resampling;
-(int)deleteTexture;
-(void)generateMipMaps;
-(void)cleanMemory;
-(void)expectTilableImage;
-(BOOL)imageIsPOT;

-(void)setResampling:(BOOL)resample;
-(void)updateFilter;
-(void)updateFilterNoBind;
-(void)updateTextureMode:(int)wrapMode;

-(void)updateTextureMatrix;

-(NSImage*)getTextureImage;
@end
