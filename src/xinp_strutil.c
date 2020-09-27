#include "util/xinp_strutil.h"

int append_char(char* dest, char chr, unsigned long maxlen) {
	int len = strnlen(dest, maxlen);
	if (len >= maxlen - 1) return len;

	*(dest + len++) = chr;
	*(dest + len) = '\0';
	return len;
}

int delete_char(char* dest, unsigned long maxlen) {
	int len = strnlen(dest, maxlen);
	if (!len) return 0;
	*(dest + --len) = '\0';
	return len;
}

int str_replace(char* dest, char* orig, char* tok, char* with) {
	if (orig == NULL || tok == NULL || with == NULL) return 0;
	char* dup_ptr = dest;
	char* orig_ptr = orig;
	int tok_len = strnlen(tok, 2);
	do {
		if (strncmp(orig_ptr, tok, tok_len) == 0) {
			strcat(dup_ptr, with);
			dup_ptr += strlen(with);
			orig_ptr += tok_len;
		}

	} while ((*dup_ptr++ = *orig_ptr++));
	return strlen(dest);
}
