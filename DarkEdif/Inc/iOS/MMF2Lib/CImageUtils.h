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
// CImageUtils: libraries
//
//----------------------------------------------------------------------------------
#pragma once

#ifndef CImageUtils_h
#define CImageUtils_h

#import "Classes/turbojpeg.h"


void loadJPEGFromData(unsigned char *jpegBuf, unsigned long jpegSize, int *width, int *height, unsigned int **rawData);

void loadJPEGFromData(unsigned char *jpegBuf, unsigned long jpegSize, int *width, int *height, unsigned int **rawData)
{
    tjhandle decoder = tjInitDecompress();
    
    int w, h, jpegSubsamp;
    
    int result = tjDecompressHeader2(decoder, jpegBuf, jpegSize, &w, &h, &jpegSubsamp);
    if (result < 0)
    {
        NSLog(@"Error in Header");
        tjDestroy(decoder);
        return;
    }
    
    int pitch = w * 4;

    *rawData = (unsigned int*)calloc(w * h, sizeof(unsigned int));

    result = tjDecompress2(decoder, jpegBuf, jpegSize, (unsigned char *)*rawData, w, pitch, h, TJPF_RGBA, 0);

    if (result < 0)
    {
        free(*rawData);
        *rawData = nil;
        *width = *height = 0;
        NSLog(@"Error in Decompression");
    }

    *width = w;
    *height = h;
    tjDestroy(decoder);
}

#endif /* CImageUtils_h */
