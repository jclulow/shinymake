#ifndef _MKSH_MACRO_H
#define _MKSH_MACRO_H
/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms.
 */

#include <mksh/defs.h>

boolean_t is_magic_macro_name(magic_macro_id_t, name_t *);
name_t *get_magic_macro(magic_macro_id_t);

void expand_macro(source_t *source, string_t *destination,
    wchar_t *current_string, boolean_t cmd);
void expand_value(name_t *value, string_t *destination, boolean_t cmd);
name_t *getvar(name_t *name);

macro_t *setvar_daemon(name_t *name, void *, boolean_t, daemon_t, boolean_t,
    short);

boolean_t is_vpath_defined(void);

#endif	/* _MKSH_MACRO_H */
