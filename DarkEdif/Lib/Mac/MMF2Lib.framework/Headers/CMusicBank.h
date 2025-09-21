//
//  CMusicBank.h
//  MacRuntime
//
//  Created by Anders Riggelsen on 10/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@class CRunApp;
@class CMusic;

@interface CMusicBank : NSObject
{
@public
	CRunApp* runApp;
    CMusic** musics;
    int nHandlesReel;
    int nHandlesTotal;
    int nMusics;
    NSUInteger* offsetsToMusics;
    short* handleToIndex;
    short* useCount;
}

-(id)initWithApp:(CRunApp*)app;
-(void)dealloc;
-(void)preLoad;
-(CMusic*)getMusicFromHandle:(short)handle;
-(CMusic*)getMusicFromIndex:(short)index;
-(void)cleanMemory;
@end
