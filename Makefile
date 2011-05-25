#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include config/make.cfg

SAGA_SUBDIRS += config
SAGA_SUBDIRS += external
SAGA_SUBDIRS += ogf_job_hpcbp
# SAGA_SUBDIRS += ogf_job_occi


all:: config/make.cfg

config/make.cfg: 
	@echo ""
	@echo " ================================= "
	@echo "  you need to run configure first  "
	@echo " ================================= "
	@echo ""
	@false


-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

# directory dependencies
ogf_job_hpcbp: external

