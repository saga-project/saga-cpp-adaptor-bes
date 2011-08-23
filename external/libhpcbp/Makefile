
#  Copyright (c) 2009-2011 Andre Merzky <andre@merzky.net>
#  Distributed under the GPLv.2 - see accompanying LICENSE file.


-include ../../config/make.cfg

SOAP_NS           = BESFactoryBinding.nsmap

SAGA_ARC          = hpcbp_client
SAGA_SRC          = $(wildcard *.cpp)
SAGA_LIB_OBJ     += $(SAGA_SRC:%.cpp=%.o)

SAGA_DONT_INSTALL = 1

# use C++ also  for the C source files
%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

SAGA_CXXFLAGS  += -I. -DDEBUG
SAGA_CXXFLAGS  += -DWITH_DOM -DWITH_OPENSSL
SAGA_LDFLAGS   += -lssl -lcrypto

