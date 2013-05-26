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
# @(#)sparc-S2.mk 1.13 06/12/12
#

include $(TOP)/rules/solaris.mk

# Please keep each list alphabetically sorted.

#
# Make variables
#

#
# Paths
#
TOOLS_PATH =	$(TOP)/exe

#
# Programs
#

#
# Flags
#
CPPFLAGS +=	-DSUN5_0 -DSYSV

#
# v9 Flags
#
sparcv9 := V9FLAGS = -xarch=v9
sparcv9 := VAR_DIR = /sparcv9

#
# This is temporary until both 32- and 64-bit stuff are to be compiled
# with the same compiler. Currently 32-bit should use 4.2 compiler
# but 64-bit should use 5.0 compiler.
#
sparcv9 := CC = $(CC64)
sparcv9 := CCC = $(CCC64)
