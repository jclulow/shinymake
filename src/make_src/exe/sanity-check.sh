#! /bin/sh
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
# @(#)sanity-check.sh 1.6 06/12/12
#

# Run a sanity check on $1.  Sanity is loosely defined as "ok to ship to a customer".
# Currently checks for the following:
#	no forbidden RPATH
#	no forbidden shared libraries
# Since sanity is only important for those things we ship to customers, don't
# check it on debugging builds.

top=$1
variant=`/bin/sh $top/exe/variant.sh`
if [ -f $top/debug-$variant.opt ]; then
  exit 0
fi

prog=$2
status=0

/usr/ccs/bin/dump -Lv $prog \
 | /bin/grep RPATH | \
 /bin/egrep 'net|export|set' > /dev/null
if [ $? -eq 0 ]; then
 echo Error: $prog has forbidden RPATH
 /usr/ccs/bin/dump -Lv $prog | /bin/grep RPATH
 status=1
fi

/bin/ldd $prog |/bin/egrep 'libC|libsunmath|net|export|set' > /dev/null
if [ $? -eq 0 ]; then
 echo Error: $prog has forbidden libraries
 /bin/ldd $prog |/bin/egrep 'libC|libsunmath|net|export|set'
 status=2
fi

exit $status
