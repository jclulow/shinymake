

#ifndef	_MKSH_CDEFS_H_
#define	_MKSH_CDEFS_H_

typedef enum op_result {
	OPR_FAILURE = 0,
	OPR_SUCCESS = 1
} op_result_t;

/*
 * string_t defines a string with the following layout:
 *	"xxxxxxxxxxxxxxxCxxxxxxxxxxxxxxx________"
 *	 ^              ^               ^       ^
 *	 |              |               |        \- str_buf_end
 *	 str_buf_start  str_p           str_end
 *
 *	str_p points to the next char to read/write.
 *
 *	str_buf_length is the number of wchar_t that
 *	  will fit in the buffer -- i.e. the byte length
 *	  divided by (sizeof (wchar_t)).
 */
typedef struct _string {
	wchar_t	*str_p;		/* Read/Write Pointer */
	wchar_t	*str_end;	/* Read Limit Pointer */

	/*
	 * Underlying Physical Buffer:
	 */
	wchar_t *str_buf_start;	/* Start of Buffer */
	wchar_t	*str_buf_end;	/* One Past End of Buffer */

	boolean_t str_free_after_use;
} string_t;

#define	STRING_BUFFER_LENGTH	1024
#define	INIT_STRING_FROM_STACK(str, buf) 			\
	do {							\
		str.str_buf_start = (buf); 			\
		str.str_buf_end = (buf) +			\
		    (sizeof (buf) / SIZEOFWCHAR_T);		\
		str.str_p = (buf);				\
		str.str_end = NULL;				\
		str.str_free_after_use = B_FALSE;		\
	} while (0)

#if 0
#define	APPEND_NAME(np, dest, len)				\
	append_string((np)->string_mb, (dest), (len));
#endif

typedef struct _wstring {
	string_t	wstr_str;
	wchar_t		wstr_buf[STRING_BUFFER_LENGTH];
} wstring_t;


/*
 * The specials are markers for targets that the reader should special
 * case:
 */
typedef enum special {
	SPC_NONE = 0,
	SPC_BUILT_LAST_MAKE_RUN,
	SPC_DEFAULT,
	SPC_GET_POSIX,
	SPC_GET,
	SPC_IGNORE,
	SPC_KEEP_STATE_FILE,
	SPC_KEEP_STATE,
	SPC_MAKE_VERSION,
	SPC_NO_PARALLEL,
	SPC_PARALLEL,
	SPC_POSIX,
	SPC_PRECIOUS,
	SPC_SCCS_GET_POSIX,
	SPC_SCCS_GET,
	SPC_SILENT,
	SPC_SUFFIXES,
	SPC_SVR4,
	SPC_LOCALHOST
} special_t;

typedef enum doname {
	DON_DONT_KNOW = 0,
	DON_FAILED,
	DON_OK,
	DON_IN_PROGRESS,
	DON_RUNNING,
	DON_PENDING,
	DON_SERIAL,
	DON_SUBTREE
} doname_t;

typedef enum separator {
	SEP_UNKNOWN = 0,
	SEP_NO_COLON,
	SEP_ONE_COLON,
	SEP_TWO_COLON,
	SEP_EQUAL_SEEN,
	SEP_CONDITIONAL_SEEN,
	SEP_NONE_SEEN
} separator_t;

typedef enum makefile_type {
	MFT_READING_NOTHING = 0,
	MFT_READING_MAKEFILE,
	MFT_READING_STATEFILE,
	MFT_REREADING_STATEFILE,
	MFT_READING_CPP_FILE
} makefile_type_t;

typedef enum daemon {
	DN_NO_DAEMON = 1,
	DN_CHAIN_DAEMON
} daemon_t;

/*
 * Bits stored in the character classification vector, for some reason
 * previously called the "funny vector". Ha ha.
 */
typedef enum char_class {
	CHC_NONE = 0x00,
	CHC_DOLLAR = 0x01,
	CHC_META = 0x02,
	CHC_PERCENT = 0x04,
	CHC_WILDCARD = 0x08,
	CHC_COMMAND_PREFIX = 0x10,
	CHC_SPECIAL_MACRO = 0x20,
	CHC_COLON = 0x40,
	CHC_PARENLEFT = 0x80
} char_class_t;

typedef enum sccs_stat {
	SCCS_DONT_KNOW = 0,
	SCCS_NOT_PRESENT,
	SCCS_PRESENT
} sccs_stat_t;

typedef struct name_stat {
	timestruc_t	ns_time;           /* Modification */
	int		ns_stat_errno;     /* error from "stat" */
	off_t		ns_size;           /* Of file */
	mode_t		ns_mode;           /* Of file */
	boolean_t	ns_is_file;
	boolean_t	ns_is_dir;
	boolean_t	ns_is_sym_link;
	boolean_t	ns_is_precious;
	sccs_stat_t	ns_has_sccs;
} name_stat_t;

/*
 * Each name_t has a list of property_t.  Each property_t stores
 * information specific to a subset of name_t structs.  The type
 * of a property_t is identified by a property_id_t.
 */
typedef enum property_id {
	PT_NONE = 0,
	PT_CONDITIONAL,
	PT_LINE,
	PT_MACRO,
	PT_MAKEFILE,
	PT_MEMBER,
	PT_RECURSIVE,
	PT_SCCS,
	PT_SUFFIX,
	PT_TARGET,
	PT_TIME,
	PT_VPATH_ALIAS,
	PT_LONG_MEMBER_NAME,
	PT_MACRO_APPEND,
	PT_ENV_MEM
} property_id_t;

typedef struct property {
	struct property		*p_next;
	property_id_t		p_type;
	void			*p_body;
} property_t;

typedef struct macro_list {
	struct macro_list	*ml_next;
	char			*ml_macro_name;
	char			*ml_value;
} macro_list_t;

typedef struct name {
	/*
	 * The canonical key name for this name_t.
	 */
	wchar_t *n_key;
	size_t n_key_length;

	name_stat_t n_stat;

	/*
	 * List of properties.
	 */
	property_t *n_prop;

	/*
	 * Count instances of :: definitions for this target.
	 */
	short n_colon_splits;
	/*
	 * We only clear the automatic depes once per target per report.
	 */
	short n_temp_file_number;
	/*
	 * Count how many conditional macros this target has defined.
	 */
	short n_conditional_cnt;
	/*
	 * A conditional macro was used when building this target.
	 */
	boolean_t n_depends_on_conditional;
	/*
	 * Pointer to list of conditional macros which were used to build
	 * this target.
	 */
	macro_list_t *n_conditional_macro_list;
	boolean_t n_has_member_depe;
	boolean_t n_is_member;
	/*
	 * This target is a directory that has been read.
	 */
	boolean_t n_has_read_dir;
	/*
	 * This name is a macro that is now being expanded.
	 */
	boolean_t n_being_expanded;

	/*
	 * This name is a magic name that the reader must know about.
	 */
	special_t		n_special_reader;
	doname_t		n_state;
	separator_t		n_colons;

	boolean_t		n_has_depe_list_expanded;
	boolean_t		n_suffix_scan_done;
	boolean_t		n_has_complained;	/* For sccs */
	/*
	 * This target has been built during this make run
	 */
	boolean_t		n_ran_command;
	boolean_t		n_with_squiggle;	/* for .SUFFIXES */
	boolean_t		n_without_squiggle;	/* for .SUFFIXES */
	boolean_t		n_has_read_suffixes;	/* Suffix list cached*/
	boolean_t		n_has_suffixes;
	boolean_t		n_has_target_prop;
	boolean_t		n_has_vpath_alias_prop;
	boolean_t		n_dependency_printed;	/* For dump_make_state() */
	boolean_t		n_dollar;		/* In namestring */
	boolean_t		n_meta;			/* In namestring */
	boolean_t		n_percent;		/* In namestring */
	boolean_t		n_wildcard;		/* In namestring */
        boolean_t		n_has_parent;
        boolean_t		n_is_target;
	boolean_t		n_has_built;
	boolean_t		n_colon;		/* In namestring */
	boolean_t		n_parenleft;		/* In namestring */
	boolean_t		n_has_recursive_dependency;
	boolean_t		n_has_regular_dependency;
	boolean_t		n_is_double_colon;
	boolean_t		n_is_double_colon_parent;
	boolean_t		n_has_long_member_name;
	/*
	 * allowed to run in parallel
	 */
	boolean_t		n_parallel;
	/*
	 * not allowed to run in parallel
	 */
	boolean_t		n_no_parallel;
	/*
	 * used in dependency_conflict
	 */
	boolean_t		n_checking_subtree;
	boolean_t		n_added_pattern_conditionals;
	/*
	 * rechecking target for possible rebuild
	 */
	boolean_t		n_rechecking_target;
	/*
	 * build this target in silent mode
	 */
	boolean_t		n_silent_mode;
	/*
	 * build this target in ignore error mode
	 */
	boolean_t		n_ignore_error_mode;
	boolean_t		n_dont_activate_cond_values;
	/*
	 * allowed to run serially on local host
	 */
	boolean_t		n_localhost;
} name_t;

typedef struct cmd_line {
	struct cmd_line	*cl_next;
	name_t		*cl_cmd_line;
	boolean_t	cl_make_refd; /* $(MAKE) referenced? */
	/*
	 * Remember any command line prefixes given.
	 */
	boolean_t	cl_ignore_command_dependency;	/* `?' */
	boolean_t	cl_assign;			/* `=' */
	boolean_t	cl_ignore_error;		/* `-' */
	boolean_t	cl_silent;			/* `@' */
	boolean_t	cl_always_exec;			/* `+' */
} cmd_line_t;

typedef struct conditional {
	/*
	 * For "foo := ABC [+]= xyz" constructs.
	 * Name "foo" gets one conditional property.
	 */
	name_t		*c_target;
	name_t		*c_name;
	name_t		*c_value;
	int		c_sequence;
	boolean_t	c_append;
} conditional_t;

/*
 * Used for storing the $? list, and also for the "target + target:"
 * construct.
 */
typedef struct chain {
	struct chain	*ch_next;
	name_t		*ch_name;
	struct percent	*ch_percent_member;
} chain_t;

/*
 * Stores the %-matched default rules.
 */
typedef struct percent {
	struct percent	*pc_next;
	name_t		**pc_patterns;
	name_t		*pc_name;
	struct percent	*pc_dependencies;
	cmd_line_t	*pc_command_template;
	struct chain	*pc_target_group;
	size_t		pc_patterns_total;
	boolean_t	pc_being_expanded;
} percent_t;

/*
 * Linked list of targets/files.
 */
typedef struct dependency {
	struct dependency	*dp_next;
	name_t			*dp_name;
	boolean_t		dp_automatic;
	boolean_t		dp_stale;
	boolean_t		dp_built;
} dependency_t;

typedef struct line {
	/*
	 * For "target : dependencies" constructs.
	 * Name "target" gets one line_t property.
	 */
	cmd_line_t	*li_command_template;
	cmd_line_t	*li_command_used;
	dependency_t	*li_dependencies;
	timestruc_t	li_dependency_time;
	chain_t		*li_target_group;
	boolean_t	li_is_out_of_date;
	boolean_t	li_sccs_command;
	boolean_t	li_command_template_redefined;
	boolean_t	li_dont_rebuild_command_used;
	/*
	 * Values for the dynamic macros.
	 */
	name_t		*li_target;
	name_t		*li_star;
	name_t		*li_less;
	name_t		*li_percent;
	chain_t		*li_query;
} line_t;

typedef struct env_mem {
	char		*em_value;
} env_mem_t;

typedef struct envvar {
	name_t		*ev_name;
	name_t		*ev_value;
	struct envvar	*ev_next;
	char		*ev_env_string;
	boolean_t	already_put;
} envvar_t;

typedef struct makefile {
	/*
	 * Names that reference makefiles get one property.
	 */
	wchar_t		*mf_contents;
	size_t		mf_size;
} makefile_t;

/*
 * Structure for dynamic "ascii" arrays.
 */
typedef struct ascii_dyn_array {
	char		*ada_start;
	size_t		ada_size;
} ascii_dyn_array_t;

typedef struct member {
	/*
	 * For "lib(member)" and "lib((entry))" constructs.
	 * Name "lib(member)" gets one member_t property.
	 * Name "lib((entry))" gets one member_t property.
	 * The member field is filled in when the property is refd.
	 */
	name_t		*mm_library;
	name_t		*mm_entry;
	name_t		*mm_member;
} member_t;

typedef struct macro {
	/*
	 * Use if m_daemon == DN_NO_DAEMON, otherwise use m_chain:
	 */
	name_t		*m_value;
	boolean_t	m_exported;
	boolean_t	m_read_only;
	boolean_t	m_is_conditional;
	daemon_t	m_daemon;
	/*
	 * Use if m_daemon == DN_CHAIN_DAEMON:
	 */
	chain_t		*m_chain;
} macro_t;

typedef struct macro_appendix {
	name_t		*map_value;
	name_t		*map_value_to_append;
} macro_appendix_t;

typedef struct source {
	string_t	*src_str;
	struct source	*src_prev;
	off_t		src_bytes_left_in_file;
	int		src_fd;
	boolean_t	src_already_expanded;
	boolean_t	src_error_converting;
	char		*src_inp_buf;
	char		*src_inp_buf_end;
	char		*src_inp_buf_ptr;
} source_t;

typedef struct long_member_name {
	/*
	 * Targets with a truncated member name carry the full
	 * lib(member) name for the state file.
	 */
	name_t		*lmn_member_name;
} long_member_name_t;

typedef struct vpath_alias {
	/*
	 * If a file was found using the VPATH, it gets a vpath_alias_t
	 * property.
	 */
	name_t		*va_alias;
} vpath_alias_t;

typedef struct stime {
	timestruc_t	st_time;
} stime_t;

typedef struct target {
	/*
	 * For "target:: dependencies" constructs.
	 * The "::" construct is handled by converting it to:
	 *    "foo: 1@foo" + "1@foo: dependencies"
	 *
	 * Here,"1@foo" gets on target property.  This target property causes
	 * $@ to be bound to "foo", rather than "1@foo", when the rule is
	 * evaluated.
	 */
	name_t		*ta_name;
} target_t;

typedef struct suffix {
	/*
	 * Cached list of suffixes that can build this target.  Built
	 * from ".SUFFIXES".
	 */
	name_t		*sx_suffix;
	cmd_line_t	*sx_command_template;
} suffix_t;

typedef struct sccs {
	/*
	 * Each file that has a SCCS s. file gets one property.
	 */
	name_t		*sc_file;
} sccs_t;

typedef struct recursive {
	/*
	 * For "target: .RECURSIVE dir makefiles" constructs.
	 * Used to track recursive calls to make.
	 * Name "target" gets one recursive property.
	 */
	name_t		*rc_directory;
	name_t		*rc_target;
	dependency_t	*rc_makefiles;
	boolean_t	rc_has_built;
	boolean_t	rc_in_depinfo;
} recursive_t;





typedef struct name_set_entry {
	name_t *nse_name;
	struct name_set_entry *nse_next;
} name_set_entry_t;

typedef struct name_set {
	name_set_entry_t *nms_head;
} name_set_t;

#endif	/* _MKSH_CDEFS_H_ */
