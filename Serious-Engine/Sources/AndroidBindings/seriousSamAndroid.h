#ifndef SERIOUSSAMANDROID_H
#define SERIOUSSAMANDROID_H

#include <Engine/StdH.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <pthread.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/Stream.h>
#include <Engine/Engine.h>

static_assert(sizeof(void *) == 4, "Serious engine require 32 bit address space");

extern pthread_mutex_t g_mySeriousMutex;
extern pthread_t g_mySeriousThreadId;
extern pthread_t g_gameRunning;
extern ANativeWindow *g_currentWindow;
extern float g_uiScale;

void seriousSamDoGame(CDrawPort *pvp);

#define MAX_AXIS 10
#define AXIS_SHIFT 1
#define NS_IN_S 1000000000
extern float g_AxisValue[MAX_AXIS];

#endif // SERIOUSSAMANDROID_H
