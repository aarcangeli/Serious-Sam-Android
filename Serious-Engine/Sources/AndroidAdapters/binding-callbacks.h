#ifndef BINDING_CALLBACKS_H
#define BINDING_CALLBACKS_H

#include <AndroidAdapters/android-adapters.h>
#include <Engine/Base/CTString.h>
#include <config.h>

#include <AndroidAdapters/playerCommons.h>

enum GameState {
    GS_LOADING,
    GS_INTRO,
    GS_MENU,
    GS_DEMO,
    GS_CONSOLE,
    GS_COMPUTER,
    GS_NORMAL,
    GS_QUIT_SCREEN,
};

struct BindingCallbacks {
    void (*setSeriousState)(GameState state);
    void (*syncSeriousThreads)();
    CViewPort *(*getViewPort)();
    void (*openSettings)();
    void (*editText)(const CTString &string, void (*onOk)(CTString str), void (*onCancel)());
    void (*restart)();
    void (*setSeriousBombCount)(int bombs);
	PlayerControls g_IncomingControls {};
    GameState gameState;
    int seriousBombCount = 0;
    int isShiftPressed = false;
    float globalScale = 1;
    CTString WifiIP = "";
    bool drawBanner = true;
    int ping = 0;
#ifdef FIRST_ENCOUNTER
    bool tfe = true;
#else
    bool tfe ;
#endif
};

extern BindingCallbacks g_cb;

#endif // BINDING_CALLBACKS_H
