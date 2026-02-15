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
//----------------------------------------------------------------------------------
//
// CVALUE : classe de calcul et de stockage de valeurs
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>

#define TYPE_INT 0
#define TYPE_DOUBLE 1
#define TYPE_STRING 2

@interface CValue : NSObject
{
@public
	short type;
	int intValue;
	double doubleValue;
	NSString* stringValue;
}

-(id)init;
-(id)initWithInt:(int)value;
-(id)initWithDouble:(double)value;
-(id)initWithString:(NSString*)string;
-(id)initWithValue:(CValue*)value;
-(void)dealloc;
-(void)releaseString;
-(short)getType;
-(int)getInt;
-(double)getDouble;
-(NSString*)getString;
-(void)forceInt:(int)value;
-(void)forceDouble:(double)value;
-(void)forceString:(NSString*)value;
-(void)forceValue:(CValue*)value;
-(void)setValue:(CValue*)value;
-(void)convertToDouble;
-(void)convertToInt;
-(void)add:(CValue*)value;
-(void)addInt:(int)lvalue;
-(void)addDouble:(double)dvalue;
-(void)sub:(CValue*)value;
-(void)subInt:(int)lvalue;
-(void)subDouble:(double)dvalue;
-(void)negate;
-(void)mul:(CValue*)value;
-(void)div:(CValue*)value;
-(void)pow:(CValue*)value;
-(void)mod:(CValue*)value;
-(void)andLog:(CValue*)value;
-(void)orLog:(CValue*)value;
-(void)xorLog:(CValue*)value;
-(BOOL)equal:(CValue*)value;
-(BOOL)equalInt:(int)lvalue;
-(BOOL)equalDouble:(double)dvalue;
-(BOOL)greater:(CValue*)value;
-(BOOL)greaterInt:(int)lvalue;
-(BOOL)greaterDouble:(double)dvalue;
-(BOOL)lower:(CValue*)value;
-(BOOL)lowerInt:(int)lvalue;
-(BOOL)lowerDouble:(double)dvalue;
-(BOOL)greaterThan:(CValue*)value;
-(BOOL)greaterThanInt:(int)lvalue;
-(BOOL)greaterThanDouble:(double)dvalue;
-(BOOL)lowerThan:(CValue*)value;
-(BOOL)lowerThanInt:(int)lvalue;
-(BOOL)lowerThanDouble:(double)dvalue;
-(BOOL)notEqual:(CValue*)value;
-(BOOL)notEqualInt:(int)lvalue;
-(BOOL)notEqualDouble:(double)dvalue;
-(NSString*)description;

@end
