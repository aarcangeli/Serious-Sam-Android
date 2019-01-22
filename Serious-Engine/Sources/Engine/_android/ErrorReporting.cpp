#include <Engine/StdH.h>
#include <Engine/Base/ErrorReporting.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/ErrorTable.h>
#include <android/log.h>

#define  LOG_TAG    "seriousSamNative"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

INDEX con_bNoWarnings = 0;

void ThrowF_t(char *strFormat, ...) {
    va_list args;
    size_t len;
    char *space;

    va_start(args, strFormat);
    len = (size_t) vsnprintf(0, 0, strFormat, args);
    va_end(args);
    if ((space = (char *) malloc(len + 1)) != 0) {
        va_start(args, strFormat);
        vsnprintf(space, len + 1, strFormat, args);
        va_end(args);
        LOGE("%s", space);
        free(space);
    }

    exit(EXIT_FAILURE);
}

ENGINE_API extern void FatalError(const char *strFormat, ...) {
    va_list args;
    size_t len;
    char *space;

    va_start(args, strFormat);
    len = (size_t) vsnprintf(0, 0, strFormat, args);
    va_end(args);
    if ((space = (char *) malloc(len + 1)) != 0) {
        va_start(args, strFormat);
        vsnprintf(space, len + 1, strFormat, args);
        va_end(args);
        LOGE("%s", space);
        free(space);
    }

    exit(EXIT_FAILURE);
}

ENGINE_API extern void WarningMessage(const char *strFormat, ...) {
    va_list args;
    size_t len;
    char *space;

    va_start(args, strFormat);
    len = (size_t) vsnprintf(0, 0, strFormat, args);
    va_end(args);
    if ((space = (char *) malloc(len + 1)) != 0) {
        va_start(args, strFormat);
        vsnprintf(space, len + 1, strFormat, args);
        va_end(args);
        LOGW("%s", space);
        free(space);
    }
}

ENGINE_API extern void InfoMessage(const char *strFormat, ...) {
    va_list args;
    size_t len;
    char *space;

    va_start(args, strFormat);
    len = (size_t) vsnprintf(0, 0, strFormat, args);
    va_end(args);
    if ((space = (char *) malloc(len + 1)) != 0) {
        va_start(args, strFormat);
        vsnprintf(space, len + 1, strFormat, args);
        va_end(args);
        LOGI("%s", space);
        free(space);
    }
}

ENGINE_API extern BOOL YesNoMessage(const char *strFormat, ...) {
    va_list args;
    size_t len;
    char *space;

    va_start(args, strFormat);
    len = (size_t) vsnprintf(0, 0, strFormat, args);
    va_end(args);
    if ((space = (char *) malloc(len + 1)) != 0) {
        va_start(args, strFormat);
        vsnprintf(space, len + 1, strFormat, args);
        va_end(args);
        LOGI("%s", space);
        free(space);
    }

    // todo: choose
    return true;
}

ENGINE_API extern const CTString GetWindowsError(DWORD dwWindowsErrorCode) {
    FatalError("GetWindowsError");
    return "unimplemented";
}

extern void Breakpoint(void) {
    // place a breakpoint here
    int t = 0;
}

/*
 * Get the description string for error code.
 */
const char *ErrorDescription(const struct ErrorTable *pet, SLONG ulErrCode) {
    for (INDEX i = 0; i < pet->et_Count; i++) {
        if (pet->et_Errors[i].ec_Code == ulErrCode) {
            return pet->et_Errors[i].ec_Description;
        }
    }
    return "Unknown error";
}
