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
//  CIni.h
//  RuntimeIPhone


#import <Foundation/Foundation.h>

@class CArrayList;

#define    INI_UTF8                    0x0008
#define    INI_DONTWRITECHANGES        0x0040

@interface CIni : NSObject
{
@public
	NSMutableDictionary* groups;
    NSString* currentFileName;
    short iniFlags;
	BOOL hasUnsavedChanges;
}

+(CIni*)getINIforFile:(NSString*)filename andFlags:(short)_iniFlags;
+(void)saveAllOpenINIfiles;
+(void)closeIni:(CIni*)ini;

-(id)initWithFilename:(NSString*)filename andFlags:(short)_iniFlags;
-(void)dealloc;
-(void)saveIni;

-(NSString*)getValueFromGroup:(NSString*)groupName withKey:(NSString*)keyName andDefaultValue:(NSString*)defaultString;
-(void)writeValueToGroup:(NSString*)groupName withKey:(NSString*)keyName andValue:(NSString*)value;
-(void)deleteItemFromGroup:(NSString*)group withKey:(NSString*)keyname;
-(void)deleteGroup:(NSString*)groupName;

@end
