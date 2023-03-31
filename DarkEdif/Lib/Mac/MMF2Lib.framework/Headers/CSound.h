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
// CSOUND : un echantillon
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <OpenAL/al.h>
#import <OpenAL/alc.h>

#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/ExtendedAudioFile.h>


@class CSoundPlayer;
@class CFile;
@class CALPlayer;

class OggVorbis_File;


#define PSOUNDFLAG_IPHONE_AUDIOPLAYER 0x0004
#define PSOUNDFLAG_IPHONE_OPENAL 0x0008
#define PSOUNDFLAG_OGG 0x0008

typedef ALvoid    AL_APIENTRY    (*alBufferDataStaticProcPtr) (const ALint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq);
ALvoid  alBufferDataStaticProc(const ALint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq);
void* GetOpenALAudioData(NSData* data, ALsizei *outDataSize, ALenum *outDataFormat, ALsizei*    outSampleRate);
void* ExitFunction(ExtAudioFileRef extRef, AudioFileID fid, void* theData);


size_t vorbisRead(void *ptr, size_t size, size_t nmemb, void *datasource);
int    vorbisSeek(void *datasource, int64_t offset, int whence);
int    vorbisClose(void *datasource);
long   vorbisTell(void *datasource);

#ifdef __MAC_10_7
@interface CSound : NSObject <AVAudioPlayerDelegate>
#else
@interface CSound : NSObject <NSSoundDelegate>
#endif
{
@public
    CFile* file;
    NSUInteger pointer;
    NSUInteger fileDataSize;
    NSUInteger soundDataPointer;
    CSoundPlayer* soundPlayer;
    CALPlayer* ALPlayer;
#ifdef __MAC_10_7
    AVAudioPlayer* AVPlayer;
#else
    NSSound* sound;
#endif
    OggVorbis_File* vorbisFile;
    AudioStreamBasicDescription soundDescription;

    short handle;
    BOOL bUninterruptible;
    BOOL bPlaying;
    BOOL bPaused;
    BOOL gamePaused;
    BOOL externalSound;
    NSString* name;
    double volume;
    float pan;
    int duration;
    int frequency;

    BOOL bAudioPlayer;
    ALuint bufferID;
    int nSound;
    NSTimeInterval pauseTime;
}
-(id)initWithSoundPlayer:(CSoundPlayer*)p andALPlayer:(CALPlayer*)alp;
-(void)dealloc;
-(void)load:(CFile*)file flags:(short)flags;
-(void)loadExternal:(NSData*)data;
-(void)play:(int)nLoops channel:(int)channel volume:(float)_volume pitch:(float)_pitch;
-(void)pause:(BOOL)gamePause;
-(BOOL)isPaused;
-(void)resume:(BOOL)gameResume;
-(void)stop;
#ifdef __MAC_10_7
-(void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag;
-(void)audioPlayerDecodeErrorDidOccur:(AVAudioPlayer*)player error:(NSError*)error;
#else
-(void)sound:(NSSound *)sound didFinishPlaying:(BOOL)aBool;
#endif
-(void)setVolume:(int)v;
-(int)getVolume;
-(int)getPitch;
-(void)setPosition:(int)p;
-(int)getPosition;
-(int)getDuration;
-(void)cleanMemory;
-(void)checkPlaying;
-(void)setPitch:(float)p;
-(void)setPan:(float)p;
-(float)getPan;
-(float)fillPan:(float)p;

@end
