#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
# Copyright 2006 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms.
#
# @(#)master.mk 1.142 06/12/12
#

include $(TOP)/rules/variant.mk

.KEEP_STATE_FILE: .make.state_${VARIANT}

include $(TOP)/rules/$(VARIANT).mk
#include $(TOP)/lib/Lib.mk

#include $(TOP)/rules/libraries.mk

DESTDIR =

#
# Default target to build if no target is specified.
# All Makefiles should either define an "all" target, or set
# DEFAULT_TARGET to the default target
#
DEFAULT_TARGET = all
default_target: $$(DEFAULT_TARGET)

#Tools needed for i18n
GENMSG	=	genmsg

##-----------------------------------------------------------------

CHMOD =		/bin/chmod
LEX =		/usr/ccs/bin/lex
MV =		/bin/mv
RANLIB =		/bin/ranlib
YACC =		/usr/ccs/bin/yacc

##-----------------------------------------------------------------

CFLAGS +=	-Wall -Wextra -Werror

CPPFLAGS +=	-DINTER \
		-I$(TOP)/include \
		-DPREFIX=$(PREFIX) \
		-DTEAMWARE_MAKE_CMN

LINTFLAGS +=	-buxz


#LIB +=		$(LIBCMM) $(LIBSCCS) $(LIBUTIL) -lw

##-----------------------------------------------------------------
##  Rules for building derived files from $(SRC)
##  
##  Run check_catgets before compiling to see if there are any strings that need
##  i18n attention.
##
##  Use .C extension for derrived source files (e.g., yacc output) and on any
##  others on which you don't want to first run check_catgets. 

%.o: $(SRC)/%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

%.o:$(SRC)/%.C
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

%.o:$(SRC)/%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

### Generate preprocessor output
CPP.c =	$(CC) $(CFLAGS) $(CINCLS) -E
CPP.cc =	$(CCC) $(CCFLAGS) $(CCINCLS) -E

%.E: $(SRC)/%.cc
	$(CPP.cc) $(CPPFLAGS) $(SRC)/$(@:%.E=%.cc) > $*.E

%.E:$(SRC)/%.C
	$(CPP.cc) $(CPPFLAGS) $(SRC)/$(@:%.E=%.C) > $*.E

%.E:$(SRC)/%.c
	$(CPP.c) $(CPPFLAGS) $(SRC)/$(@:%.E=%.c) > $*.E

### Lex and yacc
%.cc: $(SRC)/%.y
	$(RM) $*.cc $*.h
	$(YACC) $(YFLAGS) $(SRC)/$*.y
	$(MV) y.tab.c $*.cc
	$(MV) y.tab.h $*.h
	$(CHMOD) a-w $*.cc $*.h

%.C: $(SRC)/%.l
	$(RM) $*.C
	$(LEX) $(LEXOPT)  $(SRC)/$*.l > $*.C
	$(CHMOD) a-w $@
