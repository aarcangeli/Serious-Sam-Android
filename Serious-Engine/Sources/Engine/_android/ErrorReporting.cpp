#include <Engine/StdH.h>
#include <Engine/Base/ErrorReporting.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/ErrorTable.h>
#include <android/log.h>

#define  LOG_TAG    "SeriousSamNative"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

extern void CPrintLog(CTString strBuffer);

INDEX con_bNoWarnings = 0;
err_callback_t g_errorCalllback = nullptr;

extern void AndroidLogPrintI(CTString log) {
  LOGI("%s", log.str_String);
  CPrintLog(CTString("INFO: ") + log + "\n");
}

extern void AndroidLogPrintW(CTString log) {
  LOGW("%s", log.str_String);
  CPrintLog(CTString("WARN: ") + log + "\n");
}

extern void AndroidLogPrintE(CTString log) {
  LOGE("%s", log.str_String);
  CPrintLog(CTString("ERROR: ") + log + "\n");
}

ENGINE_API extern const CTString GetWindowsError(DWORD dwWindowsErrorCode) {
  // should never be called on android
  return "GetWindowsError: unimplemented";;
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

void AndroidCloseApplication() {
  // kill process
  throw 0;
}

void AssertFailed(const char *string) {
  CPrintLog(stringFormatter::format("%s\n", string));
}
