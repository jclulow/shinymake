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
# @(#)solaris.mk 1.19 06/12/12
#

# Please keep each list alphabetically sorted.

#
# Make variables
#
OPENWINHOME =	/usr/openwin

#
# Paths
#
#LD_LIBRARY_PATH = /usr/dt/lib:$(OPENWINHOME)/lib
#
SWHOME =

#
# Programs
#
#CC =		cc
INSTALL =	/opt/local/bin/install
RPCGEN =	/usr/5bin/rpcgen

#
# Flags
#
I18LIB =	-lintl
LEXOPT =	-tw
MOTIF_LD_RUN_PATH =	/usr/dt/lib:/usr/openwin/lib
