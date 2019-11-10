package com.github.aarcangeli.serioussamandroid;

import org.greenrobot.eventbus.EventBus;

// invoked from native
public class NativeEvents {
    public static void reportError(boolean fatal, String message) {
        EventBus.getDefault().postSticky(new ErrorEvent(fatal, message));
    }

    public static void reportStateChange(int state, int bombs) {
        EventBus.getDefault().postSticky(new StateChangeEvent(GameState.values()[state], bombs));
    }

    public static void openSettings() {
        EventBus.getDefault().post(new OpenSettingsEvent());
    }

    public static void requestRestard() {
        EventBus.getDefault().post(new RestartEvent());
    }

    public static void editText(String defaultText) {
        EventBus.getDefault().post(new EditTextEvent(defaultText));
    }

    public static class ErrorEvent {
        public final boolean fatal;
        public final String message;

        public ErrorEvent(boolean fatal, String message) {
            this.fatal = fatal;
            this.message = message;
        }
    }

    public static class StateChangeEvent {
        public final GameState state;
        public final int bombs;

        public StateChangeEvent(GameState state, int bombs) {
            this.state = state;
            this.bombs = bombs;
        }
    }

    public static class OpenSettingsEvent {
    }

    public static class RestartEvent {
    }

    public static class EditTextEvent {
        public final String defaultText;

        public EditTextEvent(String defaultText) {
            this.defaultText = defaultText;
        }
    }

    enum GameState {
        LOADING,
        INTRO,
        MENU,
        DEMO,
        CONSOLE,
        COMPUTER,
        NORMAL,
        QUIT_SCREEN,
    }
}
