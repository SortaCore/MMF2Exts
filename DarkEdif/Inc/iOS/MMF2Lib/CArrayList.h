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
// CARRAYLIST : classe extensible de stockage
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>

#define GROWTH_STEP 5

@interface CArrayList : NSObject
{
	void** pArray;
	NSUInteger length;
	NSUInteger numberOfEntries;
}
-(id)init;
-(void)dealloc;
-(void)getArray:(NSUInteger)max;
-(void)ensureCapacity:(NSUInteger)max;
-(NSUInteger)add:(void*)o;
-(NSUInteger)addInt:(int)o;
-(void)addIndex:(NSUInteger)index object:(void*)o;
-(void)addIndex:(NSUInteger)index integer:(int)o;
-(void*)get:(NSUInteger)index;
-(int)getInt:(NSUInteger)index;
-(void)set:(NSUInteger)index object:(void*)o;
-(void)set:(NSUInteger)index integer:(int)o;
-(void)setAtGrow:(NSUInteger)index object:(void*)o;
-(void)setAtGrow:(NSUInteger)index integer:(int)o;
-(void)removeIndex:(NSUInteger)index;
-(void)removeIndexFree:(NSUInteger)index;
-(NSInteger)indexOf:(void*)o;
-(NSInteger)indexOfInt:(int)o;
-(void)removeObject:(void*)o;
-(void)removeInt:(NSInteger)o;
-(void)removeClearIndex:(NSUInteger)index;
-(int)size;
-(void)clear;
-(void)clearRelease;
-(void)removeIndexRelease:(NSUInteger)n;
-(void)freeRelease;
-(void)removeObjectRelease:(void*)o;

-(NSInteger)findString:(NSString*)string startingAt:(NSUInteger)startIndex;
-(NSInteger)findStringExact:(NSString*)string startingAt:(NSUInteger)startIndex;

-(void)sortCListItems;
-(NSMutableArray*)getNSArray;

@end





@interface CListItem : NSObject
{
@public
	NSString* string;
	int	data;
}
-(id)initWithString:(NSString*)s andData:(int)d;
-(void)dealloc;
@end