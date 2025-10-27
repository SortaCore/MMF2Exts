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
// CSOUNDPLAYER : synthetiseur MIDI
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>

@class CRunApp;
@class CSound;

void audioSessionListenerCallback(void* inClientData, UInt32 inInterruptionState);

#define NCHANNELS 48

@interface CSoundPlayer : NSObject
{
@public
	CRunApp* runApp;
	BOOL bOn;
    CSound** channels;
    BOOL bMultipleSounds;
	BOOL* bLocked;
	int* volumes;
	int* frequencies;
    float* panes;
	int mainVolume;
    int mainPan;
	CSoundPlayer* parentPlayer;
    int pausedBy;
}

-(id)initWithApp:(CRunApp*)app;
-(id)initWithApp:(CRunApp*)app andSoundPlayer:(CSoundPlayer*)player;
-(void)audioSessionInterrupted:(NSNotification*)notification;
-(void)dealloc;
-(void)reset;
-(void)play:(short)handle withNLoops:(int)nLoops andChannel:(int)channel andPrio:(BOOL)bPrio andVolume:(int)_volume andPan:(int)_pan andFreq:(int)_freq;
-(void)setMultipleSounds:(BOOL)bMultiple;
-(void)keepCurrentSounds;
-(void)setOnOff:(BOOL)bState;
-(BOOL)getOnOff;
-(void)stopAllSounds;
-(void)stopSample:(short)handle;
-(BOOL)isSoundPlaying;
-(BOOL)isSamplePlaying:(short)handle;
-(BOOL)isChannelPlaying:(int)channel;
-(void)setPositionSample:(short)handle withPosition:(int)pos;
-(int)getPositionSample:(short)handle;
-(void)pauseSample:(short)handle;
-(BOOL)isSamplePaused:(short)handle;
-(BOOL)isChannelPaused:(int)channel;
-(void)resumeSample:(short)handle;
-(void)pause:(int)pausemode;
-(void)resume:(int)pausemode;
-(void)pauseChannel:(int)channel;
-(void)stopChannel:(int)channel;
-(void)resumeChannel:(int)channel;
-(void)setPositionChannel:(int)channel withPosition:(int)pos;
-(int)getPositionChannel:(int)channel;
-(void)setVolumeSample:(short)handle withVolume:(int)pos;
-(void)setVolumeChannel:(int)channel withVolume:(int)v;
-(int)getVolumeChannel:(int)channel;
-(void)removeSound:(CSound*)s;
-(void)setMainVolume:(int)v;
-(void)lockChannel:(int)channel;
-(void)unLockChannel:(int)channel;
-(int)getMainVolume;
-(int)getDurationChannel:(int)channel;
-(int)getChannel:(NSString*)name;
-(int)getSamplePosition:(NSString*)name;
-(int)getSampleVolume:(NSString*)name;
-(int)getSampleDuration:(NSString*)name;
-(NSString*)getSoundNameChannel:(int)channel;
-(void)checkPlaying;
-(void)setFreqChannel:(int)channel withFreq:(int)v;
-(void)setFreqSample:(short)handle withFreq:(int)v;
-(int)getSampleFrequency:(NSString*)name;
-(int)getFrequencyChannel:(int)channel;
-(void)setAudioSessionMode:(BOOL)bflag andMode:(int)mode;
-(void)setPan:(float)pan;
-(float)getPan;
-(void)setPanSample:(short)handle withPan:(float)pan;
-(void)setPanChannel:(int)channel withPan:(float)p;
-(float)getPanChannel:(int)channel;
-(float)getSamplePan:(NSString*)name;

@end
