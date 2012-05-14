/*
 *  Reimplementation of the standard Unix "sleep" command,
 *  offering several extensions:
 *
 *   *	mm:ss and hh:mm:ss input syntax: "sleep 1:30" sleeps for
 *	a minute and a half (or 90 seconds); "sleep 1:30:00"
 *	sleeps for an hour and a half.
 *
 *   *	subsecond sleeps: "sleep 0.1" sleeps for a tenth of a
 *	second; "sleep 1.5" sleeps for a second and a half;
 *	"sleep 1:23.45" sleeps for 83.45 seconds.
 *
 *   *	"visible" option: with the -v flag, displays a running
 *	second-by-second countdown.
 *
 *   *	Absolute timing option: with the -until flag, sleeps
 *	until the given time of day.  (Example: "sleep -until 17:00"
 *	sleeps until 5:00 pm.)
 *	[Buglet: doesn't (yet) know how to wrap around to tomorrow.]
 *
 *  Too simple to need a Makefile, a configure script, or even
 *  a README file.  To compile, just invoke "cc -o sleep sleep.c"
 *  (or perhaps using a different output file name if you don't
 *  want to supersede the standard version).  Should be portable
 *  to most modern Unix and Linux systems.
 *
 *  See http://www.eskimo.com/~scs/src/#sleep for further
 *  documentation, and possible updates.
 *
 *  Steve Summit
 *  scs@eskimo.com
 *  2002-12-08 (program first written 2001-12-10)
 *
 *  This program is released to the Public Domain.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0

char *progname = "slp";
char usage[] = "usage: %s [-u] [-v] sleeptime\n";

struct timeval tvadd();
struct timeval tvdiff();

void tvsleep();

main(argc, argv)
int argc;
char *argv[];
{
int argi;
char *p;
int h, m;
float s;
float sleeptime;
struct timeval tv1, tv2;
struct timeval sleeptv;
int verbose = 0;
int until = 0;
int printedhelp = 0;

for(argi = 1; argi < argc && argv[argi][0] == '-'; argi++)
	{
	for(p = &argv[argi][1]; *p != '\0'; p++)
		{
		if(p == &argv[argi][1] && *p == '-')
			{
			/* kludge to make --longname work */
			/* more or less as expected */
			continue;
			}

		switch(*p)
			{
			case '?':
printhelp:			printf(usage, progname);
				printf("options:\n");
				printf("\t-u\tsleep until absolute time-of-day\n");
				printf("\t-v\tvisibly display countdown\n");
				printedhelp = TRUE;
				break;

			case 'h':
				if(strcmp(p, "help") == 0)
					p = "x";    /* short circuit */
				goto printhelp;

			case 'u':
				if(strcmp(p, "until") == 0)
					p = "x";	/* short circuit */
				until = 1;
				break;

			case 'v':
				verbose=1;
				break;

			default:
				if(p == &argv[argi][2] && p[-1] == '-')
					{
					fprintf(stderr,
						"%s: unknown option --%s\n",
								progname, p);
					p = "x";	/* short circuit */
					}
				else	{
					fprintf(stderr,
						"%s: unknown option -%c\n",
								progname, *p);
					}
			}
		}
	}

if(argi >= argc)
	{
	if(printedhelp)
		return 0;

	fprintf(stderr, "%s: no sleep time given; 1 second assumed\n", progname);
	sleeptime = 1;	
	}
else if(argc - argi > 1)
	{
	fprintf(stderr, usage, progname);
	exit(1);
	}
else	{
	if(sscanf(argv[argi], "%d:%d:%f", &h, &m, &s) == 3)
		sleeptime = 60 * (60 * h + m) + s;
	else if(sscanf(argv[argi], "%d:%f", &m, &s) == 2)
		{
		sleeptime = 60 * m + s;
		if(until)
			{
			h = m;
			m = s;
			if(m != s)
				{
				fprintf(stderr, "%s: please use H:M:S.f\n",
								progname);
				exit(1);
				}
			s = 0;
			}
		}
	else	{
		if(until)
			{
			fprintf(stderr, "%s: need H:M or H:M:S with -u\n",
								progname);
			exit(1);
			}

		sleeptime = strtod(argv[argi], &p);

		if(p == NULL)
			{
			fprintf(stderr, usage, progname);
			exit(1);
			}
		else if(*p == 's')
			;
		else if(*p == 'm')
			sleeptime *= 60;
		else if(*p == 'h')
			sleeptime *= 60 * 60;
		else if(*p == 'd')
			sleeptime *= 60. * 60 * 24;
		else if(*p != '\0' || p == argv[argi])
			{
			fprintf(stderr, usage, progname);
			exit(1);
			}
		}
	}

if(until || verbose)
	gettimeofday(&tv1, NULL);

if(until)
	{
	struct tm tm1, tm2;
	time_t secs = tv1.tv_sec;
	/* intermediate variable `secs' is necessary in case */
	/* sizeof(time_t) != sizeof(long int) */
	tm1 = *localtime(&secs);
	tm2 = tm1;
	tm2.tm_hour = h;
	tm2.tm_min = m;
	tm2.tm_sec = s;		/* integer part only */
	tv2.tv_sec = mktime(&tm2);
	tv2.tv_usec = (s - tm2.tm_sec) * 1000000 + 0.5;
	sleeptv = tvdiff(tv2, tv1);
	sleeptime = sleeptv.tv_sec + sleeptv.tv_usec / 1000000.;
	}
else	{
	sleeptv.tv_sec = sleeptime;
	sleeptv.tv_usec = (sleeptime - sleeptv.tv_sec) * 1000000 + 0.5;
	tv2 = tvadd(tv1, sleeptv);
	}

if(sleeptime < 0)
	{
	fprintf(stderr, "%s: no negative sleeps, please\n", progname);
	exit(1);
	}
else if(sleeptime == 0)
	return 0;

if(!verbose)
	{
	if(sleeptime == (long)sleeptime)
		sleep((unsigned)sleeptime);
	else	tvsleep(&sleeptv);
	}
else	{
	int sleepint = 1;
	int firsttime = TRUE;

	while(timercmp(&tv2, &tv1, >))
		{
		struct timeval tvd;
		struct timeval d2;
		long int ss;
		int h, m, s;
		struct timeval tvs;

		tvd = tvdiff(tv2, tv1);

		d2 = tvd;
		if(d2.tv_sec >= sleepint)
			d2.tv_sec -= sleepint;

		if(firsttime)
			d2.tv_usec = 0;
		else	{
			/* round: */
			if(d2.tv_usec >= 500000)
				d2.tv_sec++;
			d2.tv_usec = 0;
			}

		ss = tvd.tv_sec;
		if(tvd.tv_usec >= 500000 && !firsttime)
			ss++;
		s = ss % 60;
		ss /= 60;
		m = ss % 60;
		h = ss / 60;

		printf("%d:%02d:%02d   \r", h, m, s);
		fflush(stdout);

		if(tvd.tv_sec > 0)	/* XXX awkward test */
			tvs = tvdiff(tvd, d2);
		else	tvs = tvd;

		tvsleep(&tvs);

		if(tvd.tv_sec == 0)
			{
			assert(tvs.tv_sec == 0 && tvd.tv_usec == tvs.tv_usec);
			break;
			}

		firsttime = FALSE;
		gettimeofday(&tv1, NULL);
		}

	printf("\r         \r");
	fflush(stdout);
	}

return 0;
}

struct timeval
tvadd(tv1, tv2)
struct timeval tv1, tv2;
{
struct timeval ret;

ret.tv_sec = tv1.tv_sec + tv2.tv_sec;
ret.tv_usec = tv1.tv_usec + tv2.tv_usec;
if(ret.tv_usec >= 1000000)
	{
	ret.tv_sec++;
	ret.tv_usec -= 1000000;
	}

return ret;
}

struct timeval
tvdiff(tv1, tv2)
struct timeval tv1, tv2;
{
struct timeval tvd;
if(tv1.tv_usec < tv2.tv_usec)
	{
	tv1.tv_usec += 1000000;
	tv1.tv_sec--;
	}
tvd.tv_sec = tv1.tv_sec - tv2.tv_sec;
tvd.tv_usec = tv1.tv_usec - tv2.tv_usec;
return tvd;
}

void dingdong(s)
{
}

void
tvsleep(tvp)
struct timeval *tvp;
{
int r;
struct itimerval itv;

if(!timerisset(tvp))
	return;

itv.it_value = *tvp;
timerclear(&itv.it_interval);

signal(SIGALRM, dingdong);
r = setitimer(ITIMER_REAL, &itv, NULL);
pause();
signal(SIGALRM, SIG_DFL);
}
