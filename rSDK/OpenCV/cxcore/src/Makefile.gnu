TARGET := cxcore
BINTYPE := DLL
SRC_ROOT := ../../cxcore/src
INC_ROOT := ../../cxcore/include
SRC_DIRS := . ../include

CXXFLAGS := -D"CVAPI_EXPORTS" -I"$(INC_ROOT)" -I"$(SRC_ROOT)"

#INCS := cxcore.h cxcore.hpp cxerror.h \
#  cxmisc.h cxtypes.h $(SRC_ROOT)/_cxcore.h $(SRC_ROOT)/_cxipp.h

INCS := cxcore.h cxcore.hpp cxerror.h cxmisc.h cxtypes.h _cxcore.h _cxipp.h

include ../../_make/make_module_gnu.mak
