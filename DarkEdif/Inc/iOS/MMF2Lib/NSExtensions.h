#pragma once
#import <Foundation/Foundation.h>


@interface NSData (NSDataExtension)

//Base 64
- (NSString*) base64encode;

// ZLIB
- (NSData *) zlibInflate;
- (NSData *) zlibDeflate;

// GZIP
- (NSData *) gzipInflate;
- (NSData *) gzipDeflate;

@end


@interface NSString (NSStringExtension)

- (NSString *) md5;
- (NSString*)urlEncode;
- (CGSize)sizeWithFSFont:(UIFont*)font;
- (CGSize)sizeWithFSFont:(UIFont *)font constrainedToSize:(CGSize)size lineBreakMode:(NSLineBreakMode)flag;

@end


