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
#include <EntitiesMP/Common/playerCommons.h>
#include <stdint.h>

static_assert(sizeof(void *) == 4, "Serious engine require 32 bit address space");

enum GameState {
    GS_LOADING,
    GS_CONSOLE,
    GS_NORMAL,
};

extern pthread_mutex_t g_mySeriousMutex;
extern pthread_t g_mySeriousThreadId;
extern bool g_gameRunning;
extern ANativeWindow *g_currentWindow;
extern bool g_printProfiling;
extern GameState g_gameState;

void *seriousMain(void *unused);
void startSeriousPrestart();
void seriousSamDoGame(CDrawPort *pvp);
void hideComputerConsole();
void toggleComputer();

void setControls(PlayerControls &ctrls);
void androidReportError(CTString error);

#define NS_IN_S 1000000000

#endif // SERIOUSSAMANDROID_H
