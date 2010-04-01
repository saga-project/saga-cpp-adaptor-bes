#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include ../../config/make.cfg

GSOAP_TOP         = ../gsoap-2.7/gsoap

SOAP_NS      = BESFactoryBinding.nsmap

SAGA_ARC          = hpcbp_client
# SAGA_HDR          = $(wildcard *.h)
# SAGA_HDR         += $(wildcard *.hpp)
SAGA_CSRC         = $(wildcard *.c)
SAGA_CPPSRC       = $(wildcard *.cpp)
SAGA_LIB_OBJ      = $(SAGA_CSRC:%.c=%.o)
SAGA_LIB_OBJ     += $(SAGA_CPPSRC:%.cpp=%.o)

SAGA_DONT_INSTALL = 1

# use C++ also  for the C source files
%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

SAGA_CPPINCS   += -I.
SAGA_CPPFLAGS  += -DWITH_DOM -DWITH_OPENSSL
SAGA_LDFLAGS   += -lssl -lcrypto

# SAGA_CPPFLAGS  += -DDEBUG
