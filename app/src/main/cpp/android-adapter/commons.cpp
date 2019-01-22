#include <Engine/StdH.h>
#include <Engine/Base/ErrorReporting.h>
#include <time.h>
#include <android/log.h>

int64_t getTimeNsec() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (int64_t) now.tv_sec * 1000000000LL + now.tv_nsec;
}

unsigned int _rotl(unsigned int value, int shift) {
    FatalError("_rotl");
}

char *strupr(char *s) {
    char *p = s;
    while (*p = toupper(*p)) p++;
    return s;
}

HMODULE LoadLibraryA(LPCSTR lpLibFileName) {
    FatalError("LoadLibraryA('%s')", lpLibFileName);
}

void *GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
    FatalError("LoadLibraryA(lib, '%s')", lpProcName);
}

BOOL FreeLibrary(HMODULE hLibModule) {
    FatalError("FreeLibrary(lib)");
}

UINT WINAPI SetErrorMode(UINT uMode) {
    FatalError("SetErrorMode()");
}

int _CrtCheckMemory(void) {
    return 1;
}

HMODULE GetModuleHandleA(LPCSTR lpModuleName) {
    FatalError("GetModuleHandleA()");
}

u_long WINAPI htonl(u_long hostlong) {
    FatalError("htonl()");
}

size_t WINAPI inet_addr(const char *cp) {
    FatalError("inet_addr()");
}

int WINAPI gethostname(char *name, int namelen) {
    FatalError("gethostname()");
}
