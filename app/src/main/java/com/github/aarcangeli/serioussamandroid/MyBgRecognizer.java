package com.github.aarcangeli.serioussamandroid;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

public class MyBgRecognizer extends View {
    private static final String TAG = "SeriousSam::GL";

    public MyBgRecognizer(Context context) {
        this(context, null);
    }

    public MyBgRecognizer(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                Log.i(TAG, "ACTION_DOWN");
                return true;
            case MotionEvent.ACTION_MOVE:
                Log.i(TAG, "ACTION_MOVE");
                break;
            case MotionEvent.ACTION_UP:
                Log.i(TAG, "ACTION_UP");
                break;
            case MotionEvent.ACTION_CANCEL:
                Log.i(TAG, "ACTION_CANCEL");
                break;
            default:
                Log.i(TAG, "OTHER");
                break;
        }

        return false;
    }
}
