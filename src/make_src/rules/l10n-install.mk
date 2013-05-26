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
# @(#)l10n-install.mk 1.7 06/12/12
#

# READ-THIS-PART:
# Since L10N_SRCS are derived sources files, the make target
# "make_l10n_install" script MUST be executed after a 
# complete build of the integration workspace. The make_l10n_install
# scripts resides in the TOOLS_PATH directory.
#

L10N_SCRIPT =	/bin/csh $(TOOLS_PATH)/inst_l10n


# Before including this Makefile the following variables should be set:
# 
#
#	DESTDIR		the destination workspace where the L10N_SRCS will 
#			be installed.
#	SOURCE_WS	the source workspace from which the files will be
#			installed.
#	L10N_SRCS	sources that need to be installed in the above
#			DESTDIR.
#	L10N_SCRIPT	the script that will install the files under
#			sccs control in the destination directory
#			workspace.
#	TOOLS_PATH	location for the L10N_SCRIPT
#
#

ALL_USAGE = "Usage: make DESTDIR=<value> SOURCE_WS=<value> l10n_install"
CWD:sh = pwd

l10n_install:: l10n_install_usage FRC	
		@$(L10N_SCRIPT) $(DESTDIR) $(SOURCE_WS) $(CWD) \
		 $(L10N_SRCS)
FRC:

l10n_install_usage: FRC
	@if [ "$(DESTDIR)" = "" -o "$(SOURCE_WS)" = "" ]; then \
		echo $(ALL_USAGE); \
		exit 1; \
	fi;
	@if [ "$(CWD)" = "" ]; then \
		echo $(ALL_USAGE); \
		exit 1; \
	fi
FRC:
