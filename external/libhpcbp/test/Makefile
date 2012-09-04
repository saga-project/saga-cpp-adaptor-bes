
#  Copyright (c) 2009-2011 Andre Merzky <andre@merzky.net>
#  Distributed under the GPLv.2 - see accompanying LICENSE file.


-include ../../../config/make.cfg

SAGA_SRC          = $(wildcard *.cpp)
SAGA_BIN_OBJ     += $(SAGA_SRC:%.cpp=%.o)
SAGA_BIN          = test_bes

SAGA_DONT_INSTALL = 1

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

SAGA_CXXFLAGS  += -I..
SAGA_LDFLAGS   += -L.. -lhpcbp_client


