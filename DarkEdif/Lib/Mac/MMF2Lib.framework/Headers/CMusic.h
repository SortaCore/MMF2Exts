//
//  CMusic.h
//  MacRuntime
//
//  Created by Anders Riggelsen on 10/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
//#import <QTKit/QTKit.h>
//#import <AVFoundation/AVFoundation.h>

@class CMusicPlayer;
@class CFile;

@interface CMusic : NSObject
{
@public
	CFile* file;
	NSUInteger pointer;
	NSData* musicData;
	
	CMusicPlayer* musicPlayer;
	//QTMovie* player;
    //AVAudioPlayer* player;
	
    short handle;
    BOOL bPlaying;
	BOOL bPaused;
	NSString* name;
    int duration;
    
	NSTimeInterval pauseTime;
}
-(id)initWithMusicPlayer:(CMusicPlayer*)p;
-(void)dealloc;
-(void)load:(CFile*)f;
-(void)play:(int)nLoops;
-(void)pause;
-(void)resume;
-(void)stop;
-(void)cleanMemory;

@end
