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
# Copyright 1996 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms.
#
# @(#)Variant.mk 1.28 06/12/12
#

TOP =		../../../..
include $(TOP)/rules/master.mk
include $(TOP)/rules/dmake.mk

PKG_TOP =	$(TOP)/Make
CPPFLAGS +=	-I$(PKG_TOP)/include

CSRCS =	\
		access.c \
		args.c \
		chdir.c \
		chmod.c \
		chown.c \
		chroot.c \
		creat.c \
		execve.c \
		lock.c \
		lstat.c \
		mkdir.c \
		mount.c \
		open.c \
		readlink.c \
		report.c \
		rmdir.c \
		stat.c \
		statfs.c \
		truncate.c \
		unlink.c \
		unmount.c \
		utimes.c \
		vroot.c \
		setenv.c


HDRS_DIR =	$(PKG_TOP)/include/vroot

.INIT: $(HDRS_DIR)/args.h $(HDRS_DIR)/report.h $(HDRS_DIR)/vroot.h

LIBNAME =	libvroot.a
MSG_FILE = libvroot.msg
I18N_DIRS = $(SRC)

include $(TOP)/Make/lib/Lib.mk
include $(TOP)/rules/lib.mk

