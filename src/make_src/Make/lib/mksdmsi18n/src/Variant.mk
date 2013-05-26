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
# @(#)Variant.mk 1.4 06/12/12
#

TOP =		../../../..
include $(TOP)/rules/master.mk
#include $(TOP)/rules/dmake.mk

#SUBDIRS =	$(SRC)/lib
#include $(TOP)/rules/recurse.mk

CCSRCS = \
		libmksdmsi18n_init.cc 

CSRCS =

include $(SRC)/lib/version.mk

libmksdmsi18n_init.o := CPPFLAGS += -DI18N_VERSION=$(VERSION)

#PKG_TOP =	$(TOP)/Make
#CPPFLAGS +=	-I$(PKG_TOP)/include

LIBNAME =	libmksdmsi18n.a
MSG_FILE = libmksdmsi18n.msg
I18N_DIRS = $(SRC)

include $(TOP)/Make/lib/Lib.mk
include $(TOP)/rules/lib.mk

