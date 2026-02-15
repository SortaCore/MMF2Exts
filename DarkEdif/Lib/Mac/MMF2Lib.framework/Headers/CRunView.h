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
#import <Foundation/Foundation.h>
#define GL_SILENCE_DEPRECATION

@class CRunApp;
class CRenderer;
@class KeyStateWrapper;
@class PlayerControlController;
@class CRenderToTexture;

@interface CRunView : NSOpenGLView
{
@public
	CRunApp* pRunApp;

	NSTimer* timer;
	NSTimer* cleanTimer;
	NSTimer* pruneTimer;

	CRenderer* renderer;
	NSRect appRect;
	int width;
	int height;
	BOOL bTimer;
	NSOpenGLPixelFormat* pixelFormat;
	GLint rendererID;
	BOOL refreshRenderer;

	BOOL duringModalView;

	NSRect windowRect;

	NSTabView* tabView;

	CRenderToTexture* rtt;
	int rttX;
	int rttY;
	int rttWidth;
	int rttHeight;

	BOOL mouseVisible;
	NSTrackingArea* trackingArea;
}
-(id)initWithFrame:(NSRect)rect;
-(void)dealloc;
-(void)initApplication:(CRunApp*)pApp;
-(void)endApplication;
-(void)setTimer;
-(void)timerEntry;
-(void)cleanEntry;
-(void)resetFrameRate;
-(void)pauseTimer;
-(void)resumeTimer;

-(IBAction)closePlayerControlsDialog:(id)sender;

@property(nonatomic, retain) IBOutlet NSTabView* tabView;

-(void)prepareOpenGL;
-(void)drawRect:(NSRect)aRect;

-(BOOL)acceptsFirstMouse:(NSEvent *)theEvent;
-(BOOL)acceptsFirstResponder;
-(BOOL)mouseDownCanMoveWindow;
-(void)mouseDown:(NSEvent *)theEvent;
-(void)mouseUp:(NSEvent *)theEvent;
-(void)rightMouseDown:(NSEvent *)theEvent;
-(void)rightMouseUp:(NSEvent *)theEvent;
-(void)otherMouseDown:(NSEvent *)theEvent;
-(void)otherMouseUp:(NSEvent *)theEvent;
-(void)mouseMoved:(NSEvent *)theEvent;
-(void)mouseDragged:(NSEvent *)theEvent;
-(void)keyDown:(NSEvent *)theEvent;
-(void)keyUp:(NSEvent *)theEvent;
-(void)flagsChanged:(NSEvent *)theEvent;

-(void)showCursor:(BOOL)show;
-(void)createTrackingArea;
-(void)updateTrackingAreas;
//-(void)cursorUpdate:(NSEvent*)event;
-(void)mouseEntered:(NSEvent *)theEvent;
-(void)mouseExited:(NSEvent *)theEvent;

@end
