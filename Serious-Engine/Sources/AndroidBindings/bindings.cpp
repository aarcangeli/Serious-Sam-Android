#include <Engine/StdH.h>
#include <AndroidBindings/bindings.h>
#include <Engine/Graphics/ViewPort.h>
#include <jni.h>
#include <GLES2/gl2.h>
#include <Engine/Base/CTString.h>
#include "key_codes.h"

void processInputs();

pthread_mutex_t g_mySeriousMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t g_mySeriousThreadId;
bool g_gameRunning = false;
ANativeWindow *g_currentWindow;
bool g_somethingChanged = false;
PlayerControls g_IncomingControls {};
JavaVM *g_javaWM;
jclass g_NativeEvents;
jmethodID g_reportFatalError;
CTString g_command = "";
CViewPort *g_pvpViewPort = nullptr;

JNIEnv* getEnv() {
  static thread_local JNIEnv* myEnv = nullptr;
  if (!myEnv) {
    g_javaWM->AttachCurrentThread(&myEnv, nullptr);
  }
  return myEnv;
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  // cache some vm variables
  g_javaWM = vm;
  JNIEnv* env = getEnv();
  g_NativeEvents = env->FindClass("com/github/aarcangeli/serioussamandroid/NativeEvents");
  ASSERT(g_NativeEvents);
  g_NativeEvents = (jclass) env->NewGlobalRef(g_NativeEvents);
  return JNI_VERSION_1_2;
}

extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_SeriousSamSurface_nInitialize(JNIEnv* env, jobject obj, jstring homeDir_) {
  g_errorCalllback = &androidReportError;

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
Java_com_github_aarcangeli_serioussamandroid_MainActivity_shiftAxisValue(JNIEnv *env, jobject obj, jint key, jfloat value) {
  ASSERT(key >= 0 && key < 10);
  pthread_mutex_lock(&g_mySeriousMutex);
  g_IncomingControls.shiftAxisValue[key] += value;
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
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_MainActivity_nShellExecute(JNIEnv* env, jobject obj, jstring command_) {
  const char *command = env->GetStringUTFChars(command_, 0);
  pthread_mutex_lock(&g_mySeriousMutex);

  g_command += CTString(command) + ";";

  pthread_mutex_unlock(&g_mySeriousMutex);
  env->ReleaseStringUTFChars(command_, command);
}

void setSeriousState(GameState state) {
  if (g_cb.gameState != state) {
    g_cb.gameState = state;
    JNIEnv* env = getEnv();
    jmethodID method = env->GetStaticMethodID(g_NativeEvents, "reportStateChange", "(I)V");
    ASSERT(method);
    env->CallStaticVoidMethod(g_NativeEvents, method, (int) g_cb.gameState);
  }
}

void syncSeriousThreads() {
  while (true) {
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

    CTString cmdToExecute = g_command;
    g_command = "";

    pthread_mutex_unlock(&g_mySeriousMutex);

    if (cmdToExecute.Length()) {
      _pShell->Execute(cmdToExecute);
      cmdToExecute = "";
    }

    if (!running || !window) {
      usleep(50000); // wait for 50 ms
      continue;
    }

    if (somethingChanged) {
      g_pvpViewPort->Initialize(window);
    }

    return;
  };
}

CViewPort *getViewPort() {
  syncSeriousThreads();
  return g_pvpViewPort;
}

void *seriousMain(void *unused) {
  g_pvpViewPort = new CViewPort();

  g_cb.setSeriousState = &setSeriousState;
  g_cb.syncSeriousThreads = &syncSeriousThreads;
  g_cb.getViewPort = &getViewPort;

  // run all
  try {
    seriousSubMain();
  } catch (const char *msg) {
    FatalError("%s", msg);
  }

  // close app
  exit(0);
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
  if (_pTimer) _pTimer->tm_bPaused = false;
  pthread_mutex_unlock(&g_mySeriousMutex);
}

extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_SeriousSamSurface_nOnStop(JNIEnv* env, jobject obj) {
  pthread_mutex_lock(&g_mySeriousMutex);
  g_gameRunning = false;
  if (_pTimer) _pTimer->tm_bPaused = true;
  pthread_mutex_unlock(&g_mySeriousMutex);
}

void androidReportError(CTString error) {
  JNIEnv* env = getEnv();
  jmethodID method = env->GetStaticMethodID(g_NativeEvents, "reportFatalError", "(Ljava/lang/String;)V");
  ASSERT(method);
  env->CallStaticVoidMethod(g_NativeEvents, method, env->NewStringUTF(error.str_String));
}
