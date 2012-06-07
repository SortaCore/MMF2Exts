TARGET := highgui
BINTYPE := DLL
SRC_ROOT := ../../otherlibs/highgui
INC_ROOT := ../../otherlibs/highgui
CXCORE_INC := ../../cxcore/include
SRC_DIRS := . ../../cxcore/include

OBJS := bitstrm.o cvcap.o cvcap_cmu.o cvcap_images.o cvcap_dshow.o \
    cvcap_vfw.o cvcap_w32.o grfmt_base.o grfmt_bmp.o grfmt_jpeg.o \
    grfmt_png.o grfmt_pxm.o grfmt_sunras.o grfmt_tiff.o image.o \
    loadsave.o utils.o window.o window_w32.o

CXXFLAGS := -D"HAVE_JPEG" -D"HAVE_PNG" -D"HAVE_ZLIB" -D"HAVE_TIFF" -D"CVAPI_EXPORTS" \
    -I"$(INC_ROOT)" -I"$(CXCORE_INC)" -I"../../otherlibs/_graphics/include"

INCS := bitstrm.h grfmts.h grfmt_base.h grfmt_bmp.h grfmt_jpeg.h \
    grfmt_png.h grfmt_pxm.h grfmt_sunras.h grfmt_tiff.h \
    _highgui.h utils.h highgui.h \
    cxcore.h cxcore.hpp cxerror.h cxmisc.h cxtypes.h cvver.h

LIBS := -lcxcore$(DBG) -Wl,-L../../otherlibs/_graphics/lib \
    -ltiff -ljpeg -lpng -lz -lvfw_avi32 -lvfw_cap32 -lvfw_ms32 -lcomctl32

include ../../_make/make_module_gnu.mak
