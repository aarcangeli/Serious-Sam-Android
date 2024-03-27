#include <Engine/StdH.h>
#include <Engine/Base/ErrorReporting.h>
#include <time.h>
#include <android/log.h>
#include <AndroidAdapters/binding-callbacks.h>

BindingCallbacks g_cb {};

int64_t getTimeNsec() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (int64_t) now.tv_sec * 1000000000LL + now.tv_nsec;
}

uint32_t _rotl(uint32_t ul, int bits) {
    return (ul<<bits) | (ul>>(-bits&31));
}

char *strupr(char *s) {
    char *p = s;
    while (*p = toupper(*p)) p++;
    return s;
}

HMODULE LoadLibraryA(LPCSTR lpLibFileName) {
    WarningMessage("LoadLibraryA('%s')", lpLibFileName);
    return nullptr;
}

void *GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
    WarningMessage("GetProcAddress(lib, '%s')", lpProcName);
    return nullptr;
}

BOOL FreeLibrary(HMODULE hLibModule) {
    WarningMessage("FreeLibrary(lib)");
    return 1;
}

UINT WINAPI SetErrorMode(UINT uMode) {
    return 0;
}

int _CrtCheckMemory(void) {
    return 1;
}

HMODULE GetModuleHandleA(LPCSTR lpModuleName) {
    FatalError("GetModuleHandleA()");
    return 0;
}

u_long WINAPI htonl(u_long hostlong) {
    FatalError("htonl()");
    return 1;
}

size_t WINAPI inet_addr(const char *cp) {
    FatalError("inet_addr()");
    return 1;
}

int WINAPI gethostname(char *name, int namelen) {
    FatalError("gethostname()");
    return 1;
}

void reportError(const char *func) {
    WarningMessage("%s", func);
}

void blockingError(const char *func) {
  FatalError("OpenGL ERROR: %s", func);
}

void blockingError(const char *func, GLenum error) {
  FatalError("OpenGL ERROR: %s 0x%04X", func, error);
}
