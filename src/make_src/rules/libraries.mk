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
# @(#)libraries.mk 1.9 06/12/12
#

# This file defines make variables for libraries libraries which
# are provided in both debuggable (-g) and optimized (-O) forms.
#
# See $(TOP)/exe/build-debuglib, rules/{singleton, motif-app}.mk,

# Please keep both of the following lists alphabetized.

# These libraries exist in both -g and non-g forms.
#
LIBDM =		-ldm$(DEBUGLIB)
LIBDMCONF =	-ldmconf$(DEBUGLIB)
LIBDMRC =	-ldmrc$(DEBUGLIB)
LIBDMTHREAD =	-ldmthread$(DEBUGLIB)
LIBLICMOTIF =	-llicmotif$(DEBUGLIB)
LIBLICTXT =	-llictxt$(DEBUGLIB)
LIBMGUI =	-lmgui$(DEBUGLIB)
LIBPSDEFS =	-lpsdefs$(DEBUGLIB)
LIBPSWIDGETS =	-lPsWidgets$(DEBUGLIB)
LIBPSUTILS =	-lPsUtils$(DEBUGLIB)
LIBPUBDMSI18N =	-lpubdmsi18n$(DEBUGLIB)
LIBPWICLIENT =	-lpwiclient$(DEBUGLIB)
LIBPWITOOLBOX =	-lpwitoolbox$(DEBUGLIB)
LIBRX =		-lrx$(DEBUGLIB)
LIBUTIL =	-lutil$(DEBUGLIB)
LIBXDCLASS =	-lxdclass$(DEBUGLIB)

# These libraries exist only in non-g form.  That does *not* mean that they are
# not *compiled* without -g., only that they don't exist in a libXXX-g.a form.
#
LIBLOG =		-llog
LIBMSSPKG =	-lmsspkg
LIBPATHUTILS =	-lpathutils
LIBPSCCHART =	-lpscchart
LIBPSCHART =	-lpschart
LIBTIMELINE =	-ltimeline
LIBXPM =	-lXpm
