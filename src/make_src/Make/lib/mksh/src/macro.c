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
 * Copyright 2006 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms.
 */

/*
 *	macro.c
 *
 *	Handle expansion of make macros
 */

#include <strings.h>
#include <mksh/dosys.h>
#include <mksh/i18n.h>
#include <mksh/macro.h>
#include <mksh/misc.h>
#include <mksh/read.h>
#include <mksdmsi18n/mksdmsi18n.h>


static void add_macro_to_global_list(name_t *);
static void expand_value_with_daemon(name_t *, macro_t *, string_t *,
    boolean_t);

static void _get_macro_value(name_t *, string_t *, replacement_t *,
    extraction_type_t, boolean_t);
static void _process_macro_name(string_t *, string_t *, boolean_t);
static void _extract_string(extraction_type_t, wchar_t *, string_t *);
static void _replace_string(replacement_t *, string_t *, string_t *);

static boolean_t init_arch_done = B_FALSE;
static boolean_t init_mach_done = B_FALSE;

static void init_arch_macros(void);
static void init_mach_macros(void);

static const wchar_t *macro_names[] = {
	L"HOST_ARCH",			/*  1 MMI_HOST_ARCH */
	L"HOST_MACH",			/*    MMI_HOST_MACH */
	L"TARGET_ARCH",			/*    MMI_TARGET_ARCH */
	L"TARGET_MACH",			/*    MMI_TARGET_MACH */
	L"SHELL",			/*  5 MMI_SHELL */
	L"PATH",			/*    MMI_PATH */
	L"VPATH",			/*    MMI_VPATH */
	L"VIRTUAL_ROOT",		/*    MMI_VIRTUAL_ROOT */
	L"MAKE",			/*    MMI_MAKE */
	L"?",				/* 10 MMI_QUERY */
	L"^",				/*    MMI_HAT */
	L".POSIX",			/*    MMI_POSIX */
	L".SVR4",			/*    MMI_SVR4 */
	L".BUILT_LAST_MAKE_RUN",	/*    MMI_BUILT_LAST_MAKE_RUN */
	L"@",				/* 15 MMI_AT */
	L".DEFAULT",			/*    MMI_DEFAULT_RULE */
	L"$",				/*    MMI_DOLLAR */
	L".DONE",			/*    MMI_DONE */
	L".",				/*    MMI_DOT */
	L".KEEP_STATE",			/* 20 MMI_KEEP_STATE */
	L".KEEP_STATE_FILE",		/*    MMI_KEEP_STATE_FILE */
	L"",				/*    MMI_EMPTY_NAME */
	L" FORCE",			/*    MMI_FORCE */
	L".IGNORE",			/*    MMI_IGNORE */
	L".INIT",			/* 25 MMI_INIT */
	L".LOCAL",			/*    MMI_LOCALHOST */
	L".make.state",			/*    MMI_MAKE_STATE */
	L"MAKEFLAGS",			/*    MMI_MAKEFLAGS */
	L".MAKE_VERSION",		/*    MMI_MAKE_VERSION */
	L".NO_PARALLEL",		/* 30 MMI_NO_PARALLEL */
	L".NOT_AUTO",			/*    MMI_NOT_AUTO */
	L".PARALLEL",			/*    MMI_PARALLEL */
	L"+",				/*    MMI_PLUS */
	L".PRECIOUS",			/*    MMI_PRECIOUS */
	L".RECURSIVE",			/* 35 MMI_RECURSIVE */
	L".SCCS_GET",			/*    MMI_SCCS_GET */
	L".SCCS_GET_POSIX",		/*    MMI_SCCS_GET_POSIX */
	L".GET",			/*    MMI_GET */
	L".GET_POSIX",			/*    MMI_GET_POSIX */
	L".SILENT",			/* 40 MMI_SILENT */
	L".SUFFIXES",			/*    MMI_SUFFIXES */
	L"SUNPRO_DEPENDENCIES",		/*    MMI_SUNPRO_DEPENDENCIES */
	L".WAIT",			/*    MMI_WAIT */
};

static name_t *
_internal_get_magic_macro(magic_macro_id_t id)
{
	if (id == MMI_UNKNOWN || id >= _MMI_MAX_ID) {
		fprintf(stderr, "PROGRAMMING ERROR\n");
		/*
		 * XXX
		 */
		abort();
	}

	if (magic_macros[id - 1] == NULL) {
		/*
		 * Internalise the name:
		 */
		magic_macros[id - 1] = GETNAME(macro_names[id - 1],
		    FIND_LENGTH);
	}

	return (magic_macros[id - 1]);
}

/*
 * Check if this is a particular magic macro, but don't trigger the
 * initialisation of said macro while doing so.
 */
boolean_t
is_magic_macro_name(magic_macro_id_t id, name_t *n)
{
	if (n == NULL || _internal_get_magic_macro(id) != n)
		return (B_FALSE);

	return (B_TRUE);
}

name_t *
get_magic_macro(magic_macro_id_t id)
{
	name_t *xx = _internal_get_magic_macro(id);

	/*
	 * Ensure we have done the appropriate initialisation:
	 */
	switch (id) {
	case MMI_HOST_ARCH:
	case MMI_TARGET_ARCH:
		init_arch_macros();
		break;
	case MMI_HOST_MACH:
	case MMI_TARGET_MACH:
		init_mach_macros();
		break;
	default:
		break;
	}

	/*
	 * Now, return the requisite thing:
	 */
	return (xx);
}


/*
 *	getvar(name)
 *
 *	Return expanded value of macro.
 *
 *	Return value:
 *				The expanded value of the macro
 *
 *	Parameters:
 *		name		The name of the macro we want the value for
 *
 *	Global variables used:
 */
name_t *
getvar(name_t *name)
{
	string_t dest;
	wchar_t buf[STRING_BUFFER_LENGTH];
	name_t *result;
	property_t *prop;
	macro_t *macro;

	INIT_STRING_FROM_STACK(dest, buf);

	prop = maybe_append_prop(name, PT_MACRO);
	if (prop->p_type != PT_MACRO)
		abort();
	macro = (macro_t *)prop->p_body;
	expand_value(macro->m_value, &dest, B_FALSE);

	result = GETNAME(dest.str_buf_start, FIND_LENGTH);
	if (dest.str_free_after_use)
		free(dest.str_buf_start);

	return (result);
}

/*
 *	expand_value(value, destination, cmd)
 *
 *	Recursively expands all macros in the string value.
 *	destination is where the expanded value should be appended.
 *
 *	Parameters:
 *		value		The value we are expanding
 *		destination	Where to deposit the expansion
 *		cmd		If we are evaluating a command line we
 *				turn \ quoting off
 *
 *	Global variables used:
 */
void
expand_value(name_t *value, string_t *destination, boolean_t cmd)
{
	source_t *src;
	wchar_t *block_start;
	boolean_t quote_seen = B_FALSE;

	if (value == NULL) {
		/*
		 * Make sure to get a string allocated even if it
		 * will be empty.
		 * XXX this seems like it might be a misunderstanding
		 * of the interface?  Surely we can guarantee that
		 * every string_t is allocated correctly...
		 */
		append_string_wide(L"", destination, FIND_LENGTH);
		destination->str_end = destination->str_p;
		return;
	}
	if (value->n_dollar == B_FALSE) {
		/*
		 * If the value we are expanding does not contain
		 * any $, we don't have to parse it.
		 */
		append_string_name(value, destination);
		destination->str_end = destination->str_p;
		return;
	}

	if (value->n_being_expanded == B_TRUE) {
		fatal_reader_mksh(catgets(libmksdmsi18n_catd, 1, 113,
		    "Loop detected when expanding macro value `%ls'"),
		    value->n_key);
	}
	value->n_being_expanded = B_TRUE;

	src = source_from_wchar(value->n_key);

	/*
	 * We parse the string in segments.
	 * We read characters until we find a $, then we append what we have
	 * read so far (since last $ processing) to the destination.  When
	 * we find a '$', we call expand_macro() and let it expand that
	 * particular $ reference into destination.
	 */
	quote_seen = B_FALSE;
	block_start = src->src_str.str_p;
	for (;; (void) source_popchar(src)) {
		switch (source_peekchar(src)) {
		case L'\\':
			/* Quote $ in macro value */
			if (cmd == B_FALSE) {
				quote_seen = quote_seen == B_TRUE ? B_FALSE :
				    B_TRUE;
			}
			continue;
		case L'$':
			/* Save the plain string we found since */
			/* start of string or previous $ */
			if (quote_seen == B_TRUE) {
				append_string_wide(block_start, destination,
				    src->src_str.str_p - block_start - 1);
				block_start = src->src_str.str_p;
				break;
			}
			append_string_wide(block_start, destination,
			    src->src_str.str_p - block_start);
			/*
			 * Move past the $ for this macro, and expand
			 * the reference:
			 */
			(void) source_popchar(src);
			expand_macro(src, destination,
			    src->src_str.str_buf_start, cmd);
			block_start = src->src_str.str_p + 1;
			break;
		case L'\0':
			/* The string ran out. Get some more */
			append_string_wide(block_start, destination,
			    src->src_str.str_p - block_start);

			src = get_next_block_fn(src);
			if (src == NULL) {
				/*
				 * We're done.
				 */
				destination->str_end = destination->str_p;
				value->n_being_expanded = B_FALSE;
				return;
			}
			if (src->src_error_converting == B_TRUE) {
				fatal_reader_mksh(NOCATGETS("Internal error: "
				    "Invalid byte sequence in expand_value()"));
			}
			block_start = src->src_str.str_p;
			src->src_str.str_p--; /* because we're about to pop */
			continue;
		}
		quote_seen = B_FALSE;
	}
#if 0
out:
	if (src->src_string.str_free_after_use == B_TRUE)
		free(sourceb.src_string.str_buf_start);
#endif
}

static void
_parse_straight_subst(wchar_t *colon, replacement_t *rpl)
{
	wchar_t *eq = NULL;

	while (colon != NULL) {
		if ((eq = wcschr(colon + 1, L'=')) == NULL) {
			fatal_reader_mksh(catgets(libmksdmsi18n_catd, 1, 120,
			    "= missing from replacement macro reference"));
		}
		rpl->rpl_left_tail_len = eq - colon - 1;
		free(rpl->rpl_left_tail);
		rpl->rpl_left_tail = wchar_alloc(rpl->rpl_left_tail_len + 1);
		(void) wsncpy(rpl->rpl_left_tail, colon + 1, eq - colon - 1);
		rpl->rpl_left_tail[eq - colon - 1] = L'\0';

		rpl->rpl_type = RT_SUFFIX;

		if ((colon = wcschr(eq + 1, L':')) != NULL) {
			rpl->rpl_right_tail_len = colon - eq;

			free(rpl->rpl_right_tail);
			rpl->rpl_right_tail = wchar_alloc(
			    rpl->rpl_right_tail_len);
			(void) wcsncpy(rpl->rpl_right_tail, eq + 1,
			    colon - eq - 1);
			rpl->rpl_right_tail[colon - eq - 1] = L'\0';
		} else {
			free(rpl->rpl_right_tail);
			rpl->rpl_right_tail = wchar_alloc(wcslen(eq) + 1);
			(void) wcscpy(rpl->rpl_right_tail, eq + 1);
		}
	}
}

static void
_parse_percent_subst(wchar_t *colon, replacement_t *rpl)
{
	wchar_t *eq = NULL, *percent = NULL;

	if ((eq = (wchar_t *) wcschr(colon + 1, L'=')) == NULL) {
		fatal_reader_mksh(catgets(libmksdmsi18n_catd, 1, 121,
		    "= missing from replacement macro reference"));
	}

	if ((percent = (wchar_t *) wcschr(colon + 1, L'%')) == NULL) {
		fatal_reader_mksh(catgets(libmksdmsi18n_catd, 1, 122,
		    "%% missing from replacement macro reference"));
	}

	if (eq < percent) {
		fatal_reader_mksh(catgets(libmksdmsi18n_catd, 1, 123,
		    "%% missing from replacement macro reference"));
	}

	free(rpl->rpl_left_head);
	if (percent > (colon + 1)) {
		int tmp_len = percent - colon;

		rpl->rpl_left_head = wchar_alloc(tmp_len);
		(void) wcsncpy(rpl->rpl_left_head, colon + 1,
		    percent - colon - 1);
		rpl->rpl_left_head[percent - colon - 1] = L'\0';
		rpl->rpl_left_head_len = percent - colon - 1;
	} else {
		rpl->rpl_left_head = NULL;
		rpl->rpl_left_head_len = 0;
	}

	free(rpl->rpl_left_tail);
	if (eq > percent + 1) {
		int tmp_len = eq - percent;

		rpl->rpl_left_tail = wchar_alloc(tmp_len);
		(void) wcsncpy(rpl->rpl_left_tail, percent + 1, eq - percent - 1);
		rpl->rpl_left_tail[eq - percent - 1] = L'\0';
		rpl->rpl_left_tail_len = eq - percent - 1;
	} else {
		rpl->rpl_left_tail = NULL;
		rpl->rpl_left_tail_len = 0;
	}

	eq++;
	if ((percent = wcschr(eq, L'%')) == NULL) {
		rpl->rpl_right_hand[0] = wchar_alloc(wcslen(eq) + 1);
		rpl->rpl_right_hand[1] = NULL;
		(void) wcscpy(rpl->rpl_right_hand[0], eq);
	} else {
		int i = 0;
		do {
			rpl->rpl_right_hand[i] = wchar_alloc(percent - eq + 1);
			(void) wsncpy(rpl->rpl_right_hand[i], eq, percent - eq);
			rpl->rpl_right_hand[i][percent - eq] = L'\0';

			if (i++ >= REPLACEMENT_RIGHT_HAND_SIZE) {
				fatal_mksh(catgets(libmksdmsi18n_catd, 1, 124,
				    "Too many %% in pattern"));
			}

			eq = percent + 1;
			if (eq[0] == L'\0') {
				rpl->rpl_right_hand[i] = xwcsdup(L"");
				i++;
				break;
			}
		} while ((percent = wcschr(eq, L'%')) != NULL);
		if (eq[0] != (int) nul_char) {
			rpl->rpl_right_hand[i] = xwcsdup(eq);
			i++;
		}
		rpl->rpl_right_hand[i] = NULL;
	}
	rpl->rpl_type = RT_PATTERN;
}


/*
 *	expand_macro(source, destination, current_string, cmd)
 *
 *	Should be called with src->src_str.str_p pointing to
 *	the first char after the $ that starts a macro reference.
 *	source->string.text.p is returned pointing to the first char after
 *	the macro name.
 *	It will read the macro name, expanding any macros in it,
 *	and get the value. The value is then expanded.
 *	destination is a String that is filled in with the expanded macro.
 *	It may be passed in referencing a buffer to expand the macro into.
 * 	Note that most expansions are done on demand, e.g. right
 *	before the command is executed and not while the file is
 * 	being parsed.
 *
 *	Parameters:
 *		source		The source block that references the string
 *				to expand
 *		destination	Where to put the result
 *		current_string	The string we are expanding, for error msg
 *		cmd		If we are evaluating a command line we
 *				turn \ quoting off
 *
 *	Global variables used:
 *		funny		Vector of semantic tags for characters
 *		is_conditional	Set if a conditional macro is refd
 *		make_word_mentioned Set if the word "MAKE" is mentioned
 *		makefile_type	We deliver extra msg when reading makefiles
 *		query		The Name "?", compared against
 *		query_mentioned	Set if the word "?" is mentioned
 */

void
expand_macro(source_t *src, string_t *destination, wchar_t *current_string,
    boolean_t cmd)
{
	string_t str;
	wchar_t buf[STRING_BUFFER_LENGTH];
	wchar_t closer = L'\0';
	unsigned int closer_level = 0;
	wchar_t *block_start;
	boolean_t quote_seen;

	/*
	 * First, we copy the (macro-expanded) macro name into string.
	 */
	INIT_STRING_FROM_STACK(str, buf);

recheck_first_char:
	/*
	 * Check the first char of the macro name to figure out what to do.
	 */
	switch (source_popchar(src)) {
	case L'\0':
		src = get_next_block_fn(src);
		if (src == NULL) {
			fatal_reader_mksh(catgets(libmksdmsi18n_catd, 1, 114,
			    "'$' at end of string `%ls'"), current_string);
		}
		if (src->src_error_converting == B_TRUE) {
			fatal_reader_mksh(NOCATGETS("Internal error: Invalid"
			    " byte sequence in expand_macro()"));
		}
		goto recheck_first_char;
	case L'(':
		/*
		 * Multi-character name.
		 */
		closer = L')';
		break;
	case L'{':
		/*
		 * Multi-character name.
		 */
		closer = L'}';
		break;
	case L'\n':
		fatal_reader_mksh(catgets(libmksdmsi18n_catd, 1, 115,
		    "'$' at end of line"));
		break;
	default:
		/*
		 * Single-character macro name.  Just suck it up.
		 */
		append_char(*src->src_str.str_p, &str);
		goto next_step;
	}

	/*
	 * Handle multi-character macro names:
	 */
	block_start = src->src_str.str_p;
	quote_seen = B_FALSE;
	for (;; (void) source_popchar(src)) {
		wchar_t c;
		switch (c = source_peekchar(src)) {
		case L'\0':
			append_string_wide(block_start, &str,
			    src->src_str.str_p - block_start);
			src = get_next_block_fn(src);
			if (src == NULL) {
				if (current_string != NULL) {
					fatal_reader_mksh(catgets(
					    libmksdmsi18n_catd, 1, 116,
					    "Unmatched `%lc' in string `%ls'"),
					    closer == L'}' ? L'{' : L'(',
					    current_string);
				} else {
					fatal_reader_mksh(catgets(
					    libmksdmsi18n_catd, 1, 117,
					    "Premature EOF"));
				}
			}
			if (src->src_error_converting == B_TRUE) {
				fatal_reader_mksh(NOCATGETS("Internal error: "
				    "Invalid byte sequence in expand_macro()"));
			}
			block_start = src->src_str.str_p;
			src->src_str.str_p--; /* because we're about to pop */
			continue;
		case L'\n':
			fatal_reader_mksh(catgets(libmksdmsi18n_catd, 1, 118,
			    "Unmatched `%lc' on line"),
			    closer == L'}' ? L'{' : L'(');
			break;
		case L'\\':
			/*
			 * Quote '$' in macro value:
			 */
			if (cmd == B_FALSE) {
				quote_seen = quote_seen == B_TRUE ? B_FALSE :
				    B_TRUE;
			}
			continue;
		case L'$':
			/*
			 * Macro names may reference macros.
			 * This expands the value of such macros into the
			 * macro name string.
			 */
			if (quote_seen == B_TRUE) {
				append_string_wide(block_start, &str,
				    src->src_str.str_p - block_start - 1);
				block_start = src->src_str.str_p;
				break;
			}
			append_string_wide(block_start, &str,
			    src->src_str.str_p - block_start);
			/*
			 * Move past the $ for this macro, and expand
			 * the reference:
			 */
			(void) source_popchar(src);
			expand_macro(src, &str, current_string, cmd);
			block_start = src->src_str.str_p;
			src->src_str.str_p--; /* because about to pop */
			break;
		case L'(':
			/*
			 * Allow nested pairs of () in the macro name.
			 */
			if (closer == L')')
				closer_level++;
			break;
		case L'{':
			/*
			 * Allow nested pairs of {} in the macro name.
			 */
			if (closer == L'}')
				closer_level++;
			break;
		case L')':
		case L'}':
			/*
			 * End of the name.  Save the string in the macro
			 * name string.
			 */
			if (c == closer && --closer_level == 0) {
				append_string_wide(block_start, &str,
				    src->src_str.str_p - block_start);
				/*
				 * Discard the closer character, and get out:
				 */
				(void) source_popchar(src);
				goto next_step;
			}
			break;
		}
		quote_seen = B_FALSE;
	}

next_step:
	/*
	 * We have the macro name.  Inspect it to see if it specifies
	 * any translations of the value.
	 */
	_process_macro_name(&str, destination, cmd);

	if (str.str_free_after_use == B_TRUE)
		free(str.str_buf_start);
}

static void
_process_macro_name(string_t *str, string_t *dest, boolean_t cmd)
{
	const wchar_t *colon_sh = L":sh";
	const wchar_t *colon_shell = L":shell";
	extraction_type_t extraction = EXT_NO;
	name_t *name = NULL;
	wchar_t *colon, *percent;
	replacement_t rpl;
	int i;

	bzero(&rpl, sizeof (rpl));

	/*
	 * First, check if we have a $(@D) type translation.
	 */
	if ((get_char_semantics_value(str->str_buf_start[0]) &
	    CHC_SPECIAL_MACRO) != 0) {
		switch (str->str_buf_start[1]) {
		case L'D':
			extraction = EXT_DIR;
			break;
		case L'F':
			extraction = EXT_FILE;
			break;
		default:
			fatal_reader_mksh(catgets(libmksdmsi18n_catd, 1, 119,
			    "Illegal macro reference `%ls'"),
			    str->str_buf_start);
		}
		/*
		 * Internalise the macro name using the first character
		 * only:
		 */
		name = GETNAME(str->str_buf_start, 1);
		/*
		 * XXX this seems slightly dodge:
		 */
		(void) wcscpy(str->str_buf_start, str->str_buf_start + 2);
	}

	/*
	 * Check for other kinds of translations.
	 */
	if ((colon = wcschr(str->str_buf_start, L':')) != NULL) {
		/*
		 * We have a $(FOO:.c=.o) type translation.
		 * Get the name of the macro proper.
		 */
		if (name == NULL) {
			name = GETNAME(str->str_buf_start,
			    colon - str->str_buf_start);
		}
		/*
		 * Pickup all the translations.
		 */
		if (wcscmp(colon, colon_sh) == 0 || wcscmp(colon,
		    colon_shell) == 0) {
			rpl.rpl_type = RT_SHELL;
		} else if (svr4 == B_TRUE || (percent = wcschr(colon + 1,
		    L'%')) == NULL) {
			/*
			 * Straight substition -- .c=.o
			 */
			_parse_straight_subst(colon, &rpl);
		} else {
			/*
			 * Percent-style substition -- op%os=np%ns
			 */
			_parse_percent_subst(colon, &rpl);
		}
	}

	if (name == NULL) {
		/*
		 * No translations found.
		 * Use the whole string as the macro name.
		 */
		name = GETNAME(str->str_buf_start, str->str_p -
		    str->str_buf_start);
	}

	/*
	 * Take note of any special macro considerations:
	 */
	if (is_magic_macro_name(MMI_MAKE, name) == B_TRUE) {
		make_word_mentioned = B_TRUE;
	}
	if (is_magic_macro_name(MMI_QUERY, name) == B_TRUE) {
		query_mentioned = B_TRUE;
	}
	if (is_magic_macro_name(MMI_HOST_ARCH, name) == B_TRUE ||
	    is_magic_macro_name(MMI_TARGET_ARCH, name) == B_TRUE) {
		init_arch_macros();
	}
	if (is_magic_macro_name(MMI_HOST_MACH, name) == B_TRUE ||
	    is_magic_macro_name(MMI_TARGET_MACH, name) == B_TRUE) {
		init_mach_macros();
	}

	_get_macro_value(name, dest, &rpl, extraction, cmd);

	free(rpl.rpl_left_head);
	free(rpl.rpl_left_tail);
	free(rpl.rpl_right_head);
	free(rpl.rpl_right_tail);
	for (i = 0; rpl.rpl_right_hand[i] != NULL; i++)
		free(rpl.rpl_right_hand[i]);
}

static void
_get_macro_value(name_t *name, string_t *dest, replacement_t *rpl,
    extraction_type_t ext, boolean_t cmd)
{
	property_t *prop;
	macro_t *macro;
	string_t str;
	wchar_t buf[STRING_BUFFER_LENGTH];

	/*
	 * Locate the macro property for this name:
	 */
	prop = get_prop(name->n_prop, PT_MACRO);
	if (prop == NULL) {
		/*
		 * The macro property does not exist at all, so skip out.
		 */
		goto exit;
	}
	if (prop->p_type != PT_MACRO)
		abort();
	macro = (macro_t *)prop->p_body;
	switch (macro->m_daemon) {
	case DN_NO_DAEMON:
		if (macro->m_value == NULL)
			goto exit;
		break;
	case DN_CHAIN_DAEMON:
		if (macro->m_chain == NULL)
			goto exit;
		break;
	default:
		abort();
	}

	if (macro->m_is_conditional == B_TRUE) {
		conditional_macro_used = B_TRUE;
		/*
		 * Add this conditional macro to the beginning of the global
		 * list.
		 */
		add_macro_to_global_list(name);
		if (makefile_type == MFT_READING_MAKEFILE) {
			warning_mksh(catgets(libmksdmsi18n_catd, 1, 164,
			    "Conditional macro `%ls' referenced in file "
			    "`%ls', line %d"), name->n_key, file_being_read,
			    line_number);
		}
	}

	if (rpl->rpl_type == RT_SHELL) {
		/*
		 * This is a shell replacement, so expand the command and
		 * process it.
		 */
		INIT_STRING_FROM_STACK(str, buf);

		expand_value_with_daemon(name, macro, &str, cmd);
		sh_command2string(&str, dest);

	} else if (rpl->rpl_type != RT_NONE || ext != EXT_NO) {
		wchar_t *p;
		wchar_t *block_start;
		wchar_t save;

		/*
		 * There are replacements or extractions.  Process them.
		 */
		INIT_STRING_FROM_STACK(str, buf);

		expand_value_with_daemon(name, macro, &str, cmd);
		/*
		 * Scan the expanded string:
		 */
		p = str.str_buf_start;
		while (*p != L'\0') {
			string_t exstr;
			wchar_t exbuf[MAXPATHLEN];

			/*
			 * First, skip over any whitespace and append that
			 * to the destination string.
			 */
			block_start = p;
			while (*p != L'\0' && iswspace(*p))
				p++;
			append_string_wide(block_start, dest, p - block_start);
			/*
			 * Then, find the end of the next word:
			 */
			block_start = p;
			while (*p != L'\0' && !iswspace(*p))
				p++;
			/*
			 * If we do not find another word, we are done.
			 */
			if (block_start == p)
				break;
			/*
			 * Process the requested extraction:
			 */
			INIT_STRING_FROM_STACK(exstr, exbuf);

			save = *p;
			*p = L'\0';
			_extract_string(ext, block_start, &exstr);
			*p = save;

			/*
			 * Process the requested replacement:
			 */
			_replace_string(rpl, &exstr, dest);
			if (exstr.str_free_after_use == B_TRUE)
				free(exstr.str_buf_start);
		}
	} else {
		/*
		 * There are no replacements or extractions.
		 */
		if (wcscmp(name->n_key, L"GET") == 0) {
			dollarget_seen = B_TRUE;
		}
		dollarless_flag = B_FALSE;
		if (wcscmp(name->n_key, L"<") && dollarget_seen == B_TRUE) {
			dollarless_flag = B_TRUE;
			dollarget_seen = B_FALSE;
		}
		expand_value_with_daemon(name, macro, dest, cmd);
	}

exit:
	if (str.str_free_after_use == B_TRUE)
		free(str.str_buf_start);
}

static void
_extract_string(extraction_type_t ext, wchar_t *block_start, string_t *dest)
{
	wchar_t *eq;

	switch (ext) {
	case EXT_DIR:
		/*
		 * $(@D) type transform.  Extract the path from the word.
		 * Deliver "." if none is found.
		 */
		if ((eq = wcsrchr(block_start, L'/')) == NULL) {
			append_char(L'.', dest);
		} else {
			append_string_wide(block_start, dest,
			    eq - block_start);
		}
		break;
	case EXT_FILE:
		/*
		 * $(@F) type transform.  Remove the path from the word,
		 * if any.
		 */
		if ((eq = wcsrchr(block_start, L'/')) == NULL) {
			append_string_wide(block_start, dest, FIND_LENGTH);
		} else {
			append_string_wide(eq + 1, dest, FIND_LENGTH);
		}
		break;
	case EXT_NO:
		append_string_wide(block_start, dest, FIND_LENGTH);
		break;
	default:
		abort();
	}
}


static void
_replace_string(replacement_t *rpl, string_t *str, string_t *dest)
{
	switch (rpl->rpl_type) {
	case RT_SUFFIX:
		/*
		 * $(FOO:.o=.c) type transform.
		 * Possibly replace the tail of the word.
		 */
		if ((str->str_p - str->str_buf_start >=
		    rpl->rpl_left_tail_len) &&
		    wcsncmp(str->str_p - rpl->rpl_left_tail_len,
		    rpl->rpl_left_tail, rpl->rpl_left_tail_len) == 0) {
			/*
			 * Copy up to the tail:
			 */
			append_string_wide(str->str_buf_start, dest,
			    str->str_p - str->str_buf_start -
			    rpl->rpl_left_tail_len);
			/*
			 * Copy in the replacement tail:
			 */
			append_string_wide(rpl->rpl_right_tail, dest,
			    FIND_LENGTH);
		} else {
			/*
			 * Copy the string unaltered:
			 */
			append_string_wide(str->str_buf_start, dest,
			    FIND_LENGTH);
		}
		break;
	case RT_PATTERN:
		/*
		 * $(FOO:a%b=c%d) type transform.
		 */
		if ((str->str_p - str->str_buf_start >=
		    rpl->rpl_left_head_len + rpl->rpl_left_tail_len) &&
		    wcsncmp(rpl->rpl_left_head, str->str_buf_start,
		    rpl->rpl_left_head_len) == 0 &&
		    wcsncmp(rpl->rpl_left_tail, str->str_p -
		    rpl->rpl_left_tail_len, rpl->rpl_left_tail_len) == 0) {
			int i = 0;
			while (rpl->rpl_right_hand[i] != NULL) {
				append_string_wide(rpl->rpl_right_hand[i],
				    dest, FIND_LENGTH);
				i++;
				if (rpl->rpl_right_hand[i] != NULL) {
					int len = str->str_p -
					    str->str_buf_start -
					    rpl->rpl_left_head_len -
					    rpl->rpl_left_tail_len;
					append_string_wide(str->str_buf_start +
					    rpl->rpl_left_head_len, dest, len);
				}
			}
		} else {
			/*
			 * Copy the string unaltered:
			 */
			append_string_wide(str->str_buf_start, dest,
			    FIND_LENGTH);
		}
		break;
	case RT_NONE:
		append_string_wide(str->str_buf_start, dest, FIND_LENGTH);
		break;
	case RT_SHELL:
		/*
		 * XXX should we get here for this one?
		 */
		break;
	default:
		abort();
	}
	*dest->str_p = L'\0';
	dest->str_end = dest->str_p;
}

static void
add_macro_to_global_list(name_t *macro_to_add)
{
	macro_list_t *ml;
	wchar_t *name_on_list = NULL, *value_on_list = NULL;
	wchar_t *name_to_add = macro_to_add->n_key;
	wchar_t *value_to_add = NULL;
	property_t *prop;
	macro_t *macro = NULL;
       
	prop = get_prop(macro_to_add->n_prop, PT_MACRO);
	if (prop != NULL)
		macro = (macro_t *)prop->p_body;

	if (macro != NULL && macro->m_value != NULL) {
		value_to_add = macro->m_value->n_key;
	} else {
		value_to_add = L"";
	}

	/*
	 * Check if this macro is already on list; if so, do nothing.
	 */
	for (ml = cond_macro_list; ml != NULL; ml = ml->ml_next) {
		name_on_list = ml->ml_macro_name;
		value_on_list = ml->ml_value;

		if (wcscmp(name_on_list, name_to_add) == 0 &&
		    wcscmp(value_on_list, value_to_add) == 0) {
			return;
		}
	}

	/*
	 * Otherwise, put it at the head of the list:
	 */
	ml = xmalloc(sizeof (*ml));
	ml->ml_macro_name = xwcsdup(name_to_add);
	ml->ml_value = xwcsdup(value_to_add);
	ml->ml_next = cond_macro_list;
	cond_macro_list = ml;
}

/*
 *	init_arch_macros(void)
 *
 *	Set the magic macros TARGET_ARCH, HOST_ARCH,
 *
 *	Parameters:
 *
 *	Global variables used:
 * 	                        host_arch   Property for magic macro HOST_ARCH
 * 	                        target_arch Property for magic macro TARGET_ARCH
 *
 *	Return value:
 *				The function does not return a value, but can
 *				call fatal() in case of error.
 */
static void
init_arch_macros(void)
{
	FILE *pipe;
	name_t *value;
	const char *arch_command = NOCATGETS("/bin/arch");
	string_t str;
	wchar_t buf[STRING_BUFFER_LENGTH];
	char fgetsbuf[100];
	boolean_t set_host = B_FALSE, set_target = B_FALSE;
	name_t *host_arch, *target_arch;

	/*
	 * Skip out if we've already done this:
	 */
	if (init_arch_done == B_TRUE)
		return;
	init_arch_done = B_TRUE;

	host_arch = _internal_get_magic_macro(MMI_HOST_ARCH);
	target_arch = _internal_get_magic_macro(MMI_TARGET_ARCH);

	if (get_prop(host_arch->n_prop, PT_MACRO) == NULL)
		set_host = B_TRUE;
	if (get_prop(target_arch->n_prop, PT_MACRO) == NULL)
		set_target = B_TRUE;

	/*
	 * If both have been set already, don't proceed any further:
	 */
	if (set_host == B_FALSE && set_target == B_FALSE)
		return;

	INIT_STRING_FROM_STACK(str, buf);
	append_char(L'-', &str);

	if ((pipe = popen(arch_command, "r")) == NULL) {
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 185,
		    "Execute of %s failed"), arch_command);
	}
	while (fgets(fgetsbuf, sizeof (fgetsbuf), pipe) != NULL)
		append_string(fgetsbuf, &str, FIND_LENGTH);
	if (pclose(pipe) != 0) {
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 186, "Execute of %s "
		    "failed"), arch_command);
	}

	value = GETNAME(str.str_buf_start, FIND_LENGTH);
	if (set_host == B_TRUE) {
		(void) setvar_daemon(host_arch, value, B_FALSE,
		    DN_NO_DAEMON, B_TRUE, 0);
	}
	if (set_target == B_TRUE) {
		(void) setvar_daemon(target_arch, value, B_FALSE,
		    DN_NO_DAEMON, B_TRUE, 0);
	}
}

/*
 *	init_mach_macros(void)
 *
 *	Set the magic macros TARGET_MACH, HOST_MACH,
 *
 *	Parameters:
 *
 *	Global variables used:
 * 	                        host_mach   Property for magic macro HOST_MACH
 * 	                        target_mach Property for magic macro TARGET_MACH
 *
 *	Return value:
 *				The function does not return a value, but can
 *				call fatal() in case of error.
 */
static void
init_mach_macros(void)
{
	FILE *pipe;
	name_t *value;
	const char *mach_command = NOCATGETS("/bin/mach");
	string_t str;
	wchar_t buf[STRING_BUFFER_LENGTH];
	char fgetsbuf[100];
	boolean_t set_host = B_FALSE, set_target = B_FALSE;
	name_t *host_mach, *target_mach;

	/*
	 * Skip out if we've already done this:
	 */
	if (init_mach_done == B_TRUE)
		return;
	init_mach_done = B_TRUE;

	host_mach = _internal_get_magic_macro(MMI_HOST_MACH);
	target_mach = _internal_get_magic_macro(MMI_TARGET_MACH);

	/*
	 * If both have been set already, don't proceed any further:
	 */
	if (set_host == B_FALSE && set_target == B_FALSE)
		return;

	if (get_prop(host_mach->n_prop, PT_MACRO) == NULL)
		set_host = B_TRUE;
	if (get_prop(target_mach->n_prop, PT_MACRO) == NULL)
		set_target = B_TRUE;

	INIT_STRING_FROM_STACK(str, buf);
	append_char(L'-', &str);

	if ((pipe = popen(mach_command, "r")) == NULL) {
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 183,
		    "Execute of %s failed"), mach_command);
	}
	while (fgets(fgetsbuf, sizeof (fgetsbuf), pipe) != NULL)
		append_string(fgetsbuf, &str, FIND_LENGTH);
	if (pclose(pipe) != 0) {
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 184, "Execute of %s "
		    "failed"), mach_command);
	}

	value = GETNAME(str.str_buf_start, FIND_LENGTH);
	if (set_host == B_TRUE) {
		(void) setvar_daemon(host_mach, value, B_FALSE,
		    DN_NO_DAEMON, B_TRUE, 0);
	}
	if (set_target == B_TRUE) {
		(void) setvar_daemon(target_mach, value, B_FALSE,
		    DN_NO_DAEMON, B_TRUE, 0);
	}
}

/*
 *	expand_value_with_daemon(name, macro, destination, cmd)
 *
 *	Checks for daemons and then maybe calls expand_value().
 *
 *	Parameters:
 *              name            Name of the macro  (Added by the NSE)
 *		macro		The property block with the value to expand
 *		destination	Where the result should be deposited
 *		cmd		If we are evaluating a command line we
 *				turn \ quoting off
 *
 *	Global variables used:
 */
static void
expand_value_with_daemon(name_t *name __UNUSED, macro_t *macro,
    string_t *destination, boolean_t cmd)
{
	chain_t *chain;

	switch (macro->m_daemon) {
	case DN_NO_DAEMON:
		if (!svr4 && !posix) {
			expand_value(macro->m_value, destination, cmd);
		} else {
			if (dollarless_flag && tilde_rule) {
				expand_value(dollarless_value, destination,
				    cmd);
				dollarless_flag = B_FALSE;
				tilde_rule = B_FALSE;
			} else {
				expand_value(macro->m_value, destination, cmd);
			}
		}
		return;
	case DN_CHAIN_DAEMON:
		/* If this is a $? value we call the daemon to translate the */
		/* list of names to a string */
		for (chain = macro->m_chain;
		     chain != NULL;
		     chain = chain->ch_next) {
			append_string_name(chain->ch_name, destination);
			if (chain->ch_next != NULL) {
				append_char(L' ', destination);
			}
		}
		return;
	}
}

boolean_t
is_vpath_defined(void)
{
	name_t *vpath;
	property_t *prop;
	macro_t *macro;

	if ((vpath = get_magic_macro(MMI_VPATH)) == NULL)
		return (B_FALSE);

	if ((prop = get_prop(vpath->n_prop, PT_MACRO)) == NULL)
		return (B_FALSE);

	macro = (macro_t *)prop->p_body;
	if (macro->m_value->n_key[0] == L'\0')
		return (B_FALSE);

	return (B_TRUE);
}

/*
 * We use a permanent buffer to reset SUNPRO_DEPENDENCIES value.
 * XXX No: We Do Not.
 */
#if 0
char	*sunpro_dependencies_buf = NULL;
char	*sunpro_dependencies_oldbuf = NULL;
int	sunpro_dependencies_buf_size = 0;
#endif

/*
 *	setvar_daemon(name, value, append, daemon, strip_trailing_spaces)
 *
 *	Set a macro value, possibly supplying a daemon to be used
 *	when referencing the value.
 *
 *	Return value:
 *				The property block with the new value
 *
 *	Parameters:
 *		name		Name of the macro to set
 *		value		The value to set
 *		append		Should we reset or append to the current value?
 *		daemon		Special treatment when reading the value
 *		strip_trailing_spaces from the end of value->string
 *		debug_level	Indicates how much tracing we should do
 *
 *	Global variables used:
 *		makefile_type	Used to check if we should enforce read only
 *		path_name	The Name "PATH", compared against
 *		virtual_root	The Name "VIRTUAL_ROOT", compared against
 *		vpath_defined	Set if the macro VPATH is set
 *		vpath_name	The Name "VPATH", compared against
 *		envvar		A list of environment vars with $ in value
 */
macro_t *
setvar_daemon(name_t *name, void *valarg, boolean_t append, daemon_t daemon,
    boolean_t strip_trailing_spaces, short debug_level)
{
	property_t *propm, *propma;
	macro_t *macro;
	macro_appendix_t *macapx = NULL;
	name_t *val;
	string_t dest;
	wchar_t buf[STRING_BUFFER_LENGTH];
	chain_t *chain = NULL;
	name_t *value = NULL;

	/*
	 * Determine whether our value argument is a name_t or a chain_t.
	 * XXX This interface is fucking abysmal.
	 */
	switch (daemon) {
	case DN_NO_DAEMON:
		value = (name_t *)valarg;
		break;
	case DN_CHAIN_DAEMON:
		chain = (chain_t *)valarg;
		break;
	default:
		fprintf(stderr, "PROGRAMMING ERROR\n");
		abort();
	}

	/*
	 * Fetch the macro_t property for this name:
	 */
	propm = maybe_append_prop(name, PT_MACRO);
	macro = (macro_t *)propm->p_body;

	/*
	 * Fetch the macro_appendix_t property if it exists:
	 */
	propma = get_prop(name->n_prop, PT_MACRO_APPEND);
	if (propma != NULL)
		macapx = (macro_appendix_t *)propma->p_body;

	if ((makefile_type != MFT_READING_NOTHING) &&
	    macro->m_read_only == B_TRUE) {
		return (macro);
	}

	if (value != NULL) {
		/*
		 * Strip spaces from the end of the value.
		 */
		int length = wcslen(value->n_key);
		if (length > 0 && iswspace(value->n_key[length - 1])) {
			wchar_t *tmp = xwcsdup(value->n_key);
			while (strip_trailing_spaces == B_TRUE && length > 0) {
				if (!iswspace(tmp[length - 1]))
					break;
				tmp[length - 1] = (wchar_t)0;
				length--;
			}
			value = GETNAME(tmp, FIND_LENGTH);
			free(tmp);
		}
	}

	if (macapx != NULL) {
		val = macapx->map_value;
	} else {
		val = macro->m_value;
	}

	if (append == B_TRUE) {
		/*
		 * If we are appending, we just tack the new value after
		 * the old one with a space in between.
		 */
		INIT_STRING_FROM_STACK(dest, buf);
		buf[0] = L'\0';

		if ((macro != NULL) && (val != NULL))
			append_string_name(val, &dest);

		if (value != NULL) {
			if (buf[0] != L'\0' && value->n_key[0] != L'\0') {
				/*
				 * We need a separating space:
				 */
				append_string(" ", &dest, FIND_LENGTH);
			}
			/*
			 * Append our value:
			 */
			append_string_name(value, &dest);
		}
		value = GETNAME(dest.str_buf_start, FIND_LENGTH);
		if (dest.str_free_after_use == B_TRUE)
			free(dest.str_buf_start);
	}

	/*
	 * Debugging Trace:
	 */
	if (debug_level > 1) {
		chain_t *tc;

		(void) printf("%ls =", name->n_key);
		switch (daemon) {
		case DN_CHAIN_DAEMON:
			for (tc = chain; tc != NULL; tc = tc->ch_next)
				(void) printf(" %ls", tc->ch_name->n_key);
			break;
		case DN_NO_DAEMON:
			if (value != NULL)
				(void) printf(" %ls", value->n_key);
			break;
		}
		(void) printf("\n");
	}

	/*
	 * Set the new values in the macro property block.
	 */
	if (macapx != NULL) {
		macapx->map_value = value;
		INIT_STRING_FROM_STACK(dest, buf);
		buf[0] = L'\0';
		if (value != NULL)
			append_string_name(value, &dest);
		if (macapx->map_value_to_append != NULL) {
			if (buf[0] != L'\0' &&
			    macapx->map_value_to_append->n_key[0] != L'\0') {
				append_string(" ", &dest, FIND_LENGTH);
			}
			append_string_name(macapx->map_value_to_append, &dest);
		}
		value = GETNAME(dest.str_buf_start, FIND_LENGTH);
		if (dest.str_free_after_use == B_TRUE)
			free(dest.str_buf_start);
	}

	macro->m_daemon = daemon;
	if (daemon == DN_CHAIN_DAEMON) {
		macro->m_chain = chain;
		macro->m_value = NULL;
	} else {
		macro->m_value = value;
		macro->m_chain = NULL;
	}

	/*
	 * If the user changes the VIRTUAL_ROOT, we need to flush
	 * the vroot package cache.
	 */
	if (is_magic_macro_name(MMI_PATH, name) == B_TRUE) {
		flush_path_cache();
	}
	if (is_magic_macro_name(MMI_VIRTUAL_ROOT, name) == B_TRUE) {
		flush_vroot_cache();
	}

	/*
	 * For environment variables, we also set the environment value
	 * each time.
	 */
	if (macro->m_exported == B_TRUE) {
		if (reading_environment == B_FALSE && (value != NULL) &&
		    value->n_dollar == B_TRUE) {
			boolean_t found = B_FALSE;
			envvar_t *ev;

			for (ev = envvar; ev != NULL; ev = ev->ev_next) {
				if (ev->ev_name == name) {
					ev->ev_value = value;
					ev->ev_already_put = B_FALSE;
					found = B_TRUE;
					break;
				}
			}

			if (found == B_FALSE) {
				ev = xmalloc(sizeof (*ev));
				ev->ev_name = name;
				ev->ev_value = value;
				ev->ev_next = envvar;
				ev->ev_env_string = NULL;
				ev->ev_already_put = B_FALSE;
				envvar = ev;
			}
		} else {
			property_t *env_prop = maybe_append_prop(name,
			    PT_ENV_MEM);
			env_mem_t *env_mem = (env_mem_t *)env_prop->p_body;

			free(env_mem->em_value);

			if (asprintf(&env_mem->em_value, "%ls=%ls", name->n_key,
			   value == NULL ? L"" : value->n_key) == -1)
				abort(); /* XXX */
			(void) putenv(env_mem->em_value);
		}
	}
	/*
	 * XXX NOT IMPLEMENTED; QUESTIONABLE VALUE.
	 */
	if (is_magic_macro_name(MMI_TARGET_ARCH, name) == B_TRUE) {
		fprintf(stderr, "ABORT - DO NOT PRESENTLY SUPPORT SETTING "
		   "TARGET_ARCH MACRO.\n");
		abort();
	}
#if 0
		name_t *ha = get_magic_macro(MMI_HOST_ARCH);
		name_t *ta = get_magic_macro(MMI_TARGET_ARCH);
		name_t *va = get_magic_macro(MMI_VIRTUAL_ROOT);
		int length;
		wchar_t *newval;
		wchar_t *oldvr;
		boolean_t *newalloc = B_FALSE;

		length = 32 + wcslen(ha->n_key) + wcslen(ta->n_key) +
		    wcslen(vr->n_key);

		/* XXX SIGH, YOU KNOW WHAT... NO. */
	}
	if (name == target_arch) {
		Name		ha = getvar(host_arch);
		Name		ta = getvar(target_arch);
		Name		vr = getvar(virtual_root);
		int		length;
		wchar_t		*new_value;
		wchar_t		*old_vr;
		Boolean		new_value_allocated = false;

		Wstring		ha_str(ha);
		Wstring		ta_str(ta);
		Wstring		vr_str(vr);

		wchar_t * wcb_ha = ha_str.get_string();
		wchar_t * wcb_ta = ta_str.get_string();
		wchar_t * wcb_vr = vr_str.get_string();

		length = 32 +
		  wslen(wcb_ha) +
		    wslen(wcb_ta) +
		      wslen(wcb_vr);
		old_vr = wcb_vr;
		MBSTOWCS(wcs_buffer, NOCATGETS("/usr/arch/"));
		if (IS_WEQUALN(old_vr,
			       wcs_buffer,
			       wslen(wcs_buffer))) {
			old_vr = (wchar_t *) wschr(old_vr, (int) colon_char) + 1;
		}
		if ( (ha == ta) || (wslen(wcb_ta) == 0) ) {
			new_value = old_vr;
		} else {
			new_value = ALLOC_WC(length);
			new_value_allocated = true;
			WCSTOMBS(mbs_buffer, old_vr);
			(void) wsprintf(new_value,
				        NOCATGETS("/usr/arch/%s/%s:%s"),
				        ha->string_mb + 1,
				        ta->string_mb + 1,
				        mbs_buffer);
		}
		if (new_value[0] != 0) {
			(void) setvar_daemon(virtual_root,
					     GETNAME(new_value, FIND_LENGTH),
					     false,
					     no_daemon,
					     true,
					     debug_level);
		}
		if (new_value_allocated) {
			retmem(new_value);
		}
	}
#endif
	return (macro);
}
