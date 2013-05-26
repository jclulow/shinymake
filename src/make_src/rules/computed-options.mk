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
# @(#)computed-options.mk 1.3 06/12/12
#

#
# Including this file causes re-computation of the options used during a build
#

$(TOP)/rules/options-$(VARIANT).mk: $(TOP)/exe/options.sh FORCE
	/bin/rm -f $(TOP)/rules/options-$(VARIANT).mk
	/bin/sh $(TOP)/exe/options.sh $(TOP) $(VARIANT) > $(TOP)/rules/options-$(VARIANT).mk

FORCE:

include $(TOP)/rules/options-$(VARIANT).mk
