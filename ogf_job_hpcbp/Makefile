#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, Version 1.0. 
#  (See accompanying file LICENSE or copy at 
#  http://www.boost.org/LICENSE_1_0.txt)

ifndef SAGA_LOCATION
 $(error "SAGA_LOCATION must be set for this Makefile to work")
endif

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_TYPE = job
SAGA_ADAPTOR_NAME = ogf_hpcbp_job
SAGA_ADAPTOR_INI  = ogf_hpcbp_job_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = job

SAGA_ADAPTOR_REQUIREMENTS = package_filesystem

include $(SAGA_LOCATION)/share/saga/make/saga.adaptor.mk
include $(SAGA_LOCATION)/share/saga/make/saga.doc.mk

SAGA_CPPINCS     += -I../external/libhpcbp
SAGA_LDFLAGS     += -L../external/libhpcbp -lhpcbp_client -lssl -lcrypto

