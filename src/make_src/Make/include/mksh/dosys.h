#ifndef _MKSH_DOSYS_H
#define _MKSH_DOSYS_H
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
#include <vroot/vroot.h>

boolean_t await(boolean_t ignore_error, boolean_t silent_error,
    name_t *target, wchar_t *command, pid_t running_pid,
    boolean_t send_mtool_msgs, void *xdrs, int job_msg_id);
int doexec(wchar_t *command, boolean_t ignore_error, boolean_t redirect_out_err,
    char *stdout_file, char *stderr_file, pathpt vroot_path, int nice_prio);
int doshell(wchar_t *command, boolean_t ignore_error, boolean_t redirect_out_err,
    char *stdout_file, char *stderr_file, int nice_prio);
doname_t dosys_mksh(name_t *command, boolean_t ignore_error,
    boolean_t call_make, boolean_t silent_error, boolean_t always_exec,
    name_t *target, boolean_t redirect_out_err, char *stdout_file,
    char *stderr_file, pathpt vroot_path, int nice_prio);
void redirect_io(char *stdout_file, char *stderr_file);
void sh_command2string(string_t *command, string_t *destination);

#endif	/* _MKSH_DOSYS_H */
