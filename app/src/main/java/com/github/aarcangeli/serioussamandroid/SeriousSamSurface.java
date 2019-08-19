package com.github.aarcangeli.serioussamandroid;

import android.content.Context;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class SeriousSamSurface extends SurfaceView implements GestureDetector.OnGestureListener, GestureDetector.OnDoubleTapListener {
    private final GestureDetector gestureDetector;
    private MainActivity activity;
    public float scale = 0.5f;

    public SeriousSamSurface(Context context) {
        this(context, null);
    }

    public SeriousSamSurface(Context context, AttributeSet attrs) {
        super(context, attrs);

        // load native library
        loadNativeLibrary();

        gestureDetector = new GestureDetector(context, this);
        gestureDetector.setIsLongpressEnabled(false);
        gestureDetector.setOnDoubleTapListener(this);

        getHolder().setKeepScreenOn(true);
        getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                nSetSurface(holder.getSurface());
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                nSetSurface(null);
            }
        });

        setFocusable(true);
        setFocusableInTouchMode(true);
    }

    void start() {
        nOnStart();
    }

    void stop() {
        nOnStop();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        float width = MeasureSpec.getSize(widthMeasureSpec);
        float height = MeasureSpec.getSize(heightMeasureSpec);
        getHolder().setFixedSize((int) (width * scale), (int) (height * scale));
    }

    public float getScale() {
        return scale;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            int x = (int) (event.getX() * scale);
            int y = (int) (event.getY() * scale);
            MainActivity.executeShell("TouchDown(" + x + ", " + y + ");");
        }
        if (event.getAction() == MotionEvent.ACTION_MOVE) {
            int x = (int) (event.getX() * scale);
            int y = (int) (event.getY() * scale);
            MainActivity.executeShell("TouchMove(" + x + ", " + y + ");");
        }
        if (event.getAction() == MotionEvent.ACTION_UP) {
            int x = (int) (event.getX() * scale);
            int y = (int) (event.getY() * scale);
            MainActivity.executeShell("TouchUp(" + x + ", " + y + ");");
        }
        gestureDetector.onTouchEvent(event);
        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e) {
        return false;
    }

    @Override
    public boolean onDoubleTap(MotionEvent e) {
        return false;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e) {
        return false;
    }

    @Override
    public boolean onDown(MotionEvent e) {
        return true;
    }

    @Override
    public void onShowPress(MotionEvent e) {
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e) {
        return false;
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
        return false;
    }

    @Override
    public void onLongPress(MotionEvent e) {
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        return false;
    }

    @Override
    public boolean onKeyPreIme(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && this.activity != null) {
            this.activity.keyboardHidden();
        }
        return super.onKeyPreIme(keyCode, event);
    }

    private static void loadNativeLibrary() {
        synchronized (SeriousSamSurface.class) {
            if (!isLoaded) {
                System.loadLibrary("SeriousSamNatives");
                isLoaded = true;
            }
        }
    }

    public static void initializeLibrary(String homeDir) {
        loadNativeLibrary();
        synchronized (SeriousSamSurface.class) {
            if (!isInitialized) {
                nInitialize(homeDir);
                isInitialized = true;
            }
        }
    }

    public void setActivity(MainActivity activity) {
        this.activity = activity;
    }

    // native bindings
    private static boolean isLoaded, isInitialized;
    private static native void nInitialize(String homeDir);
    private static native void nSetSurface(Surface surface);
    private static native void nOnStart();
    private static native void nOnStop();

}
