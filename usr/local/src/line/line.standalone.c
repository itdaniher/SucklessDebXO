/*
 *  Program for extracting selected lines (by line number) from text files.
 *  (Roughly equivalent to "sed -n -e 3p -e 5p", etc.)
 *  Inspired by a program of the same name and basic functionality
 *  conceived by Philip Lantz at Tektronix MDP circa 1985.
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

#ifdef notdef
#ifdef __STDC__
#define SAFEREALLOC
#endif
#endif

#define Ctod(c)  ((c) - '0')

#define TRUE 1
#define FALSE 0

char *linefmt = "%6ld: ";
int exitstatus = 0;

char *progname = "line";

int nflag = FALSE;
int vflag = FALSE;

usage()
{
fprintf(stderr, "usage: %s [-n] lines [files]\n", progname);
}

main(argc, argv)
int argc;
char *argv[];
{
int argi;
char *wantlist = NULL;
FILE *fd;

for(argi = 1; argi < argc; argi++)
	{
	/* XXX need better argv parse */
	if(strcmp(argv[argi], "-n") == 0)
		nflag = TRUE;
	else if(strcmp(argv[argi], "-v") == 0)
		vflag = TRUE;
	else if(isdigit(argv[argi][0]) ||
			argv[argi][0] == '-' && argv[argi][1] != '\0')
		parselines(argv[argi]);
	else	break;
	}

if(!haveranges())
	{
	usage();
	exit(1);
	}

if(argi >= argc)
	scan("standard input", stdin);
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

	scan(argv[argi], fd);

	if(fd != stdin)
		fclose(fd);
	else	clearerr(fd);

	/*
	 *  Third-order minor bug, which I'm not gonna worry about now:
	 *  If you say
	 *	line 1-3 - -
	 *  it starts reading the second - right away, without waiting for EOF.
	 */

	restart();
	}

exit(exitstatus);
}

scan(file, fd)
char *file;
FILE *fd;
{
register long int lineno;
register int c;

lineno = 1;

do	{
	if(want(lineno) ^ vflag)
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

	} while(c != EOF && (!finished(lineno) || vflag));

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

struct range
	{
	long int start, end;
	} *ranges = NULL;

int nranges = 0;
struct range *rangep;

haveranges()
{
return ranges != NULL;
}

parselines(desc)
char *desc;
{
char *p = desc;
struct range *rp;

while(TRUE)
	{
	nranges++;

#ifndef SAFEREALLOC
	if(ranges == NULL)
		ranges = (struct range *)malloc(nranges * sizeof(struct range));
	else
#endif
		{
		ranges = (struct range *)
			realloc((char *)ranges, nranges * sizeof(struct range));
		}

	if(ranges == NULL)
		{
		fprintf(stderr, "%s: out of memory\n", progname);
		exit(1);
		}

	rp = &ranges[nranges - 1];

	rp->start = 0;

	if(isdigit(*p))
		{
		do	{
			rp->start = 10 * rp->start + Ctod(*p);
			p++;
			} while(isdigit(*p));

		if(rp > ranges && rp->start < (rp - 1)->end)
			{
overlap:		fprintf(stderr, "%s: overlapping range\n", progname);
			exit(1);
			}
		}
	else if(*p != '-')
		{
syntax:		fprintf(stderr, "%s: range syntax error\n", progname);
		exit(1);
		}

	if(*p == ',' || *p == '\0')
		{
		rp->end = rp->start;

		if(*p == '\0')
			break;

		p++;

		continue;
		}

	if(*p != '-')
		goto syntax;

	p++;

	rp->end = 0;

	if(isdigit(*p))
		{
		do	{
			rp->end = 10 * rp->end + Ctod(*p);
			p++;
			} while(isdigit(*p));

		if(rp->end < rp->start)
			goto overlap;
		}
	else if(*p != '\0')
		goto syntax;

	if(*p == '\0')
		break;

	if(*p != ',')
		goto syntax;

	p++;
	}

rangep = ranges;	/* ready for scan */
}

restart()
{
rangep = ranges;
}

want(lineno)
long int lineno;
{
while(rangep < &ranges[nranges])
	{
	if(lineno < rangep->start && rangep->start != 0)
		return FALSE;

	if(lineno <= rangep->end || rangep->end == 0)
		return TRUE;

	rangep++;
	}

return FALSE;
}

finished(lineno)
long int lineno;
{
return (rangep >= &ranges[nranges]);
}
