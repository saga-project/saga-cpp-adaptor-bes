#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include ../../config/make.cfg

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)
SAGA_BIN          = $(SAGA_SRC:%.cpp=%)

SAGA_USE_PROFILER = yes

SAGA_CLEAN_TGT = $(wildcard *.log)

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.application.mk


