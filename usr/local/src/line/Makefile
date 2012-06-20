CFLAGS = -DRANDOMACCESS $(CFLAGS2) $(DEBUG)

line: line.o numrange.long.o
	$(CC) $(CFLAGS) -o $@ line.o numrange.long.o

extract: extract.o numrange.long.o
	$(CC) $(CFLAGS) -o $@ extract.o numrange.long.o

line.1: line.man.sgml
	faqproc -man line.man.sgml > $@

line.man.html: line.man.sgml
	faqproc -html -man -flat -o $@ line.man.sgml

extract.1: extract.man.sgml
	faqproc -man extract.man.sgml > $@

extract.man.html: extract.man.sgml
	faqproc -html -man -flat -o $@ extract.man.sgml

DISTFILES = line.c line.standalone.c numrange.c numrange.h \
	line.1 line.man.html extract.1 extract.man.html

dist: line.tar line.zip

line.tar: $(DISTFILES)
	-/bin/rm -rf tmp/line 2> /dev/null
	mkdir -p tmp/line
	cp -p README.line tmp/line/README
	cp $(DISTFILES) tmp/line
	cp -p Makefile.line tmp/line/Makefile
	tar cf $@ -C tmp line
	/bin/rm -rf tmp/line

line.zip: line.tar
	-/bin/rm -rf tmp/line 2> /dev/null
	mkdir -p tmp
	cd tmp; tar xf ../line.tar
	mv tmp/line/README tmp/line/README.txt
	cd tmp; zip -r -l ../$@ line
	/bin/rm -rf tmp/line

extract.o: line.c numrange.h
	$(CC) $(CFLAGS) -DEXTRACT -c -o $@ line.c

line.o: line.c numrange.h

numrange.long.o: numrange.c numrange.h
	$(CC) $(CFLAGS) -DNR_TYPE=long -c -o $@ numrange.c

clean:
	rm -f *o rm line
