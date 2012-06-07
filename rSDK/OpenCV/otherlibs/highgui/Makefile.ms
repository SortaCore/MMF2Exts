TARGET = highgui
BINTYPE = DLL
ROOT = ..\..
PCH = _highgui.h
PCH_STARTER = precomp

OBJS = $(OBJPATH)/bitstrm.obj $(OBJPATH)/cvcap.obj $(OBJPATH)/cvcap_cmu.obj $(OBJPATH)/cvcap_dshow.obj\
    $(OBJPATH)/cvcap_images.obj $(OBJPATH)/cvcap_vfw.obj $(OBJPATH)/cvcap_w32.obj $(OBJPATH)/grfmt_base.obj \
    $(OBJPATH)/grfmt_bmp.obj $(OBJPATH)/grfmt_jpeg.obj  $(OBJPATH)/grfmt_png.obj \
    $(OBJPATH)/grfmt_pxm.obj $(OBJPATH)/grfmt_sunras.obj $(OBJPATH)/grfmt_tiff.obj \
    $(OBJPATH)/image.obj $(OBJPATH)/loadsave.obj  $(OBJPATH)/utils.obj \
    $(OBJPATH)/window.obj  $(OBJPATH)/window_w32.obj

INCS = ./bitstrm.h ./grfmts.h ./grfmt_base.h ./grfmt_bmp.h ./grfmt_jpeg.h \
    ./grfmt_png.h ./grfmt_pxm.h ./grfmt_sunras.h ./grfmt_tiff.h \
    ./_highgui.h ./utils.h ./highgui.h \
    $(ROOT)/cxcore/include/cxcore.h $(ROOT)/cxcore/include/cxcore.hpp \
    $(ROOT)/cxcore/include/cxerror.h $(ROOT)/cxcore/include/cxmisc.h \
    $(ROOT)/cxcore/include/cxtypes.h $(ROOT)/cxcore/include/cvver.h

!if "$(MS)" == "bc"
GRLIB_SUFFIX = _bcc
LIBJASPER =
!else
GRLIB_SUFFIX =
!if "$(PLATFORM)" == "I386"
LIBJASPER = libjasper$(OUT_P_SUFFIX)$(GRLIB_SUFFIX).lib
!endif
!endif

!if "$(PLATFORM)" != "IA64"
CXXFLAGS_GRFMT = /DHAVE_JPEG /DHAVE_PNG /DHAVE_PNG /I"../_graphics/include"
LIBS_GRFMT = libjpeg$(OUT_P_SUFFIX)$(GRLIB_SUFFIX).lib \
    libpng$(OUT_P_SUFFIX)$(GRLIB_SUFFIX).lib \
    libtiff$(OUT_P_SUFFIX)$(GRLIB_SUFFIX).lib \
    zlib$(OUT_P_SUFFIX)$(GRLIB_SUFFIX).lib $(LIBJASPER)
!else
CXXFLAGS_GRFMT =
LIBS_GRFMT =
!endif

CXXFLAGS_PROJ = /I"." /I"$(ROOT)/cxcore/include" $(CXXFLAGS_GRFMT)
LIBS_PROJ = $(LIBPATH)"..\_graphics\lib" $(LIBPATH)"$(ROOT)/lib" vfw32.lib comctl32.lib \
    cxcore$(DBG)$(OUT_P_SUFFIX).lib videoinput.lib $(LIBS_GRFMT)

!include $(ROOT)/_make/make_module_$(MS).mak
