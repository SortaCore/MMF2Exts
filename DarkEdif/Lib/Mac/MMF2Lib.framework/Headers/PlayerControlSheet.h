//
//  PlayerControlController.h
//  MacRuntime
//
//  Created by Anders Riggelsen on 2/17/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface PlayerControlSheet : NSControl
{
	NSTextField *upLabel, *downLabel, *leftLabel, *rightLabel;
	NSTextField *buttonALabel, *buttonBLabel, *buttonCLabel, *buttonDLabel;
	NSProgressIndicator* upProgress, *downProgress, *leftProgress, *rightProgress, *buttonAProgress, *buttonBProgress, *buttonCProgress, *buttonDProgress;

}
-(IBAction)selectKeyUpArrow:(id)sender;
-(IBAction)selectKeyDownArrow:(id)sender;
-(IBAction)selectKeyLeftArrow:(id)sender;
-(IBAction)selectKeyRightArrow:(id)sender;
-(IBAction)selectKeyButtonA:(id)sender;
-(IBAction)selectKeyButtonB:(id)sender;
-(IBAction)selectKeyButtonC:(id)sender;
-(IBAction)selectKeyButtonD:(id)sender;

@property(nonatomic, retain) IBOutlet NSTextField* upLabel;
@property(nonatomic, retain) IBOutlet NSTextField* downLabel;
@property(nonatomic, retain) IBOutlet NSTextField* leftLabel;
@property(nonatomic, retain) IBOutlet NSTextField* rightLabel;
@property(nonatomic, retain) IBOutlet NSTextField* buttonALabel;
@property(nonatomic, retain) IBOutlet NSTextField* buttonBLabel;
@property(nonatomic, retain) IBOutlet NSTextField* buttonCLabel;
@property(nonatomic, retain) IBOutlet NSTextField* buttonDLabel;

@property(nonatomic, retain) IBOutlet NSProgressIndicator* upProgress;
@property(nonatomic, retain) IBOutlet NSProgressIndicator* downProgress;
@property(nonatomic, retain) IBOutlet NSProgressIndicator* leftProgress;
@property(nonatomic, retain) IBOutlet NSProgressIndicator* rightProgress;
@property(nonatomic, retain) IBOutlet NSProgressIndicator* buttonAProgress;
@property(nonatomic, retain) IBOutlet NSProgressIndicator* buttonBProgress;
@property(nonatomic, retain) IBOutlet NSProgressIndicator* buttonCProgress;
@property(nonatomic, retain) IBOutlet NSProgressIndicator* buttonDProgress;


@end
