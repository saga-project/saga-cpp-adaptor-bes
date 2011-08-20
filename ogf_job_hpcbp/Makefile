
#  Copyright (c) 2009-2011 Andre Merzky <andre@merzky.net>
#  Distributed under the GPLv.2 - see accompanying LICENSE file.


-include ../config/make.cfg

SAGA_SUBDIRS     += test

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_TYPE = job
SAGA_ADAPTOR_NAME = ogf_hpcbp_job
SAGA_ADAPTOR_INI  = ogf_hpcbp_job_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = job

SAGA_ADAPTOR_REQUIREMENTS = package_filesystem

include $(SAGA_MAKE_INCLUDE_ROOT)/saga.adaptor.mk
include $(SAGA_MAKE_INCLUDE_ROOT)/saga.doc.mk

SAGA_CPPINCS     += -I../external/libhpcbp
SAGA_LDFLAGS     += -L../external/libhpcbp -lhpcbp_client -lssl -lcrypto

