package com.github.aarcangeli.serioussamandroid;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.input.InputManager;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;

import com.github.aarcangeli.serioussamandroid.views.BgTrackerView;
import com.github.aarcangeli.serioussamandroid.views.JoystickView;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.io.File;

import static com.github.aarcangeli.serioussamandroid.NativeEvents.FatalErrorEvent;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.GameState;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.StateChangeEvent;
import static com.github.aarcangeli.serioussamandroid.views.JoystickView.Listener;

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

    public static final float DEAD_ZONE = 0.3f;
    private static final float MULT_VIEW_CONTROLLER = 2.5f;
    private static final float MULT_VIEW_TRACKER = 0.4f;
    private static final float MULT_VIEW_GYROSCOPE = 0.8f;

    private SeriousSamSurface glSurfaceView;
    private File homeDir;

    private boolean isGameStarted = false;
    private SensorManager sensorManager;
    private SensorEventListener motionListener;
    private volatile GameState gameState = GameState.LOADING;
    private boolean isThereController;
    private boolean useGyroscope;

    @Override
    @SuppressLint("ClickableViewAccessibility")
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main_screen);
        glSurfaceView = findViewById(R.id.main_content);

        Button loadBtn = findViewById(R.id.buttonLoad);
        loadBtn.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                executeShell("sam_bMenuLoad=1;");
                return true;
            }
        });

        Button saveBtn = findViewById(R.id.buttonSave);
        saveBtn.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                executeShell("sam_bMenuSave=1;");
                return true;
            }
        });

        Button useBtn = findViewById(R.id.input_use);
        useBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_R2, 1);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_R2, 0);
                }
                return false;
            }
        });

        Button crunchBtn = findViewById(R.id.input_crunch);
        crunchBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_B, 1);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_B, 0);
                }
                return false;
            }
        });

        Button jumpBtn = findViewById(R.id.input_jump);
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

        Button prevBtn = findViewById(R.id.buttonPrev);
        prevBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_LEFT, 1);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_LEFT, 0);
                }
                return false;
            }
        });

        Button nextbtn = findViewById(R.id.buttonNext);
        nextbtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_RIGHT, 1);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_RIGHT, 0);
                }
                return false;
            }
        });

        Button fireBtn = findViewById(R.id.input_fire);
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

        JoystickView joystick = findViewById(R.id.input_overlay);
        joystick.setListener(new Listener() {
            @Override
            public void onMove(float deltaX, float deltaY) {
                if (gameState == GameState.NORMAL) {
                    setAxisValue(AXIS_MOVE_LR, deltaX);
                    setAxisValue(AXIS_MOVE_FB, deltaY);
                }
            }
        });

        BgTrackerView bgTracker = findViewById(R.id.bgTrackerView);
        bgTracker.setListener(new BgTrackerView.Listener() {
            @Override
            public void move(float deltaX, float deltaY) {
                if (gameState == GameState.NORMAL) {
                    shiftAxisValue(AXIS_LOOK_LR, deltaX * MULT_VIEW_TRACKER);
                    shiftAxisValue(AXIS_LOOK_UD, deltaY * MULT_VIEW_TRACKER);
                }
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

        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);

        motionListener = new SensorEventListener() {
            @Override
            public void onSensorChanged(SensorEvent event) {
                if (!isThereController && event.sensor.getType() == Sensor.TYPE_GYROSCOPE && gameState == GameState.NORMAL && useGyroscope) {
                    float axisX = event.values[0];
                    float axisY = event.values[1];
                    float axisZ = event.values[2];
                    shiftAxisValue(AXIS_LOOK_LR, axisX * MULT_VIEW_GYROSCOPE);
                    shiftAxisValue(AXIS_LOOK_UD, -axisY * MULT_VIEW_GYROSCOPE);
                }
            }

            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
            }
        };

        if (!hasStoragePermission(this)) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_WRITE_STORAGE);
        } else {
            startGame();
        }

        updateSoftKeyboardVisible();
    }

    public void updateSoftKeyboardVisible() {
        isThereController = Utils.isThereControllers();
        int keyboardVisibility = gameState == GameState.NORMAL && !isThereController ? View.VISIBLE : View.GONE;
        findViewById(R.id.input_overlay).setVisibility(keyboardVisibility);
        findViewById(R.id.input_crunch).setVisibility(keyboardVisibility);
        findViewById(R.id.input_jump).setVisibility(keyboardVisibility);
        findViewById(R.id.input_fire).setVisibility(keyboardVisibility);
        findViewById(R.id.input_use).setVisibility(keyboardVisibility);
        findViewById(R.id.buttonPrev).setVisibility(keyboardVisibility);
        findViewById(R.id.buttonNext).setVisibility(keyboardVisibility);
        findViewById(R.id.bgTrackerView).setVisibility(keyboardVisibility);
        findViewById(R.id.settingsBtn).setVisibility((gameState == GameState.NORMAL || gameState == GameState.DEMO) ? View.VISIBLE : View.GONE);
        findViewById(R.id.buttonLoad).setVisibility(gameState == GameState.NORMAL ? View.VISIBLE : View.GONE);
        findViewById(R.id.buttonSave).setVisibility(gameState == GameState.NORMAL ? View.VISIBLE : View.GONE);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS, WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);

        if (gameState == GameState.MENU) {
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN);
        } else {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                            View.SYSTEM_UI_FLAG_FULLSCREEN |
                            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        sensorManager.registerListener(motionListener, sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE), 10000);
        EventBus.getDefault().register(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        sensorManager.unregisterListener(motionListener);
        EventBus.getDefault().unregister(this);
    }

    @Subscribe(sticky = true, threadMode = ThreadMode.MAIN)
    public void onFatalError(FatalErrorEvent event) {
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

    @Subscribe(sticky = true, threadMode = ThreadMode.MAIN)
    public void onConsoleVisibilityChange(StateChangeEvent event) {
        gameState = event.state;
        updateSoftKeyboardVisible();
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void openSettings(NativeEvents.OpenSettingsEvent event) {
        startActivity(new Intent(this, SettingsActivity.class));
    }

    @Override
    protected void onResume() {
        super.onResume();
        // hide verything from the screen
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_USER_LANDSCAPE);

        if (isGameStarted) {
            glSurfaceView.start();
        }

        syncOptions();
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
            setAxisValue(AXIS_MOVE_FB, applyDeadZone(-ev.getAxisValue(MotionEvent.AXIS_Y)));
            setAxisValue(AXIS_MOVE_LR, applyDeadZone(-ev.getAxisValue(MotionEvent.AXIS_X)));
            setAxisValue(AXIS_LOOK_LR, applyDeadZone(-ev.getAxisValue(MotionEvent.AXIS_Z) * MULT_VIEW_CONTROLLER));
            setAxisValue(AXIS_LOOK_UD, applyDeadZone(-ev.getAxisValue(MotionEvent.AXIS_RZ) * MULT_VIEW_CONTROLLER));
            nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_R2, ev.getAxisValue(MotionEvent.AXIS_RTRIGGER) > .5f ? 1 : 0);
            nDispatchKeyEvent(KeyEvent.KEYCODE_BUTTON_L2, ev.getAxisValue(MotionEvent.AXIS_RTRIGGER) < -.5f ? 1 : 0);
            nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_LEFT, ev.getAxisValue(MotionEvent.AXIS_HAT_X) < -.5f ? 1 : 0);
            nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_RIGHT, ev.getAxisValue(MotionEvent.AXIS_HAT_X) > .5f ? 1 : 0);
            nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_UP, ev.getAxisValue(MotionEvent.AXIS_HAT_Y) < -.5f ? 1 : 0);
            nDispatchKeyEvent(KeyEvent.KEYCODE_DPAD_DOWN, ev.getAxisValue(MotionEvent.AXIS_HAT_Y) > .5f ? 1 : 0);
        }
        return true;
    }

    private float applyDeadZone(float input) {
        if (input < -DEAD_ZONE) {
            input = (input + DEAD_ZONE) / (1 - DEAD_ZONE);
        } else if (input > DEAD_ZONE) {
            input = (input - DEAD_ZONE) / (1 - DEAD_ZONE);
        } else {
            input = 0;
        }
        return input;
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
                if (gameState == GameState.MENU) {
                    executeShell("GoMenuBack()");
                } else {
                    nDispatchKeyEvent(keyCode, 1);
                }
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

    // ui listeners
    public void showMenu(View view) {
        executeShell("sam_bMenu=1;");
    }

    public void doProfiling(View view) {
        executeShell("RecordProfile();");
    }

    public void doQuickLoad(View view) {
        executeShell("gam_bQuickLoad=1;");
    }

    public void doQuickSave(View view) {
        executeShell("gam_bQuickSave=1;");
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            updateSoftKeyboardVisible();
        }
    }

    private void startGame() {
        if (!homeDir.exists()) homeDir.mkdirs();
        SeriousSamSurface.initializeLibrary(homeDir.getAbsolutePath());
        isGameStarted = true;
        glSurfaceView.start();
    }

    public void syncOptions() {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
        useGyroscope = preferences.getBoolean("use_gyroscope", true);
        executeShell("hud_iStats=" + (preferences.getBoolean("hud_iStats", false) ? 2 : 0) + ";");
    }

    public static void executeShell(String command) {
        nShellExecute(command);
    }

    private static native void setAxisValue(int key, float value);
    private static native void shiftAxisValue(int key, float value);
    private static native void nShellExecute(String command);
    private static native void nDispatchKeyEvent(int key, int isPressed);
    private static native boolean nGetConsoleState();
}
