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
#pragma once
@class ModalInput;

@protocol ModalInputDelegate <NSObject>
- (void)endedWithAction:(UIAlertAction*)action andAlertController:(UIAlertController*)alert;
@end

#import <Foundation/Foundation.h>
#import "MainViewController.h"

//@interface ModalInput : UIAlertView
@interface ModalInput : NSObject
{
   __weak id<ModalInputDelegate> _delegate;
   UIAlertController* _alert;
   MainViewController* _mainController;
}
@property (nonatomic, weak) id delegate;

//@property (nonatomic, retain) UITextField *textField;
//@property (nonatomic, retain) UITextField *passwordField;
@property (nonatomic, retain) NSString* text;
@property (nonatomic, retain) NSString* password;

- (id)initStringWithTitle:(NSString *)title message:(NSString *)message fromViewController:(MainViewController *)viewController cancelButtonTitle:(NSString *)cancelButtonTitle okButtonTitle:(NSString *)okButtonTitle;
- (id)initNumberWithTitle:(NSString *)title message:(NSString *)message fromViewController:(MainViewController *)viewController cancelButtonTitle:(NSString *)cancelButtonTitle okButtonTitle:(NSString *)okButtonTitle;
- (id)initNamePasswordWithTitle:(NSString *)title message:(NSString *)message fromViewController:(MainViewController *)viewController cancelButtonTitle:(NSString *)cancelButtonTitle okButtonTitle:(NSString *)okButtonTitle;
-(id)initMultiLineStringWithTitle:(NSString *)title message:(NSString *)message fromViewController:(MainViewController *)controller cancelButtonTitle:(NSString *)cancelButtonTitle okButtonTitle:(NSString *)okayButtonTitle;

- (void)setTextField:(NSString*)string at:(int)index;
- (void)endedWithAction:(UIAlertAction*)action;
- (void)show;
@end
