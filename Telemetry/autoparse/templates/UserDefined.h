#pragma once

#include "AutoParse.h"

#define CUSTOMFUNC(fname, param) void fname (const char* const timestamp, const char* const prefix, param)
#define show(formatStr, ...) fprintf(outfile, "%s,%s" formatStr, timestamp, prefix, __VA_ARGS__)

