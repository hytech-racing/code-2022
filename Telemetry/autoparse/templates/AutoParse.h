#pragma once

#include "HyTech_CAN.h"
#include <stdio.h>

#define __stringify(x) #x
#define stringify(x) __stringify(x)

extern FILE* outfile;
extern int verbose;

void parseMessage (int id, const char* const timestamp, uint8_t* buf);
