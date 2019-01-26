#include <jni.h>
#include <Engine/StdH.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/Stream.h>

static_assert(sizeof(void *) == 4, "Serious engine require 32 bit address space");
const char *gHomeDir;
void startSeriousSamAndroid();

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_setHomeDir(JNIEnv *env, jobject instance,
                                                                     jstring homeDir_) {
    const char *homeDir = env->GetStringUTFChars(homeDir_, 0);

    _fnmApplicationPath = CTString(homeDir) + "/";
    _fnmApplicationExe = _fnmApplicationPath + "Bin/SeriousSam.exe";

    env->ReleaseStringUTFChars(homeDir_, homeDir);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_startEngine(JNIEnv *env,
                                                                      jobject instance) {
    startSeriousSamAndroid();
}
