#ifndef _MKSH_DEFS_H
#define _MKSH_DEFS_H
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
 * This is not "#ifdef TEAMWARE_MAKE_CMN" because we're currently
 * using the TW fake i18n headers and libraries to build both
 * SMake and PMake on SPARC/S1 and x86/S2.
 */

#include <avo/intl.h>
#include <limits.h>		/* MB_LEN_MAX */
#include <stdio.h>
#include <stdlib.h>		/* wchar_t */
#include <string.h>		/* strcmp() */
#include <nl_types.h>		/* catgets() */
#include <sys/param.h>		/* MAXPATHLEN */
#include <sys/types.h>		/* time_t, caddr_t */
#include <vroot/vroot.h>	/* pathpt */
#include <sys/time.h>		/* timestruc_t */
#include <errno.h>		/* errno */
#include <wctype.h>
#include <widec.h>

#include <mksh/cdefs.h>

/*
 * Some random constants (in an enum so dbx knows their values)
 */
enum {
	update_delay = 30,		/* time between rstat checks */
#ifdef sun386
	ar_member_name_len = 14,
#else
#if defined(SUN5_0) || defined(linux)
	ar_member_name_len = 1024,
#else
	ar_member_name_len = 15,
#endif
#endif

	hashsize = 2048			/* size of hash table */
};


/*
 * Symbols that defines all the different char constants make uses
 */
enum {
	ampersand_char =	'&',
	asterisk_char =		'*',
	at_char =		'@',
	backquote_char =	'`',
	backslash_char =	'\\',
	bar_char =		'|',
	braceleft_char =	'{',
	braceright_char =	'}',
	bracketleft_char =	'[',
	bracketright_char =	']',
	colon_char =		':',
	comma_char =		',',
	dollar_char =		'$',
	doublequote_char =	'"',
	equal_char =		'=',
	exclam_char =		'!',
	greater_char =		'>',
	hat_char =		'^',
	hyphen_char =		'-',
	less_char =		'<',
	newline_char =		'\n',
	nul_char =		'\0',
	numbersign_char =	'#',
	parenleft_char =	'(',
	parenright_char =	')',
	percent_char =		'%',
	period_char =		'.',
	plus_char =		'+',
	question_char =		'?',
	quote_char =		'\'',
	semicolon_char =	';',
	slash_char =		'/',
	space_char =		' ',
	tab_char =		'\t',
	tilde_char =		'~'
};

/*
 * For make i18n. Codeset independent.
 * Setup character semantics by identifying all the special characters
 * of make, and assigning each an entry in the char_semantics[] vector.
 */
enum {
	ampersand_char_entry = 0,	/*  0 */
	asterisk_char_entry,		/*  1 */
	at_char_entry,			/*  2 */
	backquote_char_entry,		/*  3 */
	backslash_char_entry,		/*  4 */
	bar_char_entry,			/*  5 */
	bracketleft_char_entry,		/*  6 */
	bracketright_char_entry,	/*  7 */
	colon_char_entry,		/*  8 */
	dollar_char_entry,		/*  9 */
	doublequote_char_entry,		/* 10 */
	equal_char_entry,		/* 11 */
	exclam_char_entry,		/* 12 */
	greater_char_entry,		/* 13 */
	hat_char_entry,			/* 14 */
	hyphen_char_entry,		/* 15 */
	less_char_entry,		/* 16 */
	newline_char_entry,		/* 17 */
	numbersign_char_entry,		/* 18 */
	parenleft_char_entry,		/* 19 */
	parenright_char_entry,		/* 20 */
	percent_char_entry,		/* 21 */
	plus_char_entry,		/* 22 */
	question_char_entry,		/* 23 */
	quote_char_entry,		/* 24 */
	semicolon_char_entry,		/* 25 */
	no_semantics_entry		/* 26 */
};

/*
 * CHAR_SEMANTICS_ENTRIES should be the number of entries above.
 * The last entry in char_semantics[] should be blank.
 */
#define CHAR_SEMANTICS_ENTRIES	27
/*
#define CHAR_SEMANTICS_STRING	"&*@`\\|[]:$=!>-\n#()%+?;^<'\""
 */

/*
 * Some utility macros
 */
#define ALLOC(x)		((struct _##x *)getmem(sizeof (struct _##x)))
#define ALLOC_WC(x)		((wchar_t *)getmem((x) * SIZEOFWCHAR_T))
#define FIND_LENGTH		(size_t)-1UL
#define GETNAME(a,b)		getname_fn((a), (b), B_FALSE, NULL)
#define IS_EQUAL(a,b)		(!strcmp((a), (b)))
#define IS_EQUALN(a,b,n)	(!strncmp((a), (b), (n)))
#define IS_WEQUAL(a,b)		(!wscmp((a), (b)))
#define IS_WEQUALN(a,b,n)	(!wsncmp((a), (b), (n)))
#define MBLEN(a)		mblen((a), MB_LEN_MAX)
#define MBSTOWCS(a,b)		(void) mbstowcs_with_check((a), (b), MAXPATHLEN)
#define	MBTOWC(a,b)		mbtowc((a), (b), MB_LEN_MAX)
#define	SIZEOFWCHAR_T		(sizeof (wchar_t))
#define VSIZEOF(v)		(sizeof (v) / sizeof ((v)[0]))
#define WCSTOMBS(a,b)		(void) wcstombs((a), (b), (MAXPATHLEN * MB_LEN_MAX))
#define WCTOMB(a,b)		(void) wctomb((a), (b))
#define	HASH(v, c)		(v = (v)*31 + (unsigned int)(c))

extern void mbstowcs_with_check(wchar_t *pwcs, const char *s, size_t n);


#if 0
#define STRING_BUFFER_LENGTH	1024
#define INIT_STRING_FROM_STACK(str, buf) { \
			str.buffer.start = (buf); \
			str.text.p = (buf); \
			str.text.end = NULL; \
			str.buffer.end = (buf) \
                          + (sizeof (buf)/SIZEOFWCHAR_T); \
			str.free_after_use = false; \
		  }
#endif

#if 0
#define APPEND_NAME(np, dest, len)	append_string((np)->string_mb, (dest), (len));
#endif

#if 0
class Wstring {
	public:
		struct _String	string;
		wchar_t		string_buf[STRING_BUFFER_LENGTH];

	public:
		Wstring();
		Wstring(struct _Name * name);
		~Wstring();

		void init(struct _Name * name);
		void init(wchar_t * name, unsigned length);
		unsigned length() {
			return wslen(string.buffer.start);
		};
		void append_to_str(struct _String * str, unsigned off, unsigned length);

		wchar_t * get_string() {
			return string.buffer.start;
		};

		wchar_t * get_string(unsigned off) {
			return string.buffer.start + off;
		};

		Boolean equaln(wchar_t * str, unsigned length);
		Boolean equal(wchar_t * str);
		Boolean equal(wchar_t * str, unsigned off);
		Boolean equal(wchar_t * str, unsigned off, unsigned length);

		Boolean equaln(Wstring * str, unsigned length);
		Boolean equal(Wstring * str);
		Boolean equal(Wstring * str, unsigned off);
		Boolean equal(Wstring * str, unsigned off, unsigned length);
};
#endif


/*
 * Magic values for the timestamp stored with each name object
 */

extern const timestruc_t file_no_time;
extern const timestruc_t file_doesnt_exist;
extern const timestruc_t file_is_dir;
extern const timestruc_t file_min_time;
extern const timestruc_t file_max_time;


/*
 * Macros for the reader
 */
#define GOTO_STATE(new_state) { \
				  SET_STATE(new_state); \
				    goto enter_state; \
			      }
#define SET_STATE(new_state) state = (new_state)

#define UNCACHE_SOURCE()	if (source != NULL) { \
					source->string.text.p = source_p; \
				  }
#define CACHE_SOURCE(comp)	if (source != NULL) { \
					source_p = source->string.text.p - \
					  (comp); \
					source_end = source->string.text.end; \
				  }
#define GET_NEXT_BLOCK_NOCHK(source)	{ UNCACHE_SOURCE(); \
				 source = get_next_block_fn(source); \
				 CACHE_SOURCE(0) \
			   }
#define GET_NEXT_BLOCK(source)	{ GET_NEXT_BLOCK_NOCHK(source); \
				 if (source != NULL && source->error_converting) { \
				 	GOTO_STATE(illegal_bytes_state); \
				 } \
			   }
#define GET_CHAR()		((source == NULL) || \
				(source_p >= source_end) ? 0 : *source_p)

/*
 *	extern declarations for all global variables.
 *	The actual declarations are in globals.cc
 */
extern char_class_t	char_semantics[];
extern wchar_t		char_semantics_char[];
extern macro_list_t	*cond_macro_list;
extern boolean_t	conditional_macro_used;
extern boolean_t	do_not_exec_rule;		/* `-n' */
extern boolean_t	dollarget_seen;
extern boolean_t	dollarless_flag;
extern name_t		*dollarless_value;
extern char		**environ;
extern envvar_t		*envvar;
#if defined(SUN5_0) || defined(HP_UX) || defined(linux)
extern int		exit_status;
#endif
extern wchar_t		*file_being_read;
/* Variable gnu_style=true if env. var. SUN_MAKE_COMPAT_MODE=GNU (RFE 4866328) */
extern boolean_t	gnu_style;
extern name_set_t	hashtab;
extern int		line_number;
extern char		*make_state_lockfile;
extern boolean_t	make_word_mentioned;
extern makefile_type_t	makefile_type;
extern char		mbs_buffer[];
extern boolean_t	posix;
extern boolean_t	query_mentioned;
extern boolean_t	reading_environment;
extern boolean_t	svr4;
extern boolean_t	tilde_rule;
extern wchar_t		wcs_buffer[];
extern boolean_t	working_on_targets;
#if 0
extern name_t		*query;
extern name_t		*hat;
extern name_t		*virtual_root;
extern name_t		*path_name;
extern boolean_t	vpath_defined;
extern name_t		*vpath_name;
#endif
extern boolean_t	make_state_locked;
#if defined (TEAMWARE_MAKE_CMN) && defined(REDIRECT_ERR)
extern boolean_t	out_err_same;
#endif
extern pid_t		childPid;
extern nl_catd		libmksh_catd;


/*
 * Magic Macros:
 *   HOST_ARCH, TARGET_ARCH
 *   HOST_MACH, TARGET_MACH
 *     etc
 */
typedef enum magic_macro_id {
	MMI_UNKNOWN = 0,
	MMI_HOST_ARCH,		/*  1 */
	MMI_HOST_MACH,
	MMI_TARGET_ARCH,
	MMI_TARGET_MACH,
	MMI_SHELL,		/*  5 */
	MMI_PATH,
	MMI_VPATH,
	MMI_VIRTUAL_ROOT,
	MMI_MAKE,
	MMI_QUERY,		/* 10 */
	MMI_HAT,
	MMI_POSIX,
	MMI_SVR4,
	MMI_BUILT_LAST_MAKE_RUN,
	MMI_AT,			/* 15 */
	MMI_DEFAULT_RULE,
	MMI_DOLLAR,
	MMI_DONE,
	MMI_DOT,
	MMI_KEEP_STATE,		/* 20 */
	MMI_KEEP_STATE_FILE,
	MMI_EMPTY_NAME,
	MMI_FORCE,
	MMI_IGNORE,
	MMI_INIT,		/* 25 */
	MMI_LOCALHOST,
	MMI_MAKE_STATE,
	MMI_MAKEFLAGS,
	MMI_MAKE_VERSION,
	MMI_NO_PARALLEL,	/* 30 */
	MMI_NOT_AUTO,
	MMI_PARALLEL,
	MMI_PLUS,
	MMI_PRECIOUS,
	MMI_RECURSIVE,		/* 35 */
	MMI_SCCS_GET,
	MMI_SCCS_GET_POSIX,
	MMI_GET,
	MMI_GET_POSIX,
	MMI_SILENT,		/* 40 */
	MMI_SUFFIXES,
	MMI_SUNPRO_DEPENDENCIES,
	MMI_WAIT,
	_MMI_MAX_ID
} magic_macro_id_t;
extern name_t *magic_macros[];


#endif
