// C verison of dwm status script -- no need for xsetroot
// by Jeremy Jay
//
// compile with: gcc -lX11 -s -o dwmstatus dwmstatus.c

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

// modify BAT0 here to suit (yes this can be done better, but i'm lazy)
const char *batt_rate_script="sed 's/unknown/1/g' /proc/acpi/battery/BAT0/state |grep rate |tr -d [:alpha:],':',' '";
const char *batt_rem_script="grep remaining /proc/acpi/battery/BAT0/state |tr -d [:alpha:],':',' '";
const char *batt_cap_script="grep full /proc/acpi/battery/BAT0/info |tr -d [:alpha:],':',' '";
const char *batt_stat_script="grep charging /proc/acpi/battery/BAT0/state |cut -b 26-";

// modify wlan0 here to suit
const char *wififunc = "/sbin/iwconfig wlan0 |grep Quality | sed 's/:\\|=/./g' |tr -d [:alpha:],[:space:] |cut -d'.' -f2 |cut -d'/' -f1";

// this is slow to return, but gives a true percent, replace with load if you want
const char *cpufunc = "top -b -d1 -n2 | awk '/^Cpu\\(s\\): / {gsub(\"%us,\",\"\",$2); print $2; }' | tail -n 1";

char text[256];

// runs a shell script and returns the output in text[]
int runScript(const char *script) {
	int pd[2], l=0, r=0, p=0;

	if( pipe(pd) == -1 )
		return 0;

	if( (p=fork())==0 ) {
		close(pd[0]);
		close(2);
		close(0);
		dup2( pd[1], 1 );
		execlp("/bin/sh", "/bin/sh", "-c", script, NULL);
		perror("exec failed");
		exit(1);
	} else if( p==-1 ) {
		return 0;
	}
	close(pd[1]);
	waitpid(p, NULL, 0);
	while( (r=read(pd[0], text+l, 256-l)) > 0 )
		l+=r;
	close(pd[0]);

	text[l] = 0;
	return 1;
}

/////////////////////////////////////
// update with your own status info

void getstatus(char *status) {
	time_t now = time(NULL);
	char datestr[21], bstate[2]={0,0};
	int batt=0, btime=0, cpu=0, wifi=0;

	////////////////////////////////////
	// retrieve all the stats
	////////////////////////////////////

	// battery
	if( runScript(batt_rate_script) ) {
		int rate, rem=0, cap=0;
		
		rate = atoi(text);

		runScript(batt_rem_script);
		rem = atoi(text);

		runScript(batt_cap_script);
		cap = atoi(text);

		runScript(batt_stat_script);
		if( strncmp(text, "charging", 8)==0 ) 
			bstate[0]='+';
		else if( strncmp(text, "discharging", 11)==0 ) 
			bstate[0]='-';
		
		btime=(60*rem) / rate;
		batt=(100*rem) / cap;
		if( batt>100 ) batt=100;
	}

	// wifi
	if( runScript(wififunc) )
		wifi = atoi(text);

	// cpu
	if( runScript(cpufunc) )
		cpu = atoi(text);

	//////////////////////////////////
	// build the string
	//////////////////////////////////
	status[0]=0;

	// dont show cpu unless its high
	if( cpu > 50 ) 
		snprintf(status, 256, "%sc=%d%% | ", status, cpu);

	// dont show wifi unless its weak
	if( wifi < 30 )
		snprintf(status, 256, "%sw=%d%% | ", status, wifi);

	// show battery info only if not full (time rem is handy when not plugged in)
	if( bstate[0]=='-' || batt < 50 ) {
		if( batt <= 15 ) {
			// if you have my colored status patch, this will highlight the battery indicator
			// otherwise put your own popup message/whatever here
			snprintf(status, 256, "%s%cb=%d%%%s [%d:%02d] \x01| ", status, (batt<10?4:3), batt, bstate, btime/60, btime%60);
		} else
			snprintf(status, 256, "%sb=%d%%%s [%d:%02d] | ", status, batt, bstate, btime/60, btime%60);
	}

	// add the date/time to the end
	strftime(status+strlen(status), 256-strlen(status), "%d %b %Y - %I:%M", localtime( &now ) );
}

int main(int argc, char **argv) {
	char stext[256];
	Display *dpy;
	Window root;

	if(!(dpy = XOpenDisplay(0))) {
		fprintf(stderr, "dwmstatus: cannot open display\n");
		return 1;
	}
	root = DefaultRootWindow(dpy);

	while( 1 ) {
		sleep(2);
		getstatus(stext);
		
		XChangeProperty(dpy, root, XA_WM_NAME, XA_STRING,
				8, PropModeReplace, (unsigned char*)stext, strlen(stext));
		XFlush(dpy);
	}

	return 0;
}
