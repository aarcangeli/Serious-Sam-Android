package com.github.aarcangeli.serioussamandroid;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.AlarmManager;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.input.InputManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;

import com.github.aarcangeli.serioussamandroid.input.InputProcessor;
import com.github.aarcangeli.serioussamandroid.views.JoystickView;
import com.hold1.keyboardheightprovider.KeyboardHeightProvider;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.io.File;
import java.util.Locale;

import static com.github.aarcangeli.serioussamandroid.NativeEvents.EditTextEvent;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.FatalErrorEvent;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.GameState;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.OpenSettingsEvent;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.RestartEvent;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.StateChangeEvent;
import static com.github.aarcangeli.serioussamandroid.input.VirtualKeyboard.*;
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

    private static final float MULT_VIEW_CONTROLLER = 2.5f;
    private static final float MULT_VIEW_TRACKER = 0.4f;
    private static final float MULT_VIEW_GYROSCOPE = 0.8f;

    private SeriousSamSurface glSurfaceView;
    private File homeDir;

    private boolean isGameStarted = false;
    private SensorManager sensorManager;
    private SensorEventListener motionListener;
    private volatile GameState gameState = GameState.LOADING;
    private volatile int bombs;
    private boolean useGyroscope;
    private String showTouchController;
    private float gyroSensibility;
    private float aimViewSensibility;
    private float ctrlAimSensibility;
    public float deadZone;
    private boolean enableTouchController;

    private InputProcessor processor = new InputProcessor();
    private InputMethodManager inputMethodManager;

    private KeyboardHeightProvider keyboardHeightProvider;
    private KeyboardHeightProvider.KeyboardListener listener = new KeyboardHeightProvider.KeyboardListener() {
        @Override
        public void onHeightChanged(int height) {
            if (gameState == GameState.CONSOLE || gameState == GameState.MENU || gameState == GameState.COMPUTER) {
                Display display = getWindowManager().getDefaultDisplay();
                Point size = new Point();
                display.getSize(size);
                executeShell(String.format(Locale.ENGLISH, "con_fHeightFactor = %.6f", (size.y - height) / (float) size.y));
            }
        }
    };

    @Override
    @SuppressLint("ClickableViewAccessibility")
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        inputMethodManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);

        setContentView(R.layout.main_screen);
        glSurfaceView = findViewById(R.id.main_content);
        glSurfaceView.setActivity(this);

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

        findViewById(R.id.input_use).setOnTouchListener(new MyBtnListener(KeyEvent.KEYCODE_BUTTON_R2));
        findViewById(R.id.input_crunch).setOnTouchListener(new MyBtnListener(KeyEvent.KEYCODE_BUTTON_B));
        findViewById(R.id.input_jump).setOnTouchListener(new MyBtnListener(KeyEvent.KEYCODE_BUTTON_A));
        findViewById(R.id.buttonPrev).setOnTouchListener(new MyBtnListener(KeyEvent.KEYCODE_DPAD_LEFT));
        findViewById(R.id.buttonNext).setOnTouchListener(new MyBtnListener(KeyEvent.KEYCODE_DPAD_RIGHT));
        findViewById(R.id.input_fire).setOnTouchListener(new MyBtnListener(KeyEvent.KEYCODE_BUTTON_R1));
        findViewById(R.id.input_SeriousBomb).setOnTouchListener(new MyBtnListener(KeyEvent.KEYCODE_BUTTON_Y));
        findViewById(R.id.bgTrackerView).setOnTouchListener(new MyBtnListener());

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
                if (event.sensor.getType() == Sensor.TYPE_GYROSCOPE && gameState == GameState.NORMAL && useGyroscope && enableTouchController) {
                    float axisX = event.values[0];
                    float axisY = event.values[1];
                    float axisZ = event.values[2];
                    shiftAxisValue(AXIS_LOOK_LR, axisX * MULT_VIEW_GYROSCOPE * gyroSensibility);
                    shiftAxisValue(AXIS_LOOK_UD, -axisY * MULT_VIEW_GYROSCOPE * gyroSensibility);
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

        keyboardHeightProvider = new KeyboardHeightProvider(this);

//        getWindow().getDecorView().getViewTreeObserver().addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
//            @Override
//            public boolean onPreDraw() {
//                return true;
//            }
//        });

//        getWindow().getDecorView().getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
//            @Override
//            public void onGlobalLayout() {
//
//            }
//        });
    }

    public void updateSoftKeyboardVisible() {
        enableTouchController = false;
        if (gameState == GameState.NORMAL) {
            if ("Yes".equalsIgnoreCase(showTouchController)) {
                enableTouchController = true;
            } else if ("No".equalsIgnoreCase(showTouchController)) {
                enableTouchController = false;
            } else {
                enableTouchController = !Utils.isThereControllers();
            }
        }
        int keyboardVisibility = enableTouchController ? View.VISIBLE : View.GONE;
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
        findViewById(R.id.buttonConsole).setVisibility(gameState == GameState.NORMAL ? View.VISIBLE : View.GONE);
        findViewById(R.id.buttonSave).setVisibility(gameState == GameState.NORMAL ? View.VISIBLE : View.GONE);
        findViewById(R.id.input_SeriousBomb).setVisibility(enableTouchController && bombs > 0 ? View.VISIBLE : View.GONE);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);

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

        if (gameState == GameState.CONSOLE) {
            if (glSurfaceView.requestFocus()) {
                inputMethodManager.showSoftInput(glSurfaceView, 0);
            }
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        sensorManager.registerListener(motionListener, sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE), 10000);
        EventBus.getDefault().register(this);
        keyboardHeightProvider.addKeyboardListener(listener);
    }

    @Override
    protected void onStop() {
        super.onStop();
        sensorManager.unregisterListener(motionListener);
        EventBus.getDefault().unregister(this);
        keyboardHeightProvider.removeKeyboardListener(listener);
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
        bombs = event.bombs;
        updateSoftKeyboardVisible();
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void openSettings(OpenSettingsEvent event) {
        startActivity(new Intent(this, SettingsActivity.class));
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void editText(final EditTextEvent event) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        // Set up the input
        final EditText input = new EditText(this);
        input.setText(event.defaultText);
        builder.setView(input);

        // Set up the buttons
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                nConfirmEditText(input.getText().toString());
            }
        });
        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
                nCancelEditText();
            }
        });

        builder.show();
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
        keyboardHeightProvider.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        glSurfaceView.stop();
        executeShell("HideConsole();");
        executeShell("HideComputer();");
        keyboardHeightProvider.onPause();
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void restartEvent(RestartEvent event) {
        final MainActivity context = MainActivity.this;
        runOnUiThread(new Runnable() {
            public void run() {
                ProgressDialog dialog = new ProgressDialog(context);
                dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                dialog.setTitle("SeriousSam");
                dialog.setMessage("Restarting");
                dialog.setCancelable(false);
                dialog.setMax(0);
                dialog.show();
            }
        });
        new Handler().postDelayed(new Runnable() {
            public void run() {
                // restart
                Intent mStartActivity = new Intent(context, MainActivity.class);
                PendingIntent mPendingIntent = PendingIntent.getActivity(context, 123456, mStartActivity, PendingIntent.FLAG_CANCEL_CURRENT);
                AlarmManager mgr = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
                mgr.set(AlarmManager.RTC, System.currentTimeMillis() + 100, mPendingIntent);
                System.exit(0);
            }
        }, 1000);
    }

    private static boolean hasStoragePermission(Context context) {
        return ContextCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
    }

    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent ev) {
        if (ev.getAction() == MotionEvent.ACTION_MOVE) {
            setAxisValue(AXIS_MOVE_FB, applyDeadZone(-ev.getAxisValue(MotionEvent.AXIS_Y)));
            setAxisValue(AXIS_MOVE_LR, applyDeadZone(-ev.getAxisValue(MotionEvent.AXIS_X)));
            setAxisValue(AXIS_LOOK_LR, applyDeadZone(-ev.getAxisValue(MotionEvent.AXIS_Z)) * MULT_VIEW_CONTROLLER * ctrlAimSensibility);
            setAxisValue(AXIS_LOOK_UD, applyDeadZone(-ev.getAxisValue(MotionEvent.AXIS_RZ)) * MULT_VIEW_CONTROLLER * ctrlAimSensibility);
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
        if (input < -deadZone) {
            return (input + deadZone) / (1 - deadZone);
        } else if (input > deadZone) {
            return (input - deadZone) / (1 - deadZone);
        } else {
            return 0;
        }
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
        if (event.getAction() == KeyEvent.ACTION_DOWN && event.isPrintingKey()) {
            executeShell("MenuChar(" + event.getUnicodeChar() + ")");
        }
        if (gameState == GameState.MENU || gameState == GameState.CONSOLE) {
            if (event.getAction() == KeyEvent.ACTION_DOWN) {
                switch (keyCode) {
                    case KeyEvent.KEYCODE_DPAD_DOWN:
                        executeShell("MenuEvent(" + VK_DOWN + ")");
                        break;
                    case KeyEvent.KEYCODE_DPAD_RIGHT:
                        executeShell("MenuEvent(" + VK_RIGHT + ")");
                        break;
                    case KeyEvent.KEYCODE_DPAD_UP:
                        executeShell("MenuEvent(" + VK_UP + ")");
                        break;
                    case KeyEvent.KEYCODE_DPAD_LEFT:
                        executeShell("MenuEvent(" + VK_LEFT + ")");
                        break;
                    case KeyEvent.KEYCODE_MOVE_HOME:
                        executeShell("MenuEvent(" + VK_HOME + ")");
                        break;
                    case KeyEvent.KEYCODE_MOVE_END:
                        executeShell("MenuEvent(" + VK_END + ")");
                        break;
                    case KeyEvent.KEYCODE_DEL:
                        executeShell("MenuEvent(" + VK_BACK + ")");
                        break;
                    case KeyEvent.KEYCODE_FORWARD_DEL:
                        executeShell("MenuEvent(" + VK_DELETE + ")");
                        break;
                    case KeyEvent.KEYCODE_ENTER:
                    case KeyEvent.KEYCODE_NUMPAD_ENTER:
                    case KeyEvent.KEYCODE_BUTTON_A:
                        executeShell("MenuEvent(" + VK_RETURN + ")");
                        break;
                    case KeyEvent.KEYCODE_BUTTON_B:
                    case KeyEvent.KEYCODE_ESCAPE:
                    case KeyEvent.KEYCODE_BACK:
                        executeShell("GoMenuBack()");
                        break;
                    case KeyEvent.KEYCODE_F1:
                        executeShell("MenuEvent(" + VK_F1 + ")");
                        break;
                    case KeyEvent.KEYCODE_F2:
                        executeShell("MenuEvent(" + VK_F2 + ")");
                        break;
                    case KeyEvent.KEYCODE_F3:
                        executeShell("MenuEvent(" + VK_F3 + ")");
                        break;
                    case KeyEvent.KEYCODE_F4:
                        executeShell("MenuEvent(" + VK_F4 + ")");
                        break;
                    case KeyEvent.KEYCODE_F5:
                        executeShell("MenuEvent(" + VK_F5 + ")");
                        break;
                    case KeyEvent.KEYCODE_F6:
                        executeShell("MenuEvent(" + VK_F6 + ")");
                        break;
                    case KeyEvent.KEYCODE_F7:
                        executeShell("MenuEvent(" + VK_F7 + ")");
                        break;
                    case KeyEvent.KEYCODE_F8:
                        executeShell("MenuEvent(" + VK_F8 + ")");
                        break;
                    case KeyEvent.KEYCODE_F9:
                        executeShell("MenuEvent(" + VK_F9 + ")");
                        break;
                    case KeyEvent.KEYCODE_F10:
                        executeShell("MenuEvent(" + VK_F10 + ")");
                        break;
                    case KeyEvent.KEYCODE_F11:
                        executeShell("MenuEvent(" + VK_F11 + ")");
                        break;
                    case KeyEvent.KEYCODE_F12:
                        executeShell("MenuEvent(" + VK_F12 + ")");
                        break;
                    case KeyEvent.KEYCODE_TAB:
                        executeShell("MenuEvent(" + VK_TAB + ")");
                        break;
                }
                if (event.getRepeatCount() == 0 && gameState == GameState.CONSOLE && keyCode == KeyEvent.KEYCODE_BUTTON_START) {
                    executeShell("HideConsole();");
                    keyboardHeightProvider.hideKeyboard();
                }
            }
        } else if (gameState == GameState.COMPUTER) {
            if (event.getRepeatCount() == 0 && event.getAction() == KeyEvent.ACTION_DOWN && keyCode == KeyEvent.KEYCODE_BACK) {
                executeShell("HideComputer();");
            }
        } else if (gameState != GameState.INTRO) {
            if (event.getRepeatCount() == 0) {
                if (event.getAction() == KeyEvent.ACTION_DOWN) {
                    if (keyCode == KeyEvent.KEYCODE_ESCAPE) {
                        executeShell("sam_bMenu=1;");
                    }
                    nDispatchKeyEvent(keyCode, 1);
                }
                if (event.getAction() == KeyEvent.ACTION_UP) {
                    nDispatchKeyEvent(keyCode, 0);
                }
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

    public void doConsole(View view) {
        executeShell("ToggleConsole();");
    }

    public void keyboardHidden() {
        executeShell("HideConsole();");
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
        showTouchController = preferences.getString("showTouchController", "Auto");
        gyroSensibility = preferences.getInt("gyro_sensibility", 100) / 100.f;
        aimViewSensibility = preferences.getInt("aimView_sensibility", 100) / 100.f;
        ctrlAimSensibility = preferences.getInt("ctrl_aimSensibility", 100) / 100.f;
        deadZone = preferences.getInt("ctrl_deadZone", 20) / 100.f;
        executeShell("hud_iStats=" + (preferences.getBoolean("hud_iStats", false) ? 2 : 0) + ";");
        updateSoftKeyboardVisible();
    }

    private class MyBtnListener implements View.OnTouchListener {
        boolean isTracking;
        float lastX, lastY;
        private int btnToBind;

        public MyBtnListener() {
            this.btnToBind = 0;
        }

        public MyBtnListener(int btnToBind) {
            this.btnToBind = btnToBind;
        }

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                isTracking = true;
                lastX = event.getRawX();
                lastY = event.getRawY();
                if (this.btnToBind != 0) {
                    nDispatchKeyEvent(btnToBind, 1);
                }
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                if (this.btnToBind != 0) {
                    nDispatchKeyEvent(btnToBind, 0);
                }
            } else if (event.getAction() == MotionEvent.ACTION_POINTER_UP) {
                isTracking = false;
            } else if (event.getAction() == MotionEvent.ACTION_MOVE && isTracking) {
                float rawX = event.getRawX();
                float rawY = event.getRawY();
                shiftAxisValue(AXIS_LOOK_LR, -Utils.convertPixelsToDp(rawX - lastX, MainActivity.this) * MULT_VIEW_TRACKER * aimViewSensibility);
                shiftAxisValue(AXIS_LOOK_UD, -Utils.convertPixelsToDp(rawY - lastY, MainActivity.this) * MULT_VIEW_TRACKER * aimViewSensibility);
                lastX = rawX;
                lastY = rawY;
            }
            return true;
        }
    }

    public static void executeShell(String command) {
        nShellExecute(command);
    }

    private static native void setAxisValue(int key, float value);
    private static native void shiftAxisValue(int key, float value);
    private static native void nShellExecute(String command);
    private static native void nDispatchKeyEvent(int key, int isPressed);
    private static native void nConfirmEditText(String newText);
    private static native void nCancelEditText();
}
