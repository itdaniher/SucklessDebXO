/* © 2009 Ian Daniher <it.daniher@gmail.com> */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>


int main(int argc, char* argv[]) {
	char curs[] = {0, 0, 0, 0, 0, 0, 0, 0};
	int screen;
	unsigned int len;
	char screencolor[8] = "#660000";
	Cursor invisible;
	Display *dpy;
	Pixmap pmap;
	Window root, w;
	XColor black, dummy;
	XEvent ev;
	XSetWindowAttributes wa;

	if (argc > 2) {
		fprintf(stderr, "usage: redscr [--version]\n"
		                "       redscr <color> (color has to be 6 hex chars)\n");
		exit(EXIT_FAILURE);
	}

	if ((argc == 2) && (strcmp("--version", argv[1]) == 0)) {
		fprintf(stderr, "redscr-%s, (c) 2006-2007 Anselm R. Garbe, 2007 markus schnalke\n", VERSION);
		exit(EXIT_SUCCESS);
	}

	if ((argc == 2)) {
		/* set color */
		screencolor[0] = '#';
		screencolor[1] = '\0';
		strcat(screencolor, argv[1]);
	}

	if(!(dpy = XOpenDisplay(0))) {
		fprintf(stderr, "redscr: cannot open display\n");
		exit(EXIT_FAILURE);
	}

	/* init */
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	Colormap cmap = DefaultColormap(dpy, screen);
	XColor color;

	if(!XAllocNamedColor(dpy, cmap, screencolor, &color, &color)) {
		fprintf(stderr,"error, cannot allocate color\n");
		exit(EXIT_FAILURE);
	}

	wa.override_redirect = 1;
	wa.background_pixel = color.pixel;

	w = XCreateWindow(dpy, root, 0, 0, DisplayWidth(dpy, screen), DisplayHeight(dpy, screen),
	    0, DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	    CWOverrideRedirect | CWBackPixel, &wa);
	XAllocNamedColor(dpy, DefaultColormap(dpy, screen), "black", &black, &dummy);
	pmap = XCreateBitmapFromData(dpy, w, curs, 8, 8);
	invisible = XCreatePixmapCursor(dpy, pmap, pmap, &black, &black, 0, 0);
	XDefineCursor(dpy, w, invisible);
	XMapRaised(dpy, w);

	/* main event loop */
	while (1) {
		XNextEvent(dpy, &ev);
		usleep(100000);
	}
	XFreePixmap(dpy, pmap);
	XDestroyWindow(dpy, w);
	XCloseDisplay(dpy);
	return 0;
}
