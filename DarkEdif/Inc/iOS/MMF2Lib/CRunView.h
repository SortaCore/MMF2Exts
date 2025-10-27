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
//  CRunView.h
//  RuntimeIPhone
//
//  Created by Francois Lionet on 08/10/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#pragma once
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "CRenderer.h"

@class CRunApp;

@interface CRunView : UIView
{
@public
	CRunApp* pRunApp;

	NSTimer* timer;
	NSTimer* cleanTimer;
	NSTimer* pruneTimer;
	CADisplayLink* displayLink;
	BOOL usesDisplayLink;
	int frameInterval;

	CRenderer* renderer;
	CGRect appRect;
	CGRect screenRect;

	NSMutableArray* touchesBegan;
	NSMutableArray* touchesMoved;
	NSMutableArray* touchesEnded;
	NSMutableArray* touchesCanceled;

	CFAbsoluteTime gamepadConnectionTime;

	BOOL bTimer;
	float viewScale;
}
-(id)initWithFrame:(CGRect)rect;
-(void)dealloc;
-(void)initApplication:(CRunApp*)pApp;
-(void)endApplication;
-(void)timerEntry;
-(void)cleanEntry;
-(void)drawNoUpdate;
-(void)clearPostponedInput;
-(void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event;
-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
-(void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event;
-(void)setMultiTouch:(BOOL)bMulti;
-(CRenderer*)getRenderer;
-(void)resetFrameRate;
-(void)pauseTimer;
-(void)resumeTimer;
-(BOOL)hasActiveGameControllerConnected;
-(BOOL)hasAnyGameControllerConnected;
@end
