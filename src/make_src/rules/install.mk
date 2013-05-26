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
# Copyright 1998 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms.
#
# @(#)install.mk 1.53 06/12/12
#

#
# This is an included Makefile that contains the standard
# install lines for a program.
#
# INCLUDE THIS ONLY AFTER INCLUDING $(TOP)/rules/master.mk
#
# Before including this Makefile the following variables should be set:
#	PROG		the name of the program
#	PROG_DIR	bin || etc,  lowest level directory for executables
#	LIB_DIR	usually just lib; destination for libraries,
#			non-localized bitmaps, etc.
#	LICENSE_DIR_SRC where LICENSE_DIR will link to
#	DESTDIR		destination directory for the install
#	PACKAGE		name of the package containing the program
#
#	  PACKAGE is now being overridden by PRODVER for installation
#		purposes since multiple packages are being installed
#		into a single install path below PRODVER  2/96  --emk
#
#	HELP_INFO	optionally set to a list of .info files to install
#			HELP_INFO is initially set in gui.Makefile
#	RES_FILE		optionally set name by which resource file is
#		known to the app itself (e.g. XCodeManager).
#	CDE_RES_FILE	optionally set name of resource file for CDE guis
#		(e.g. XCodeManager.cde).
#	NCDE_RES_FILE	optionally set name of resource file for motif guis
#	PWI_FILE	optionally set name of PWI configuration file for guis
#			Installed in RES_DIR
#
# This install technique uses make's dependencies to decide whether
# or not to install something.  If the installed file is more recent
# than the one in the source heirarchy, then no install will occur.
#
# The INST_PROG definition makes this install rule work for makefiles
# that build more than one program, e.g. several scripts that share
# the same source directory.  To work, this install.Makefile must be
# included after the "all" target has been defined but before the
# rules used to build the programs to be installed.  (see trans/scripts)
#
# For makefiles that build only one program, this install.Makefile can
# be included at the end, after the rules used to build the program.
# (see bin.cc.Makefile).

#
# The references to PROG_DIR below must be $$ references because
# PROG_DIR is conditionally set in the CodeManager/etc/diff3_merge
# Makefile.  But,  if it is conditional,  the implicit target for
# $(DESTDIR)/$(PROG_DIR) doesn't work.  Make returns that it doesn't
# know how to build that target.
#
# I made changes to add PROG_DIR=<value> in all the makefiles which include
# install.mk.  This is needed because of the assignment to
# PGK_DIR_PROG_DIR.  SYML_DIR is still conditional and set according to
# to which target is being built.  it would be nice to set PROG_DIR by
# default,  but to do this,  the reference in the PKG_DIR_PROG_DIR assignment
# must be re-done... LK
#

PKG_DIR =	$(DESTDIR)/$(PRODVER)
LOCALE_PATH =	lib/locale/C
HELP_DIR =	$(PKG_DIR)/$(LOCALE_PATH)/LC_MESSAGES
PWI_PATH =	$(LOCALE_PATH)/pwi_config
PWI_DIR =	$(PKG_DIR)/$(PWI_PATH)
LICENSE_DIR =	$(DESTDIR)/license_dir
PKG_DIR_PROG_DIR =	$(DESTDIR)/$(PRODVER)/$(PROG_DIR)
PKG_DIR_LIB_DIR =	$(DESTDIR)/$(PRODVER)/$(LIB_DIR)
CDE_RES_DIR =	$(DESTDIR)/$(PRODVER)/$(LOCALE_PATH)/app-defaults/CDE
NCDE_RES_DIR =	$(DESTDIR)/$(PRODVER)/$(LOCALE_PATH)/app-defaults/non-CDE
INST_PROG =	$(PROG:%=$(PKG_DIR_PROG_DIR)/%)
INST_LIB  =     $(LIBSS:%=$(PKG_DIR_LIB_DIR)/%)
#
# INST_HELP is the list of .info files to be installed
#
INST_HELP =	$(HELP_INFO:%=$(HELP_DIR)/%)

# 
INST_SYMLINK   	=	/bin/csh $(TOOLS_PATH)/inst_symlink 

# INSTALLING PERSISTENT PROCESSES:
#
# If the macro INST_PERSIST is set to TRUE,  move a copy of $(PROG) out of
# the way into a unique name so it doesn't clobber the last version moved.
#
# A way to make this conditional in the individual Makefiles is to include
# something like this in the Makefile which needs to install a persistent
# process:
#
# INST_PERSIST = $(SWTEAM_PERSIST)
#
# Where SWTEAM_PERSIST is set in your .cshrc or .login to TRUE. This allows
# it to be turned off and on on a global scale, and still only apply to those
# executables which have INST_PERSIST set in their Makefiles.
#

#install::	\
#	$(PKG_DIR_PROG_DIR) \
#	.WAIT \
#	$(PKG_DIR_LIB_DIR) \
#	.WAIT \
#	$(HELP_DIR) \
#	.WAIT \
#	$(PWI_DIR) \
#	.WAIT \
#	$(PWI_DIR)/$(PWI_FILE) \
#	.WAIT \
#	symlinks \
#	$(LICENSE_DIR) \
#	$(CDE_RES_DIR) \
#	$(NCDE_RES_DIR) \
#	.WAIT \
#	$(INST_PROG) \
#	$(INST_LIB) \
#	$(INST_HELP) \
#	$$(INST_ABOX_ABOX) \
#	$$(INST_ABOX_ADDR) \
#	$(CDE_RES_DIR)/$(RES_FILE) \
#	$(NCDE_RES_DIR)/$(RES_FILE) \
#	.WAIT \
#	post-install

#
# Empty rule here; packages can add their own functionality.
#
post-install:: FRC

#
# Number of earlier versions to save.
#
SAVE_NUM =		1

$(PKG_DIR_PROG_DIR)/%: %
	@if [ "$(INST_PERSIST)" = "TRUE" -a -f $@ ]; then \
		while [ `ls $@.* | wc -l` -gt $(SAVE_NUM) ]; do \
		echo "rm -f `ls $@.* | head -1`";\
		rm -f `ls $@.* | head -1`;\
	done; \
	echo "mv -f $@ $@.`date +%m.%d.%H.%M`"; \
		mv -f $@ $@.`date +%m.%d.%H.%M`; \
	fi
	echo "$(INSTALL) $< $@"
	$(INSTALL) $< $@


#
# Don't put the dependency upon $(PKG_DIR_PROG_DIR) on the 
# $(PKG_DIR_PROG_DIR)/% target line because it causes the executables to be
# installed twice regardless of their relation to the current executable 
#
$(PKG_DIR_PROG_DIR):
	@if [ ! -d $@ ]; then \
		$(INSTALL) -d $@; \
	fi; \

$(PKG_DIR_LIB_DIR):
	@if [ ! -d $@ ]; then \
		$(INSTALL) -d $@; \
	fi; \

#
# Install the help files listed in $(INST_HELP).  Each of the files
# listed in $(INST_HELP) lives in directory $(HELP_DIR).
#
# Needs to be able to support HELP_INFO with values in current directory
# as well as info files from other directories.  
#
$(HELP_DIR)/%.info: %.info 
	$(INSTALL) $< $@

$(HELP_DIR)/%.info:  $(TOP)/libgui/%.info
	$(INSTALL) $< $(HELP_DIR)/$(@F)

$(HELP_DIR):
	@if [ ! -d $(HELP_DIR)  ]; then \
		$(INSTALL) -d $@; \
	fi

$(PWI_DIR):
	@if [ ! -d $(PWI_DIR)  ]; then \
		$(INSTALL) -d $@; \
	fi

#
#	hack in license_dir link -- probably outta be in symlinks section
#
$(LICENSE_DIR):
		echo "$(INST_SYMLINK) $(LICENSE_DIR_SRC) $(DESTDIR) license_dir"
		      $(INST_SYMLINK) $(LICENSE_DIR_SRC) $(DESTDIR) license_dir

$(PWI_DIR)/$(PWI_FILE): $(SRC)/$(PWI_FILE) 
	@if [ "$(PWI_FILE)" != "" ]; then \
		$(INSTALL) -m 0444 $(SRC)/$(PWI_FILE) $@; \
	fi

$(CDE_RES_DIR)/$(RES_FILE): $(CDE_RES_FILE) 
	@if [ "$(CDE_RES_FILE)" != "" ]; then \
		$(INSTALL) -m0444 $(CDE_RES_FILE) $(CDE_RES_DIR)/$(RES_FILE); \
	fi

$(NCDE_RES_DIR)/$(RES_FILE): $(NCDE_RES_FILE) 
	@if [ "$(NCDE_RES_FILE)" != "" ]; then \
		$(INSTALL) -m0444 $(NCDE_RES_FILE) $(NCDE_RES_DIR)/$(RES_FILE); \
	fi

$(CDE_RES_DIR):
	@if [ "$(CDE_RES_FILE)" != "" ]; then \
		$(INSTALL) -d $@; \
	fi

$(NCDE_RES_DIR):
	@if [ "$(NCDE_RES_FILE)" != "" ]; then \
		$(INSTALL) -d $@; \
	fi

FRC:
