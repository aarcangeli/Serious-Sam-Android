#include <Engine/StdH.h>
#include <AndroidBindings/bindings.h>
#include <Engine/Graphics/ViewPort.h>
#include <GLES2/gl2.h>
#include <Engine/Base/CTString.h>
#include "key_codes.h"

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>

#define ACTION_DOWN 0
#define ACTION_UP 1
#define ACTION_OUTSIDE 4
#define ACTION_MOVE 2
#define ACTION_HOVER_MOVE 7
#define ACTION_SCROLL 8
#define ACTION_BUTTON_RELEASE 12
#define BUTTON_PRIMARY 1
#define BUTTON_SECONDARY 2
#define BUTTON_TERTIARY 4
#define BUTTON_BACK 8
#define BUTTON_FORWARD 16
#define BUTTON_LEFT     1
#define BUTTON_MIDDLE   2
#define BUTTON_RIGHT    3
#define BUTTON_X1       4
#define BUTTON_X2       5
#define KEYCODE_W		51
#define KEYCODE_A		29
#define KEYCODE_S		47
#define KEYCODE_D		32
#define KEYCODE_SPACE	62

void processInputs();
static int last_state;
bool scrollFinished = false;

pthread_mutex_t g_mySeriousMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t g_mySeriousThreadId;
bool g_gameRunning = false;
ANativeWindow *g_currentWindow;
bool g_somethingChanged = false;
JavaVM *g_javaWM;
jclass g_NativeEvents;
jmethodID g_reportFatalError;
CTString g_command = "";
CTString g_newText = "";
bool g_textCancelled = false;
CViewPort *g_pvpViewPort = nullptr;
void (*g_textOnOk)(CTString str) = nullptr;
void (*g_textOnCancel)() = nullptr;

CThreadLocal<JNIEnv*> javaEnv;

JNIEnv* getEnv() {
  if (!*javaEnv) {
    g_javaWM->AttachCurrentThread(&javaEnv.get(), nullptr);
  }
  return *javaEnv;
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
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_SeriousSamSurface_nInitialize(JNIEnv* env, jobject obj, jstring homeDir_, jstring libDir_) {
  g_errorCalllback = &androidReportError;

  // set home dir
  const char *homeDir = env->GetStringUTFChars(homeDir_, 0);
  const char *libDir = env->GetStringUTFChars(libDir_, 0);
  _fnmApplicationPath = CTString(homeDir) + "/";
  _fnmApplicationLibPath = CTString(libDir) + "/";
  _fnmApplicationExe = CTFILENAME("Bin/SeriousSam.exe");
  env->ReleaseStringUTFChars(homeDir_, homeDir);
  env->ReleaseStringUTFChars(libDir_, libDir);
  InfoMessage("[libSeriousSamNative] Creating SeriousSamMain thread");

  // start main thread
  pthread_create(&g_mySeriousThreadId, 0, &seriousMain, nullptr);
  pthread_setname_np(g_mySeriousThreadId, "SeriousSamMain");
}

extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_MainActivity_nConfirmEditText(JNIEnv* env, jobject obj, jstring newText_) {
  pthread_mutex_lock(&g_mySeriousMutex);

  const char *newText = env->GetStringUTFChars(newText_, 0);
  g_newText = CTString(newText);
  env->ReleaseStringUTFChars(newText_, newText);

  pthread_mutex_unlock(&g_mySeriousMutex);
}

extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_MainActivity_nCancelEditText(JNIEnv* env, jobject obj) {
  pthread_mutex_lock(&g_mySeriousMutex);
  g_textCancelled = true;
  pthread_mutex_unlock(&g_mySeriousMutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_setAxisValue(JNIEnv *env, jobject obj, jint key, jfloat value) {
  ASSERT(key >= 0 && key < 10);
  pthread_mutex_lock(&g_mySeriousMutex);
  g_cb.g_IncomingControls.axisValue[key] = value;
  pthread_mutex_unlock(&g_mySeriousMutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_shiftAxisValue(JNIEnv *env, jobject obj, jint key, jfloat value) {
  ASSERT(key >= 0 && key < 10);
  pthread_mutex_lock(&g_mySeriousMutex);
  g_cb.g_IncomingControls.shiftAxisValue[key] += value;
  pthread_mutex_unlock(&g_mySeriousMutex);
}

constexpr unsigned int hash(const char *s, int off = 0) {                        
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];                           
} 

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_nTouchKeyEvent(JNIEnv *env, jobject obj, jstring key_, jint isPressed) {
#define BTN_CASE(cod, name) case cod: name = isPressed != 0; break;
  pthread_mutex_lock(&g_mySeriousMutex);
  const char *key = env->GetStringUTFChars(key_, 0);
  switch(hash(key)) {
	// additional keys
	BTN_CASE(hash("Tab"), g_cb.g_IncomingControls.bShowTabInfo)
	BTN_CASE(hash("DropMoney"), g_cb.g_IncomingControls.bDropMoney)
	// end
    BTN_CASE(hash("Fire"), g_cb.g_IncomingControls.bFire)
    BTN_CASE(hash("Use"), g_cb.g_IncomingControls.bUse)
    BTN_CASE(hash("Flip"), g_cb.g_IncomingControls.bWeaponFlip)
    BTN_CASE(hash("SeriousBomb"), g_cb.g_IncomingControls.bFireBomb)
    BTN_CASE(hash("Reload"), g_cb.g_IncomingControls.bReload)
    BTN_CASE(hash("Jump"), g_cb.g_IncomingControls.bMoveUp)
    BTN_CASE(hash("Crouch"), g_cb.g_IncomingControls.bMoveDown)
    BTN_CASE(hash("PrevWeapon"), g_cb.g_IncomingControls.bWeaponPrev)
    BTN_CASE(hash("NextWeapon"), g_cb.g_IncomingControls.bWeaponNext)
    BTN_CASE(hash("Computer"), g_cb.g_IncomingControls.bComputer)
    BTN_CASE(hash("Start"), g_cb.g_IncomingControls.bStart)
  }
  pthread_mutex_unlock(&g_mySeriousMutex);
#undef BTN_CASE
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_nDispatchKeyEvent(JNIEnv *env, jobject obj, jint key, jint isPressed) {
#define BTN_CASE(cod, name) case cod: name = isPressed != 0; break;
  pthread_mutex_lock(&g_mySeriousMutex);
  switch(key) {
    // Xbox 360
    BTN_CASE(KEYCODE_BUTTON_R1, g_cb.g_IncomingControls.bFire)
    BTN_CASE(KEYCODE_BUTTON_R2, g_cb.g_IncomingControls.bUse)
    BTN_CASE(KEYCODE_BUTTON_L1, g_cb.g_IncomingControls.bWeaponFlip)
    BTN_CASE(KEYCODE_BUTTON_Y, g_cb.g_IncomingControls.bFireBomb)
    BTN_CASE(KEYCODE_BUTTON_X, g_cb.g_IncomingControls.bReload)
    BTN_CASE(KEYCODE_BUTTON_A, g_cb.g_IncomingControls.bMoveUp)
    BTN_CASE(KEYCODE_BUTTON_B, g_cb.g_IncomingControls.bMoveDown)
    BTN_CASE(KEYCODE_DPAD_LEFT, g_cb.g_IncomingControls.bWeaponPrev)
    BTN_CASE(KEYCODE_DPAD_RIGHT, g_cb.g_IncomingControls.bWeaponNext)
    BTN_CASE(KEYCODE_BACK, g_cb.g_IncomingControls.bComputer)
    BTN_CASE(KEYCODE_BUTTON_START, g_cb.g_IncomingControls.bStart)
  }
  pthread_mutex_unlock(&g_mySeriousMutex);
#undef BTN_CASE
}

int TranslateButton(int state)
{
    if (state & BUTTON_PRIMARY) {
        return BUTTON_LEFT;
    } else if (state & BUTTON_SECONDARY) {
        return BUTTON_RIGHT;
    } else if (state & BUTTON_TERTIARY) {
        return BUTTON_MIDDLE;
    } else if (state & BUTTON_FORWARD) {
        return BUTTON_X1;
    } else if (state & BUTTON_BACK) {
        return BUTTON_X2;
    } else {
        return 0;
    }
}

void changeWeapon(int weapon)
{
	if (weapon == 1) {
		if (g_cb.g_IncomingControls.bWeaponNext == 0) {
			g_cb.g_IncomingControls.bWeaponNext = 1;
		} else {
			g_cb.g_IncomingControls.bWeaponNext = 1;
			g_cb.g_IncomingControls.bWeaponNext = 0;
		}
		//CPrintF("Button right = %d\n", g_cb.g_IncomingControls.bWeaponNext);
	} else {
		if (g_cb.g_IncomingControls.bWeaponPrev == 0) {
			g_cb.g_IncomingControls.bWeaponPrev = 1;
		} else {
			g_cb.g_IncomingControls.bWeaponPrev = 1;
			g_cb.g_IncomingControls.bWeaponPrev = 0;
		}
		//CPrintF("Button left = %d\n", g_cb.g_IncomingControls.bWeaponPrev);
	}
	return;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_nSendMouseNative(JNIEnv *env, jobject obj, jint state, jint action, jfloat scroll) {
  pthread_mutex_lock(&g_mySeriousMutex);
    int changes;
	int button;
    switch(action) {
        case ACTION_DOWN:
		    changes = state & ~last_state;
            button = TranslateButton(changes);	
			//CPrintF("Mouse button clicked - %d\n", button);
			if (button == 1) {
			g_cb.g_IncomingControls.bFire = 1;
			}
            break;
        case ACTION_UP:
		    changes = state & ~last_state;
            button = TranslateButton(changes);
			//CPrintF("Mouse button released\n");
			if (button == 0) {
			g_cb.g_IncomingControls.bFire = 0;
			}
            break;
        case ACTION_SCROLL:
			if (scroll > 0.0f) {
				//CPrintF("Mouse scrolling up\n");
				//g_cb.g_IncomingControls.bWeaponNext = 1;
				changeWeapon(1);
				} 
				
			if (scroll < 0.0f) {
				//CPrintF("Mouse scrolling down\n");
				//g_cb.g_IncomingControls.bWeaponPrev = 1;
				changeWeapon(2);	
				}
			
				//CPrintF("g_cb.g_IncomingControls.bWeaponNext= %f\n", g_cb.g_IncomingControls.bWeaponNext);
				//CPrintF("g_cb.g_IncomingControls.bWeaponPrev= %f\n", g_cb.g_IncomingControls.bWeaponPrev);
				//CPrintF("Mouse scroll= %f\n", scroll); 
				break;
        default:
            break;
    }
  pthread_mutex_unlock(&g_mySeriousMutex);
}
/*
extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_nSendKeyboardButtonDown(JNIEnv *env, jobject obj, jint key) {
  pthread_mutex_lock(&g_mySeriousMutex);
    switch(key) {
        case KEYCODE_W:		
			CPrintF("W pressed!");
			//g_cb.g_IncomingControls.bFire = 1;
            break;
        case KEYCODE_A:
			CPrintF("A pressed!");
            break;
        default:
            break;
    }
  pthread_mutex_unlock(&g_mySeriousMutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_nSendKeyboardButtonUp(JNIEnv *env, jobject obj, jint key) {
  pthread_mutex_lock(&g_mySeriousMutex);
    switch(key) {
        case KEYCODE_W:
			CPrintF("W released!\n");
			//g_cb.g_IncomingControls.bFire = 1;
            break;
        case KEYCODE_A:
			CPrintF("A released!\n");
            break;
        default:
            break;
    }
  pthread_mutex_unlock(&g_mySeriousMutex);
}
*/
extern "C"
JNIEXPORT void JNICALL Java_com_github_aarcangeli_serioussamandroid_MainActivity_nShellExecute(JNIEnv* env, jobject obj, jstring command_) {
  const char *command = env->GetStringUTFChars(command_, 0);
  pthread_mutex_lock(&g_mySeriousMutex);

  g_command += CTString(command) + ";";

  pthread_mutex_unlock(&g_mySeriousMutex);
  env->ReleaseStringUTFChars(command_, command);
}

void refreshJavaState() {
  JNIEnv* env = getEnv();
  jmethodID method = env->GetStaticMethodID(g_NativeEvents, "reportStateChange", "(II)V");
  ASSERT(method);
  env->CallStaticVoidMethod(g_NativeEvents, method, (int) g_cb.gameState, (int) g_cb.seriousBombCount);
}

void updateUI() {
  JNIEnv* env = getEnv();
  jmethodID method = env->GetStaticMethodID(g_NativeEvents, "updateUI", "()V");
  ASSERT(method);
  env->CallStaticVoidMethod(g_NativeEvents, method);
}

void setSeriousState(GameState state) {
  if (g_cb.gameState != state) {
    g_cb.gameState = state;
    refreshJavaState();
  }
  	updateUI();
}

void setSeriousBombCount(int bombs) {
  if (g_cb.seriousBombCount != bombs) {
    g_cb.seriousBombCount = bombs;
    refreshJavaState();
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
	
    CTString cmdToExecute = g_command;
    g_command = "";

    if (g_newText.Length()) {
      if (g_textOnOk) {
        g_textOnOk(g_newText);
        g_textOnOk = nullptr;
      }
      g_newText = "";
    }
    if (g_textCancelled) {
      if (g_textOnCancel) {
        g_textOnCancel();
        g_textOnCancel = nullptr;
      }
      g_textCancelled = false;
    }

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

void openSettings() {
  JNIEnv* env = getEnv();
  jmethodID method = env->GetStaticMethodID(g_NativeEvents, "openSettings", "()V");
  ASSERT(method);
  env->CallStaticVoidMethod(g_NativeEvents, method);
}

void editText(const CTString &string, void (*onOk)(CTString str), void (*onCancel)()) {
  g_textOnOk = onOk;
  g_textOnCancel = onCancel;

  JNIEnv* env = getEnv();
  jmethodID method = env->GetStaticMethodID(g_NativeEvents, "editText", "(Ljava/lang/String;)V");
  ASSERT(method);
  env->CallStaticVoidMethod(g_NativeEvents, method, env->NewStringUTF(string.str_String));
}

void requestRestard() {
  JNIEnv* env = getEnv();
  jmethodID method = env->GetStaticMethodID(g_NativeEvents, "requestRestard", "()V");
  ASSERT(method);
  env->CallStaticVoidMethod(g_NativeEvents, method);
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
  g_cb.openSettings = &openSettings;
  g_cb.editText = &editText;
  g_cb.restart = &requestRestard;
  g_cb.setSeriousBombCount = &setSeriousBombCount;
  InfoMessage("[libSeriousSamNative] Starting game");

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

void androidReportError(bool fatal, CTString error) {
  JNIEnv* env = getEnv();
  jmethodID method = env->GetStaticMethodID(g_NativeEvents, "reportError", "(ZLjava/lang/String;)V");
  ASSERT(method);
  env->CallStaticVoidMethod(g_NativeEvents, method, fatal, env->NewStringUTF(error.str_String));
}
