#include <cstdint>
#include <ctype.h>
#include <cstdarg>
#include <errno.h>

#define ENGINE_API
#define PLATFORM_UNIX 1
#define USE_PORTABLE_C 1

int64_t getTimeNsec();

unsigned int _rotl(unsigned int value, int shift);

char *strupr(char *string);

#include <AndroidAdapters/win-constants.h>
#include <AndroidAdapters/stringFormatter.h>
