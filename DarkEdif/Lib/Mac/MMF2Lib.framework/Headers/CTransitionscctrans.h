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
// CTransitionCTrans : point d'entree des transitions standart
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "CTransitions.h"
#import "CTrans.h"



@class CTransitionData;
@class CFile;
class CRenderer;
@class CRenderToTexture;

@interface CTransitionscctrans : CTransitions
{
}
-(CTrans*)getTrans:(CTransitionData*)data;
@end

@interface CTransAdvancedScrolling : CTrans
{
    int dwStyle;
    int m_source2Width;
    int m_source2Height;
    int m_style;
	CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransBack : CTrans
{
    int dwStyle;
    int m_source2Width;
    int m_source2Height;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransBand : CTrans
{
    int bpNbBands;
    int bpDirection;
    int m_wbande;
    int m_rw;
	CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransCell : CTrans
{
    int dwPos;
    int dwPos2;
    int m_source2Width;
    int m_source2Height;
    CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransDoor : CTrans
{
    short m_direction;
    int m_wbande;
    int m_rw;
    CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransFade : CTrans
{
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransLine : CTrans
{
    int dwPos;
    int dwStyle;
    int dwScrolling;
    int m_source2Width;
    int m_source2Height;
    CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransMosaic : CTrans
{
    // Parameters
    int m_spotPercent;
	
    // Runtime
    int m_spotSize;
    int m_nbBlockPerLine;
    int m_nbBlockPerCol;
    int m_nbBlocks;
    int m_lastNbBlocks;
    unsigned char* m_bitbuf;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransOpen : CTrans
{
    int dwStyle;
    int m_source2Width;
    int m_source2Height;
	CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransPush : CTrans
{
    int dwStyle;
    int m_source2Width;
    int m_source2Height;
    BOOL m_refresh;
    CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransScroll : CTrans
{
    int m_direction;
    int m_wbande;
    int m_rw;
    CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransSquare : CTrans
{
    int dwStyle;
    int dwPos;
    int dwStretch;
    int m_source2Width;
    int m_source2Height;
    CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransStretch : CTrans
{
    int dwStyle;
    int m_source2Width;
    int m_source2Height;
    CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransStretch2 : CTrans
{
    int dwStyle;
    int m_source2Width;
    int m_source2Height;
    int m_phase;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransTrame : CTrans
{
    int dwStyle;
    int m_source2Width;
    int m_source2Height;
    int m_index;
    int m_index2;
	CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransTurn : CTrans
{
    int dwPos;
    int dwCheck1;
    int dwCheck2;
    int m_source2Width;
    int m_source2Height;
    double m_angle;
	CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransTurn2 : CTrans
{
    int dwPos;
    int dwCheck1;
    int m_source2Width;
    int m_source2Height;
    int m_curcircle;
	CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransZigZag : CTrans
{
    int zpSpotPercent;
    short zpStartPoint;
    short zpDirection;
    int m_spotSize;
    int m_nbBlockPerLine;
    int m_nbBlockPerCol;
    int m_nbBlocks;
    int m_lastNbBlocks;
    int m_curx;
    int	m_cury;
    int	m_currentDirection;
    int	m_currentStartPoint;
    int	m_left;
    int	m_top;
    int	m_right;
    int	m_bottom;
	CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransZigZag2 : CTrans
{
    int dwStyle;
    int dwPos;
    int m_source2Width;
    int m_source2Height;
    int m_linepos;
    int m_dir;
	CRenderToTexture* tempBuffer;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransZoom : CTrans
{
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

@interface CTransZoom2 : CTrans
{
    int dwPos;
    int m_source2Width;
    int m_source2Height;
}
-(void)initialize:(CTransitionData*)data withFile:(CFile*)file andRenderer:(CRenderer*)renderer andStart:(CRenderToTexture*)source andEnd:(CRenderToTexture*)dest andType:(int)type;
-(char*)stepDraw:(int)flag;
-(void)end;
@end

