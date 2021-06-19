#include "AutoParse.h"
#include "UserDefined.h"

uint64_t neg;

#define getNeg(buf) ((uint8_t*) &(neg = ~(*(uint64_t*)(buf))))

