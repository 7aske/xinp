#ifndef __7ASKE_XINP_XINP_STRUTIL_H
#define __7ASKE_XINP_XINP_STRUTIL_H

#pragma once

#include <string.h>

int append_char(char* _dest_str, char _chr, unsigned long _dest_str_maxlen);

int delete_char(char* _dest_str, unsigned long _dest_str_maxlen);

int str_replace(char* _dest_str, char* _src_str, char* _repl_tok, char* _with);

#endif
