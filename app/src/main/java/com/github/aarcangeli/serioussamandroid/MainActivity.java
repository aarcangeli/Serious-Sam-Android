package com.github.aarcangeli.serioussamandroid;

import android.Manifest;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.hardware.input.InputManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;

import java.io.File;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity {
    public static final String TAG = "SeriousSamJava";
    private final int REQUEST_WRITE_STORAGE = 1;

    private static final int AXIS_MOVE_UD = 0;
    private static final int AXIS_MOVE_LR = 1;
    private static final int AXIS_MOVE_FB = 2;
    private static final int AXIS_TURN_UD = 3;
    private static final int AXIS_TURN_LR = 4;
    private static final int AXIS_TURN_BK = 5;
    private static final int AXIS_LOOK_UD = 6;
    private static final int AXIS_LOOK_LR = 7;
    private static final int AXIS_LOOK_BK = 8;

    private MyGLSurface glSurfaceView;
    private File homeDir;
    private JoyStick leftStick, rightStick;
    public static final Object GAME_INPUT_LOCK = new Object();

    private AtomicBoolean toggleConsoleState = new AtomicBoolean();
    private AtomicBoolean printProfiling = new AtomicBoolean();
    private float DRAG_SENSIBILITY = 0.3f;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        System.loadLibrary("SeriousSamNatives");
        super.onCreate(savedInstanceState);

        homeDir = new File(Environment.getExternalStorageDirectory(), "SeriousSam");

        setContentView(R.layout.main_screen);
        leftStick = findViewById(R.id.left_stick);
        rightStick = findViewById(R.id.right_stick);
        glSurfaceView = findViewById(R.id.main_content);

        InputManager systemService = (InputManager) getSystemService(Context.INPUT_SERVICE);
        systemService.registerInputDeviceListener(new InputManager.InputDeviceListener() {
            @Override
            public void onInputDeviceAdded(int deviceId) {
                updateSoftKeyboardVisible();
            }

            @Override
            public void onInputDeviceRemoved(int deviceId) {
                updateSoftKeyboardVisible();
            }

            @Override
            public void onInputDeviceChanged(int deviceId) {
                updateSoftKeyboardVisible();
            }
        }, null);

        leftStick.setListener(new LeftJoystickListener());
        rightStick.setListener(new RightJoystickListener());

        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);

        checkPermission();
        updateSoftKeyboardVisible();
    }

    @Override
    protected void onResume() {
        super.onResume();
        // hide verything from the screen
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
    }

    private void checkPermission() {
        int permission = ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);
        if (permission != PackageManager.PERMISSION_GRANTED) {
            requestPermission();
        } else {
            setup();
        }
    }

    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent ev) {
        if (ev.getAction() == MotionEvent.ACTION_MOVE) {
            synchronized (GAME_INPUT_LOCK) {
                setAxisValue(AXIS_MOVE_FB, -ev.getAxisValue(MotionEvent.AXIS_Y));
                setAxisValue(AXIS_MOVE_LR, -ev.getAxisValue(MotionEvent.AXIS_X));
                setAxisValue(AXIS_LOOK_LR, -ev.getAxisValue(MotionEvent.AXIS_Z));
                setAxisValue(AXIS_LOOK_UD, -ev.getAxisValue(MotionEvent.AXIS_RZ));
            }
        }
        return true;
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (event.getAction() == KeyEvent.ACTION_DOWN && event.getRepeatCount() == 0) {
            switch (event.getKeyCode()) {
                case KeyEvent.KEYCODE_BUTTON_START:
                    toggleConsoleState.set(true);
                    break;
            }
        }
        return true;
    }

    private void requestPermission() {
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_WRITE_STORAGE);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == REQUEST_WRITE_STORAGE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                setup();
            } else {
                requestPermission();
            }
        }
    }

    public void updateSoftKeyboardVisible() {
        int keyboardVisibility = isThereControllers() ? View.GONE : View.VISIBLE;
        leftStick.setVisibility(keyboardVisibility);
        rightStick.setVisibility(keyboardVisibility);
    }

    private boolean isThereControllers() {
        for (int id : InputDevice.getDeviceIds()) {
            InputDevice dev = InputDevice.getDevice(id);
            int sources = dev.getSources();
            if ((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) {
                return true;
            } else if ((sources & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK) {
                return true;
            }
        }
        return false;
    }

    // ui listeners
    public void hideMenu(View view) {
        toggleConsoleState.set(true);
    }

    public void doProfiling(View view) {
        printProfiling.set(true);
    }

    private class LeftJoystickListener implements JoyStick.JoyStickListener {
        @Override
        public void onMove(JoyStick joyStick, double angle, double power, int direction) {
            synchronized (GAME_INPUT_LOCK) {
                setAxisValue(AXIS_MOVE_FB, (float) (Math.sin(angle) * power / 100));
                setAxisValue(AXIS_MOVE_LR, (float) (Math.cos(angle) * power / 100));
            }
        }

        @Override
        public void onTap() {
        }

        @Override
        public void onDoubleTap() {
        }
    }

    private class RightJoystickListener implements JoyStick.JoyStickListener {
        @Override
        public void onMove(JoyStick joyStick, double angle, double power, int direction) {
            synchronized (GAME_INPUT_LOCK) {
                setAxisValue(AXIS_LOOK_UD, (float) (Math.sin(angle) * power / 100));
                setAxisValue(AXIS_LOOK_LR, (float) (Math.cos(angle) * power / 100));
            }
        }

        @Override
        public void onTap() {
        }

        @Override
        public void onDoubleTap() {
        }
    }

    private void setup() {
        if (!homeDir.exists()) homeDir.mkdirs();
        Log.i(TAG, "HomeDir: " + homeDir.getAbsolutePath());
        setHomeDir(homeDir.getAbsolutePath());

        glSurfaceView.getHolder().setKeepScreenOn(true);
        glSurfaceView.setEGLContextClientVersion(2);
        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                startEngine();
                init();
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                resize(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                synchronized (GAME_INPUT_LOCK) {
                    processInputs();
                }
                if (toggleConsoleState.getAndSet(false)) toggleConsoleState();
                setUIScale(glSurfaceView.getScale() * Utils.convertDpToPixel(1, MainActivity.this));
                doGame();
                if (printProfiling.getAndSet(false)) printProfilingData();
            }
        });
    }

    public native void setHomeDir(String homeDir);

    public native void init();

    private native void startEngine();

    private native void resize(int width, int height);

    private native void processInputs();

    private native void doGame();

    private native void setUIScale(float scale);

    private native void toggleConsoleState();

    private native void setAxisValue(int key, float value);

    private native void printProfilingData();
}
