package com.github.aarcangeli.serioussamandroid;

import org.greenrobot.eventbus.EventBus;

public class NativeEvents {
    // invoked from native
    public static void reportFatalError(String message) {
        EventBus.getDefault().postSticky(new FatalErrorEvent(message));
    }

    public static class FatalErrorEvent {
        public final String message;

        public FatalErrorEvent(String message) {
            this.message = message;
        }
    }
}
