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
# Copyright 1997 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms.
#
# @(#)i18n-install.mk 1.23 06/12/12
#

include $(TOP)/rules/master.mk

#
# Included Makefile that installs i18n message catalogs.
#
# Before including this Makefile the following variables should be set:
#	PACKAGE		name of the package being installed
#	  replaced by PRODVER in order to install multiple pkgs in
#	  a single path.  --emk  2/96
#
#	TEXTDOMAIN	name of default textdomain used by gettext() calls 
#	APPPATH	location to find the msg file 
#

PRODVER = usr

LIB_DESTDIR =		$(DESTDIR)/$(PRODVER)/lib
CAT_DESTDIR =		$(LIB_DESTDIR)/locale/C/LC_MESSAGES
HELP_DIR =		help
HELP_SYML_INST_DIR =	$(LIB_DESTDIR)
HELP_SYML_PTS_TO_DIR =	./locale/C
LANG_DIR = $(LIB_DESTDIR)/local/C

#i18n_install: $(CAT_DESTDIR) .WAIT msg_catalogs help_symlink
i18n_install: 
i18n_gui_install: $(CAT_DESTDIR) .WAIT gui_msg_catalogs help_symlink

$(CAT_DESTDIR):
	$(INSTALL) -d $@

#
# Find the message catalog in the $VAriant dir
# install the files into the package's destination.
#

msg_catalogs: FRC
	cp $(APPPATH)/$(TEXTDOMAIN).msg $(CAT_DESTDIR)

gui_msg_catalogs: FRC
	cp $(APPPATH)/$(TEXTDOMAIN).po $(CAT_DESTDIR)

help_symlink: FRC
#	@ if [! -d $(LANG_DIR)/$(HELP_DIR) ]; then \
#		cd $(LANG_DIR); ln -s LC_MESSAGES $(HELP_DIR) \
#	fi \

FRC:
