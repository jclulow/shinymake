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
 *	read.c
 *
 *	This file contains the makefile reader.
 */

/*
 * Included files
 */
#include <mksh/misc.h>
#include <mksh/read.h>
#include <mksdmsi18n/mksdmsi18n.h>
#include <sys/uio.h>
#include <unistd.h>

#define	STRING_LEN_TO_CONVERT	(8*1024)

boolean_t make_state_locked;

/*
 *	get_next_block_fn(source)
 *
 *	Will get the next block of text to read either
 *	by popping one source bVSIZEOFlock of the stack of Sources
 *	or by reading some more from the makefile.
 *
 *	Return value:
 *				The new source block to read from
 *
 *	Parameters:
 *		source		The old source block
 *
 *	Global variables used:
 *		file_being_read	The name of the current file, error msg
 */
source_t *
get_next_block_fn(source_t *src)
{
	size_t num_wc_chars;
	char *ptr;

	if (src == NULL) {
		return (NULL);
	}
	if ((src->src_fd == -1) || ((src->src_bytes_left_in_file <= 0) &&
	    (src->src_inp_buf_ptr >= src->src_inp_buf_end))) {
		/*
		 * We can't read from the makefile, so pop the source block.
		 */
		if (src->src_fd > 2) {
			(void) close(src->src_fd);
			if (make_state_lockfile != NULL) {
				(void) unlink(make_state_lockfile);
				free(make_state_lockfile);
				make_state_lockfile = NULL;
				make_state_locked = B_FALSE;
			}
		}

		if (src->src_str.str_free_after_use == B_TRUE &&
		    (src->src_str.str_buf_start != NULL)) {
			free(src->src_str.str_buf_start);
			src->src_str.str_buf_start = NULL;
		}
		if (src->src_inp_buf != NULL) {
			free(src->src_inp_buf);
			src->src_inp_buf = NULL;
			src->src_inp_buf_ptr = NULL;
			src->src_inp_buf_end = NULL;
		}
		src = src->src_prev;
		if (src != NULL) {
			src->src_error_converting = B_FALSE;
		}
		return (src);
	}
	if (src->src_bytes_left_in_file > 0) {
		size_t length;
		size_t to_read = src->src_bytes_left_in_file;
		/*
		 * Read the whole makefile.
		 * Hopefully the kernel managed to prefetch the stuff.
		 */
	 	src->src_inp_buf_ptr = src->src_inp_buf = xmalloc(to_read + 1);
		src->src_inp_buf_end = src->src_inp_buf + to_read;

		length = read(src->src_fd, src->src_inp_buf, to_read);
		if (length != to_read) {
			if (length == 0) {
				fatal_mksh(catgets(libmksdmsi18n_catd, 1, 140,
				    "Error reading `%ls': Premature EOF"),
				    file_being_read);
			} else {
				fatal_mksh(catgets(libmksdmsi18n_catd, 1, 141,
				    "Error reading `%s': %s"), file_being_read,
				    errmsg(errno));
			}
		}
		*src->src_inp_buf_end = '\0';
		src->src_bytes_left_in_file = 0;
	}
	/*
	 * Try to convert the next piece.
	 */
	ptr = src->src_inp_buf_ptr + STRING_LEN_TO_CONVERT;
	if (ptr > src->src_inp_buf_end) {
		ptr = src->src_inp_buf_end;
	}
	for (num_wc_chars = 0; ptr > src->src_inp_buf_ptr; ptr--) {
		char ch_save = *ptr;
		*ptr = '\0';
		num_wc_chars = mbstowcs(src->src_str.str_end,
		    src->src_inp_buf_ptr, STRING_LEN_TO_CONVERT);
		*ptr = ch_save;
		if (num_wc_chars != (size_t)-1) {
			break;
		}
	}

	if (num_wc_chars == (size_t)-1) {
		src->src_error_converting = B_TRUE;
		return (src);
	}

	src->src_error_converting = B_FALSE;
	src->src_inp_buf_ptr = ptr;
	src->src_str.str_end += num_wc_chars;
	*src->src_str.str_end = L'\0';

	if (src->src_inp_buf_ptr >= src->src_inp_buf_end) {
		if (*(src->src_str.str_end - 1) != L'\n') {
			warning_mksh(catgets(libmksdmsi18n_catd, 1, 142,
			    "newline is not last character in file %ls"),
			    file_being_read);
			*src->src_str.str_end++ = L'\n';
			*src->src_str.str_end = L'\0';
			/*
			 * XXX this seems dubious:
			 */
			src->src_str.str_buf_end++;
		}
		if (src->src_inp_buf != NULL) {
			free(src->src_inp_buf);
			src->src_inp_buf = NULL;
		}
	}
	return (src);
}
