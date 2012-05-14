/* dterm.c	Dumb terminal program				19/11/2004/dcs
 */

/*
 * Copyright 2007 Knossos Networks Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef VERSION
#define VERSION "unknown"
#endif
#define COPYRIGHT \
"dterm version " VERSION " Copyright 2007 Knossos Networks Ltd.\n" \
"\n" \
"This program is free software; you can redistribute it and/or\n" \
"modify it under the terms of the GNU General Public License version 2\n" \
"as published by the Free Software Foundation.\n" \
"\n" \
"This program is distributed in the hope that it will be useful,\n" \
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
"GNU General Public License for more details.\n" \
"\n" \
"You should have received a copy of the GNU General Public License\n" \
"along with this program; if not, write to the Free Software\n" \
"Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA\n" \
"02110-1301, USA.\n"


#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>

#define FIXTTY	if(istty) tcsetattr(0, TCSADRAIN, &savetio)
#define DIEP(s)  { FIXTTY; perror(s); exit(1); }
#define DIE(s,m) { FIXTTY; fprintf(stderr, "%s: %s\n", s, m); exit(1); }

/*
 Sigh
 */
#ifndef CCTS_OFLOW
#ifdef CRTSCTS
#define CCTS_OFLOW CRTSCTS
#endif /* CRTSCTS */
#endif /* ! CCTS_OFLOW */


/*
 Valid speed settings. 
 */
static struct { int s, c; } speeds[] = {
	{ 50, 		B50 	},
	{ 75,		B75 	},
	{ 110,		B110	},
	{ 134,		B134	},
	{ 150,		B150	},
	{ 200,		B200	},
	{ 300,		B300	},
	{ 600,		B600	},
	{ 1200,		B1200	},
	{ 1800,		B1800	},
	{ 2400,		B2400	},
	{ 4800,		B4800	},
	{ 9600,		B9600	},
	{ 19200,	B19200	},
	{ 38400,	B38400	},
#ifdef B7200
	{ 7200,		B7200	},
#endif
#ifdef B14400
	{ 14400,	B14400	},
#endif
#ifdef B28800
	{ 28800,	B28800	},
#endif
#ifdef B57600
	{ 57600,	B57600	},
#endif
#ifdef B76800
	{ 76800,	B76800	},
#endif
#ifdef B115200
	{ 115200,	B115200	},
#endif
#ifdef B230400
	{ 230400,	B230400	},
#endif
#ifdef B460800
	{ 460800,	B460800	},
#endif
#ifdef B921600
	{ 921600,	B921600	},
#endif
	{ 0,		0	}};


/*
 Save the terminal settings here
 */
static struct termios intio, savetio;

static int cmdchar = 035;	/* Command characte, default ^]	 	*/
static char *device = 0;	/* Device, no default			*/
static int ispeed = B9600;	/* Input speed, default 9600 bps	*/
static int ospeed = B9600;	/* Output speed, default 9600 bps	*/
static int twostop = 0;		/* Stop bits, if set to CSTOPB, use two	*/
static int parity = 0;		/* Parity, -1 = mark, -2 = space 0=none */
				/* PARENB enables parity, PARODD = odd	*/
static int bits = CS8;		/* CS5-CS8 for 5-8 bits per character	*/
static int ctsflow = 0;		/* CCTS_OFLOW enables CTS flow control	*/
static int xonflow = 0;		/* IXON | IXOFF for in & out soft F/c	*/
static int modemcontrol = 0;	/* Set to HUPCLto enable modem control	*/
static int markparity = 0;	/* Mark parity -- set high bit on out	*/
static int backspace = 0;	/* Backspace char, send on BS or DEL 	*/
static int maplf = 0;		/* If set, map LF to CR 		*/
static int ignorecr = 0;	/* Ignore carriage returns		*/
static int crlf = 0;		/* Send CRLF on CR			*/
static int istty;		/* Result of isatty()			*/
static char *connname = 0;	/* Connection name found in config	*/
static int sendbreak = 0;	/* Break requested			*/
static int modem = 0;		/* Current modem status			*/
static int setmodem = 0;	/* Set modem status to this		*/

/* Hayyyeeellllllppppp!!!!
 */
static void
help() {
	fprintf(stderr, 
"dterm commands:\n"
"	^%c		Enter command mode\n"
"	<device>	Connect to <device> (or /dev/<device>\n"
"	300, 1200, 9600	Set speed\n"
"	5, 6, 7, 8	Set bits per character\n"
"	1, 2		Set number of stop bits\n"
"	e, o, n, m, s	Set parity to even, odd, none, mark, space\n"
#ifdef CCTS_OFLOW
"	cts,nocts	Enable / disable CTS flow control\n"
#endif
"	xon,noxon	Enable / disable XON/XOFF flow control\n"
"	modem		Hang up modem on exit, exit if modem hangs up\n"
"	nomodem		Do not do modem control\n"
"	bs, nobs	Enable / disable mapping of Delete to Backspace\n"
"	del, nodel	Enable / disable mapping of Backspace to Delete\n"
"	maplf, nomaplf	Enable / disable mapping of LF to CR\n"
"	igncr, noigncr	Ignore / output carriage returns\n"
"	crlf, nocrlf	Enable / disable sending LF after each CR\n"
"	esc=<c>		Set command mode character to Ctrl/<c>\n"
"	dtr, nodtr	Raise / lower DTR\n"
"	rts, norts	Raise / lower RTS\n"
"	d, r		Toggle DTR, RTS\n"
"	b		Send a 500ms break\n"
"	@<filename>	Get configuration from <filename>\n"
"	show		Display modem configuration\n"
"	help, h, ?	Display this message\n"
"	!<command>	Execute shell command\n"
"	version		Display version and warranty information\n"
"	quit, q		Exit\n",
	cmdchar + '@');
}


/*
 Show current setup
 */
static void
showsetup() {
	int i;
	fprintf(stderr, "Port: %s\n", device);
	fprintf(stderr, "Communications parameters: ");
	for(i = 0; speeds[i].s && speeds[i].c != ispeed; )
		i++;
	fprintf(stderr, "%d", speeds[i].s);
	if(ospeed != ispeed) {
		for(i = 0; speeds[i].s && speeds[i].c != ospeed; )
			i++;
		fprintf(stderr, "/%d", speeds[i].s);
	}
	if(bits == CS5) fprintf(stderr, " 5");
	if(bits == CS6) fprintf(stderr, " 6");
	if(bits == CS7) fprintf(stderr, " 7");
	if(bits == CS8) fprintf(stderr, " 8");
	if(parity == 0) 		fprintf(stderr, " n");
	if(parity == PARENB)		fprintf(stderr, " e");
	if(parity == (PARENB|PARODD))	fprintf(stderr, " o");
	if(parity == -1)		fprintf(stderr, " m");
	if(parity == -2)		fprintf(stderr, " s");
	if(twostop)	fprintf(stderr, " 2\n");
	else		fprintf(stderr, " 1\n");
	fprintf(stderr, "Flow/modem control:");
	if(xonflow)	fprintf(stderr, " xon");
	if(ctsflow)	fprintf(stderr, " cts");
	if(modemcontrol)fprintf(stderr, " modem"); 
	putc('\n', stderr);
	fprintf(stderr, "Character mappings:");
	if(backspace == 8)	fprintf(stderr, " bs");
	if(backspace == 127)	fprintf(stderr, " del");
	if(maplf)		fprintf(stderr, " maplf");
	if(ignorecr)		fprintf(stderr, " igncr");
	if(crlf)		fprintf(stderr, " crlf");
	putc('\n', stderr);
	printf("Modem control: DTR: %s RTS: %s CTS: %s DSR: %s DCD: %s\n",
			setmodem & TIOCM_DTR ? "on" : "off",
			setmodem & TIOCM_RTS ? "on" : "off",
			   modem & TIOCM_CTS ? "on" : "off",
			   modem & TIOCM_DSR ? "on" : "off",
			   modem & TIOCM_CD  ? "on" : "off");
	fprintf(stderr, "Escape character: ^%c\n", cmdchar + '@');
	fflush(stderr);
}



static int readconfig(char *, char *);

/*
 Process commands passed as strings, may be several commands on a line
 Allow ugliness like 8n1 -- treat as 8, n, 1
 */
static int
setup(char *s, char *cffile, int cfline) {
	char *t, *d;
	int j,k;
	int ret = 0;
	struct stat sb;
	char ttybuf[128];
	int justospeed = 0;

	while(*s) {
		/*
		 Skip whitespace, commas
		 */
		while(isspace(*s) || *s == ',') 
			s++;
		if(!*s) break;

		/*
		 '?' = help
		 */
		if(*s == '?') {
			help();
			return -4;
		}

		/*
		 '!' = shell command
		 */
		if(*s == '!') {
			setenv("DTERM_DEVICE", device, 1);
			if((t = strchr(s, '\n')))
				*t = 0;
			system(++s);
			break;
		}

		/*
		 If a number, process it.
		 If it's a valid speed, use that -- if nnn/mmm, set both to nnn 
		 for the first time, and flag things so mmm just sets ospeed
		 5-8 = data bits, 1,2 sets stops
		 Anything else is error
		 */
		if(isdigit(*s)) {
			j = strtol(s, &s, 10);
			for(k = 1; speeds[k].s; k++)
				if(speeds[k].s == j)
					break;
			if(speeds[k].s) {
				ospeed = speeds[k].c;
				if(!justospeed)
					ispeed = ospeed;
				if(*s == '/') {
					s++;
					justospeed = 1;
				}
				else	justospeed = 0;
			}
			else if(j == 5)	bits = CS5;
			else if(j == 6)	bits = CS6;
			else if(j == 7)	bits = CS7;
			else if(j == 8)	bits = CS8;
			else if(j == 1) twostop = 0;
			else if(j == 2) twostop = CSTOPB;
			else {
				if(cffile)
					fprintf(stderr, "in %s:%d: ",
						cffile, cfline);
				fprintf(stderr, "%d: invalid speed/bits\n", j);
				ret = -1;
			}
		}

		/*
		 Otherwise, get the alpha-only keyword, see if it matches anything 
		 useful.
		 */
		else {
			t = s;
			while(isalpha(*t)) t++;
			j = *t;
			*t = 0;
			if(	!strcasecmp(s, "xon"))
				xonflow = IXON | IXOFF;
			else if(!strcasecmp(s, "noxon"))
				xonflow = 0;

#ifdef CCTS_OFLOW
			else if(!strcasecmp(s, "cts"))
				ctsflow = CCTS_OFLOW;
			else if(!strcasecmp(s, "nocts"))
				ctsflow = 0;
#endif
			else if(!strcasecmp(s, "modem"))
				modemcontrol = HUPCL;
			else if(!strcasecmp(s, "nomodem"))
				modemcontrol = 0;
			else if(!strcasecmp(s, "E"))
				parity = PARENB;
			else if(!strcasecmp(s, "O"))
				parity = PARENB | PARODD;
			else if(!strcasecmp(s, "M"))
				parity = -1;
			else if(!strcasecmp(s, "S"))
				parity = -2;
			else if(!strcasecmp(s, "N"))
				parity = 0;
			else if(!strcasecmp(s, "q") || !strcasecmp(s, "quit"))
				ret = -3;
			else if(!strcasecmp(s, "h") || !strcasecmp(s, "help"))
				help();
			else if(!strcasecmp(s, "del"))
				backspace = 127;
			else if(!strcasecmp(s, "bs"))
				backspace = 8;
			else if(!strcasecmp(s,"nobs") || !strcasecmp(s,"nodel"))
				backspace = 0;
			else if(!strcasecmp(s, "noesc"))
				cmdchar = 0;
			else if(!strcasecmp(s, "maplf"))
				maplf = 1;
			else if(!strcasecmp(s, "nomaplf"))
				maplf = 0;
			else if(!strcasecmp(s, "igncr"))
				ignorecr = 1;
			else if(!strcasecmp(s, "noigncr"))
				ignorecr = 0;
			else if(!strcasecmp(s, "crlf"))
				crlf = 1;
			else if(!strcasecmp(s, "nocrlf"))
				crlf = 0;
			else if(!strcasecmp(s, "b")) 
				sendbreak = 1;
			else if(!strcasecmp(s, "d"))
				setmodem ^= TIOCM_DTR;
			else if(!strcasecmp(s, "r"))
				setmodem ^= TIOCM_RTS;
			else if(!strcasecmp(s, "dtr"))
				setmodem |= TIOCM_DTR;
			else if(!strcasecmp(s, "rts"))
				setmodem |= TIOCM_RTS;
			else if(!strcasecmp(s, "nodtr"))
				setmodem &= ~TIOCM_DTR;
			else if(!strcasecmp(s, "norts"))
				setmodem &= ~TIOCM_RTS;
			else if(!strcasecmp(s, "show"))
				showsetup();
			else if(!strcasecmp(s, "version"))
				fputs(COPYRIGHT, stderr);

			/*
			 No?
			 @<filename>	includes a file
			 !cmd		Run a command
			 esc=c		sets escape char
			 <device>	select device
			 */
			else {
				*t = j;
				while(*t && !isspace(*t) && *t != ',')
					t++;
				j = *t;
				*t = 0;
				if(*s == '@') {
					k = readconfig(++s, 0);
					if(k == -2) {
						if(cffile)
							fprintf(stderr,
								"in %s:%d: ",
								cffile, cfline);
						fprintf(stderr, "%s: %s\n",
							s, strerror(errno));
						ret = -1;
					}
					goto next;
				}
				if(!strncasecmp(s, "esc=", 4)) {
					cmdchar = s[4] & 0x1f;
					goto next;
				}

				d = s;	
				k = stat(d, &sb);
				if(k && *d != '/') {
					sprintf(ttybuf, "/dev/%.100s", d);
					d = ttybuf;
					k = stat(d, &sb);
				}
				if(!k) {
					if((sb.st_mode & S_IFMT) == S_IFCHR) {
						ret = 1;
						device = strdup(d);
						goto next;
					}
				}
				if(cffile)
					fprintf(stderr, "in %s:%d: ",
						cffile, cfline);
				fprintf(stderr,
					"%s: unrecognised keyword/device\n", s);
				ret = -1;
			}
		next:	*t = j;
			s = t;
		}
	}
	return ret;
}


/*
 Read a config file
 Input lines can be lists of config words, or in the form
 name: words
 If name: form, only lines matching passed name are used
 */
static int
readconfig(char *file, char *name) {
	char buf[1024];
	FILE *f;
	char *s, *t;
	int lineno = 0;
	int ret = 0, r;

	/*
	 ~/file = get file from $HOME/file
	 */
	if(*file == '~' && file[1] == '/' && (s = getenv("HOME"))) {
		snprintf(buf, sizeof(buf), "%s/%s", s, file+2);
		file = buf;
	}

	/*
	 Return -2 if can't open
	 */
	f = fopen(file, "r");
	if(!f)	return -2;

	/*
	 Read input, strip # commends
	 Count lines
	 Keep track of return code
	 */
	while(fgets(buf, sizeof(buf), f)) {
		lineno++;
		if((s = strchr(buf, '#')))
			*s = 0;
		for(s = buf; isspace(*s);)
			s++;
		if(!*s) continue;
		for(t = s; *t && *t != ':' && *t != ',' && !isspace(*t); )
			t++;
		if(*t == ':') {
			*t++ = 0;
			if(!name)
				continue;
			if(strcmp(name, s))
				continue;
			s = t;
			connname = name;
		}

		r = setup(s, file, lineno);
		if(r < 0)
			ret = r;
		if(r > 0 && !ret)
			ret = 1;
	}

	fclose(f);
	return ret;
}


/*
 Sleep n milliseconds
 */
static void
millisleep(int n) {
	struct timespec t;
	t.tv_sec = n / 1000;
	t.tv_nsec = (n % 1000) * 1000000;
	nanosleep(&t, 0);
}


/*
 Set up the port
 */
static int
setupport(int fd) {
	int parityflags;
	int modemflags;
	int spaceparity = 0;
	struct termios tio;
	int m;

	/*
	 See what to do with parity
	 */
	markparity = 0;
	parityflags = parity;
	if(parity == -1) {
		parityflags = 0;
		markparity = ISTRIP;
	}
	else if(parity == -2) {
		parityflags = 0;
		spaceparity = ISTRIP;
	}

	/*
	 If no modem control, use local mode
	 */
	modemflags = ctsflow | modemcontrol;
	if(!modemflags)
		modemflags = CLOCAL;

	/*
	 Set the speed and params
	 */
	tcgetattr(fd, &tio);
	tio.c_iflag = IGNBRK | IGNPAR | xonflow | spaceparity | markparity;
	tio.c_cflag = CREAD | HUPCL | modemflags | bits | parityflags;
	tio.c_oflag = 0;
	tio.c_lflag = 0;
	cfsetispeed(&tio, ispeed);
	cfsetospeed(&tio, ospeed);
	if(tcsetattr(fd, TCSAFLUSH, &tio) == -1)
		return -1;

	/*
	 Set up the modem lines
	 */
	ioctl(fd, TIOCMGET, &modem);
	if((modem & (TIOCM_RTS | TIOCM_DTR)) != (setmodem & (TIOCM_RTS |
							     TIOCM_DTR))) {
		m = setmodem 	& (TIOCM_RTS | TIOCM_DTR);
		ioctl(fd, TIOCMBIS, &m);
		m = (~setmodem) & (TIOCM_RTS | TIOCM_DTR);
		ioctl(fd, TIOCMBIC, &m);
	}

	/*
	 Send a break if requested
	 */
	if(sendbreak) {
		ioctl(fd, TIOCSBRK, 0);
		millisleep(500);
		ioctl(fd, TIOCCBRK, 0);
		sendbreak = 0;
	}

	/*
	 Get the current modem status
	 */
	ioctl(fd, TIOCMGET, &modem);
	setmodem = modem;

	return 0;
}


/*
 Set up the modem.  This is a little tricky due to the need to
 ensure the modem does not block before we set it up.
 */
static int
openport(char *device) {
	int fd;

	if((fd = open(device, O_RDWR|O_NONBLOCK, 0)) < 0)
		DIEP(device)
	if(setupport(fd) < 0)
		DIEP(device)
	millisleep(10);
	fcntl(fd, F_SETFL, 0);

	ioctl(fd, TIOCMGET, &modem);
	setmodem = modem;

	setenv("DTERM_PORT", device, 1);
	return fd;
}


/*
 Usage
 */
static void
usage(char *this) {
	fprintf(stderr, "Usage: %s port/setup\n"
			"	'%s help' for help\n", this, this);
	exit(1);
}


int
main(int argc, char **argv) {
	int fd, nfd;
	int i, j;
	unsigned char inbuf;
	char buf[256];
	char *s;
	int done;
	fd_set fds;
	struct timeval delay_tv, *readdelay;

	/*
	 Do the right things depending on whethet stdin & stdout are TTYs
	 */
	istty = isatty(0);
	if(istty) {
		tcgetattr(0, &savetio);
		maplf = 0;
	}
	else 	maplf = 1;
	if(isatty(1))
		ignorecr = 0;
	else	ignorecr = 1;

	/*
	 Read default config, if no private .dtermrc, use /etc/dtermrc
	 */
	setmodem = modem = TIOCM_DTR | TIOCM_RTS;
	if(readconfig("~/.dtermrc", argv[1]) == -2)
		readconfig("/etc/dtermrc", argv[1]);

	/*
	 Parse args
	 If only arg is "help", exit
	 */
	i = 1;
	if(connname) i = 2;
	for(; i < argc; i++) 
		if(setup(argv[i], 0, 0) < 0)
			usage(argv[0]);
	if(argc == 2 && !strcasecmp(argv[1], "help"))
		exit(0);

	/*
	 If no device, no default
	 */
	if(!device)
		usage(argv[0]);

	/*
	 If TTY, set it up
	 */
	if(istty) {
		intio = savetio;
		intio.c_oflag = 0;
		intio.c_lflag = 0;
		intio.c_iflag = savetio.c_iflag & ~(INLCR|IGNCR|ICRNL);

		intio.c_cc[VEOF]	= _POSIX_VDISABLE;
		intio.c_cc[VEOL]	= _POSIX_VDISABLE;
		intio.c_cc[VEOL2]	= _POSIX_VDISABLE;
		intio.c_cc[VERASE]	= _POSIX_VDISABLE;
		intio.c_cc[VWERASE]	= _POSIX_VDISABLE;
		intio.c_cc[VKILL]	= _POSIX_VDISABLE;
		intio.c_cc[VREPRINT]	= _POSIX_VDISABLE;
		intio.c_cc[VINTR]	= _POSIX_VDISABLE;
		intio.c_cc[VQUIT]	= _POSIX_VDISABLE;
		intio.c_cc[VSUSP]	= _POSIX_VDISABLE;
#ifdef VDSUSP
		intio.c_cc[VDSUSP]	= _POSIX_VDISABLE;
#endif
		intio.c_cc[VLNEXT]	= _POSIX_VDISABLE;
		intio.c_cc[VDISCARD]	= _POSIX_VDISABLE;
#ifdef VSTATUS
		intio.c_cc[VSTATUS]	= _POSIX_VDISABLE;
#endif
		tcsetattr(0, TCSADRAIN, &intio);
	}

	/*
	 Connect to serial port
	 */
	fd = openport(device);
	if(fd < 0) exit(1);

	/*
	 Main loop
	 */
	readdelay = 0;
	done = 0;
	while(!done) {

		/*
		 Set up the select() call
		 If readdelay is not 0, we're waiting for things to go quiet so we
		 can exit.
		 Errors kill us, execpt for interrupted calls
		 0 return only happens if readdelay is set, so we exit then
		 */
		FD_ZERO(&fds);
		if(!readdelay) 
			FD_SET(0, &fds);
		FD_SET(fd, &fds);
		i = select(fd + 1, &fds, 0,0, readdelay);
		if(i == -1 && errno != EINTR)
			DIEP("select");
		if(i == 0 && readdelay)
			break;

		/*
		 Input on stdin
		 Read a character
		 If EOF, set readdelay to 1 second
		 */
		if(FD_ISSET(0, &fds)) {
			if(read(0, &inbuf, 1) < 1) {
				delay_tv.tv_sec = 1;
				delay_tv.tv_usec = 0;
				readdelay = &delay_tv;
				continue;
			}
			/*
			 If command character received, read commands 
			 */
			if(inbuf == cmdchar && istty) {
				FIXTTY;
				putchar('\n');
				for(;;) {
					fprintf(stderr, "dterm> ");
					if(!fgets(buf, sizeof(buf), stdin)) 
						return 0;
					if((s = strchr(buf, '#')))
						*s = 0;
					for(s = buf; *s; s++)
						if(!isspace(*s)) break;
					if(!*s) break;
					ioctl(fd, TIOCMGET, &modem);
					i = setup(buf, 0, 0);
					if(i == -3) 
						return 0;
					if(i == 1) {
						nfd = openport(device);
						if(nfd >= 0) {
							close(fd);
							fd = nfd;
						}
					}
					else if(setupport(fd))
						fprintf(stderr,
							"invalid parameters\n");
				}
				if(istty) tcsetattr(0, TCSADRAIN, &intio);
			}
			/*
			 Otherwise do any processing on the character
			 Add dread high bit disease if mark parity
			 BS <-> DEL mapping
			 LF -> CR mapping for files
			 CR -> CRLF mapping for TTYs
			 */
			else {
				if(markparity)
					inbuf |= 0x80;
				if(backspace && (inbuf == 8 || inbuf == 127))
					inbuf = backspace;
				if(maplf && inbuf == '\n')
					inbuf = '\r';
				write(fd, &inbuf, 1);
				if(crlf && inbuf == '\r') {
					inbuf = '\n';
					write(fd, &inbuf, 1);
				}
			}
		}

		/*
		 If input, read a full buffer in
		 IgnoreCR means sucking CRs out of the buffer (yuck)
		 If EOF (e.g. hangup), exit nicely.
		 */
		if(FD_ISSET(fd, &fds)) {
			i = read(fd, buf, sizeof(buf));
			if(i < 0) 
				DIEP(device);
			if(!i)	break;
			if(ignorecr) {
				j = 0;
				for(s = buf; s < buf + i; s++) 
					if(*s != '\r')
						buf[j++] = *s;
				i = j;
			}			
			write(1, buf, i);
		}
	}

	/*
	 Fall out the bottom, cleaning up
	 */
	FIXTTY;
	return 0;
}
