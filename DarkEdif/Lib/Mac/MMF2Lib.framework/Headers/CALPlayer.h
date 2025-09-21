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
//  CALPlayer.h
//  RuntimeIPhone
//
//  Created by Francois Lionet on 24/03/11.
//  Copyright 2011 Clickteam. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <OpenAL/al.h>
#import <OpenAL/alc.h>



#define NALCHANNELS 32

@class CSound;

@interface CALPlayer : NSObject 
{
 	ALCcontext* mContext; // stores the context (the 'air')
	ALCdevice* mDevice; // stores the device
   
    ALuint pSources[NALCHANNELS];
    CSound* pSounds[NALCHANNELS];
    int nLoops[NALCHANNELS];
    CALPlayer* parentPlayer;
    BOOL bPaused;
}

-(id)init;
-(id)initWithPlayer:(CALPlayer*)parent;
-(void)dealloc;
-(int)play:(CSound*)pSound loops:(int)nl channel:(int)channel volume:(float)_volume pitch:(float)_pitch;
-(void)stop:(int)nSound;
-(void)pause:(int)nSound;
-(void)resume:(int)nSound;
-(void)rewind:(int)nSound;
-(void)setVolume:(int)nSound volume:(float)v;
-(void)setPitch:(int)nSound pitch:(float)v;
-(void)setPan:(int)nSound pan:(float)v;
-(BOOL)checkPlaying:(int)nSound;
-(void)resetSources;
-(void)beginInterruption;
-(void)endInterruption;
-(int)getPosition:(int)nSound;
@end

OSStatus read_Proc (void *inClientData, SInt64 inPosition, UInt32 requestCount, void *buffer, UInt32 *actualCount);
OSStatus write_Proc (void *inClientData, SInt64 inPosition, UInt32 requestCount, const void *buffer, UInt32  *actualCount);
SInt64 getSize_Proc (void *inClientData);
OSStatus setSize_Proc (void *inClientData, SInt64 size);
