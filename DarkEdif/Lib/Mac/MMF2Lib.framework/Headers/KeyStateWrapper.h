//
//  KeyStateWrapper.h
//  MacRuntime
//
//  Created by Anders Riggelsen on 3/8/11.
//  Copyright 2011 Clickteam. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class CRunApp;

@interface KeyStateWrapper : NSObject
{
	NSMutableSet* keyStates;
	BOOL shift;
	BOOL command;
	BOOL control;
	BOOL alternate;
	BOOL function;
	CRunApp* app;

}
-(id)initWithApp:(CRunApp*)app;
-(void)dealloc;

-(void)keyDown:(NSEvent*)event;
-(void)keyUp:(NSEvent*)event;
-(void)flagsChanged:(NSEvent*)event;
-(BOOL)isPressed:(short)windowsKey;
-(BOOL)hasChar:(unichar)key;
-(unichar)getCharFromWindowsKey:(short)key;
-(unsigned char)getWindowsKeyFromChar:(unichar)ch;

-(NSString*)nameForWindowsKey:(short)key;

@end
