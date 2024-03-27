#ifndef ANDROID_ADAPTERS_H
#define ANDROID_ADAPTERS_H

#include <cstdint>
#include <ctype.h>
#include <cstdarg>
#include <errno.h>
#include <unistd.h>

#define ENGINE_API
#define PLATFORM_UNIX 1
#define USE_PORTABLE_C 1

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(_ARCH_PPC64) \
    || defined(_M_IA64) || defined(__IA64__) || defined(__e2k__)

  #define PLATFORM_64BIT 1

#elif defined(__i386) || defined(_M_IX86) || defined(__arm__) || defined(_M_ARM) || defined(__POWERPC__) \
      || defined(_M_PPC) || defined(_ARCH_PPC)

  #define PLATFORM_32BIT 1

#else
  #error "Unknown CPU-Architecture, adapt this code to detect 32/64bitness of your system!"
#endif

#ifdef PLATFORM_32BIT
static_assert(sizeof(void *) == 4, "Serious engine require 32 bit address space");
#endif

#ifdef PLATFORM_64BIT
static_assert(sizeof(void *) == 8, "Serious engine require 32 bit address space");
#endif

static_assert(sizeof(size_t *) == sizeof(void *), "");

int64_t getTimeNsec();

unsigned int _rotl(unsigned int value, int shift);

char *strupr(char *string);

static inline void Sleep(DWORD dwMilliseconds) {
  usleep(dwMilliseconds * 1000);
}

#include <AndroidAdapters/win-constants.h>

#endif // ANDROID_ADAPTERS_H
