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
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms.
#
# @(#)derived.mk 1.15 06/12/12
#

# Include this makefile in directories which contain source whose derived
# files are to be built in ../<variant>.

# The source directory must contain a makefile with these contents:
#
# TOP = <as appropriate>
# include $(TOP)/rules/master.mk
# SRC = <as appripriate>
# include $(TOP)/rules/variant.mk
#
# The source directory must also contain a makefile named "Derived.mk"
# which is used to build the derived files.
#
# TOP is defined as a relative path to the root of the workspace
# SRC is defined as a relative path from the derived files' directory to the
# source files' directory, and is typically ../src.

include $(TOP)/rules/$(VARIANT).mk

all :=		TARG = all 
install :=	TARG = install 
clean :=	TARG = clean 
test :=		TARG = test
l10n_install :=	TARG = l10n_install
i18n_install :=	TARG = i18n_install

all install clean test l10n_install i18n_install: $(VARIANT) $(V9_VARIANT)

SRC =		../src
MFLAGS += 	SRC=$(SRC)

# Conditional is in place so a "make" from the top level will work on partial
# source heirarchies.
$(VARIANT) $(V9_VARIANT): FRC
	if [ ! -d $@ ]; then \
		mkdir $@ ; \
	fi
	cd $@; $(MAKE) $(MFLAGS) -f $(SRC)/Variant.mk RELEASE_VER="$(RELEASE_VER)" DESTDIR=$(DESTDIR) V9FLAGS="$(V9FLAGS)" VAR_DIR=$(VAR_DIR) CC=$(CC) CCC=$(CCC) $(TARG);

FRC:
