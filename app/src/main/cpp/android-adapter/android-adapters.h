#include <cstdint>
#include <ctype.h>
#include <cstdarg>
#include <errno.h>

#define ENGINE_API
#define PLATFORM_UNIX 1
#define USE_PORTABLE_C 1

#define _vsnprintf vsnprintf
#define _snprintf snprintf

int64_t getTimeNsec();

unsigned int _rotl(unsigned int value, int shift);

char *strupr(char *string);

#include "win-constants.h"
