#include <Engine/StdH.h>
#include <Engine/Base/Console.h>
#include "DeterministicFP.h"
#include <stdint.h>
#include <fenv.h>
#if defined(__SSE__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
#include <xmmintrin.h>
#endif

#if defined(__aarch64__)
static inline uint64_t read_fpcr() {
  uint64_t r; __asm__ volatile("mrs %0, fpcr" : "=r"(r)); return r;
}
static inline void write_fpcr(uint64_t v) {
  __asm__ volatile("msr fpcr, %0" :: "r"(v));
  __asm__ volatile("isb");
}
#elif defined(__arm__) && !defined(__aarch64__)
static inline uint32_t read_fpscr() {
  uint32_t r; __asm__ volatile("vmrs %0, fpscr" : "=r"(r)); return r;
}
static inline void write_fpscr(uint32_t v) {
  __asm__ volatile("vmsr fpscr, %0" :: "r"(v));
  __asm__ volatile("isb");
}
#elif defined(__SSE__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
static inline uint32_t read_mxcsr() {
  return _mm_getcsr();
}
static inline void write_mxcsr(uint32_t v) {
  _mm_setcsr(v);
}
#endif

void SetDeterministicFP() {
#if defined(__aarch64__)
    uint64_t fp = read_fpcr();
  fp |= (1ull << 24);
  fp |= (1ull << 25);
  fp &= ~(3ull << 22);
  write_fpcr(fp);
  CPrintF("[DeterministicFP] AArch64 FPCR=0x%llx (FZ=1, DN=1, RN)\n", (unsigned long long)fp);
#elif defined(__arm__) && !defined(__aarch64__)
    uint32_t fp = read_fpscr();
  fp |= (1u << 24);
  fp |= (1u << 25);
  fp &= ~(3u << 22);
  write_fpscr(fp);
  CPrintF("[DeterministicFP] ARMv7 FPSCR=0x%08x (FZ=1, DN=1, RN)\n", fp);
#elif defined(__SSE__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
    uint32_t mxcsr = read_mxcsr();
  mxcsr |= (1u << 15);        // FZ
  mxcsr |= (1u << 6);         // DAZ
  mxcsr &= ~(3u << 13);       // RN
  write_mxcsr(mxcsr);
  fesetround(FE_TONEAREST);
  CPrintF("[DeterministicFP] x86 MXCSR=0x%08x (FZ=1, DAZ=1, RN)\n", mxcsr);
#else
    CPrintF("[DeterministicFP] Non-ARM platform: using toolchain flags only\n");
#endif
}
