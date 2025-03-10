/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//						Intel License Agreement
//				For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//	* Redistribution's of source code must retain the above copyright notice,
//	 this list of conditions and the following disclaimer.
//
//	* Redistribution's in binary form must reproduce the above copyright notice,
//	 this list of conditions and the following disclaimer in the documentation
//	 and/or other materials provided with the distribution.
//
//	* The name of Intel Corporation may not be used to endorse or promote products
//	 derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef _GRFMT_BMP_H_
#define _GRFMT_BMP_H_

#include "grfmt_base.h"

enum BmpCompression
{
	BMP_RGB = 0,
	BMP_RLE8 = 1,
	BMP_RLE4 = 2,
	BMP_BITFIELDS = 3
};


// Windows Bitmap reader
class GrFmtBmpReader : public GrFmtReader
{
public:

	GrFmtBmpReader( const char* filename );
	~GrFmtBmpReader();

	bool  ReadData( uchar* data, int step, int color );
	bool  ReadHeader();
	void  Close();

protected:

	RLByteStream	m_strm;
	PaletteEntry	m_palette[256];
	int			 m_bpp;
	int			 m_offset;
	BmpCompression  m_rle_code;
};


// ... writer
class GrFmtBmpWriter : public GrFmtWriter
{
public:

	GrFmtBmpWriter( const char* filename );
	~GrFmtBmpWriter();

	bool  WriteImage( const uchar* data, int step,
					  int width, int height, int depth, int channels );
protected:

	WLByteStream  m_strm;
};


// ... and filter factory
class GrFmtBmp : public GrFmtFilterFactory
{
public:

	GrFmtBmp();
	~GrFmtBmp();

	GrFmtReader* NewReader( const char* filename );
	GrFmtWriter* NewWriter( const char* filename );

};

#endif/*_GRFMT_BMP_H_*/
