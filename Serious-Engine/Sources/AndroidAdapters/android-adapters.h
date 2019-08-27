#ifndef ANDROID_ADAPTERS_H
#define ANDROID_ADAPTERS_H

#include <cstdint>
#include <ctype.h>
#include <cstdarg>
#include <errno.h>

#define ENGINE_API
#define PLATFORM_UNIX 1
#define USE_PORTABLE_C 1

static_assert(sizeof(void *) == 4, "Serious engine require 32 bit address space");

int64_t getTimeNsec();

unsigned int _rotl(unsigned int value, int shift);

char *strupr(char *string);

#include <AndroidAdapters/win-constants.h>
#include <AndroidAdapters/stringFormatter.h>

#endif // ANDROID_ADAPTERS_H
