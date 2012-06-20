build:
	gcc -DHAVE_CONFIG_H -I. -g -O2 -Wall -I/usr/include -I/usr/include/freetype2 -Wall -W -c common.c
	gcc -DHAVE_CONFIG_H -I. -g -O2 -Wall -I/usr/include -I/usr/include/freetype2 -Wall -W -c fs.c
	gcc -DHAVE_CONFIG_H -I. -g -O2 -Wall -I/usr/include -I/usr/include/freetype2 -Wall -W -c linux.c
	gcc -DHAVE_CONFIG_H -I. -g -O2 -Wall -I/usr/include -I/usr/include/freetype2 -Wall -W -c torsmo.c
	gcc -DHAVE_CONFIG_H -I. -g -O2 -Wall -I/usr/include -I/usr/include/freetype2 -Wall -W -c x11.c
	gcc -g -O2 -Wall -INONE -I/usr/include -I/usr/include/freetype2 -Wall -W -LNONE -o torsmo -lX11 -lXext -lX11 -lfreetype -lz -lfontconfig -lXrender -lX11 fs.o common.o linux.o torsmo.o x11.o
clean:
	rm -f torsmo
	rm -f *.o core *.core
install: build
	cp torsmo /usr/local/bin
