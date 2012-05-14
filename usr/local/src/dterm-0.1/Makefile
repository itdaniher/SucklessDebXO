COPT		= -O3 -Wall -Werror
BIN		= /usr/local/bin
VERSION		= 0.1

dterm:		dterm.c
		gcc ${COPT} -o dterm dterm.c -DVERSION='"$(VERSION)"'

install:
		install dterm ${BIN}

clean:
		rm -f dterm *.o *.bak

package:	dterm.c LICENCE.txt README.txt Makefile
		mkdir -p dterm-$(VERSION)
		cp dterm.c LICENCE.txt README.txt Makefile dterm-$(VERSION)
		tar cfvz dterm-$(VERSION).tgz dterm-$(VERSION)
		rm -rf dterm-$(VERSION)
