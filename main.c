#include <err.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>

#include "i3/ipc.h"
#include "xinp.h"
#include "util/xinp_strutil.h"

Display* display;
Window window;
XEvent event;
int screen;
bool running = true;

winprop_t winprops = {
		.x=100,
		.y=100,
		.width=200,
		.height=10,
		.border=0
};

struct {
	char* format;
	char* command;

	struct {
		char* data;
		int x;
		int y;
	} buffer;

	struct {
		char* data;
		int x;
		int y;
	} prompt;

	unsigned char limit;


	int send_i3;
} xinp;

typedef XftColor Color;
struct {
	Colormap cmap;
	Color prompt_color;
	Color buffer_color;
	Color bg_color;
	XftDraw* draw;
	XftFont* font;
} dc;

enum resource_type {
	STRING = 0,
	INTEGER = 1,
	FLOAT = 2
};

typedef struct {
	char* name;
	enum resource_type type;
	void* dst;
} ResourcePref;

enum {
	COLOR_BG = 0,
	COLOR_PR = 7,
	COLOR_FG = 10,
};
static const char* colorname[] = {
		/* 8 normal colors */
		"#3b4252", /* black   */
		"#bf616a", /* red     */
		"#a3be8c", /* green   */
		"#ebcb8b", /* yellow  */
		"#81a1c1", /* blue    */
		"#b48ead", /* magenta */
		"#88c0d0", /* cyan    */
		"#e5e9f0", /* white   */
		/* 8 bright colors */
		"#4c566a", /* black   */
		"#bf616a", /* red     */
		"#a3be8c", /* green   */
		"#ebcb8b", /* yellow  */
		"#81a1c1", /* blue    */
		"#b48ead", /* magenta */
		"#8fbcbb", /* cyan    */
		"#eceff4", /* white   */
		[255] = 0,
		"#2e3440", /* background */
		"#d8dee9", /* foreground */
};

ResourcePref resources[] = {
		{"font",       STRING, &def_fontname},
		{"color0",     STRING, &colorname[0]},
		{"color1",     STRING, &colorname[1]},
		{"color2",     STRING, &colorname[2]},
		{"color3",     STRING, &colorname[3]},
		{"color4",     STRING, &colorname[4]},
		{"color5",     STRING, &colorname[5]},
		{"color6",     STRING, &colorname[6]},
		{"color7",     STRING, &colorname[7]},
		{"color8",     STRING, &colorname[8]},
		{"color9",     STRING, &colorname[9]},
		{"color10",    STRING, &colorname[10]},
		{"color11",    STRING, &colorname[11]},
		{"color12",    STRING, &colorname[12]},
		{"color13",    STRING, &colorname[13]},
		{"color14",    STRING, &colorname[14]},
		{"color15",    STRING, &colorname[15]},
		{"background", STRING, &colorname[256]},
		{"foreground", STRING, &colorname[257]},
};

XftFont* xload_font(const char* _fontname);
int xresource_load(XrmDatabase db, char* name, enum resource_type rtype, void* dst);
void _cleanup(void);
void _exit(int code);
void _output(void);
void handle_keypress(XEvent* _xevent);
void init_prompt(void);
void xcolor_allocate(unsigned long hex_color, XftColor* xftcolor);
void xcolor_load(const char* name, XftColor* color);
void xfont_init(void);
void xinit(void);
void xpose(void);
void xresize_window(void);
void xresources_init(void);


int main(int argc, char* argv[]) {
	memset(&xinp, 0, sizeof(xinp));
	xinit();
	xfont_init();
	xresources_init();

	xcolor_load(colorname[COLOR_PR], &dc.prompt_color);
	xcolor_load(colorname[COLOR_FG], &dc.buffer_color);
	xcolor_load(colorname[COLOR_BG], &dc.bg_color);

	xinp.limit = BUFFER_MAXLEN;
	xinp.format = strdup("%s");
	xinp.command = calloc(COMMAND_MAXLEN, sizeof(char));
	xinp.prompt.data = strdup("> ");
	xinp.buffer.data = calloc(BUFFER_MAXLEN, sizeof(char));

	int option_index, o;
	static struct option long_options[] = {
			{"class", no_argument, 0, 'C'},
			{"font", required_argument, 0, 'f'},
			{"format", required_argument, 0, 'F'},
			{"limit", required_argument, 0, 'l'},
			{"help", no_argument, 0, 'h'},
			{"prompt", required_argument, 0, 'p'},
			{"i3", no_argument, &xinp.send_i3, 1},
			{"bg", required_argument, 0, 0},
			{"fg", required_argument, 0, 0},
			{"pr", required_argument, 0, 0},
			{"br", required_argument, 0, 0},
			NULL};

	char* options_string = "l:p:F:f:C:h";
	unsigned char val;

	while ((o = getopt_long_only(argc, argv, options_string, long_options, &option_index)) != -1) {
		switch (o) {
			case 0:
				if (strncmp(long_options[option_index].name, "bg", 2) == 0) {
					XftColorFree(display, DefaultVisual(display, screen), dc.cmap, &dc.bg_color);
					xcolor_load(optarg, &dc.bg_color);
				} else if (strncmp(long_options[option_index].name, "pr", 2) == 0) {
					XftColorFree(display, DefaultVisual(display, screen), dc.cmap, &dc.prompt_color);
					xcolor_load(optarg, &dc.prompt_color);
				} else if (strncmp(long_options[option_index].name, "fg", 2) == 0) {
					XftColorFree(display, DefaultVisual(display, screen), dc.cmap, &dc.buffer_color);
					xcolor_load(optarg, &dc.buffer_color);
				} else if (strncmp(long_options[option_index].name, "br", 2) == 0) {
					if (*optarg == '#') optarg++;
					XSetWindowBorder(display, window, strtol(optarg, NULL, 16));
				}
				break;
			case 'p':
				free(xinp.prompt.data);
				xinp.prompt.data = strdup(optarg);
				break;
			case 'F':
				free(xinp.format);
				xinp.format = strdup(optarg);
				break;
			case 'f':
				dc.font = xload_font(optarg);
				break;
			case 'C':
				clhint.res_class = strdup(optarg);
				break;
			case 'l':
				val = *optarg - '0';
				xinp.limit = val < BUFFER_MAXLEN ? val : BUFFER_MAXLEN;
				break;
			case 'h':
				return 0;
		}
	}

	init_prompt();

	/* event loop */
	while (running) {
		XNextEvent(display, &event);

		switch (event.type) {
			case KeyPress:
				handle_keypress(&event);
			case ClientMessage:
			case Expose:
				xpose();
		}
	}

	_cleanup();
	return 0;
}

void xpose(void) {
	XClearWindow(display, window);
	XftDrawRect(dc.draw, &dc.bg_color, 0, 0, winprops.width, winprops.height);
	XftDrawStringUtf8(dc.draw, &dc.prompt_color, dc.font, xinp.prompt.x, xinp.prompt.y,
					  (FcChar8*) xinp.prompt.data,
					  xinp.prompt.data ? strnlen(xinp.prompt.data, PROMPT_MAXLEN) : 0);
	XftDrawStringUtf8(dc.draw, &dc.buffer_color, dc.font, xinp.buffer.x, xinp.buffer.y,
					  (FcChar8*) xinp.buffer.data,
					  xinp.buffer.data ? strnlen(xinp.buffer.data, BUFFER_MAXLEN) : 0);
}

void xcolor_load(const char* col_str, XftColor* color) {
	if (col_str == NULL) return;
	if (*col_str == '#') col_str++;
	unsigned long hex_color = strtol(col_str, NULL, 16);
	xcolor_allocate(hex_color, color);
}

void xcolor_allocate(unsigned long hex_color, XftColor* xftcolor) {
	XRenderColor xcolor;
	xcolor.red = (uint8_t) (hex_color >> 16) << 8; // << 8 is required to pad the color to #XX00 format
	xcolor.green = (uint8_t) (hex_color >> 8) << 8;
	xcolor.blue = (uint8_t) (hex_color) << 8;
	xcolor.alpha = ~0;

	XftColorAllocValue(display, DefaultVisual(display, screen), dc.cmap, &xcolor, xftcolor);
}

void xfont_init(void) {
	dc.font = xload_font(def_fontname);
}

XftFont* xload_font(const char* fontname) {
	XftFont* font = NULL;

	if (fontname != NULL)
		font = XftFontOpenName(display, screen, fontname);

	if (font == NULL)
		font = XftFontOpenName(display, screen, def_fontname);

	return font;
}

void xinit(void) {
	/* open connection with the server */
	display = XOpenDisplay(NULL);
	if (display == NULL) die("Cannot open display\n");

	screen = DefaultScreen(display);

	/* create window */
	window = XCreateSimpleWindow(display,
								 RootWindow(display, screen),
								 winprops.x,
								 winprops.y,
								 winprops.width,
								 winprops.height,
								 winprops.border,
								 0, 0);

	XSetClassHint(display, window, &clhint);

	/* process window close event through event handler so XNextEvent does not fail */
	Atom del_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(display, window, &del_window, 1);

	/* select kind of events we are interested in */
	XSelectInput(display, window, ExposureMask | KeyPressMask);
	XAllowEvents(display, SyncKeyboard | AsyncKeyboard, CurrentTime);
	/* display the window */
	XMapWindow(display, window);

	dc.cmap = DefaultColormap(display, screen);
	dc.draw = XftDrawCreate(display, window, DefaultVisual(display, screen), dc.cmap);
	XGrabKeyboard(display, RootWindow(display, screen), false, GrabModeAsync, GrabModeAsync, CurrentTime);
}

void handle_keypress(XEvent* ev) {
	KeySym keySym = XLookupKeysym(&ev->xkey, 0);
	int len = 0;
	char buf[2];

	switch (keySym) {
		case XK_Return:
			_output();
		case XK_Shift_L:
		case XK_Shift_R:
		case XK_Super_L:
		case XK_Super_R:
		case XK_Control_L:
		case XK_Control_R:
		case XK_Alt_L:
		case XK_Alt_R:
			break;
		case XK_BackSpace:
			len = delete_char(xinp.buffer.data, BUFFER_MAXLEN);
			break;
		case XK_space:
			buf[0] = ' ';
			len = append_char(xinp.buffer.data, buf[0], BUFFER_MAXLEN);
			break;
		case XK_Escape:
			_exit(EXIT_SUCCESS);
		case XK_c:
		case XK_d:
			if (ev->xkey.state & ControlMask) _exit(EXIT_SUCCESS);
		default:
			if (ev->xkey.state & ShiftMask) XConvertCase(keySym, &keySym, &keySym);
			XLookupString(&ev->xkey, buf, 2, &keySym, NULL);
			len = append_char(xinp.buffer.data, buf[0], BUFFER_MAXLEN);
	}

	if (len >= xinp.limit) {
		_output();
	}
}

void _cleanup(void) {
	XUngrabKeyboard(display, CurrentTime);
	/* destroy window */
	XDestroyWindow(display, window);

	/* close connection to server */
	XCloseDisplay(display);

	free(xinp.buffer.data);
	free(xinp.prompt.data);
	free(xinp.command);
	free(xinp.format);
}


void init_prompt() {
	XGlyphInfo info;
	int prompt_len = strnlen(xinp.prompt.data, PROMPT_MAXLEN);
	XftTextExtentsUtf8(display, dc.font, (FcChar8*) xinp.prompt.data, prompt_len, &info);

	xinp.prompt.x = dc.font->height / 2;
	xinp.prompt.y = dc.font->height;
	xinp.buffer.x = info.width + dc.font->height;
	xinp.buffer.y = dc.font->height;
	winprops.height = dc.font->height * 1.5;
	winprops.width = info.width + dc.font->max_advance_width * 7;
	xresize_window();
}

void xresize_window(void) {
	XResizeWindow(display, window, winprops.width, winprops.height);
}

void _output(void) {
	int fd, len;
	len = str_replace(xinp.command, xinp.format, REPL_TOK, xinp.buffer.data);
	fwrite(xinp.command, sizeof(char), len, stdout);
	fwrite("\n", sizeof(char), 1, stdout);

	if (xinp.send_i3) {
		fd = i3_ipc_connect(NULL);
		i3_ipc_send_message(fd, strlen(xinp.command), MESSAGE_RUN_COMMAND, (uint8_t*) xinp.command);
	}
	_exit(EXIT_SUCCESS);
}

void _exit(int code) {
	_cleanup();
	exit(code);
}


int xresource_load(XrmDatabase db, char* name, enum resource_type rtype, void* dst) {
	char** sdst = dst;
	int* idst = dst;
	float* fdst = dst;

	char fullname[256];
	char fullclass[256];
	char* type;
	XrmValue ret;

	snprintf(fullname, sizeof(fullname), "%s.%s",
			 clhint.res_name ? clhint.res_name : "i3", name);
	snprintf(fullclass, sizeof(fullclass), "%s.%s",
			 clhint.res_class ? clhint.res_class : "i3", name);
	fullname[sizeof(fullname) - 1] = fullclass[sizeof(fullclass) - 1] = '\0';

	XrmGetResource(db, fullname, fullclass, &type, &ret);
	if (ret.addr == NULL || strncmp("String", type, 64))
		return 1;

	switch (rtype) {
		case STRING:
			*sdst = ret.addr;
			break;
		case INTEGER:
			*idst = strtoul(ret.addr, NULL, 10);
			break;
		case FLOAT:
			*fdst = strtof(ret.addr, NULL);
			break;
	}
	return 0;
}

void xresources_init(void) {
	char* resm;
	XrmDatabase db;
	ResourcePref* p;

	XrmInitialize();
	resm = XResourceManagerString(display);
	if (!resm) return;

	db = XrmGetStringDatabase(resm);
	for (p = resources; p < resources + LEN(resources);
	p++);
	xresource_load(db, p->name, p->type, p->dst);
	XrmDestroyDatabase(db);
}

