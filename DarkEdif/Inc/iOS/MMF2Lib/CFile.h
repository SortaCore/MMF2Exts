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
// CFILE : chargement des fichiers
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>

@class CFontInfo;

@interface CFile : NSObject
{
@public
	NSData* data;
	char* pData;
	NSUInteger pointer;
	NSUInteger maxLength;
	BOOL bUnicode;
	BOOL useData;
}

#define STRINGENCODING	NSUTF8StringEncoding
#define UTFENCODING		NSUTF16LittleEndianStringEncoding

- (void)dealloc;
- (id)initWithMemoryMappedFile:(NSString*)path;
- (id)initWithPath:(NSString*)path;
- (id)initWithBytes:(unsigned char*)bytes length:(NSUInteger)length;
- (id)initWithNSDataNoRelease:(NSData*)data;
- (id)initWithNSData:(NSData*)dt;
- (void)setUnicode:(BOOL)bUnicode;
- (char)readAChar;
- (short)readAShort;
- (unichar)readAUnichar;
- (unsigned char)readAByte;
- (void)readACharBuffer:(char*)pBuffer withLength:(NSUInteger)length;
- (void)readAUnicharBuffer:(unichar*)pBuffer withLength:(NSUInteger)length;
- (int)readAInt;
- (int)readAColor;
- (float)readAFloat;
- (double)readADouble;
- (NSString*)readAStringWithSize: (NSUInteger)size;
- (NSString*)readAString;
- (NSString*)readAStringEOL;
- (void)skipAString;
- (NSUInteger)getFilePointer;
- (void)setFilePointer:(NSUInteger)pos;
- (void)seek: (NSUInteger)newPointer;
- (void)skipBack: (NSUInteger)n;
- (void)skipBytes: (NSUInteger)n;
- (NSData*)readNSData: (NSUInteger)l;
- (int)readUnsignedByte;
- (NSData*)getSubData:(NSUInteger)size;
-(CFontInfo*)readLogFont16;
-(CFontInfo*)readLogFont;
-(void)adjustTo8;
-(void)skipStringOfLength:(NSUInteger)length;
-(BOOL)IsEOF;

@end
