

#include <strings.h>
#include <mksh/misc.h>
#include <mksh/cdefs.h>

/*
 * XXX REPLACE THIS TIMESTRUCT COMPARISON CODE?
 *        (FROM Make/include/mksh/defs.h)
 */
#if 0

/*
 * RFE 1257407: make does not use fine granularity time info available from stat.
 * High resolution time comparison.
 */

inline int
operator==(const timestruc_t &t1, const timestruc_t &t2) {
	return ((t1.tv_sec == t2.tv_sec) && (t1.tv_nsec == t2.tv_nsec));
}

inline int
operator!=(const timestruc_t &t1, const timestruc_t &t2) {
	return ((t1.tv_sec != t2.tv_sec) || (t1.tv_nsec != t2.tv_nsec));
}

inline int
operator>(const timestruc_t &t1, const timestruc_t &t2) {
	if (t1.tv_sec == t2.tv_sec) {
		return (t1.tv_nsec > t2.tv_nsec);
	}
	return (t1.tv_sec > t2.tv_sec);
}

inline int
operator>=(const timestruc_t &t1, const timestruc_t &t2) {
	if (t1.tv_sec == t2.tv_sec) {
		return (t1.tv_nsec >= t2.tv_nsec);
	}
	return (t1.tv_sec > t2.tv_sec);
}

inline int
operator<(const timestruc_t &t1, const timestruc_t &t2) {
	if (t1.tv_sec == t2.tv_sec) {
		return (t1.tv_nsec < t2.tv_nsec);
	}
	return (t1.tv_sec < t2.tv_sec);
}

inline int
operator<=(const timestruc_t &t1, const timestruc_t &t2) {
	if (t1.tv_sec == t2.tv_sec) {
		return (t1.tv_nsec <= t2.tv_nsec);
	}
	return (t1.tv_sec < t2.tv_sec);
}
#endif


/*
 * A malloc() wrapper that aborts if we cannot allocate.
 */
void *
xmalloc(size_t size)
{
	void *ret;

	if ((ret = malloc(size)) == NULL) {
		fprintf(stderr, "ERROR: could not get memory: %s\n",
		    strerror(errno));
		exit(1);
	}

	return (ret);
}

void *
zxmalloc(size_t size)
{
	void *ret = xmalloc(size);

	bzero(ret, size);

	return (ret);
}

char *
xstrdup(const char *s)
{
	char *ret;

	if ((ret = strdup(s)) == NULL) {
		fprintf(stderr, "ERROR: could not strdup: %s\n",
		    strerror(errno));
		exit(1);
	}

	return (ret);
}

wchar_t *
xwcsdup(const wchar_t *s)
{
	wchar_t *ret;

	if ((ret = wcsdup(s)) == NULL) {
		fprintf(stderr, "ERROR: could not wcsdup: %s\n",
		    strerror(errno));
		exit(1);
	}

	return (ret);
}

name_t *
name_alloc(const wchar_t *key)
{
	name_t *n = zxmalloc(sizeof (*n));

	n->n_key = xwcsdup(key);
	n->n_key_length = wcslen(key);

	return (n);
}

void
free_name(name_t *n)
{
	if (n == NULL)
		return;
	property_t *prop, *propn = NULL;
	free(n->n_key);
	for (prop = n->n_prop; prop != NULL; prop = propn) {
		propn = prop->p_next;
		free(prop);
	}
	free(n);
}

name_set_t *
nms_alloc(void)
{
	name_set_t *nms = zxmalloc(sizeof (*nms));

	return (nms);
}

name_t *
nms_lookup(name_set_t *nms, const wchar_t *key)
{
	name_set_entry_t *nse;

	for (nse = nms->nms_head; nse != NULL; nse = nse->nse_next) {
		if (wcscmp(nse->nse_name->n_key, key) == 0)
			return (nse->nse_name);
	}

	return (NULL);
}

/*
 * XXX NOTE that this structure is just a linked list right now,
 * but should probably use an AVL tree or whatever.
 */
static name_set_entry_t *
nse_alloc(name_t *n)
{
	name_set_entry_t *nse = zxmalloc(sizeof (*nse));;

	nse->nse_name = n;

	return (nse);
}

name_t *
nms_insert(name_set_t *nms, const wchar_t *key, boolean_t *found)
{
	name_t *n;
	name_set_entry_t *nse, *last = NULL;

	if (nms->nms_head == NULL) {
		*found = B_FALSE;
		n = name_alloc(key);
		nms->nms_head = nse_alloc(n);
		return (n);
	}

	for (nse = nms->nms_head; nse != NULL; nse = nse->nse_next) {
		last = nse;

		if (wcscmp(nse->nse_name->n_key, key) == 0) {
			*found = B_TRUE;
			return (nse->nse_name);
		}
	}

	*found = B_FALSE;
	n = name_alloc(key);
	last->nse_next = nse_alloc(n);
	return (n);
}

void
nms_insert_name(name_set_t *nms, name_t *n)
{
	name_set_entry_t *nse, *last = NULL;

	if (nms->nms_head == NULL) {
		nms->nms_head = nse_alloc(n);
		return;
	}

	for (nse = nms->nms_head; nse != NULL; nse = nse->nse_next) {
		last = nse;

		if (wcscmp(nse->nse_name->n_key, n->n_key) == 0) {
			/*
			 * XXX duplicate name should be an error.
			 */
		}
	}

	last->nse_next = nse_alloc(n);
}

name_t *
nms_next(name_set_t *nms, void **iterptr)
{
	name_set_entry_t **ip = (name_set_entry_t **)iterptr;

	/*
	 * Begin or Advance the Walk:
	 */
	if (*ip == NULL) {
		*ip = nms->nms_head;
	} else {
		*ip = (*ip)->nse_next;
	}

	if (*ip == NULL)
		return (NULL);
	else
		return ((*ip)->nse_name);
}

wchar_t *
wchar_alloc(size_t length)
{
	wchar_t *ret = (wchar_t *)xmalloc(length * sizeof (wchar_t));

	ret[0] = L'\0'; /* '\0' */

	return (ret);
}

/*
 * Allocate more memory for strings that run out of space.
 */
static void
expand_string(string_t *s, size_t length)
{
	wchar_t *p;

	if (s->str_buf_start == NULL) {
		/*
		 * This string has no buffer allocated.
		 */
		s->str_buf_start = wchar_alloc(length);
		s->str_buf_end = s->str_buf_start + length;
		s->str_free_after_use = B_TRUE;

		s->str_p = s->str_end = s->str_buf_start;

		return;
	}

	if (s->str_buf_start + length <= s->str_buf_end) {
		/*
		 * We don't really need more memory.
		 */
		return;
	}

	/*
	 * Get more memory, copy the string and free the old b uffer if
	 * it came from malloc().
	 */
	p = wchar_alloc(length);
	(void) wcscpy(p, s->str_buf_start);

	s->str_p = p + (s->str_p - s->str_buf_start);
	s->str_end = p + (s->str_end - s->str_buf_start);

	if (s->str_free_after_use == B_TRUE)
		free(s->str_buf_start);

	s->str_buf_start = p;
	s->str_buf_end = p + length;
	s->str_free_after_use = B_TRUE;
}

static void
append_string_common(string_t *to, size_t length)
{
	if (to->str_buf_start == NULL) {
		expand_string(to, 32 + length);
	}
	if (to->str_p + length >= to->str_buf_end) {
		expand_string(to, (to->str_buf_end - to->str_buf_start) * 2 +
		    length);
	}
}

/*
 * Append a wide C string to a Make string, expanding it if necessary.
 */
void
append_string_wide(wchar_t *from, string_t *to, size_t length)
{
	if (length == FIND_LENGTH)
		length = wcslen(from);

	append_string_common(to, length);

	if (length > 0) {
		(void) wcsncpy(to->str_p, from, length);
		to->str_p += length;
	}
	*(to->str_p) = L'\0';
}

/*
 * Append a name_t's key to a Make string, expanding it if necessary.
 */
void
append_string_name(name_t *from, string_t *to)
{
	append_string_wide(from->n_key, to, FIND_LENGTH);
}

/*
 * Append a single character to a Make string, expanding it if necessary.
 */
void
append_char(wchar_t from, string_t *to)
{
	if (to->str_buf_start == NULL)
		expand_string(to, 32);
	if (to->str_buf_end - to->str_p <= 2)
		expand_string(to, to->str_buf_end - to->str_buf_start + 32);

	*(to->str_p)++ = from;
	*(to->str_p) = L'\0';
}

/*
 * Append a C string to a Make string, expanding it if necessary.
 */
void
append_string(char *from, string_t *to, size_t length)
{
	if (length == FIND_LENGTH)
		length = strlen(from);

	append_string_common(to, length);

	if (length > 0) {
		(void) mbstowcs(to->str_p, from, length);
		to->str_p += length;
	}
	*(to->str_p) = L'\0';
}

char *
extract_cstring(wchar_t *start, size_t length)
{
	size_t ipos = 0, opos = 0;
	size_t sz = 32 * MB_CUR_MAX;
	char *out = realloc(NULL, sz);
	boolean_t done = B_FALSE;

	while (done == B_FALSE) {
		size_t dlt;

		if (opos + MB_CUR_MAX + 1 >= sz) {
			sz *= 2;
			out = realloc(out, sz);
		}

		if (length != FIND_LENGTH && ipos >= length) {
			/*
			 * End the string.
			 */
			dlt = wcrtomb(&out[opos], L'\0', NULL);
			done = B_TRUE;
		} else {
			dlt = wcrtomb(&out[opos], start[ipos], NULL);
			if (start[ipos] == L'\0')
				done = B_TRUE;
		}
		if (dlt == (size_t)-1) {
			/*
			 * XXX
			 */
			fprintf(stderr, "ERROR: string encoding: %s\n",
			    strerror(errno));
			exit(34);
		}
		opos += dlt;
		ipos++;
	}

	return (out);
}

source_t *
source_from_wchar(wchar_t *in)
{
	source_t *src = zxmalloc(sizeof (*src));
	size_t len = wcslen(in);

	src->src_fd = -1;

	append_string_wide(in, &src->src_str, len);
	src->src_str.str_p = src->src_str.str_buf_start;
	src->src_str.str_end = src->src_str.str_p + len;

	return (src);
}

wchar_t
source_popchar(source_t *src)
{
	wchar_t ret = source_peekchar(src);

	if (ret != L'\0')
		src->src_str.str_p++;

	return (ret);
}

wchar_t
source_peekchar(source_t *src)
{
	if (src == NULL)
		return (L'\0');

	if (src->src_str.str_p >= src->src_str.str_end)
		return (L'\0');

	return (*(src->src_str.str_p));
}

wstring_t *
wstr_alloc(name_t *n)
{
	wstring_t *wstr = xmalloc(sizeof (*wstr));

	/*
	 * Use the in-built buffer by default:
	 */
	wstr->wstr_str.str_buf_start = wstr->wstr_buf;
	wstr->wstr_str.str_buf_end = wstr->wstr_str.str_buf_start +
	    STRING_BUFFER_LENGTH;
	wstr->wstr_str.str_free_after_use = B_FALSE;

	wstr->wstr_str.str_p = wstr->wstr_str.str_end =
	    wstr->wstr_str.str_buf_start;
	wstr->wstr_str.str_p[0] = L'\0';

	if (n != NULL)
		append_string_wide(n->n_key, &wstr->wstr_str, FIND_LENGTH);

	return (wstr);
}

size_t
property_body_size(property_id_t type)
{
	switch (type) {
	case PT_CONDITIONAL:
		return (sizeof (conditional_t));
	case PT_LINE:
		return (sizeof (line_t));
	case PT_MACRO:
		return (sizeof (macro_t));
	case PT_MAKEFILE:
		return (sizeof (makefile_t));
	case PT_MEMBER:
		return (sizeof (member_t));
	case PT_RECURSIVE:
		return (sizeof (recursive_t));
	case PT_SCCS:
		return (sizeof (sccs_t));
	case PT_SUFFIX:
		return (sizeof (suffix_t));
	case PT_TARGET:
		return (sizeof (target_t));
	case PT_TIME:
		return (sizeof (stime_t));
	case PT_VPATH_ALIAS:
		return (sizeof (vpath_alias_t));
	case PT_LONG_MEMBER_NAME:
		return (sizeof (long_member_name_t));
	case PT_MACRO_APPEND:
		return (sizeof (macro_appendix_t));
	case PT_ENV_MEM:
		return (sizeof (env_mem_t));
	default:
		fprintf(stderr, "INTERNAL ERROR: UNKNOWN PROP TYPE %d\n",
		   type);
		abort();
#if 0
		fatal_mksh(catgets(libmksdmsi18n_catd, 1, 136,
		    "Internal error. Unknown prop type %d"), type);
#endif
		return (0);
	}
}
