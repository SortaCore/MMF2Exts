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
//  FillViewController.h
//  RuntimeIPhone
//
//  Created by Anders Riggelsen on 3/24/11.
//  Copyright 2011 Clickteam. All rights reserved.
//
#pragma once
#import <Foundation/Foundation.h>
@class CRunApp;
@class CRunView;
@class MainView;

@interface MainViewController : UIViewController
{
@public
	CRunApp* runApp;
	MainView* mainView;
	CGRect screenRect;
	CGRect appRect;
    BOOL HomeHideIndicator;
}
-(id)initWithRunApp:(CRunApp*)rApp;
-(void)presentViewController:(UIViewController *)viewControllerToPresent animated:(BOOL)flag;
-(void)dismissViewControllerAnimated:(BOOL)flag;
@end
