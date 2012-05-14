/*
 *  Program for extracting selected lines (by line number) from text files.
 *  (Roughly equivalent to "sed -n -e 3p -e 5p", etc.)
 *  Inspired by a program of the same name and basic functionality
 *  conceived by Philip Lantz at Tektronix MDP circa 1985.
 *
 *  Can also be compiled (#ifdef EXTRACT) to extract byte ranges,
 *  rather than lines.
 *
 *  Steve Summit, scs@eskimo.com
 *
 *  Placed in the Public Domain 2002-01-05.
 *
 *  See http://www.eskimo.com/~scs/src/#line for possible updates.
 */

#include <stdio.h>
#include <stdlib.h>
#ifndef NOSTRERROR
#include <string.h>
#include <errno.h>
#endif
#include <ctype.h>
#include <assert.h>

#define NR_TYPE long int
#include "numrange.h"

#ifdef notdef
#ifdef __STDC__
#define SAFEREALLOC
#endif
#endif

#define Ctod(c)  ((c) - '0')

#define TRUE 1
#define FALSE 0

#ifdef EXTRACT
#ifndef RANDOMACCESS
#define RANDOMACCESS
#endif
#endif

char *linefmt = "%6ld: ";
int exitstatus = 0;

#ifdef EXTRACT
char *progname = "extract";
char usage[] = "usage: %s [-rv] offsets [files]\n";
#else
char *progname = "line";
char usage[] = "usage: %s [-nrv] lines [files]\n";
#endif

#define VERSION "2.3b"	/* arbitrary */

#ifndef EXTRACT
int nflag = FALSE;
#endif
int vflag = FALSE;

#ifdef EXTRACT
int canseek = FALSE;			/* XXX awkward */
#endif

main(argc, argv)
int argc;
char *argv[];
{
int argi;
char *p;
long int line1, line2;
int haveline1 = FALSE;
int ftseen = FALSE;
char tmpbuf[30];
int rangeflags = NR_RELATIVE;
struct range_iter *rp = NULL;
#ifdef EXTRACT
const struct numrange *cp;
#endif
FILE *fd;
#ifdef RANDOMACCESS
int random = FALSE;
#endif
int printedhelp = FALSE;

for(argi = 1; argi < argc; argi++)
	{
	if(argv[argi][0] == '-' && !isdigit(argv[argi][1]))
		{
		for(p = &argv[argi][1]; *p != '\0'; p++)
			{
			if(p == &argv[argi][1] && *p == '-')
				{
				/* make --longname work as expected */
				continue;
				}

			switch(*p)
				{
				case '?':
printhelp:				printf(usage, progname);
#ifdef EXTRACT
					printf("or     %s [-rv] -s seekoff -t toseek [files]\n", progname);
					printf("or     %s charcount [files]\n", progname);
#else
					printf("or     %s [-nrv] -f fromline -t toline [files]\n", progname);
#endif
					printf("options:\n");
#ifndef EXTRACT
					printf("\t-n\tnumber lines\n");
#endif
#ifdef RANDOMACCESS
					printf("\t-r\trandom access\n");
#endif
					printf("\t-v\tinvert (print %s not selected)\n",
#ifdef EXTRACT
								"bytes"
#else
								"lines"
#endif
									);
					printedhelp = TRUE;
					break;

				case 'f':
					if(argi+1 >= argc)
						{
						fprintf(stderr,
					       "%s: missing value after -%c\n",
								progname, *p);
						/* should probably abort */
						break;
						}
					if(haveline1)
						fprintf(stderr,
						     "%s: two -f with no -t\n",
								progname);
					line1 = atol(argv[++argi]);
					haveline1 = TRUE;
					ftseen = TRUE;
					break;

				case 'h':
					if(strcmp(p, "help") == 0)
						p = "x";    /* short circuit */
					goto printhelp;
#ifndef EXTRACT
				case 'n':
					nflag = TRUE;
					break;
#endif
#ifdef RANDOMACCESS
				case 'r':
					random = TRUE;
					rangeflags |= NR_SCATTER;
					break;
#endif
#ifdef EXTRACT
				case 's':
					if(argi+1 >= argc)
						{
						fprintf(stderr,
					       "%s: missing value after -%c\n",
								progname, *p);
						/* should probably abort */
						break;
						}
					if(haveline1)
						fprintf(stderr,
						     "%s: two -s with no -t\n",
								progname);
					line1 = atol(argv[++argi])+1;
					haveline1 = TRUE;
					ftseen = TRUE;
					break;
#endif
				case 't':
				case 'e':
				case 'p':
					if(argi+1 >= argc)
						{
						fprintf(stderr,
					       "%s: missing value after -%c\n",
								progname, *p);
						/* should probably abort */
						break;
						}
					line2 = atol(argv[++argi]);
					if(*p == 'e')
						line2--;
					else if(*p == 'p')
						{
						if(!haveline1)
							{
							fprintf(stderr,
							 "%s: -p without -f\n",
								progname);

							break;
							}
						line2 = line1 + line2 - 1;
						}
					if(haveline1)
						sprintf(tmpbuf, "%ld-%ld",
								line1, line2);
					else	sprintf(tmpbuf, "-%ld", line2);
					if(rp == NULL)
						rp = mk_range_iter(tmpbuf, rangeflags);
					else	rp = add_range_iter(rp, tmpbuf);
					if(rp == NULL)
						{
						fprintf(stderr,
					"%s: error parsing/allocating range\n",
								progname);
						exit(1);
						}
					haveline1 = FALSE;
					ftseen = TRUE;
					break;

				case 'v':
					if(strcmp(p, "version") == 0)
						{
						printf("%s version %s\n",
							progname, VERSION);
						printedhelp = TRUE;
						p = "x";    /* short circuit */
						break;
						}
					vflag = TRUE;
					break;

				default:
badopt:					fprintf(stderr, "%s: unknown option -%c\n",
								progname, *p);
				}
			}
		}
	else if(!ftseen && (isdigit(argv[argi][0]) ||
			argv[argi][0] == '-' && argv[argi][1] != '\0'))
		{
		if(rp == NULL)
			rp = mk_range_iter(argv[argi], rangeflags);
		else	rp = add_range_iter(rp, argv[argi]);
		if(rp == NULL)
			{
			fprintf(stderr, "%s: error parsing/allocating range\n",
								progname);
			exit(1);
			}
		}
	else	break;
	}

if(haveline1)
	{
	sprintf(tmpbuf, "%ld-", line1);
	if(rp == NULL)
		rp = mk_range_iter(tmpbuf, rangeflags);
	else	rp = add_range_iter(rp, tmpbuf);
	if(rp == NULL)
		{
		fprintf(stderr, "%s: error parsing/allocating range\n",
								progname);
		exit(1);
		}
	}

if(rp == NULL)
	{
	if(printedhelp)
		return 0;
	fprintf(stderr, usage, progname);
	exit(1);
	}

#ifdef EXTRACT

/* special case: extract n is extract 1-n */
/* (use n-n to get just n) */

if(get_num_ranges(rp) == 1 && (cp = get_cur_range(rp)) != NULL &&
						(cp->nr_flags & NR_SINGLETON))
	{
	free(rp);
	sprintf(tmpbuf, "1-%ld", cp->nr_first);
	rp = mk_range_iter(tmpbuf, rangeflags);
	if(rp == NULL)
		{
		fprintf(stderr, "%s: error parsing/allocating range\n",
								progname);
		exit(1);
		}
	}

#endif

#ifdef RANDOMACCESS
if(random && vflag)
	{
	fprintf(stderr, "%s: -r and -v are incompatible\n", progname);
	exit(1);
	}
#endif

if(argi >= argc)
	{
	if(printedhelp)
		return 0;
#ifdef RANDOMACCESS
	if(random)
		{
		fprintf(stderr, "%s: can't use -r with standard input\n",
								progname);
		/* well, *maybe* we could, if simple stdin redirect from file... */
		exit(1);
		}
#endif
	scan("standard input", stdin, rp);
	}
else for(; argi < argc; argi++)
	{
	if(strcmp(argv[argi], "-") == 0)
		fd = stdin;
	else if((fd = fopen(argv[argi], "r")) == NULL)
		{
		fprintf(stderr, "%s: can't open %s: ", progname, argv[argi]);
		perror("");
		exitstatus = 1;
		continue;
		}

#ifdef EXTRACT
	canseek = TRUE;			/* XXX awkward */
#endif

#ifdef RANDOMACCESS
	if(random)
		randscan(argv[argi], fd, rp);
	else
#endif
		scan(argv[argi], fd, rp);

	if(fd != stdin)
		fclose(fd);
	else	clearerr(fd);

	/*
	 *  Third-order minor bug, which I'm not gonna worry about now:
	 *  If you say
	 *	line 1-3 - -
	 *  it starts reading the second - right away, without waiting for EOF.
	 */

	rst_range_iter(rp);
#ifdef RANDOMACCESS
#ifndef EXTRACT
	if(random)
		flushlines();	/* arguably belongs at end of randscan() */
#endif
#endif
	}

exit(exitstatus);
}

#ifndef EXTRACT

scan(file, fd, rp)
char *file;
FILE *fd;
struct range_iter *rp;
{
register long int lineno;
register int c;

lineno = 1;

do	{
	if(in_range_iter(lineno, rp) ^ vflag)
		{
		c = getc(fd);

		if(nflag && c != EOF)
			printf(linefmt, lineno);

		/* curious!  old-fashioned, 2-getc, */
		/* non-coalesced-assign-and-test-condition loop, */
		/* just so won't print lineno for nonexistent last line */

		while(c != EOF)
			{
			putchar(c);
			if(c == '\n')
				break;
			c = getc(fd);
			}
		}
	else	{
		/* inner loop duplicated just so won't contain another test */
		while((c = getc(fd)) != EOF)
			{
			if(c == '\n')
				break;
			}
		}

	lineno++;

	} while(c != EOF && (!done_range_iter(rp) || vflag));

if(c == EOF && ferror(fd))
	{
#ifdef NOSTRERROR
	fprintf(stderr, "%s: read error on %s: ", progname, file);
	perror("");
#else
	fprintf(stderr, "%s: read error on %s: %s\n",
				progname, file, strerror(errno));
#endif
	}
}

#ifdef RANDOMACCESS

#define INDEXINTERVAL	10	/* index every this'th line */

struct lineoff
	{
	long int lineno;
	long int off;
	};

struct lineoff *findline();

randscan(file, fd, rp)
char *file;
FILE *fd;
struct range_iter *rp;
{
long int lineno;
long int wantline;
register int c;
int s;
struct lineoff *lo;
int stashing = TRUE;
int shortflag;

lineno = 1;
wantline = do_range_iter(rp, &s);
if(!s)	return;

while(TRUE)
	{
	if(stashing && lineno % INDEXINTERVAL == 0)
		stashline(fd, lineno);

	if(lineno > wantline)
		{
		if((lo = findline(wantline, &shortflag)) == NULL)
			{
#ifdef DEBUG
			fprintf(stderr, "rewinding\n");
#endif
			/* could turn off stashing */
			/* (but wouldn't necessarily know when to turn it back on...) */

			rewind(fd);
			lineno = 1;

			/* curious case (prob. hardly needed) */

			if(wantline < lineno)
				{
				/* we will never find this line */

				if(!skip_next_range(rp))
					break;

				wantline = do_range_iter(rp, &s);
				if(!s)	break;
				/* might have to turn stashing back on */
				}
			}
		else	{
#ifdef DEBUG
			fprintf(stderr, "seeking to line %ld at %ld\n", 
							lo->lineno, lo->off);
#endif
			fseek(fd, lo->off, 0);
			lineno = lo->lineno;
			if(shortflag)
				stashing = TRUE;
			}
		}

	if(lineno + INDEXINTERVAL < wantline &&
			(lo = findline(wantline, &shortflag)) != NULL &&
							lo->lineno > lineno)
		{
#ifdef DEBUG
		fprintf(stderr, "seeking to line %ld at %ld\n", 
							lo->lineno, lo->off);
#endif
		fseek(fd, lo->off, 0);
		lineno = lo->lineno;
		if(shortflag)
			stashing = TRUE;
		/* XXX shortflag logic imperfect -- we might now start */
		/* reading sequentially for a while and delve into uncharted */
		/* territory without noticing */
		}

	if((lineno == wantline) ^ vflag)
		{
		c = getc(fd);

		if(nflag && c != EOF)
			printf(linefmt, lineno);

		/* curious!  old-fashioned, 2-getc, */
		/* non-coalesced-assign-and-test-condition loop, */
		/* just so won't print lineno for nonexistent last line */

		while(c != EOF)
			{
			putchar(c);
			if(c == '\n')
				break;
			c = getc(fd);
			}
		}
	else	{
		/* inner loop duplicated just so won't contain another test */
		while((c = getc(fd)) != EOF)
			{
			if(c == '\n')
				break;
			}
		}

	if(lineno++ == wantline || c == EOF)
		{
		if(c == EOF)
			{
			if(!skip_next_range(rp))
				break;
			}
		wantline = do_range_iter(rp, &s);
		if(!s)	break;
		/* might have to turn stashing back on */
		}
	}

if(c == EOF && ferror(fd))
	{
#ifdef NOSTRERROR
	fprintf(stderr, "%s: read error on %s: ", progname, file);
	perror("");
#else
	fprintf(stderr, "%s: read error on %s: %s\n",
				progname, file, strerror(errno));
#endif
	}

/* flushlines(); *//* not sure whether it should be here or in main() */
}

#define CHUNKSIZE 10

struct linechunk
	{
	int n;
	struct lineoff offs[CHUNKSIZE];
	struct linechunk *next;
	int flags;
	};

#define CHUNK_DIRTY	0x01

struct linechunk *base = NULL;
struct linechunk *tail = NULL;

stashline(fp, lineno)
FILE *fp;
long int lineno;
{
if(tail != NULL && tail->n > 0 && tail->offs[tail->n-1].lineno >= lineno)
	{
#ifdef DEBUG
	printf("(not stashing line %ld)\n", lineno);
#endif
	return;
	}

#ifdef DEBUG
printf("stashing line %ld\n", lineno);
#endif

if(base == NULL || tail->n >= CHUNKSIZE || !(tail->flags & CHUNK_DIRTY))
	{
	struct linechunk *new = malloc(sizeof(struct linechunk));
	if(new == NULL)
		{
		fprintf(stderr, "%s: out of memory\n", progname);
		exit(1);
		}
	new->n = 0;
	new->next = NULL;
	new->flags = 0;
	if(base == NULL)
		base = new;
	if(tail != NULL)
		tail->next = new;
	tail = new;
	}

assert(tail != NULL && tail->n < CHUNKSIZE);

tail->offs[tail->n].lineno = lineno;
tail->offs[tail->n].off = ftell(fp);
tail->n++;
tail->flags |= CHUNK_DIRTY;
}

static struct linechunk *rover = NULL;

struct lineoff *
findline(lineno, shortp)
long int lineno;
int *shortp;
{
struct linechunk *cp;
struct linechunk *useme = NULL;
int i;

if(shortp != NULL)
	*shortp = FALSE;

for(cp = (rover != NULL && rover->offs[0].lineno <= lineno ? rover : base);
						cp != NULL; cp = cp->next)
	{
	assert(cp->n > 0);
	if(cp->offs[0].lineno > lineno)
		break;
	useme = cp;
	}

if(useme == NULL)
	return NULL;

rover = useme;

for(i = 0; i < useme->n && useme->offs[i].lineno <= lineno; i++)
	;

assert(i > 0);

if(lineno - useme->offs[i-1].lineno >= INDEXINTERVAL)
	{
	if(useme->next == NULL)
		*shortp = TRUE;
	}

return &useme->offs[i-1];
}

flushlines()
{
struct linechunk *cp, *nextcp;

for(cp = base; cp != NULL; cp = nextcp)
	{
	nextcp = cp->next;
	free(cp);
	}

base = tail = NULL;

rover = NULL;
}

#endif

#else

scan(file, fd, rp)
char *file;
FILE *fd;
struct range_iter *rp;
{
const struct numrange *nrp, *nrp0;
struct numrange prev;
int haveprev = FALSE;
struct numrange use;
register int c;
long int i = 1;

nrp = get_cur_range(rp);

while(TRUE)	/* almost a do/while loop, but might have to make */
	{	/* an extra trip at the end if vflag */
	nrp0 = nrp;

	if(vflag)
		{
		if(!haveprev && ((nrp->nr_flags & NR_OPNBEG) || nrp->nr_first == 1))
			{
			prev = *nrp;
			haveprev = TRUE;
			continue;
			}

		use.nr_flags = 0;

		if(haveprev)
			use.nr_first = prev.nr_last + 1;
		else	use.nr_flags |= NR_OPNBEG;
		if(nrp != NULL)
			use.nr_last = nrp->nr_first - 1;
		else	use.nr_flags |= NR_OPNEND;

		if(nrp != NULL)
			{
			prev = *nrp;
			haveprev = TRUE;
			}

		nrp = &use;
		}

	if(!(nrp->nr_flags & NR_OPNBEG))
		{
		if(canseek)
			{
			long int n = nrp->nr_first- i;
			fseek(fd, n, 1);
			i += n;
			}
		else	{
			while(i < nrp->nr_first && (c = getc(fd)) != EOF)
				i++;
			}
		}

	while(((nrp->nr_flags & NR_OPNEND) || i <= nrp->nr_last) &&
							(c = getc(fd)) != EOF)
		{
		putchar(c);
		i++;
		}

	if(nrp0 == NULL)
		break;

	if(skip_next_range(rp))
		nrp = get_cur_range(rp);
	else	{
		if(!vflag)
			break;
		nrp = NULL;
		}
	}
}

randscan(file, fd, rp)
char *file;
FILE *fd;
struct range_iter *rp;
{
const struct numrange *nrp;
register int c;
long int i;

do	{
	nrp = get_cur_range(rp);
	if(nrp->nr_flags & NR_OPNBEG)
		fseek(fd, 0L, 0);
	else	fseek(fd, nrp->nr_first-1, 0);
	for(i = nrp->nr_first; (c = getc(fd)) != EOF && 
					((nrp->nr_flags & NR_OPNEND) ||
						i <= nrp->nr_last);
							i++)
		putchar(c);
	} while(skip_next_range(rp));
}

#endif
