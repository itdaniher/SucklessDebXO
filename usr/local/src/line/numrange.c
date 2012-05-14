/*
 *  iterator for number range patterns like 1-3,5,7-9
 *  format is like -o option to n/troff, or my (prl's) "line" program
 *  code derived (barely) from my "line" program
 *  scs 12/9/2000
 *
 *  placed in the Public Domain 2002-01-05
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "numrange.h"

#define TRUE 1
#define FALSE 0

struct range_iter
	{
	NR_TYPE nexti;
	NR_TYPE firsti, lasti;	/* in current segment */
	char *nextp;
	int iflags;	/* NR_OPNBEG, NR_OPNEND, NR_SINGLETON */
	int eflags;	/* NR_SCATTER, NR_BIDIR */
	char range[1];	/* struct hack */
	};

/*
 * "For historical reasons", the range_iter structure doesn't store
 *  a parsed representation of the entire input set of ranges.
 *  It stores the bounds of the current segment in firsti and lasti,
 *  but the other segments are represented only in the `range' array,
 *  which is a copy of the original input specification string.
 *  Therefore, whenever we advance into the next segment,
 *  or rewind to the beginning, a call to reparse() is required.
 */

static int reparse();
static int checkrange();
static struct numrange *nexthunk();

/* mk_range_iter: parse the string `desc' and return a range_iter structure */

struct range_iter *
mk_range_iter(desc, flags)
char *desc;
int flags;
{
char *p = desc;
struct range_iter *rp;

rp = malloc(sizeof(struct range_iter) - sizeof(rp->range) + strlen(desc) + 1);

if(rp == NULL)
	{
	/* verbose error message? */
	return NULL;
	}

rp->iflags = 0;
rp->eflags = flags;
strcpy(rp->range, desc);
rp->nextp = rp->range;

if(!checkrange(rp) || !reparse(rp))
	{
	free(rp);
	return NULL;
	}

return rp;
}

/* add_range_iter: parse `desc' and use it to augment the range oldrp */

struct range_iter *
add_range_iter(oldrp, desc)
struct range_iter *oldrp;
char *desc;
{
char *p = desc;
struct range_iter *newrp;
int oldlen = strlen(oldrp->range);
int nextpoff = oldrp->nextp - oldrp->range;

newrp = realloc(oldrp, sizeof(struct range_iter) - sizeof(oldrp->range) +
						oldlen + 1 + strlen(desc) + 1);

if(newrp == NULL)
	{
	/* verbose error message? */
	return NULL;
	}

newrp->range[oldlen] = ',';
strcpy(&newrp->range[oldlen+1], desc);

newrp->nextp = newrp->range + nextpoff;

/* could theoretically only check new bit, but it's harder */
if(!checkrange(newrp))
	{
	/* XXX prev. not necessarily still allocated */
	return NULL;
	}

if(newrp->nextp == &newrp->range[oldlen])
	newrp->nextp++;		/* XXX over the , we just added */

return newrp;
}

/* do_range_iter: actually run the iterator rp, returning successive */
/* values with each call.  *statp is set to FALSE on completion. */

NR_TYPE
do_range_iter(rp, statp)
struct range_iter *rp;
int *statp;
{
int ret = rp->nexti;
int inc = ((rp->iflags & (NR_OPNBEG|NR_OPNEND)) ||
	rp->lasti >= rp->firsti) ? 1 : -1;

*statp = TRUE;

if(inc > 0 && (rp->nexti <= rp->lasti || (rp->iflags & NR_OPNEND)))
	/* okay; do nothing */;
else if(inc < 0 && rp->nexti >= rp->lasti)
	/* okay; do nothing */;
else if(reparse(rp))
	{
	ret = rp->nexti;
	inc = ((rp->iflags & (NR_OPNBEG|NR_OPNEND)) ||
		rp->lasti >= rp->firsti) ? 1 : -1;
	}
else	{
	*statp = FALSE;
	return 0;
	}

rp->nexti += inc;

return ret;
}

/* rst_range_iter: reset iterator rp for more scans */

void
rst_range_iter(rp)
struct range_iter *rp;
{
rp->nextp = rp->range;
reparse(rp);
}

/* in_range_iter: return TRUE if `i' is in any range pointed to by rp */

int
in_range_iter(i, rp)
NR_TYPE i;
struct range_iter *rp;
{
if(i < rp->firsti)
	{
#ifdef DEBUG
	puts("in_range_iter: rewinding");
#endif
	rp->nextp = rp->range;
	reparse(rp);
	}

rp->nexti = i + 1;

do	{
	if((i >= rp->firsti || (rp->iflags & NR_OPNBEG)) &&
				(i <= rp->lasti || (rp->iflags & NR_OPNEND)))
		return TRUE;
	} while(reparse(rp) && rp->firsti <= i);

return FALSE;
}

/* in2_range_iter: return TRUE if interval [n2, n2] overlaps */
/* any range pointed to by rp */

int
in2_range_iter(n1, n2, rp)
NR_TYPE n1, n2;
struct range_iter *rp;
{
if(n1 < rp->firsti)
	{
#ifdef DEBUG
	puts("in_range_iter: rewinding");
#endif
	rp->nextp = rp->range;
	reparse(rp);
	}

rp->nexti = n2 + 1;	/* ? */

do	{
	if(rp->lasti < n1 && !(rp->iflags & NR_OPNEND))
		/* can't possibly */;
	else if(rp->firsti > n2 && !(rp->iflags & NR_OPNBEG))
		/* can't possibly */;
	else	{
		/* it does! */
#if 0
		assert(rp->firsti >= n1 && rp->firsti <= n2 ||
			rp->lasti >= n1 && rp->lasti <= n2 ||
			rp->firsti < n1 && rp->lasti > n2);
#endif
		return TRUE;
		}
	} while(reparse(rp) && rp->firsti <= n2);

return FALSE;
}

/* get_cur_range: return the current interval which rp has advanced to */
/* (can be used as an iterator, with skip_next_range) */

const struct numrange *
get_cur_range(rp)
struct range_iter *rp;
{
static struct numrange retbuf;
retbuf.nr_first = rp->firsti;
retbuf.nr_cur = rp->nexti - 1;
retbuf.nr_last = rp->lasti;
retbuf.nr_flags = rp->iflags & (NR_OPNBEG | NR_OPNEND | NR_SINGLETON);
return &retbuf;
}

int
get_num_ranges(rp)
struct range_iter *rp;
{
int n = 0;
char *p = rp->range;
char *p2;

do	{
	if(nexthunk(rp, p, &p2) == NULL)
		return -1;
	n++;
	p = p2;
	if(*p == ',')
		p++;
	else if(*p != '\0')
		return -1;
	} while(*p != '\0');

return n;
}

/* skip_next_range: advance rp to the next range */
/* (can be used as an iterator, with get_cur_range) */

int
skip_next_range(rp)
struct range_iter *rp;
{
if(*rp->nextp == '\0')
	return FALSE;
if(!reparse(rp))
	return FALSE;
return TRUE;
}

/* done_range_iter: return TRUE if rp is exhausted (nothing more could match) */

int
done_range_iter(rp)
struct range_iter *rp;
{
return rp->nexti > rp->lasti && !(rp->iflags & NR_OPNEND) && *rp->nextp == '\0';
}

static struct numrange *
nexthunk(rp, p, endp)
struct range_iter *rp;
char *p;
char **endp;
{
char *p2;
static struct numrange retbuf;

retbuf.nr_first = 1;		/* default */
retbuf.nr_flags = 0;

if(*p == '-')
	retbuf.nr_flags |= NR_OPNBEG;
else	{
	retbuf.nr_first = strtol(p, &p2, 10);
	if(p2 == p)
		return NULL;		/* syntax error (bad/missing #) */
	p = p2;
	retbuf.nr_last = retbuf.nr_first;		/* in case singleton */
	retbuf.nr_flags |= NR_SINGLETON;
	}

if(*p == '-' || (rp->eflags & NR_RELATIVE) && *p == ':')
	{
	char c = *p;
	p++;
	if(*p == ',' || *p == '\0')
		{
		if((retbuf.nr_flags & NR_OPNBEG) || c == ':')
			return NULL;	/* syntax error (lone -) */
		retbuf.nr_flags |= NR_OPNEND;
		}
	else	{
		retbuf.nr_last = strtol(p, &p2, 10);
		if(p2 == p)
			return NULL;	/* syntax error (bad/missing #) */
		p = p2;
		if(c == ':')
			{
			assert(!(retbuf.nr_flags & NR_OPNBEG));
			retbuf.nr_last = retbuf.nr_first + retbuf.nr_last - 1;
			}
		}

	retbuf.nr_flags &= ~NR_SINGLETON;
	}

if(endp != NULL)
	*endp = p;

return &retbuf;
}

static int
reparse(rp)
struct range_iter *rp;
{
struct numrange *nr;
char *p;

nr = nexthunk(rp, rp->nextp, &p);

if(nr == NULL)
	return FALSE;

if(*p == ',')
	p++;
else if(*p != '\0')
	return FALSE;			/* syntax error */

rp->nexti = rp->firsti = nr->nr_first;
rp->lasti = nr->nr_last;
rp->iflags = nr->nr_flags;
rp->nextp = p;

return TRUE;
}

static int
checkrange(rp)
struct range_iter *rp;
{
char *p = rp->range;
char *p2;
struct numrange *nr;
struct numrange prev;
int haveprev = FALSE;

do	{
	nr = nexthunk(rp, p, &p2);
	if(nr == NULL)
		return FALSE;

	if(nr->nr_last < nr->nr_first && !(rp->eflags & NR_BIDIR))
		{
		/* error: descending segment */
		return FALSE;
		}

	if(haveprev && !(rp->eflags & NR_SCATTER) &&
		((prev.nr_flags & NR_OPNEND) || nr->nr_first <= prev.nr_last))
		{
		/* error: overlapping range */
		return FALSE;
		}

	prev = *nr;
	haveprev = TRUE;

	p = p2;

	if(*p == ',')
		p++;
	else if(*p != '\0')
		return FALSE;
	} while(*p != '\0');	/* hmm... allows trailing , */

return TRUE;
}

#ifdef MAIN

#include <stdio.h>

/* this harness tests in_range_iter(), in2_range_iter(), and done_range_iter() */
/* harness for do_range_iter() is count.c */

int main()
{
char range[100];
char num[100];
struct range_iter *rp;
char *p;
int n1, n2;
int r;

while(TRUE)
	{
	printf("range? "); fflush(stdout);
	if(fgets(range, sizeof(range), stdin) == NULL)
		break;
	if((p = strrchr(range, '\n')) != NULL) *p = '\0';
	if((rp = mk_range_iter(range, 0)) == NULL)
		{
		printf("bad\n");
		continue;
		}

	while(TRUE)
		{
		printf("num? "); fflush(stdout);
		if(fgets(num, sizeof(num), stdin) == NULL)
			break;
		if((p = strrchr(num, '\n')) != NULL) *p = '\0';
		if(strcmp(num, "done") == 0)
			{
			printf("%s\n", done_range_iter(rp) ? "yes" : "no");
			continue;
			}
		if(sscanf(num, "%d-%d", &n1, &n2) == 2)
			r = in2_range_iter(n1, n2, rp);
		else	r = in_range_iter(atoi(num), rp);
		printf("%s\n", r ? "yes" : "no");
		}
	}

return 0;
}

#endif

#ifdef MAIN2

#include <stdio.h>

/* this harness tests get_cur_range and skip_next_range, used as an iterator */

int main()
{
char range[100];
struct range_iter *rp;
char *p;

while(TRUE)
	{
	printf("range? "); fflush(stdout);
	if(fgets(range, sizeof(range), stdin) == NULL)
		break;
	if((p = strrchr(range, '\n')) != NULL) *p = '\0';
	if((rp = mk_range_iter(range, 0)) == NULL)
		{
		printf("bad\n");
		continue;
		}

	do	{
		const struct numrange *nrp = get_cur_range(rp);
		printf("%d - %d (%d)\n",
				nrp->nr_first, nrp->nr_last, nrp->nr_flags);
		} while(skip_next_range(rp));
	}

return 0;
}

#endif
