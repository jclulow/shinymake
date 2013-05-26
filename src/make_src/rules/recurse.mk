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
# Copyright 2002 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms.
#
# @(#)recurse.mk 1.15 06/12/12
#

# Include this Makefile in directories which simply cd down a level
# and continue the build.
#
# The SUBDIRS macro must be set before including recurse.Makefile
# The OTHER_TARGS macro can be set to process targets other than
# directories.

all :=		TARG = all 
install :=		TARG = install 
clean :=		TARG = clean 
l10n_install :=	TARG = l10n_install
i18n_install :=	TARG = i18n_install

# Are we _really_ using  OTHER_TARGS???
#all install clean l10n_install: $(SUBDIRS) $(OTHER_TARGS)
all install clean l10n_install i18n_install:: $(SUBDIRS) $(OTHER_TARGS)

# Conditional is in place so a "make" from the top level will work on partial
# source heirarchies.
#
# "MAKEFILE" can be set by makes driving this one to cause a particular
# makefile to be used.   E.g., MAKEFILE=-f foo.mk
#
$(SUBDIRS):	FRC
	@if [ -d $@ ]; then \
		echo "cd $@; $(MAKE) $(MAKEFILE) $(MFLAGS) CC=$(CC) CCC=$(CCC) DESTDIR=$(DESTDIR) $(TARG)"; \
		cd $@; $(MAKE) $(MAKEFILE) $(MFLAGS) CC=$(CC) CCC=$(CCC) DESTDIR=$(DESTDIR) $(TARG); \
	fi

FRC:


