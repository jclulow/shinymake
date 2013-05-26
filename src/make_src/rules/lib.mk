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
# @(#)lib.mk 1.33 06/12/12
#

#
# This Makefile contains common rules for building libraries.
# It expects the following variables to be defined:
#	
#	LIBNAME	   the name of the library being built
#	CCSRCS	   C++ source files
#	CSRCS	   C source files
#	BIGCSRCS   C++ source files with .C suffix (so checkgettext isn't run)
#

CCOBJS =		$(CCSRCS:%.cc=%.o)
COBJS =		$(CSRCS:%.c=%.o)
BIGCOBJS =	$(BIGCSRCS:%.C=%.o)

OBJS +=		$(CCOBJS) $(COBJS) $(BIGCOBJS)

L10N_SRCS = 	$(LIBNAME)_srrcat.po messages.po

#
# Is this '[d]make' connected to a terminal?
#
TTY :sh=		 if test -t 2; then echo true; else echo false; fi

#
# This complicated set of commands determine if there are
# any .o files in the library that no longer belong there.
# If so, the library must be removed and rebuilt from scratch.
# If the old .o files are left in the library, then a program that
# references a routine in one of these .o files will still build
# until the library gets completely rebuilt, in which case it will
# no longer build.
# In addition, because $? macro is used in the command that builds
# this library, command dependency checking is disabled.  Even if
# this command changes, the library won't be rebuilt.  The library's
# list of dependencies appear in a separate command, which is subject
# to command dependency checking (echo to /dev/null), to ensure that
# the library rebuilt when its list of dependencies change.
#

#	XD_INCLS is defined in rules/motif-lib.mk and will
#	be empty if not called from motif-lib.mk

$(LIBNAME): $(OBJS) 
	@echo "Building $(LIBNAME)" > \
	    `$(TTY) && test -t 0 && tty || echo /dev/null`
	@echo $(OBJS) > /dev/null
	@?ar t $(LIBNAME) | sed -e /__.SYMDEF/d | sort > ar.t.$$$$; \
	for i in $(OBJS); do \
		echo $$i; \
	done | sort | comm -23 ar.t.$$$$ - > extras.$$$$; \
	if [ -s extras.$$$$ ]; then \
		echo rm -f $(LIBNAME); \
		rm -f $(LIBNAME); \
		echo ar rv $(LIBNAME) $(OBJS); \
		ar rv $(LIBNAME) $(OBJS); \
	else \
		echo ar crv $(LIBNAME) $?; \
		ar crv $(LIBNAME) $?; \
	fi; \
	rm -f ar.t.$$$$ extras.$$$$
	@if [ -f $(RANLIB) ]; then \
		echo $(RANLIB) $(LIBNAME); \
		$(RANLIB) $(LIBNAME); \
	fi

#all install:: $(LIBNAME) catalogs
###all install:: $(LIBNAME)
all: $(LIBNAME)

install: $(LIBNAME)

#all:: $(LIBNAME)
#
#install:: all catalogs

clean::
	$(RM) $(OBJS) $(LIBNAME) $(CLEANFILES) $(CATALOG)

# temporarily install catalogs here - will move to l10n-install later
# when mesg files are versioned
LIB_DESTDIR =       $(DESTDIR)/$(PRODVER)/lib
CAT_DESTDIR =       $(LIB_DESTDIR)/locale/C/LC_MESSAGES

#also temporary, until I find a variant specific place to find this tool
COMPMSG =

catalogs::
	$(GENMSG) -l $(SRC)/genmsg.project -o $(MSG_FILE) `find $(I18N_DIRS) \( -name '*.cc' -o -name '*.c' -o -name '*.y' -o -name '*.h' \) -print | grep -v /SCCS/`
	rm -f *.cc.new
#	@ if [ -f $(SRC)/$(MSG_FILE) ]; then \
#		echo "running compmsg on $(MSG_FILE)"; \
#		$(COMPMSG) -n $(SRC)/$(MSG_FILE) $(MSG_FILE); \
#	fi


#	@ if [ ! -d $(CAT_DESTDIR) ]; then \
#		$(INSTALL) -d $@ \
#	fi
#	cp $(MSG_FILE) $(CAT_DESTDIR)

FRC:

include $(TOP)/rules/l10n-install.mk
