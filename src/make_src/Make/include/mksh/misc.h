#ifndef _MKSH_MISC_H
#define _MKSH_MISC_H
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
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms.
 */

#include <mksh/defs.h>
#include <mksh/cdefs.h>

/*
 * New C versions:
 */
name_set_t *nms_alloc(void);
name_t *nms_lookup(name_set_t *, const wchar_t *);
name_t *nms_insert(name_set_t *, const wchar_t *, boolean_t *);
void nms_insert_name(name_set_t *, name_t *);
name_t *nms_next(name_set_t *, void **);
void append_char(wchar_t, string_t *);
void append_string(char *, string_t *, size_t);
void append_string_wide(wchar_t *, string_t *, size_t);
size_t property_body_size(property_id_t);
wstring_t *wstr_alloc(name_t *);
property_t *append_prop(name_t *, property_id_t);
name_t *getname_fn(wchar_t *, int, boolean_t, boolean_t *);
property_t *get_prop(property_t, property_id_t);
property_t *maybe_append_prop(name_t *, property_id_t);
void store_name(name_t *name);
void free_name(name_t *name);

void *xmalloc(size_t);
char *xstrdup(const char *);
wchar_t *xwcsdup(const wchar_t *);

/*
 * Old C++ versions:
 */
extern void	enable_interrupt(void (*handler) (int));
extern char	*errmsg(int errnum);
extern void	fatal_mksh(char * message, ...);
extern void	fatal_reader_mksh(char * pattern, ...);
extern char	*get_current_path_mksh(void);
extern void	handle_interrupt_mksh(int);
extern void	setup_char_semantics(void);
extern void	setup_interrupt(void (*handler) (int));
extern void	warning_mksh(char * message, ...);

extern wchar_t	*get_wstring(char * from);


#endif	/* _MKSH_MISC_H */
