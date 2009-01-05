/* See LICENSE file for copyright and license details. */

/* appearance */
static const char font[]            = "-*-profont-*-*-*-*-22-*-*-*-*-*-*-*";
//determines window border
static const char normbordercolor[] = "#000000";
static const char selbordercolor[]  = "#006699";
//determines 'tab' and 'background' colors
static const char normbgcolor[]		= "#339966";
static const char selbgcolor[]		= "#990000";
//determines text colors for 'tab' and 'background
static const char normfgcolor[]		= "#000000";
static const char selfgcolor[]		= "#FFFFFF";

static unsigned int borderpx        = 4;        /* border pixel of windows */
static unsigned int snap            = 32;       /* snap pixel */
static Bool showbar                 = True;     /* False means no bar */
static Bool topbar                  = True;     /* False means bottom bar */
static Bool readin                  = True;     /* False means do not read stdin */
static Bool usegrab                 = False;    /* True means grabbing the X server */


/* tagging */
static const char tags[][MAXTAGLEN] = { "rnd", "doc", "rnd", "www", "art" };
static unsigned int tagset[] = {1, 1}; 

static Rule rules[] = {
		  /* class		instance		title		 tags mask	  isfloating */
		{ "Firefox",	NULL,			NULL,			1 << 3,		False },
		{ "Links",  	NULL,			NULL,			1 << 3,		False },
		{ "Inkscape",	NULL,			NULL,			1 << 4,		False },
		{ "oofice",		NULL,			NULL,			1 << 1,		False },
		{ "frame",		NULL,			NULL,			1 << 0,		True },
		{ "Xchat",		NULL,			NULL,			1 << 2,		False },
		{ "OpenOffice.org",NULL,	NULL,			1 << 1,		False },
		{ "Xpdf",		NULL,			NULL,			1 << 1,		False },
		{ "feh",			NULL, 		NULL,			NULL, 		True },
		{ "stalonetray",NULL, 		NULL,			~0, 		True },
};

/* layout(s) */
static float mfact      = 0.55; /* factor of master area size [0.05..0.95] */
static Bool resizehints = True; /* False means respect size hints in tiled resizals */

#include "cgrid.c"
static Layout layouts[] = {
	/* symbol	  arrange function */
		{ "[M]",		monocle },
		{ "[#]",		grid },
		{ "[]=",		tile },	 /* first entry is default */
		{ "><>",		NULL },	 /* no layout function means floating behavior */
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,								KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,				KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,					KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask,	KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define BASHCMD(cmd) { .v = (const char*[]){ "/bin/bash", "-c", cmd, NULL } }

/* commands */
static const char *termcmd[] = { "xterm", "-rv", "-fn", font, NULL };
static const char *menucmd[] = { "dmenu_path", "-b", "-fn", font, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *dclpcmd[] = { "dclip", "paste", "-fn", font, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor , "-sf", selfgcolor, NULL }; 


static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY|ShiftMask,					XK_c,		spawn,          BASHCMD("exec dclip copy") }, 
	{ MODKEY|ShiftMask,					XK_k,		spawn,          BASHCMD("xkill") }, 
	{ MODKEY|ShiftMask,					XK_v,		spawn,          {.v = dclpcmd } },
	{ MODKEY,								XK_d,		spawn,			 {.v = menucmd } },
	{ MODKEY|ShiftMask,					XK_Return,	spawn,		 {.v = termcmd } },
	{ MODKEY,								XK_F4,	killclient,     {0} },
	{ MODKEY,								XK_b,		togglebar,      {0} },
	{ MODKEY,								XK_j,		focusstack,     {.i = +1 } },
	{ MODKEY,								XK_k,		focusstack,     {.i = -1 } },
	{ MODKEY,								XK_h,		setmfact,       {.f = -0.05} },
	{ MODKEY,								XK_l,		setmfact,       {.f = +0.05} },
	{ MODKEY,								XK_Return,	zoom,           {0} },
	{ MODKEY,								XK_Tab,   	view,           {0} },
	{ MODKEY,								XK_m,		setlayout,      {.v = &layouts[0]} },
	{ MODKEY,								XK_g,		setlayout,      {.v = &layouts[1]} },
	{ MODKEY,								XK_t,		setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                        XK_n,		setlayout,      {.v = &layouts[3]} },
	{ MODKEY,								XK_space,	setlayout,      {0} },
	{ MODKEY|ShiftMask,					XK_space,	togglefloating, {0} },
	{ MODKEY,							XK_0,		view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,				XK_0,		tag,            {.ui = ~0 } },
	TAGKEYS(								XK_1,                      0)
	TAGKEYS(								XK_2,                      1)
	TAGKEYS(								XK_3,                      2)
	TAGKEYS(								XK_4,                      3)
	TAGKEYS(								XK_5,                      4)
	TAGKEYS(								XK_6,                      5)
	TAGKEYS(								XK_7,                      6)
	TAGKEYS(								XK_8,                      7)
	TAGKEYS(								XK_9,                      8)
	{ MODKEY|ShiftMask,				XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be a tag number (starting at 0),
 * ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

