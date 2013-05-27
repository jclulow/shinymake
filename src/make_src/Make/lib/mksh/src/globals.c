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

/*
 *	globals.c
 *
 *	This declares all global variables
 */

/*
 * Included files
 */
#include <mksh/globals.h>

/*
 * Defined macros
 */

/*
 * typedefs & structs
 */

/*
 * Global variables
 */
char_class_t char_semantics[CHAR_SEMANTICS_ENTRIES];
wchar_t char_semantics_char[] = {
	ampersand_char,
	asterisk_char,
	at_char,
	backquote_char,
	backslash_char,
	bar_char,
	bracketleft_char,
	bracketright_char,
	colon_char,
	dollar_char,
	doublequote_char,
	equal_char,
	exclam_char,
	greater_char,
	hat_char,
	hyphen_char,
	less_char,
	newline_char,
	numbersign_char,
	parenleft_char,
	parenright_char,
	percent_char,
	plus_char,
	question_char,
	quote_char,
	semicolon_char,
#ifdef SGE_SUPPORT
	space_char,
	tab_char,
#endif
	nul_char
};
macro_list_t	cond_macro_list = NULL;
boolean_t	conditional_macro_used;
boolean_t	do_not_exec_rule;		/* `-n' */
boolean_t	dollarget_seen;
boolean_t	dollarless_flag;
name_t		*dollarless_value = NULL;
envvar_t	*envvar = NULL;
#ifdef lint
char		**environ;
#endif
#ifdef SUN5_0
int		exit_status;
#endif
wchar_t		*file_being_read;
/* Variable gnu_style=true if env. var. SUN_MAKE_COMPAT_MODE=GNU (RFE 4866328) */
boolean_t	gnu_style = B_FALSE;
name_set_t	hashtab = { NULL };
int		line_number;
char		*make_state_lockfile;
boolean_t	make_word_mentioned;
makefile_type_t	makefile_type = MFT_READING_NOTHING;
char		mbs_buffer[(MAXPATHLEN * MB_LEN_MAX)];
boolean_t	posix = B_TRUE;
name_t		*hat = NULL;
name_t		*query = NULL;
boolean_t	query_mentioned;
boolean_t	reading_environment;
boolean_t	svr4 = B_FALSE;
boolean_t	tilde_rule;
#if 0
name_t		*virtual_root = NULL;
name_t		*path_name = NULL;
boolean_t	vpath_defined;
name_t		*vpath_name = NULL;
#endif
wchar_t		wcs_buffer[MAXPATHLEN];
boolean_t	working_on_targets;
#if defined (TEAMWARE_MAKE_CMN) && defined(REDIRECT_ERR)
boolean_t	out_err_same;
#endif
pid_t		childPid = -1;	// This variable is used for killing child's process
				// Such as qrsh, running command, etc.

/*
 * Magic Macros:
 *   HOST_ARCH, TARGET_ARCH
 *   HOST_MACH, TARGET_MACH
 *   SHELL
 */
name_t *magic_macros[_MMI_MAX_ID - 1] = { NULL };

#if 0
name_t		host_arch = { 0 };
name_t		host_mach = { 0 };
name_t		target_arch = { 0 };
name_t		target_mach = { 0 };
name_t		*shell_name = NULL;
#endif

/*
 * timestamps defined in defs.h
 */
const timestruc_t file_no_time		= { -1, 0 };
const timestruc_t file_doesnt_exist	= { 0, 0 };
const timestruc_t file_is_dir		= { 1, 0 };
const timestruc_t file_min_time		= { 2, 0 };
const timestruc_t file_max_time		= { INT_MAX, 0 };
