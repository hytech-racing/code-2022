#pragma once

#include <cstring>

#include "Exception.h"
#include "InputBuffer.h"

#define eatToken(input,token) (input).eat((token), ct_strlen(token))

#define strempty(str) ((str)[0] == '\0')
#define streq(str1,str2) (strcmp((str1),(str2)) == 0)
#define strstart(str,prefix) (strncmp((str),(prefix),ct_strlen(prefix)) == 0)

#define isPow2(x) (((x) & ((x) - 1)) == 0)
#define isValidDataLength(x) ((x) & (8 | 16 | 32 | 64))

inline void attemptCopy(char* const dest, const char* src, const char* const TOKEN) {
	if (!strempty(dest))
		throw ReassignmentException(TOKEN, dest);
	strcpy(dest, src);
}