#pragma once

#include <cstring>

#define eatToken(input,token) (input).eat((token), ct_strlen(token))

#define strempty(str) ((str)[0] == '\0')
#define streq(str1,str2) (strcmp((str1),(str2)) == 0)
#define strstart(str,prefix) (strncmp((str),(prefix),ct_strlen(prefix)) == 0)

#define isPow2(x) (((x) & ((x) - 1)) == 0)
#define isValidDataLength(x) ((x) & (8 | 16 | 32 | 64))

inline bool attemptCopy(char* const dest, const char* src) {
	if (!strempty(dest))
		return false;
	strcpy(dest, src);
	return true;
}
/*
inline bool whitespace(const char c) { return c == ' '; }
inline bool whitespace(const char* c) { return whitespace(*c); }
inline bool eatspace(char*& text) { return text += whitespace(text); }

// moves str past character c
inline bool strskip (char*& str, char c) {
	while (*str && *str != c)
		++str;
	return *str != '\0';
}

inline bool strstart(const char* text, const char* prefix) {
	while (*prefix)
		if (*prefix++ != *text++)
			return false;
	return *prefix == '\0';
}

inline bool eat(char*& text, const char* prefix, size_t len) {
	if (strstart(text, prefix)) {
		text += len;
		return true;
	}
	return false;
}

inline bool eatchar(char*& text, char c) {
	if (whitespace(text))
		++text;
	if (*text == c) {
		++text;
		return true;
	}
	return false;
}

inline bool load(char* dest, const char* src, char stop) {
	while (*src && *src != stop)
		*dest++ = *src++;
	*dest = '\0';
	return *src == stop;
}


// #define eat(text, prefix) (eat((text), (prefix), ct_strlen(prefix)))
*/