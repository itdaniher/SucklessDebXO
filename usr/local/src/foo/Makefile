# foo - set background 
# See LICENSE file for copyright and license details.

include config.mk

SRC = foo.c
OBJ = ${SRC:.c=.o}

all: options foo

options:
	@echo foo build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

foo: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f foo ${OBJ} foo-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p foo-${VERSION}
	@cp -R LICENSE Makefile README config.mk \
		foo.1 ${SRC} foo-${VERSION}
	@tar -cf foo-${VERSION}.tar foo-${VERSION}
	@gzip foo-${VERSION}.tar
	@rm -rf foo-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f foo ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/foo
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < foo.1 > ${DESTDIR}${MANPREFIX}/man1/foo.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/foo.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/foo
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/foo.1

.PHONY: all options clean dist install uninstall
