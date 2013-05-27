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
# @(#)intel-S2.mk 1.16 06/12/12
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
CPPFLAGS +=	-DSUN5_0 -DSYSV -Dx86
CFLAGS +=	-fPIC

#
# amd64-S2 Flags
#
amd64-S2 := V9FLAGS = -m64 -msave-args
amd64-S2 := VAR_DIR = /amd64
