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
 * Copyright 2005 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms.
 */

/*
 *	dosys.c
 *
 *	Execute one commandline
 */

/*
 * Included files
 */
#include <sys/wait.h>			/* WIFEXITED(status) */
#include <alloca.h>

#include <strings.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <mksh/dosys.h>
#include <mksh/macro.h>		/* getvar() */
#include <mksh/misc.h>		/* getmem(), fatal_mksh(), errmsg() */
#include <mksdmsi18n/mksdmsi18n.h>	/* libmksdmsi18n_init() */
#include <sys/signal.h>		/* SIG_DFL */
#include <sys/stat.h>		/* open() */
#include <sys/wait.h>		/* wait() */
#include <ulimit.h>
#include <unistd.h>

/*
 * Defined macros
 */
#define SEND_MTOOL_MSG(cmds)

/*
 * typedefs & structs
 */

/*
 * Static variables
 */

/*
 * File table of contents
 */
static op_result_t exec_vp(char *name, char **argv, char **envp,
    boolean_t ignore_error, pathpt vroot_path);

/*
 * Workaround for NFS bug. Sometimes, when running 'open' on a remote
 * dmake server, it fails with "Stale NFS file handle" error.
 * The second attempt seems to work.
 * XXX THIS IS WONDERFUL
 */
int
my_open(const char *path, int oflag, mode_t mode) {
	int res = open(path, oflag, mode);
	if (res < 0 && (errno == ESTALE || errno == EAGAIN)) {
		/* Stale NFS file handle. Try again */
		res = open(path, oflag, mode);
	}
	return res;
}

/*
 *	void
 *	redirect_io(char *stdout_file, char *stderr_file)
 *
 *	Redirects stdout and stderr for a child mksh process.
 */
void
redirect_io(char *stdout_file, char *stderr_file)
{
	long		descriptor_limit;
	int		i;

	if ((descriptor_limit = ulimit(UL_GDESLIM)) < 0) {
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 89,
		    "ulimit() failed: %s"), errmsg(errno));
	}
	for (i = 3; i < descriptor_limit; i++) {
		(void) close(i);
	}
	if ((i = my_open(stdout_file, O_WRONLY | O_CREAT | O_TRUNC | O_DSYNC,
	    S_IREAD | S_IWRITE)) < 0) {
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 90,
		    "Couldn't open standard out temp file `%s': %s"),
		    stdout_file, errmsg(errno));
	} else {
		if (dup2(i, 1) == -1) {
			fatal_mksh(NOCATGETS("*** Error: dup2(3, 1) "
			    "failed: %s"), errmsg(errno));
		}
		close(i);
	}
	if (stderr_file == NULL) {
		if (dup2(1, 2) == -1) {
			fatal_mksh(NOCATGETS("*** Error: dup2(1, 2) failed: %s"),
				errmsg(errno));
		}
	} else if ((i = my_open(stderr_file,
	                O_WRONLY | O_CREAT | O_TRUNC | O_DSYNC,
	                S_IREAD | S_IWRITE)) < 0) {
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 91, "Couldn't open standard error temp file `%s': %s"),
		      stderr_file,
		      errmsg(errno));
	} else {
		if (dup2(i, 2) == -1) {
			fatal_mksh(NOCATGETS("*** Error: dup2(3, 2) failed: %s"),
				errmsg(errno));
		}
		close(i);
	}
}

/*
 *	dosys_mksh(command, ignore_error, call_make, silent_error, target)
 *
 *	Check if command string contains meta chars and dispatch to
 *	the proper routine for executing one command line.
 *
 *	Return value:
 *				Indicates if the command execution failed
 *
 *	Parameters:
 *		command		The command to run
 *		ignore_error	Should we abort when an error is seen?
 *		call_make	Did command reference $(MAKE) ?
 *		silent_error	Should error messages be suppressed for dmake?
 *		target		Target we are building
 *
 *	Global variables used:
 *		do_not_exec_rule Is -n on?
 *		working_on_targets We started processing real targets
 */
doname_t
dosys_mksh(name_t *command, boolean_t ignore_error, boolean_t call_make,
    boolean_t silent_error, boolean_t always_exec, name_t *target,
    boolean_t redirect_out_err, char *stdout_file, char *stderr_file,
    pathpt vroot_path, int nice_prio)
{
#if 0
	int		length = command->hash.length;
#endif
	pid_t waitfor;
	wchar_t	*cmd_string = command->n_key;
	wchar_t	*q;
	/*
	struct stat before;
	*/
	doname_t result = DON_DONT_KNOW;
	boolean_t working_on_targets_mksh = B_TRUE;

	/*
	 * Strip spaces from head of command string.
	 */
	while (iswspace(*cmd_string))
		cmd_string++;
	/*
	 * Fail if the string is now empty:
	 */
	if (*cmd_string == (wchar_t)0)
		return (DON_FAILED);

	/*
	 * If we are faking it we just return.
	 */
	if (do_not_exec_rule == B_TRUE &&
	    working_on_targets_mksh == B_TRUE &&
	    call_make == B_FALSE &&
	    always_exec == B_FALSE) {
		return (DON_OK);
	}

	/*
	 * Copy string to make it OK to write it.
	 */
	q = xwcsdup(cmd_string);

	/* Write the state file iff this command uses make. */
/* XXX - currently does not support recursive make's, $(MAKE)'s
	if (call_make && command_changed) {
		write_state_file(0, false);
	}
	(void) stat(make_state->string_mb, &before);
 */

	/*
	 * Run command directly if it contains no shell meta chars,
	 * else run it using the shell.
	 *
	 * XXX - command->meta *may* not be set correctly.
	 */
	if (command->n_meta == B_TRUE) {
		waitfor = doshell(q, ignore_error, redirect_out_err,
		    stdout_file, stderr_file, nice_prio);
	} else {
		waitfor = doexec(q, ignore_error, redirect_out_err,
		    stdout_file, stderr_file, vroot_path, nice_prio);
	}
	if (await(ignore_error, silent_error, target, cmd_string, waitfor,
	    B_FALSE, NULL, -1)) {
		result = DON_OK;
	} else {
		result = DON_FAILED;
	}
	free(q);

/* XXX - currently does not support recursive make's, $(MAKE)'s
	if ((report_dependencies_level == 0) &&
	    call_make) {
		make_state->stat.time = (time_t)file_no_time;
		(void)exists(make_state);
		if (before.st_mtime == make_state->stat.time) {
			return result;
		}
		makefile_type = reading_statefile;
		if (read_trace_level > 1) {
			trace_reader = true;
		}
		(void) read_simple_file(make_state,
					false,
					false,
					false,
					false,
					false,
					true);
		trace_reader = false;
	}
 */
	return (result);
}

/*
 *	doshell(command, ignore_error)
 *
 *	Used to run command lines that include shell meta-characters.
 *	The make macro SHELL is supposed to contain a path to the shell.
 *
 *	Return value:
 *				The pid of the process we started
 *
 *	Parameters:
 *		command		The command to run
 *		ignore_error	Should we abort on error?
 *
 *	Global variables used:
 *		filter_stderr	If -X is on we redirect stderr
 *		shell_name	The Name "SHELL", used to get the path to shell
 */
int
doshell(wchar_t *command, boolean_t ignore_error, boolean_t redirect_out_err, char *stdout_file, char *stderr_file, int nice_prio)
{
	char			*argv[6];
	int			argv_index = 0;
	int			cmd_argv_index;
	int			length;
	char			nice_prio_buf[MAXPATHLEN];
	char		*shellname;
	char			*tmp_mbs_buffer;

	name_t *shell;
	char *snbuf[MAXPATHLEN], *shellname;

	/*
	 * Attempt to expand the SHELL macro:
	 */
	shell = getvar(get_magic_macro(MMI_SHELL));
	if (wcslen(shell->n_key) == 0) {
		/*
		 * The expansion resulted in the empty string.  Use the value
		 * without expansion:
		 */
		shell = get_magic_macro(MMI_SHELL);
	}

	if ((shellname = strrchr(shell->string_mb, (int) slash_char)) == NULL) {
		shellname = shell->string_mb;
	} else {
		shellname++;
	}

	/*
	 * Only prepend the /usr/bin/nice command to the original command
	 * if the nice priority, nice_prio, is NOT zero (0).
	 * Nice priorities can be a positive or a negative number.
	 */
	if (nice_prio != 0) {
		argv[argv_index++] = NOCATGETS("nice");
		(void) sprintf(nice_prio_buf, NOCATGETS("-%d"), nice_prio);
		argv[argv_index++] = strdup(nice_prio_buf);
	}
	argv[argv_index++] = shellname;
	argv[argv_index++] = (char*)(ignore_error ? NOCATGETS("-c") :
	    NOCATGETS("-ce"));
	if ((length = wslen(command)) >= MAXPATHLEN) {
		tmp_mbs_buffer = getmem((length * MB_LEN_MAX) + 1);
                (void) wcstombs(tmp_mbs_buffer, command, (length * MB_LEN_MAX) + 1);
		cmd_argv_index = argv_index;
                argv[argv_index++] = strdup(tmp_mbs_buffer);
                retmem_mb(tmp_mbs_buffer);
	} else {
		WCSTOMBS(mbs_buffer, command);
		cmd_argv_index = argv_index;
#if defined(linux)
		int mbl = strlen(mbs_buffer);
		if(mbl > 2) {
			if(mbs_buffer[mbl-1] == '\n' && mbs_buffer[mbl-2] == '\\') {
				mbs_buffer[mbl] = '\n';
				mbs_buffer[mbl+1] = 0;
			}
		}
#endif
		argv[argv_index++] = strdup(mbs_buffer);
	}
	argv[argv_index] = NULL;
	(void) fflush(stdout);
	if ((childPid = fork()) == 0) {
		enable_interrupt((void (*) (int)) SIG_DFL);
		if (redirect_out_err) {
			redirect_io(stdout_file, stderr_file);
		}
#if 0
		if (filter_stderr) {
			redirect_stderr();
		}
#endif
		if (nice_prio != 0) {
			(void) execve(NOCATGETS("/usr/bin/nice"), argv, environ);
			fatal_mksh(catgets(libmksdmsi18n_catd, 1, 92, "Could not load `/usr/bin/nice': %s"),
			      errmsg(errno));
		} else {
			(void) execve(shell->string_mb, argv, environ);
			fatal_mksh(catgets(libmksdmsi18n_catd, 1, 93, "Could not load Shell from `%s': %s"),
			      shell->string_mb,
			      errmsg(errno));
		}
	}
	if (childPid  == -1) {
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 94, "fork failed: %s"),
		      errmsg(errno));
	}
	retmem_mb(argv[cmd_argv_index]);
	return childPid;
}

/*
 *	exec_vp(name, argv, envp, ignore_error)
 *
 *	Like execve, but does path search.
 *	This starts command when make invokes it directly (without a shell).
 *
 *	Return value:
 *				Returns false if the exec failed
 *
 *	Parameters:
 *		name		The name of the command to run
 *		argv		Arguments for the command
 *		envp		The environment for it
 *		ignore_error	Should we abort on error?
 *
 *	Global variables used:
 *		shell_name	The Name "SHELL", used to get the path to shell
 *		vroot_path	The path used by the vroot package
 */
static op_result_t
exec_vp(char *name, char **argv, char **envp, boolean_t ignore_error,
    pathpt vroot_path)
{
	Name		shell = getvar(shell_name);
	char		*shellname;
	char			*shargv[4];
	Name			tmp_shell;

	if (IS_EQUAL(shell->string_mb, "")) {
		shell = shell_name;
	}

	for (int i = 0; i < 5; i++) {
		(void) execve_vroot(name,
				    argv + 1,
				    envp,
				    vroot_path,
				    VROOT_DEFAULT);
		switch (errno) {
		case ENOEXEC:
		case ENOENT:
			/* That failed. Let the shell handle it */
			shellname = strrchr(shell->string_mb, (int) slash_char);
			if (shellname == NULL) {
				shellname = shell->string_mb;
			} else {
				shellname++;
			}
			shargv[0] = shellname;
			shargv[1] = (char*)(ignore_error == B_TRUE ?
			    NOCATGETS("-c") : NOCATGETS("-ce"));
			shargv[2] = argv[0];
			shargv[3] = NULL;
			tmp_shell = getvar(shell_name);
			if (IS_EQUAL(tmp_shell->string_mb, "")) {
				tmp_shell = shell_name;
			}
			(void) execve_vroot(tmp_shell->string_mb,
					    shargv,
					    envp,
					    vroot_path,
					    VROOT_DEFAULT);
			return (OPR_FAILURE);
		case ETXTBSY:
			/*
			 * The program is busy (debugged?).
			 * Wait and then try again.
			 */
			(void) sleep((unsigned) i);
		case EAGAIN:
			break;
		default:
			return (OPR_FAILURE);
		}
	}
	return (OPR_FAILURE);
}

/*
 *	doexec(command, ignore_error)
 *
 *	Will scan an argument string and split it into words
 *	thus building an argument list that can be passed to exec_ve()
 *
 *	Return value:
 *				The pid of the process started here
 *
 *	Parameters:
 *		command		The command to run
 *		ignore_error	Should we abort on error?
 *
 *	Global variables used:
 *		filter_stderr	If -X is on we redirect stderr
 */
int
doexec(wchar_t *command, boolean_t ignore_error, boolean_t redirect_out_err,
    char *stdout_file, char *stderr_file, pathpt vroot_path, int nice_prio)
{
	int			arg_count = 5;
	char			**argv;
	int			length;
	char			nice_prio_buf[MAXPATHLEN];
	char		**p;
	wchar_t			*q;
	wchar_t	*t;
	char			*tmp_mbs_buffer;

	/*
	 * Only prepend the /usr/bin/nice command to the original command
	 * if the nice priority, nice_prio, is NOT zero (0).
	 * Nice priorities can be a positive or a negative number.
	 */
	if (nice_prio != 0) {
		arg_count += 2;
	}
	for (t = command; *t != (int) nul_char; t++) {
		if (iswspace(*t)) {
			arg_count++;
		}
	}
	argv = (char **)alloca(arg_count * (sizeof(char *)));
	/*
	 * Reserve argv[0] for sh in case of exec_vp failure.
	 * Don't worry about prepending /usr/bin/nice command to argv[0].
	 * In fact, doing it may cause the sh command to fail!
	 */
	p = &argv[1];
	if ((length = wslen(command)) >= MAXPATHLEN) {
		tmp_mbs_buffer = getmem((length * MB_LEN_MAX) + 1);
		(void) wcstombs(tmp_mbs_buffer, command, (length * MB_LEN_MAX) + 1);
		argv[0] = strdup(tmp_mbs_buffer);
		retmem_mb(tmp_mbs_buffer);
        } else {
		WCSTOMBS(mbs_buffer, command);
		argv[0] = strdup(mbs_buffer);
	}

	if (nice_prio != 0) {
		*p++ = strdup(NOCATGETS("/usr/bin/nice"));
		(void) sprintf(nice_prio_buf, NOCATGETS("-%d"), nice_prio);
		*p++ = strdup(nice_prio_buf);
	}
	/* Build list of argument words. */
	for (t = command; *t;) {
		if (p >= &argv[arg_count]) {
			/* This should never happen, right? */
			WCSTOMBS(mbs_buffer, command);
			fatal_mksh(catgets(libmksdmsi18n_catd, 1, 95, "Command `%s' has more than %d arguments"),
			      mbs_buffer,
			      arg_count);
		}
		q = t;
		while (!iswspace(*t) && (*t != (int) nul_char)) {
			t++;
		}
		if (*t) {
			for (*t++ = (int) nul_char; iswspace(*t); t++);
		}
		if ((length = wslen(q)) >= MAXPATHLEN) {
			tmp_mbs_buffer = getmem((length * MB_LEN_MAX) + 1);
			(void) wcstombs(tmp_mbs_buffer, q, (length * MB_LEN_MAX) + 1);
			*p++ = strdup(tmp_mbs_buffer);
			retmem_mb(tmp_mbs_buffer);
		} else {
			WCSTOMBS(mbs_buffer, q);
			*p++ = strdup(mbs_buffer);
		}
	}
	*p = NULL;

	/* Then exec the command with that argument list. */
	(void) fflush(stdout);
	if ((childPid = fork()) == 0) {
		enable_interrupt((void (*) (int)) SIG_DFL);
		if (redirect_out_err) {
			redirect_io(stdout_file, stderr_file);
		}
#if 0
		if (filter_stderr) {
			redirect_stderr();
		}
#endif
		(void) exec_vp(argv[1], argv, environ, ignore_error, vroot_path);
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 96, "Cannot load command `%s': %s"), argv[1], errmsg(errno));
	}
	if (childPid  == -1) {
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 97, "fork failed: %s"),
		      errmsg(errno));
	}
	for (int i = 0; argv[i] != NULL; i++) {
		retmem_mb(argv[i]);
	}
	return childPid;
}

/*
 *	await(ignore_error, silent_error, target, command, running_pid)
 *
 *	Wait for one child process and analyzes
 *	the returned status when the child process terminates.
 *
 *	Return value:
 *				Returns true if commands ran OK
 *
 *	Parameters:
 *		ignore_error	Should we abort on error?
 *		silent_error	Should error messages be suppressed for dmake?
 *		target		The target we are building, for error msgs
 *		command		The command we ran, for error msgs
 *		running_pid	The pid of the process we are waiting for
 *
 *	Static variables used:
 *		filter_file	The fd for the filter file
 *		filter_file_name The name of the filter file
 *
 *	Global variables used:
 *		filter_stderr	Set if -X is on
 */
op_result_t
await(boolean_t ignore_error, boolean_t silent_error, Name target,
    wchar_t *command, pid_t running_pid, boolean_t send_mtool_msgs,
    void *xdrs_p, int job_msg_id)
{
#ifdef SUN5_0
        int                     status;
#else
#ifndef WEXITSTATUS
#define WEXITSTATUS(stat)       stat.w_T.w_Retcode
#endif
#ifndef WTERMSIG
#define WTERMSIG(stat)          stat.w_T.w_Termsig
#endif
#ifndef WCOREDUMP
#define WCOREDUMP(stat)         stat.w_T.w_Coredump
#endif
#if defined (HP_UX) || defined (linux)
	int			status;
#else
	union wait		status;
#endif
#endif
	char			*buffer;
	int			core_dumped;
	int			exit_status;
#if defined(DISTRIBUTED) || defined(MAKETOOL) /* tolik */
	Avo_CmdOutput		*make_output_msg;
#endif
	FILE			*outfp;
	pid_t		pid;
	struct stat		stat_buff;
	int			termination_signal;
	char			tmp_buf[MAXPATHLEN];
#if defined(DISTRIBUTED) || defined(MAKETOOL) /* tolik */
	RWCollectable		*xdr_msg;
#endif

	while ((pid = wait(&status)) != running_pid) {
		if (pid == -1) {
			fatal_mksh(catgets(libmksdmsi18n_catd, 1, 98, "wait() failed: %s"), errmsg(errno));
		}
	}
	(void) fflush(stdout);
	(void) fflush(stderr);

#if defined(SUN5_0) || defined(HP_UX) || defined(linux)
        if (status == 0) {

#ifdef PRINT_EXIT_STATUS
		warning_mksh(NOCATGETS("I'm in await(), and status is 0."));
#endif

                return (OPR_SUCCESS);
	}

#ifdef PRINT_EXIT_STATUS
	warning_mksh(NOCATGETS("I'm in await(), and status is *NOT* 0."));
#endif

#else
        if (status.w_status == 0) {
                return (OPR_SUCCESS);
	}
#endif

        exit_status = WEXITSTATUS(status);

#ifdef PRINT_EXIT_STATUS
	warning_mksh(NOCATGETS("I'm in await(), and exit_status is %d."), exit_status);
#endif

        termination_signal = WTERMSIG(status);
        core_dumped = WCOREDUMP(status);

	/*
	 * If the child returned an error, we now try to print a
	 * nice message about it.
	 */
	SEND_MTOOL_MSG(
		make_output_msg = new Avo_CmdOutput();
		(void) sprintf(tmp_buf, "%d", job_msg_id);
		make_output_msg->appendOutput(AVO_STRDUP(tmp_buf));
	);

	tmp_buf[0] = (int) nul_char;
	if (!silent_error) {
		if (exit_status != 0) {
			(void) fprintf(stdout,
				       catgets(libmksdmsi18n_catd, 1, 103, "*** Error code %d"),
				       exit_status);
			SEND_MTOOL_MSG(
				(void) sprintf(&tmp_buf[strlen(tmp_buf)],
					       catgets(libmksdmsi18n_catd, 1, 104, "*** Error code %d"),
					       exit_status);
			);
		} else {
#if ! defined(SUN5_0) && ! defined(HP_UX) && ! defined(linux)
			if (termination_signal > NSIG) {
#endif
				(void) fprintf(stdout,
					       catgets(libmksdmsi18n_catd, 1, 105, "*** Signal %d"),
					       termination_signal);
				SEND_MTOOL_MSG(
					(void) sprintf(&tmp_buf[strlen(tmp_buf)],
						       catgets(libmksdmsi18n_catd, 1, 106, "*** Signal %d"),
						       termination_signal);
				);
#if ! defined(SUN5_0) && ! defined(HP_UX) && ! defined(linux)
			} else {
				(void) fprintf(stdout,
					       "*** %s",
					       sys_siglist[termination_signal]);
				SEND_MTOOL_MSG(
					(void) sprintf(&tmp_buf[strlen(tmp_buf)],
						       "*** %s",
						       sys_siglist[termination_signal]);
				);
			}
#endif
			if (core_dumped) {
				(void) fprintf(stdout,
					       catgets(libmksdmsi18n_catd, 1, 107, " - core dumped"));
				SEND_MTOOL_MSG(
					(void) sprintf(&tmp_buf[strlen(tmp_buf)],
						       catgets(libmksdmsi18n_catd, 1, 108, " - core dumped"));
				);
			}
		}
		if (ignore_error) {
			(void) fprintf(stdout,
				       catgets(libmksdmsi18n_catd, 1, 109, " (ignored)"));
			SEND_MTOOL_MSG(
				(void) sprintf(&tmp_buf[strlen(tmp_buf)],
					       catgets(libmksdmsi18n_catd, 1, 110, " (ignored)"));
			);
		}
		(void) fprintf(stdout, "\n");
		(void) fflush(stdout);
		SEND_MTOOL_MSG(
			make_output_msg->appendOutput(AVO_STRDUP(tmp_buf));
		);
	}
	SEND_MTOOL_MSG(
		xdr_msg = (RWCollectable*) make_output_msg;
		xdr(xdrs_p, xdr_msg);
		delete make_output_msg;
	);

#ifdef PRINT_EXIT_STATUS
	warning_mksh(NOCATGETS("I'm in await(), returning failed."));
#endif

	return (OPR_FAILURE);
}

/*
 *	sh_command2string(command, destination)
 *
 *	Run one sh command and capture the output from it.
 *
 *	Return value:
 *
 *	Parameters:
 *		command		The command to run
 *		destination	Where to deposit the output from the command
 *
 *	Static variables used:
 *
 *	Global variables used:
 */
void
sh_command2string(String command, String destination)
{
	FILE		*fd;
	int		chr;
	int			status;
	boolean_t command_generated_output = B_FALSE;

	command->text.p = (int) nul_char;
	WCSTOMBS(mbs_buffer, command->buffer.start);
	if ((fd = popen(mbs_buffer, "r")) == NULL) {
		WCSTOMBS(mbs_buffer, command->buffer.start);
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 111, "Could not run command `%s' for :sh transformation"),
		      mbs_buffer);
	}
	while ((chr = getc(fd)) != EOF) {
		if (chr == (int) newline_char) {
			chr = (int) space_char;
		}
		command_generated_output = B_TRUE;
		append_char(chr, destination);
	}

	/*
	 * We don't want to keep the last LINE_FEED since usually
	 * the output of the 'sh:' command is used to evaluate
	 * some MACRO. ( /bin/sh and other shell add a line feed
	 * to the output so that the prompt appear in the right place.
	 * We don't need that
	 */
	if (command_generated_output){
		if ( *(destination->text.p-1) == (int) space_char) {
			* (-- destination->text.p) = '\0';
		}
	} else {
		/*
		 * If the command didn't generate any output,
		 * set the buffer to a null string.
		 */
		*(destination->text.p) = '\0';
	}

	status = pclose(fd);
	if (status != 0) {
		WCSTOMBS(mbs_buffer, command->buffer.start);
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 112, "The command `%s' returned status `%d'"),
		      mbs_buffer,
		      WEXITSTATUS(status));
	}
}


