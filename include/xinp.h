#ifndef __7ASKE_XINP_H
#define __7ASKE_XINP_H

#pragma once

#define die(...) errx(EXIT_FAILURE, __VA_ARGS__)
#define LEN(a)   (sizeof(a) / sizeof(a)[0])

#define ARG_MAXLEN 255
#define BUFFER_MAXLEN 64
#define COMMAND_MAXLEN 1024
#define FORMAT_MAXLEN 255
#define PROMPT_MAXLEN 64

#define REPL_TOK "%s"

static char def_fontname[] = "fixed-10";
XClassHint clhint = {
		.res_name = "xinp",
		.res_class = "floating"
};

typedef struct {
	int x;
	int y;
	int width;
	int height;
	int border;
} winprop_t;

#endif
