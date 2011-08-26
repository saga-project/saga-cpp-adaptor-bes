#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

# allow for 'make srcdist' before configure
-include $(SAGA_LOCATION)/share/saga/make/saga.dist.mk

-include config/make.cfg

SAGA_SUBDIRS += config
SAGA_SUBDIRS += external
SAGA_SUBDIRS += bes_job_hpcbp


all:: config/make.cfg

ifndef SAGA_IS_PACKAGING
config/make.cfg: 
	@echo ""
	@echo " ================================= "
	@echo "  you need to run configure first  "
	@echo " ================================= "
	@echo ""
	@false
endif


-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk
-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.dist.mk

# directory dependencies
bes_job_hpcbp: external


