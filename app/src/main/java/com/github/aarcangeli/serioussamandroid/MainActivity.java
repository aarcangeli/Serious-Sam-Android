package com.github.aarcangeli.serioussamandroid;

import android.Manifest;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
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
import android.view.WindowManager;

import com.erz.joysticklibrary.JoyStick;

import java.io.File;
import java.util.ArrayList;
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

    private GLSurfaceView glSurfaceView;
    private File homeDir;
    private JoyStick leftStick, rightStick;
    private final Object GAME_INPUT_LOCK = new Object();

    private AtomicBoolean toggleConsoleState = new AtomicBoolean();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        System.loadLibrary("SeriousSamNatives");
        super.onCreate(savedInstanceState);

        // hide verything from the screen
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE | View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);

        homeDir = new File(Environment.getExternalStorageDirectory(), "SeriousSam");

        setContentView(R.layout.main_screen);
//        JoyStick joyStick = new JoyStick(this);
//        setContentView(joyStick);
        glSurfaceView = findViewById(R.id.main_content);
        leftStick = findViewById(R.id.left_stick);
        rightStick = findViewById(R.id.right_stick);
        rightStick.setVisibility(View.INVISIBLE);
        leftStick.setVisibility(View.INVISIBLE);

        leftStick.setListener(new JoyStick.JoyStickListener() {
            @Override
            public void onMove(JoyStick joyStick, double angle, double power, int direction) {
            }

            @Override
            public void onTap() {
            }

            @Override
            public void onDoubleTap() {
            }
        });

        checkPermission();
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
            Log.i(TAG, "dispatchKeyEvent: " + event.getKeyCode());
        }
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

    public ArrayList<Integer> getGameControllerIds() {
        ArrayList<Integer> gameControllerDeviceIds = new ArrayList<Integer>();
        int[] deviceIds = InputDevice.getDeviceIds();
        for (int deviceId : deviceIds) {
            InputDevice dev = InputDevice.getDevice(deviceId);
            int sources = dev.getSources();

            // Verify that the device has gamepad buttons, control sticks, or both.
            if (((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
                    ((sources & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)) {
                // This device is a game controller. Store its device ID.
                if (!gameControllerDeviceIds.contains(deviceId)) {
                    gameControllerDeviceIds.add(deviceId);
                }
            }
        }
        return gameControllerDeviceIds;
    }

    private void setup() {
        if (!homeDir.exists()) homeDir.mkdirs();
        Log.i(TAG, "HomeDir: " + homeDir.getAbsolutePath());
        setHomeDir(homeDir.getAbsolutePath());

        glSurfaceView.getHolder().setKeepScreenOn(true);
//        glSurfaceView.getHolder().setFixedSize(100, 100);
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
                doGame();
            }
        });
    }

    public native void setHomeDir(String homeDir);

    public native void init();

    private native void startEngine();

    private native void resize(int width, int height);

    private native void processInputs();

    private native void doGame();

    private native void toggleConsoleState();

    private native void setAxisValue(int key, float value);
}
