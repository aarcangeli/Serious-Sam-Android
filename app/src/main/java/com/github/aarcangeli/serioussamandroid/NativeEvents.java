package com.github.aarcangeli.serioussamandroid;

import org.greenrobot.eventbus.EventBus;

// invoked from native
public class NativeEvents {
    public static void reportFatalError(String message) {
        EventBus.getDefault().postSticky(new FatalErrorEvent(message));
    }

    public static void reportStateChange(int state) {
        EventBus.getDefault().postSticky(new StateChangeEvent(GameState.values()[state]));
    }

    public static void openSettings() {
        EventBus.getDefault().post(new OpenSettingsEvent());
    }

    public static class FatalErrorEvent {
        public final String message;

        public FatalErrorEvent(String message) {
            this.message = message;
        }
    }

    public static class StateChangeEvent {
        public final GameState state;

        public StateChangeEvent(GameState state) {
            this.state = state;
        }
    }

    public static class OpenSettingsEvent {
    }

    enum GameState {
        LOADING,
        INTRO,
        MENU,
        DEMO,
        CONSOLE,
        NORMAL,
        QUIT_SCREEN,
    }
}
