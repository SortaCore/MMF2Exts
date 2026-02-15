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
// CSOUNDBANK: stockage des sons
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>
#import "IEnum.h"

@class CRunApp;
@class CSound;

@interface CSoundBank : NSObject
{
@public
	CRunApp* runApp;
    CSound** sounds;
    int nHandlesReel;
    int nHandlesTotal;
    int nSounds;
    NSUInteger* offsetsToSounds;
    short* handleToIndex;
    short* useCount;
    short* audioFlags;
}

-(id)initWithApp:(CRunApp*)app;
-(void)dealloc;
-(void)preLoad;
-(CSound*)getSoundFromHandle:(short)handle;
-(short)getSoundHandleFromName:(NSString*)soundName;
-(CSound*)getSoundFromIndex:(short)index;
-(void)resetToLoad;
-(void)setToLoad:(short)handle;
-(void)setFlags:(short)handle flags:(short)f;
-(void)load;
-(void)cleanMemory;

@end
