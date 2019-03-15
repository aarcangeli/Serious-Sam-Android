package com.github.aarcangeli.serioussamandroid;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.hardware.input.InputManager;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.io.File;

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

    private static final float VIEW_MULT = 2.5f;

    private SeriousSamSurface glSurfaceView;
    private File homeDir;

    private boolean isGameStarted = false;
    private JoyStick leftStick;
    private JoyStick rightStick;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main_screen);
        glSurfaceView = findViewById(R.id.main_content);
        leftStick = findViewById(R.id.left_stick);
        rightStick = findViewById(R.id.right_stick);

        leftStick.setListener(new JoyStick.JoyStickListener() {
            @Override
            public void onMove(JoyStick joyStick, double angle, double power, int direction) {
                setAxisValue(AXIS_MOVE_FB, (float) (Math.sin(angle) * power / 100));
                setAxisValue(AXIS_MOVE_LR, (float) (Math.cos(angle) * power / 100));
            }

            @Override
            public void onTap() {
            }

            @Override
            public void onDoubleTap() {
            }
        });

        rightStick.setListener(new JoyStick.JoyStickListener() {
            @Override
            public void onMove(JoyStick joyStick, double angle, double power, int direction) {
                setAxisValue(AXIS_LOOK_UD, (float) (Math.sin(angle) * power / 100) * VIEW_MULT);
                setAxisValue(AXIS_LOOK_LR, (float) (Math.cos(angle) * power / 100) * VIEW_MULT);
            }

            @Override
            public void onTap() {
            }

            @Override
            public void onDoubleTap() {
            }
        });

        Button jumpBtn = findViewById(R.id.jumpBtn);
        jumpBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_A, 1);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_A, 0);
                }
                return false;
            }
        });

        Button fireBtn = findViewById(R.id.fireBtn);
        fireBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_R1, 1);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_R1, 0);
                }
                return false;
            }
        });

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

        homeDir = getHomeDir();
        Log.i(TAG, "HomeDir: " + homeDir);

        if (!hasStoragePermission(this)) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_WRITE_STORAGE);
        } else {
            startGame();
        }

        updateSoftKeyboardVisible();
    }

    public void updateSoftKeyboardVisible() {
        int keyboardVisibility = Utils.isThereControllers() ? View.GONE : View.VISIBLE;
        leftStick.setVisibility(keyboardVisibility);
        rightStick.setVisibility(keyboardVisibility);
        findViewById(R.id.fireBtn).setVisibility(keyboardVisibility);
        findViewById(R.id.jumpBtn).setVisibility(keyboardVisibility);
    }

    @Override
    protected void onStart() {
        super.onStart();
        EventBus.getDefault().register(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        EventBus.getDefault().unregister(this);
    }

    @Subscribe(sticky = true, threadMode = ThreadMode.MAIN)
    public void onFatalError(NativeEvents.FatalErrorEvent event) {
        AlertDialog.Builder dlgAlert = new AlertDialog.Builder(this);
        dlgAlert.setMessage(event.message);
        dlgAlert.setTitle("Fatal Error");
        dlgAlert.setPositiveButton("Close", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                System.exit(1);
            }
        });
        dlgAlert.setCancelable(false);
        dlgAlert.create().show();
    }

    @Override
    protected void onResume() {
        super.onResume();
        // hide verything from the screen
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_USER_LANDSCAPE);
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);

        if (isGameStarted) {
            glSurfaceView.start();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        glSurfaceView.stop();
    }

    private static boolean hasStoragePermission(Context context) {
        return ContextCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
    }

    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent ev) {
        if (ev.getAction() == MotionEvent.ACTION_MOVE) {
            setAxisValue(AXIS_MOVE_FB, -ev.getAxisValue(MotionEvent.AXIS_Y));
            setAxisValue(AXIS_MOVE_LR, -ev.getAxisValue(MotionEvent.AXIS_X));
            setAxisValue(AXIS_LOOK_LR, -ev.getAxisValue(MotionEvent.AXIS_Z) * VIEW_MULT);
            setAxisValue(AXIS_LOOK_UD, -ev.getAxisValue(MotionEvent.AXIS_RZ) * VIEW_MULT);
            nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_R2, ev.getAxisValue(MotionEvent.AXIS_RTRIGGER) > .5f ? 1 : 0);
            nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_L2, ev.getAxisValue(MotionEvent.AXIS_RTRIGGER) < -.5f ? 1 : 0);
            nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_LEFT, ev.getAxisValue(MotionEvent.AXIS_HAT_X) < -.5f ? 1 : 0);
            nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_RIGHT, ev.getAxisValue(MotionEvent.AXIS_HAT_X) > .5f ? 1 : 0);
            nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_UP, ev.getAxisValue(MotionEvent.AXIS_HAT_Y) < -.5f ? 1 : 0);
            nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_DOWN, ev.getAxisValue(MotionEvent.AXIS_HAT_Y) > .5f ? 1 : 0);
        }
        return true;
    }

    public static void tryPremain(Context context) {
        if (hasStoragePermission(context)) {
            File homeDir = getHomeDir();
            if (!homeDir.exists()) homeDir.mkdirs();
            SeriousSamSurface.initializeLibrary(homeDir.getAbsolutePath());
        }
    }

    @NonNull
    private static File getHomeDir() {
        return new File(Environment.getExternalStorageDirectory(), "SeriousSam").getAbsoluteFile();
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        if (keyCode == KeyEvent.KEYCODE_VOLUME_UP || keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            return false;
        }
        if (event.getRepeatCount() == 0) {
            if (event.getAction() == KeyEvent.ACTION_DOWN) {
                nDispatchKeyEvent(keyCode, 1);
                System.out.println(keyCode);
            }
            if (event.getAction() == KeyEvent.ACTION_UP) {
                nDispatchKeyEvent(keyCode, 0);
            }
        }
        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == REQUEST_WRITE_STORAGE) {
            if (grantResults.length > 0) {
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    startGame();
                } else {
                    finish();
                }
            }
        }
    }

    private void requestPermission() {
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_WRITE_STORAGE);
    }

    // ui listeners
    public void hideMenu(View view) {
        nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_R2, 1);
    }

    public void doProfiling(View view) {
        printProfilingData();
    }

    private void startGame() {
        if (!homeDir.exists()) homeDir.mkdirs();
        SeriousSamSurface.initializeLibrary(homeDir.getAbsolutePath());
        isGameStarted = true;
        glSurfaceView.start();
    }

    private static native void setAxisValue(int key, float value);
    private static native void printProfilingData();
    private static native void nDispatchKeyEvent(int key, int isPressed);
}
