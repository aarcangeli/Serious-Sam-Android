package com.github.aarcangeli.serioussamandroid;

import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.View;

public class MyDragListener implements View.OnTouchListener {
    private static final String TAG = "SeriousSamJava";
    private MainActivity mainActivity;
    private boolean isDragging = false;
    private int currentIndex;
    private float lastX, lastY;

    private float currentStackX, currentStackY;

    public MyDragListener(MainActivity mainActivity) {
        this.mainActivity = mainActivity;
    }

    @Override
    public boolean onTouch(View view, MotionEvent motionEvent) {
        synchronized (MainActivity.GAME_INPUT_LOCK) {
            int action = motionEvent.getActionMasked();
            int index = motionEvent.getActionIndex();
            DisplayMetrics dm = mainActivity.getResources().getDisplayMetrics();
            float x = motionEvent.getX() * (160.0f / dm.densityDpi);
            float y = motionEvent.getY() * (160.0f / dm.densityDpi);
            if (!isDragging && (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_POINTER_DOWN)) {
                lastX = x;
                lastY = y;
                isDragging = true;
                currentIndex = index;
                return true;
            }
            if (isDragging && action == MotionEvent.ACTION_MOVE && currentIndex == index) {
                float deltaX = x - lastX;
                float deltaY = y - lastY;
                lastX = x;
                lastY = y;
                currentStackX += deltaX;
                currentStackY += deltaY;
                return true;
            }
            if (currentIndex == index && (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_POINTER_UP || action == MotionEvent.ACTION_CANCEL)) {
                isDragging = false;
                return true;
            }
        }
        return false;
    }

    public float getCurrentStackX() {
        return currentStackX;
    }

    public float getCurrentStackY() {
        return currentStackY;
    }

    public void clearCurrentStack() {
        currentStackX = 0;
        currentStackY = 0;
    }
}
