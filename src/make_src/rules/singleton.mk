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
# Copyright 1996 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms.
#
# @(#)singleton.mk 1.64 06/12/12
#

# This is an included Makefile for a program that consists of a single C++
# source file and is built with the standard Avocet library.

# Before including this Makefile the following variables should be set:
#	TOP 	the path from the including Makefile to this one
#	PROG	the name of the program
#	DESTDIR	destination directory for the install 
#
# All other information is derived from PROG.

SRCS = $(PROG).cc $(MORE_SRC)
OBJS = $(SRCS:%.cc=%.o)

LIB +=		$(LIBCLI) $(LIBLICTXT) -lintl -lm

##-----------------------------------------------------------------

all install:: $(PROG)

include $(TOP)/rules/verstring.mk

$(PROG): $(OBJS)
	@echo "#pragma ident \"@(#)$(VERSTRING1)\"" > timestamp.c
	@echo $(VERSTRING2) >> timestamp.c
	cc -w -c timestamp.c
	$(CCC) $(CCFLAGS) $(LDFLAGS) timestamp.o -o $@ \
		$(OBJS) $(LIB) $(I18LIB) $(LDLIB)
	/bin/sh $(TOP)/exe/sanity-check.sh $(TOP) $(PROG)

clean:
	$(RM) $(OBJS) $(PROG)

PROG_DIR =	bin

include $(TOP)/rules/install.mk

include $(TOP)/rules/computed-options.mk
