TARGET = cxcore
BINTYPE = DLL
ROOT = ..\..
PCH = _cxcore.h
PCH_STARTER = cxprecomp

OBJS = $(OBJPATH)/cxalloc.obj $(OBJPATH)/cxarithm.obj $(OBJPATH)/cxarray.obj \
       $(OBJPATH)/cxcmp.obj $(OBJPATH)/cxconvert.obj $(OBJPATH)/cxcopy.obj \
       $(OBJPATH)/cxdatastructs.obj $(OBJPATH)/cxdrawing.obj $(OBJPATH)/cxdxt.obj \
       $(OBJPATH)/cxerror.obj $(OBJPATH)/cximage.obj $(OBJPATH)/cxjacobieigens.obj \
       $(OBJPATH)/cxlogic.obj $(OBJPATH)/cxlut.obj $(OBJPATH)/cxmathfuncs.obj \
       $(OBJPATH)/cxmatmul.obj $(OBJPATH)/cxmatrix.obj $(OBJPATH)/cxmean.obj \
       $(OBJPATH)/cxmeansdv.obj $(OBJPATH)/cxminmaxloc.obj $(OBJPATH)/cxnorm.obj \
       $(OBJPATH)/cxouttext.obj $(OBJPATH)/cxpersistence.obj \
       $(OBJPATH)/cxrand.obj $(OBJPATH)/cxsumpixels.obj $(OBJPATH)/cxsvd.obj \
       $(OBJPATH)/cxswitcher.obj $(OBJPATH)/cxtables.obj $(OBJPATH)/cxutils.obj

INC = ../include/cxcore.h ../include/cxcore.hpp ../include/cxerror.h \
    ../include/cxmisc.h ../include/cxtypes.h ./_cxcore.h ./_cxipp.h ../include/cvver.h

CXXFLAGS_PROJ = /I"." /I"../include"

!include $(ROOT)/_make/make_module_$(MS).mak
