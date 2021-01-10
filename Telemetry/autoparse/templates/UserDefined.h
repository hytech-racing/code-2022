#pragma once

#include "AutoParse.h"

#define CUSTOMFUNC(fname, data, prev) void fname (const char* const timestamp, const char* const prefix, data, prev)
#define show(formatStr, ...) fprintf(outfile, "%s,%s" formatStr, timestamp, prefix, __VA_ARGS__)

