#include <AndroidBindings/seriousSamAndroid.h>
#include <Engine/Graphics/ViewPort.h>
#include <jni.h>
#include <GLES2/gl2.h>
#include "key_codes.h"

void processInputs();

pthread_mutex_t g_mySeriousMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t g_mySeriousThreadId;
bool g_gameRunning = false;
ANativeWindow *g_currentWindow;
bool g_somethingChanged = false;
bool g_printProfiling = false;
PlayerControls g_IncomingControls {};

extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_SeriousSamSurface_nInitialize(JNIEnv* env, jobject obj, jstring homeDir_) {
  // set home dir
  const char *homeDir = env->GetStringUTFChars(homeDir_, 0);
  _fnmApplicationPath = CTString(homeDir) + "/";
  _fnmApplicationExe = CTFILENAME("Bin/SeriousSam.exe");
  env->ReleaseStringUTFChars(homeDir_, homeDir);

  // start main thread
  pthread_create(&g_mySeriousThreadId, 0, &seriousMain, nullptr);
  pthread_setname_np(g_mySeriousThreadId, "SeriousSamMain");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_setAxisValue(JNIEnv *env, jobject obj, jint key, jfloat value) {
  ASSERT(key >= 0 && key < 10);
  pthread_mutex_lock(&g_mySeriousMutex);
  g_IncomingControls.axisValue[key] = value;
  pthread_mutex_unlock(&g_mySeriousMutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_nDispatchKeyEvent(JNIEnv *env, jobject obj, jint key, jint isPressed) {
#define BTN_CASE(cod, name) case cod: name = isPressed != 0; break;
  pthread_mutex_lock(&g_mySeriousMutex);
  switch(key) {
    // Xbox 360
    BTN_CASE(KEYCODE_BUTTON_R1, g_IncomingControls.bFire)
    BTN_CASE(KEYCODE_BUTTON_R2, g_IncomingControls.bUse)
    BTN_CASE(KEYCODE_BUTTON_L1, g_IncomingControls.bWeaponFlip)
    BTN_CASE(KEYCODE_BUTTON_X, g_IncomingControls.bReload)
    BTN_CASE(KEYCODE_BUTTON_A, g_IncomingControls.bMoveUp)
    BTN_CASE(KEYCODE_BUTTON_B, g_IncomingControls.bMoveDown)
    BTN_CASE(KEYCODE_DPAD_LEFT, g_IncomingControls.bWeaponPrev)
    BTN_CASE(KEYCODE_DPAD_RIGHT, g_IncomingControls.bWeaponNext)
    BTN_CASE(KEYCODE_BACK, g_IncomingControls.bComputer)
    BTN_CASE(KEYCODE_BUTTON_START, g_IncomingControls.bStart)
  }
  pthread_mutex_unlock(&g_mySeriousMutex);
#undef BTN_CASE
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_printProfilingData(JNIEnv *env, jobject obj) {
  pthread_mutex_lock(&g_mySeriousMutex);
  g_printProfiling = true;
  pthread_mutex_unlock(&g_mySeriousMutex);
}

void *seriousMain(void *unused) {
  CViewPort *pvpViewPort = new CViewPort();
  CDrawPort *pdp = &pvpViewPort->vp_Raster.ra_MainDrawPort;

  startSeriousPrestart();

  while(true) {
    // get parameters with mutex
    pthread_mutex_lock(&g_mySeriousMutex);

    bool running = g_gameRunning;
    ANativeWindow *window = g_currentWindow;
    bool somethingChanged = false;
    if (running && window) {
      somethingChanged = g_somethingChanged;
      g_somethingChanged = false;
    }

    // resolve input
    setControls(g_IncomingControls);

    pthread_mutex_unlock(&g_mySeriousMutex);

    if (!running || !window) {
      usleep(50000); // wait for 50 ms
      continue;
    }

    if (somethingChanged) {
      pvpViewPort->Initialize(window);
    }

    seriousSamDoGame(pdp);
  }
}

extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_SeriousSamSurface_nSetSurface(JNIEnv* env, jobject obj, jobject surface) {
  pthread_mutex_lock(&g_mySeriousMutex);

  g_somethingChanged = true;
  if (surface) {
    g_currentWindow = ANativeWindow_fromSurface(env, surface);
  } else {
    g_currentWindow = nullptr;
  }

  pthread_mutex_unlock(&g_mySeriousMutex);
}

extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_SeriousSamSurface_nOnStart(JNIEnv* env, jobject obj) {
  // wake up thread
  pthread_mutex_lock(&g_mySeriousMutex);
  g_gameRunning = true;
  _pTimer->tm_bPaused = false;
  pthread_mutex_unlock(&g_mySeriousMutex);
}

extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_SeriousSamSurface_nOnStop(JNIEnv* env, jobject obj) {
  pthread_mutex_lock(&g_mySeriousMutex);
  g_gameRunning = false;
  _pTimer->tm_bPaused = true;
  pthread_mutex_unlock(&g_mySeriousMutex);
}
