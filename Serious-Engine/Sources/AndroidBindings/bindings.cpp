#include <AndroidBindings/seriousSamAndroid.h>
#include <Engine/Graphics/ViewPort.h>
#include <jni.h>
#include <GLES2/gl2.h>

void toggleConsoleState();
void processInputs();
void printProfilingData();

pthread_mutex_t g_mySeriousMutex;
pthread_t g_mySeriousThreadId;
pthread_t g_gameRunning = false;
ANativeWindow *g_currentWindow;
bool g_somethingChanged = false;

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_SeriousSamSurface_nSetHomeDir(JNIEnv *env, jobject instance,
                                                                           jstring homeDir_) {
  const char *homeDir = env->GetStringUTFChars(homeDir_, 0);

  // NB: no locking since it should be setted before start
  if (_fnmApplicationPath.Length() == 0) {
    _fnmApplicationPath = CTString(homeDir) + "/";
    _fnmApplicationExe = CTFILENAME("Bin/SeriousSam.exe");
  }

  env->ReleaseStringUTFChars(homeDir_, homeDir);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_toggleConsoleState(JNIEnv *env, jobject instance) {
  toggleConsoleState();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_setAxisValue(JNIEnv *env, jobject instance, jint key, jfloat value) {
  // warning: no locking
  ASSERT(key >= 0 && key < MAX_AXIS);
  g_AxisValue[key] = value;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_setUIScale(JNIEnv *env, jobject instance, jfloat scale) {
  // warning: no locking
  g_uiScale = scale;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_printProfilingData(JNIEnv *env, jobject instance) {
  printProfilingData();
}

void *seriousMain(void *unused) {
  CViewPort *pvpViewPort = new CViewPort();
  CDrawPort *pdp = &pvpViewPort->vp_Raster.ra_MainDrawPort;

  while(true) {
    // get parameters with mutex
    pthread_mutex_lock(&g_mySeriousMutex);

    bool running = g_gameRunning;
    ANativeWindow *window = g_currentWindow;
    bool somethingChanged = g_somethingChanged;
    g_somethingChanged = false;

    // put input values into CInput class
    processInputs();

    pthread_mutex_unlock(&g_mySeriousMutex);

    if (!running || !window) {
      usleep(10000); // wait for 10 ms
      continue;
    }

    if (somethingChanged) {
      pvpViewPort->Initialize(window);
    }

    seriousSamDoGame(pdp);
  }
}

extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_SeriousSamSurface_nInitialize(JNIEnv* env, jobject obj) {
  pthread_mutex_init(&g_mySeriousMutex, 0);
  pthread_create(&g_mySeriousThreadId, 0, &seriousMain, nullptr);
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
  pthread_mutex_unlock(&g_mySeriousMutex);
}

extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_SeriousSamSurface_nOnStop(JNIEnv* env, jobject obj) {
  pthread_mutex_lock(&g_mySeriousMutex);
  g_gameRunning = false;
  pthread_mutex_unlock(&g_mySeriousMutex);
}
