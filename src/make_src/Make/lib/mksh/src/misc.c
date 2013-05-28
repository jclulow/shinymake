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
 *	misc.c
 *
 *	This file contains various unclassified routines. Some main groups:
 *		getname
 *		Memory allocation
 *		String handling
 *		Property handling
 *		Error message handling
 *		Make internal state dumping
 *		main routine support
 */

/*
 * Included files
 */
#include <bsd/bsd.h>
#include <mksh/i18n.h>
#include <mksh/misc.h>
#include <mksdmsi18n/mksdmsi18n.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <string.h>

/*
 * Static variables
 */
void (*sigivalue)(int) = SIG_DFL;
void (*sigqvalue)(int) = SIG_DFL;
void (*sigtvalue)(int) = SIG_DFL;
void (*sighvalue)(int) = SIG_DFL;

long	getname_bytes_count = 0;
long	getname_names_count = 0;
long	getname_struct_count = 0;

long	freename_bytes_count = 0;
long	freename_names_count = 0;
long	freename_struct_count = 0;

long	expandstring_count = 0;
long	getwstring_count = 0;


/*
static void expand_string(string_t *string, size_t length);
*/

#define	FATAL_ERROR_MSG_SIZE 200

static void
classify_nameblock(name_t *n)
{
	char_class_t sems = CHC_NONE;
	wchar_t *pos;

	/*
	 * Scan the namestring to classify it:
	 */
	for (pos = n->n_key; *pos != (wchar_t)0; pos++) {
		sems |= get_char_semantics_value(*pos);
	}

	n->n_dollar = (sems & CHC_DOLLAR) != 0 ? B_TRUE : B_FALSE;
	n->n_meta = (sems & CHC_META) != 0 ? B_TRUE : B_FALSE;
	n->n_percent = (sems & CHC_PERCENT) != 0 ? B_TRUE : B_FALSE;
	n->n_wildcard = (sems & CHC_WILDCARD) != 0 ? B_TRUE : B_FALSE;
	n->n_colon = (sems & CHC_COLON) != 0 ? B_TRUE : B_FALSE;
	n->n_parenleft = (sems & CHC_PARENLEFT) != 0 ? B_TRUE : B_FALSE;
}

/*
 *	getname_fn(name, len, dont_enter)
 *
 *	Hash a name string to the corresponding nameblock.
 *
 *	Return value:
 *				The Name block for the string
 *
 *	Parameters:
 *		name		The string we want to internalize
 *		len		The length of that string
 *		dont_enter	Don't enter the name if it does not exist
 *
 *	Global variables used:
 *		funny		The vector of semantic tags for characters
 *		hashtab		The hashtable used for the nametable
 */
name_t *
getname_fn(const wchar_t *name, size_t _len, boolean_t dont_enter,
    boolean_t *foundp)
{
	int length;
	name_t *np = NULL;
	boolean_t found = B_FALSE;

	/*
	 * First figure out how long the string is.
	 * If the len argument is -1 we count the chars here.
	 */
	if (_len == FIND_LENGTH) {
		length = wcslen(name);
	} else {
		abort(); /* XXX SIGH */
		length = _len;
	}

	if (dont_enter == B_TRUE) {
		/*
		 * We do not wish to create the entry if it does not
		 * exist, so just look for it.
		 */
		np = nms_lookup(&hashtab, name);

		if (foundp != NULL)
			*foundp = (np != NULL) ? B_TRUE : B_FALSE;

		return (np);
	}

	/*
	 * Locate an existing or create a new entry in the hash table:
	 */
	np = nms_insert(&hashtab, name, &found);
	if (foundp != NULL)
		*foundp = found;
	if (found == B_TRUE)
		return (np);

	/*
	 * Our nameblock is new, so initialise it:
	 */
	np->n_stat.ns_time = file_no_time;
	classify_nameblock(np);

	return (np);
}

void
store_name(name_t *n)
{
	nms_insert_name(&hashtab, n);
}

/*
 *	enable_interrupt(handler)
 *
 *	This routine sets a new interrupt handler for the signals make
 *	wants to deal with.
 *
 *	Parameters:
 *		handler		The function installed as interrupt handler
 *
 *	Static variables used:
 *		sigivalue	The original signal handler
 *		sigqvalue	The original signal handler
 *		sigtvalue	The original signal handler
 *		sighvalue	The original signal handler
 */
void
enable_interrupt(register void (*handler) (int))
{
	if (sigivalue != SIG_IGN) {
		(void) bsd_signal(SIGINT, (SIG_PF) handler);
	}
	if (sigqvalue != SIG_IGN) {
		(void) bsd_signal(SIGQUIT, (SIG_PF) handler);
	}
	if (sigtvalue != SIG_IGN) {
		(void) bsd_signal(SIGTERM, (SIG_PF) handler);
	}
	if (sighvalue != SIG_IGN) {
		(void) bsd_signal(SIGHUP, (SIG_PF) handler);
	}
}

/*
 *	setup_char_semantics()
 *
 *	Load the vector char_semantics[] with lexical markers
 *
 *	Parameters:
 *
 *	Global variables used:
 *		char_semantics	The vector of character semantics that we set
 */
void
setup_char_semantics(void)
{
	char		*s;
	wchar_t		wc_buffer[1];
	int		entry;

	if (svr4) {
		s = "@-";
	} else {
		s = "=@-?!+";
	}
	for (; MBTOWC(wc_buffer, s); s++) {
		entry = get_char_semantics_entry(*wc_buffer);
		char_semantics[entry] |= CHC_COMMAND_PREFIX;
	}
	char_semantics[dollar_char_entry] |= CHC_DOLLAR;
	for (s = "#|=^();&<>*?[]:$`'\"\\\n"; MBTOWC(wc_buffer, s); s++) {
		entry = get_char_semantics_entry(*wc_buffer);
		char_semantics[entry] |= CHC_META;
	}
	char_semantics[percent_char_entry] |= CHC_PERCENT;
	for (s = "@*<%?^"; MBTOWC(wc_buffer, s); s++) {
		entry = get_char_semantics_entry(*wc_buffer);
		char_semantics[entry] |= CHC_SPECIAL_MACRO;
	}
	for (s = "?[*"; MBTOWC(wc_buffer, s); s++) {
		entry = get_char_semantics_entry(*wc_buffer);
		char_semantics[entry] |= CHC_WILDCARD;
	}
	char_semantics[colon_char_entry] |= CHC_COLON;
	char_semantics[parenleft_char_entry] |= CHC_PARENLEFT;
}

/*
 *	errmsg(errnum)
 *
 *	Return the error message for a system call error
 *
 *	Return value:
 *				An error message string
 *
 *	Parameters:
 *		errnum		The number of the error we want to describe
 *
 *	Global variables used:
 *		sys_errlist	A vector of error messages
 *		sys_nerr	The size of sys_errlist
 */
char *
errmsg(int errnum)
{
	extern int sys_nerr;
	char *errbuf;

	if ((errnum < 0) || (errnum > sys_nerr)) {
		errbuf = xmalloc(6+1+11+1);
		(void) sprintf(errbuf, catgets(libmksdmsi18n_catd, 1, 127,
		    "Error %d"), errnum);
		return (errbuf);
	} else {
		return (strerror(errnum));

	}
}

static char static_buf[MAXPATHLEN*3];

/*
 *	fatal_mksh(format, args...)
 *
 *	Print a message and die
 *
 *	Parameters:
 *		format		printf type format string
 *		args		Arguments to match the format
 */
/*VARARGS*/
void
fatal_mksh(char * message, ...)
{
	va_list args;
	char    *buf = static_buf;
	char	*mksh_fat_err = catgets(libmksdmsi18n_catd, 1, 128, "mksh: Fatal error: ");
	char	*cur_wrk_dir = catgets(libmksdmsi18n_catd, 1, 129, "Current working directory: ");
	int	mksh_fat_err_len = strlen(mksh_fat_err);

	va_start(args, message);
	(void) fflush(stdout);
	(void) strcpy(buf, mksh_fat_err);
	size_t buf_len = vsnprintf(static_buf + mksh_fat_err_len,
				   sizeof(static_buf) - mksh_fat_err_len,
				   message, args)
			+ mksh_fat_err_len
			+ strlen(cur_wrk_dir)
			+ strlen(get_current_path_mksh())
			+ 3; // "\n\n"
	va_end(args);
	if (buf_len >= sizeof(static_buf)) {
		buf = xmalloc(buf_len);
		(void) strcpy(buf, mksh_fat_err);
		va_start(args, message);
		(void) vsprintf(buf + mksh_fat_err_len, message, args);
		va_end(args);
	}
	(void) strcat(buf, "\n");
/*
	if (report_pwd) {
 */
	if (1) {
		(void) strcat(buf, cur_wrk_dir);
		(void) strcat(buf, get_current_path_mksh());
		(void) strcat(buf, "\n");
	}
	(void) fputs(buf, stderr);
	(void) fflush(stderr);
	if (buf != static_buf) {
		free(buf);
	}
	exit_status = 1;
	exit(1);
}

/*
 *	fatal_reader_mksh(format, args...)
 *
 *	Parameters:
 *		format		printf style format string
 *		args		arguments to match the format
 */
/*VARARGS*/
void
fatal_reader_mksh(char * pattern, ...)
{
	va_list args;
	/*
	char	message[1000];
	*/

	va_start(args, pattern);
/*
	if (file_being_read != NULL) {
		WCSTOMBS(mbs_buffer, file_being_read);
		if (line_number != 0) {
			(void) sprintf(message,
				       catgets(libmksdmsi18n_catd, 1, 130, "%s, line %d: %s"),
				       mbs_buffer,
				       line_number,
				       pattern);
		} else {
			(void) sprintf(message,
				       "%s: %s",
				       mbs_buffer,
				       pattern);
		}
		pattern = message;
	}
 */

	(void) fflush(stdout);
	(void) fprintf(stderr, catgets(libmksdmsi18n_catd, 1, 131, "mksh: Fatal error in reader: "));
	(void) vfprintf(stderr, pattern, args);
	(void) fprintf(stderr, "\n");
	va_end(args);

/*
	if (temp_file_name != NULL) {
		(void) fprintf(stderr,
			       catgets(libmksdmsi18n_catd, 1, 132, "mksh: Temp-file %s not removed\n"),
			       temp_file_name->string_mb);
		temp_file_name = NULL;
	}
 */

/*
	if (report_pwd) {
 */
	if (1) {
		(void) fprintf(stderr,
			       catgets(libmksdmsi18n_catd, 1, 133, "Current working directory %s\n"),
			       get_current_path_mksh());
	}
	(void) fflush(stderr);
	exit_status = 1;
	exit(1);
}

/*
 *	warning_mksh(format, args...)
 *
 *	Print a message and continue.
 *
 *	Parameters:
 *		format		printf type format string
 *		args		Arguments to match the format
 */
/*VARARGS*/
void
warning_mksh(char * message, ...)
{
	va_list args;

	va_start(args, message);
	(void) fflush(stdout);
	(void) fprintf(stderr, catgets(libmksdmsi18n_catd, 1, 134, "mksh: Warning: "));
	(void) vfprintf(stderr, message, args);
	(void) fprintf(stderr, "\n");
	va_end(args);
/*
	if (report_pwd) {
 */
	if (1) {
		(void) fprintf(stderr,
			       catgets(libmksdmsi18n_catd, 1, 135, "Current working directory %s\n"),
			       get_current_path_mksh());
	}
	(void) fflush(stderr);
}

/*
 *	get_current_path_mksh()
 *
 *	Stuff current_path with the current path if it isnt there already.
 *
 *	Parameters:
 *
 *	Global variables used:
 */
char *
get_current_path_mksh(void)
{
	char			pwd[(MAXPATHLEN * MB_LEN_MAX)];
	static char		*current_path;

	if (current_path == NULL) {
		getcwd(pwd, sizeof(pwd));
		if (pwd[0] == (int) nul_char) {
			pwd[0] = (int) slash_char;
			pwd[1] = (int) nul_char;
		}
		current_path = xstrdup(pwd);
	}
	return (current_path);
}

/*
 *	append_prop(target, type)
 *
 *	Create a new property and append it to the property list of a Name.
 *
 *	Return value:
 *				A new property block for the target
 *
 *	Parameters:
 *		target		The target that wants a new property
 *		type		The type of property being requested
 *
 *	Global variables used:
 */
property_t *
append_prop(name_t *target, property_id_t type)
{
	property_t *prop;

	prop = zxmalloc(sizeof (*prop));
	prop->p_body = zxmalloc(property_body_size(type));
	prop->p_type = type;

	if (target->n_prop == NULL) {
		target->n_prop = prop;
	} else {
		property_t *last;
		while (last->p_next != NULL)
			last = last->p_next;
		last->p_next = prop;
	}

	return (prop);
}

/*
 *	maybe_append_prop(target, type)
 *
 *	Append a property to the Name if none of this type exists
 *	else return the one already there
 *
 *	Return value:
 *				A property of the requested type for the target
 *
 *	Parameters:
 *		target		The target that wants a new property
 *		type		The type of property being requested
 *
 *	Global variables used:
 */
property_t *
maybe_append_prop(name_t *target, property_id_t type)
{
	property_t *prop;

	if ((prop = get_prop(target->n_prop, type)) != NULL)
		return (prop);

	return (append_prop(target, type));
}

/*
 *	get_prop(start, type)
 *
 *	Scan the property list of a Name to find the next property
 *	of a given type.
 *
 *	Return value:
 *				The first property of the type, if any left
 *
 *	Parameters:
 *		start		The first property block to check for type
 *		type		The type of property block we need
 *
 *	Global variables used:
 */
property_t *
get_prop(property_t *start, property_id_t type)
{
	for (; start != NULL; start = start->p_next) {
		if (start->p_type == type) {
			return (start);
		}
	}
	return (NULL);
}

/*
 *	handle_interrupt_mksh()
 *
 *	This is where C-C traps are caught.
 */
void
handle_interrupt_mksh(int _x __UNUSED)
{
	(void) fflush(stdout);
	/* Make sure the processes running under us terminate first. */
	if (childPid > 0) {
		kill(childPid, SIGTERM);
		childPid = -1;
	}
	while (wait((int *) NULL) != -1);
	exit_status = 2;
	exit(2);
}

/*
 *	setup_interrupt()
 *
 *	This routine saves the original interrupt handler pointers
 *
 *	Parameters:
 *
 *	Static variables used:
 *		sigivalue	The original signal handler
 *		sigqvalue	The original signal handler
 *		sigtvalue	The original signal handler
 *		sighvalue	The original signal handler
 */
void
setup_interrupt(register void (*handler) (int))
{
	sigivalue = bsd_signal(SIGINT, SIG_IGN);
	sigqvalue = bsd_signal(SIGQUIT, SIG_IGN);
	sigtvalue = bsd_signal(SIGTERM, SIG_IGN);
	sighvalue = bsd_signal(SIGHUP, SIG_IGN);
	enable_interrupt(handler);
}


void
mbstowcs_with_check(wchar_t *pwcs, const char *s, size_t n)
{
	if (mbstowcs(pwcs, s, n) == (size_t)-1) {
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 143,
		    "The string `%s' is not valid in current locale"), s);
	}
}


#if 0
Wstring::Wstring()
{
	INIT_STRING_FROM_STACK(string, string_buf);
}

Wstring::Wstring(struct _Name * name)
{
	INIT_STRING_FROM_STACK(string, string_buf);
	append_string(name->string_mb, &string, name->hash.length);
}

Wstring::~Wstring()
{
	if(string.free_after_use) {
		free(string.buffer.start);
	}
}

void
Wstring::init(struct _Name * name)
{
	if(string.free_after_use) {
		free(string.buffer.start);
	}
	INIT_STRING_FROM_STACK(string, string_buf);
	append_string(name->string_mb, &string, name->hash.length);
}

void
Wstring::init(wchar_t * name, unsigned length)
{
	INIT_STRING_FROM_STACK(string, string_buf);
	append_string(name, &string, length);
	string.buffer.start[length] = 0;
}

Boolean
Wstring::equaln(wchar_t * str, unsigned length)
{
	return (Boolean)IS_WEQUALN(string.buffer.start, str, length);
}

Boolean
Wstring::equaln(Wstring * str, unsigned length)
{
	return (Boolean)IS_WEQUALN(string.buffer.start, str->string.buffer.start, length);
}

Boolean
Wstring::equal(wchar_t * str, unsigned off, unsigned length)
{
	return (Boolean)IS_WEQUALN(string.buffer.start + off, str, length);
}

Boolean
Wstring::equal(wchar_t * str, unsigned off)
{
	return (Boolean)IS_WEQUAL(string.buffer.start + off, str);
}

Boolean
Wstring::equal(wchar_t * str)
{
	return equal(str, 0);
}

Boolean
Wstring::equal(Wstring * str, unsigned off, unsigned length)
{
	return (Boolean)IS_WEQUALN(string.buffer.start + off, str->string.buffer.start, length);
}

Boolean
Wstring::equal(Wstring * str)
{
	return equal(str, 0);
}

Boolean
Wstring::equal(Wstring * str, unsigned off)
{
	return (Boolean)IS_WEQUAL(string.buffer.start + off, str->string.buffer.start);
}

void
Wstring::append_to_str(struct _String * str, unsigned off, unsigned length)
{
	append_string(string.buffer.start + off, str, length);
}
#endif
