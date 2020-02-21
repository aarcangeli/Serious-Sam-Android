#ifndef ANDROID_ADAPTERS_H
#define ANDROID_ADAPTERS_H

#include <cstdint>
#include <ctype.h>
#include <cstdarg>
#include <errno.h>

#define ENGINE_API
#define PLATFORM_UNIX 1
#define USE_PORTABLE_C 1

#if defined(__aarch64__)

  #define PLATFORM_64BIT 1
static_assert(sizeof(void *) == 8);

#elif defined(__i386) || defined(__arm__)

  #define PLATFORM_32BIT 1
static_assert(sizeof(void *) == 4);

#else
  #error "Unknown CPU-Architecture, adapt this code to detect 32/64bitness of your system!"
#endif

static_assert(sizeof(size_t *) == sizeof(void *));

int64_t getTimeNsec();

unsigned int _rotl(unsigned int value, int shift);

char *strupr(char *string);

#include <AndroidAdapters/win-constants.h>
#include <AndroidAdapters/stringFormatter.h>

#endif // ANDROID_ADAPTERS_H
