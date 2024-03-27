#ifndef SERIOUSSAMANDROID_H
#define SERIOUSSAMANDROID_H

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <pthread.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/Stream.h>
#include <Engine/Engine.h>
#include <stdint.h>
#include <AndroidAdapters/binding-callbacks.h>

extern pthread_mutex_t g_mySeriousMutex;
extern pthread_t g_mySeriousThreadId;
extern bool g_gameRunning;
extern ANativeWindow *g_currentWindow;

void *seriousMain(void *unused);
void seriousSubMain();
void hideComputerConsole();
void toggleComputer();

void setControls(PlayerControls &ctrls);
void androidReportError(bool fatal, CTString error);

#define NS_IN_S 1000000000

#endif // SERIOUSSAMANDROID_H
