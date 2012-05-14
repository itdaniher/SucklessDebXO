/* See LICENSE file for copyright and license details. */

/* appearance */
static const char *font            = "-*-terminus-medium-r-normal-*-14-*-*-*-*-*-*-*";
static const char *normbordercolor = "#cccccc";
static const char *normbgcolor     = "#cccccc";
static const char *normfgcolor     = "#000000";
static const char *selbordercolor  = "#0066ff";
static const char *selbgcolor      = "#0066ff";
static const char *selfgcolor      = "#ffffff";
static unsigned int borderpx        = 1;        /* border pixel of windows */
static unsigned int snap            = 32;       /* snap pixel */
static Bool showbar                 = True;     /* False means no bar */
static Bool topbar                  = True;     /* False means bottom bar */
static Bool usegrab                 = False;    /* True means grabbing the X server
                                                   during mouse-based resizals */

/* tagging */
static const char tags[][MAXTAGLEN] = { "rnd", "doc", "www", "art", "rnd" };
static unsigned int tagset[] = {1, 1}; /* after start, first tag is selected */

static Rule rules[] = {
	/* class      instance    title       tags mask     isfloating */
	{ "MPlayer",    NULL,       NULL,       ~0,         Floating|NoFocus },
	{ "stalonetray",NULL,       NULL,       ~0,         Floating|NoFocus },
	{ "feh",        NULL,       NULL,       0,          Floating },
	{ "Epdfview",   NULL,       NULL,       1 << 1,     False },
	{ "OpenOffice", NULL,       NULL,       1 << 1,     False },
	{ "Firefox",    NULL,       NULL,       1 << 2,     False },
	{ "processing-app-Base",NULL,NULL,      0,       Floating },
};

/* layout(s) */
static float mfact      = 0.50; /* factor of master area size [0.05..0.95] */
static Bool resizehints = False; /* False means respect size hints in tiled resizals */

#include "cgrid.c"
#include "monocle.c"
#include "fibonacci.c"
static Layout layouts[] = {
	/* symbol     arrange function */
	{ "[mono]",      monocle },
	{ "[grid]",      grid },
	{ "[null]",      NULL },    /* no layout function means floating behavior */
	{ "[sprl]",      spiral },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

#include "cyclelayout.c"

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_F4,     killclient,     {0} },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
//	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
//	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_space,  cyclelayout,    {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
//	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be a tag number (starting at 0),
 * ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        cyclelayout,    {0} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
