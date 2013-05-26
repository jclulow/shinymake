#!/bin/sh
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
# @(#)variant.sh 1.3 06/12/12
#



# Author: Achut Reddy
# Date: Fri Nov  4 12:00:24 PST 1994

# Handle HP-UX separately
sys=`uname -s`
case $sys in
HP-UX)
    echo hp-ux
    exit 0;
    ;;
esac

mach=`(mach || uname -p) 2> /dev/null`
case $mach in
i?86)
    mach=intel ;;
esac

rel=`uname -r`
case $rel in
5*)	os=S2;;
4*)	os=S1;;
*)	os=UNKNOWN;;
esac

echo ${mach}-${os}
