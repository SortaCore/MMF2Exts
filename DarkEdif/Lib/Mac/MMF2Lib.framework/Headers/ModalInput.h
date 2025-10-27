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

#import <Foundation/Foundation.h>

@interface ModalInput : NSObject
{
@public
	NSWindow* window;
	NSAlert* alert;

	int _returnCode;
	SEL _selector;
	id _delegate;

	NSTextField *textField;
	NSSecureTextField *passwordField;
	NSView* accessoryView;
}
@property (nonatomic, retain) NSTextField *textField;
@property (nonatomic, retain) NSTextField *passwordField;
@property (readonly) NSString* text;
@property (readonly) NSString* password;

- (id)initStringWithTitle:(NSString *)title message:(NSString *)message delegate:(id)delegate selector:(SEL)onEnd cancelButtonTitle:(NSString *)cancelButtonTitle okButtonTitle:(NSString *)okButtonTitle;
- (id)initNumberWithTitle:(NSString *)title message:(NSString *)message delegate:(id)delegate selector:(SEL)onEnd cancelButtonTitle:(NSString *)cancelButtonTitle okButtonTitle:(NSString *)okButtonTitle;
- (id)initNamePasswordWithTitle:(NSString *)title message:(NSString *)message delegate:(id)delegate selector:(SEL)onEnd cancelButtonTitle:(NSString *)cancelButtonTitle okButtonTitle:(NSString *)okButtonTitle;

- (void)resignTextField;

-(void)show;
-(NSInteger)showModal;
- (void)startModal:(NSWindow *)ignored completionHandler:(void (^ __nullable)(NSModalResponse returnCode))handler;
- (void)stopModal;

@end
