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
// COILIST : liste des OI de l'application
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>
#import "IEnum.h"

@class CRunApp;
@class COI;
@class CFile;

@interface COIList : NSObject
{
@public
    short oiMaxIndex;
    COI** ois;
    short oiMaxHandle;
    short* oiHandleToIndex;
    char* oiToLoad;
    char* oiLoaded;
    int currentOI;
}

-(void)dealloc;
-(void)preLoad:(CFile*)file;
-(COI*)getOIFromHandle:(short)handle;
-(COI*)getOIFromIndex:(short)index;
-(void)resetOICurrent;
-(void)setOICurrent:(int)handle;
-(COI*)getFirstOI;
-(COI*)getNextOI;
-(void)resetToLoad;
-(void)setToLoad:(int)n;
-(void)setToReload:(int)h;
-(void)load:(CFile*)file;
-(void)enumElements:(id)enumImages withFont:(id)enumFonts;

@end
