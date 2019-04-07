package com.github.aarcangeli.serioussamandroid.views;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RadialGradient;
import android.graphics.Shader;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import com.github.aarcangeli.serioussamandroid.Utils;

public class BgTrackerView extends View {
    public static final String TAG = "SeriousSamInput";
    private float posX;
    private float posY;
    private Listener listener;

    public BgTrackerView(Context context) {
        this(context, null);
    }

    public BgTrackerView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    @SuppressLint("ClickableViewAccessibility")
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getAction();
        switch (action) {
            case MotionEvent.ACTION_DOWN:
                posX = event.getX();
                posY = event.getY();
                break;
            case MotionEvent.ACTION_MOVE:
                float x = event.getX();
                float y = event.getY();
                float deltaX = posX - x;
                float deltaY = posY - y;
                posX = x;
                posY = y;
                if (listener != null) {
                    listener.move(Utils.convertPixelsToDp(deltaX, getContext()), Utils.convertPixelsToDp(deltaY, getContext()));
                }
                break;
        }
        return true;
    }

    public void setListener(Listener listener) {
        this.listener = listener;
    }

    public interface Listener {
        void move(float deltaX, float deltaY);
    }
}
