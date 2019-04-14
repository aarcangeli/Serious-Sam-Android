#ifndef BINDING_CALLBACKS_H
#define BINDING_CALLBACKS_H

#include <AndroidAdapters/android-adapters.h>

enum GameState {
    GS_LOADING,
    GS_INTRO,
    GS_MENU,
    GS_DEMO,
    GS_CONSOLE,
    GS_NORMAL,
    GS_QUIT_SCREEN,
};

struct BindingCallbacks {
    void (*setSeriousState)(GameState state);
    void (*syncSeriousThreads)();
    CViewPort *(*getViewPort)();
    void (*openSettings)();

    GameState gameState;
};

extern BindingCallbacks g_cb;

#endif // BINDING_CALLBACKS_H
