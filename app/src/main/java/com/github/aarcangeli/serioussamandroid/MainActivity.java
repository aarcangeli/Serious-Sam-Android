package com.github.aarcangeli.serioussamandroid;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlarmManager;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Point;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.input.InputManager;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.preference.PreferenceManager;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import com.github.aarcangeli.serioussamandroid.lists.*;
import com.github.aarcangeli.serioussamandroid.views.ButtonView;
import com.github.aarcangeli.serioussamandroid.input.InputProcessor;
import com.github.aarcangeli.serioussamandroid.views.JoystickView;
import com.hold1.keyboardheightprovider.KeyboardHeightProvider;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.BufferedReader;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.Writer;
import java.util.Locale;
import java.util.List;
import java.net.*;

import java.util.Enumeration;
import java.lang.StringBuilder;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;


import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

import static com.github.aarcangeli.serioussamandroid.NativeEvents.EditTextEvent;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.ErrorEvent;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.GameState;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.OpenSettingsEvent;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.UpdateUIEvent;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.RestartEvent;
import static com.github.aarcangeli.serioussamandroid.NativeEvents.StateChangeEvent;
import static com.github.aarcangeli.serioussamandroid.input.VirtualKeyboard.*;
import static com.github.aarcangeli.serioussamandroid.views.JoystickView.Listener;

public class MainActivity extends Activity {
	public static final String TAG = "SeriousSamJava";
	private final int REQUEST_WRITE_STORAGE = 1;
	private static final int AXIS_MOVE_LR = 1;
	private static final int AXIS_MOVE_FB = 2;
	private static final int AXIS_LOOK_UD = 6;
	private static final int AXIS_LOOK_LR = 7;

	private static final float MULT_VIEW_CONTROLLER = 2.5f;
	private static final float MULT_VIEW_TRACKER = 0.4f;
	private static final float MULT_VIEW_GYROSCOPE = 0.8f;

	public int curVersionCode;
	private SeriousSamSurface glSurfaceView;
	private File homeDir;
	private boolean isGameStarted = false;
	private SensorManager sensorManager;
	private SensorEventListener motionListener;
	private volatile GameState gameState = GameState.LOADING;
	private volatile int bombs = 0;
	private boolean useGyroscope;
	private String showTouchController;
	private float gyroSensibility;
	private float aimViewSensibility;
	private float ctrlAimSensibility;
	public float deadZone;
	private boolean enableTouchController;
	private String din_uiScale;
	private String ui_drawBanner;
	private boolean useAimAssist;
	private float autoAimSens;
	public float uiScale;
	public boolean ButtonsMapping = false;
	public boolean isTracking;
	public boolean ControlsInitialized = false;
	public int transparency;
	private boolean useVolumeKeys;
	public String volumeUpKey, volumeDownKey;
	private InputProcessor processor = new InputProcessor();
	private InputMethodManager inputMethodManager;
	private KeyboardHeightProvider keyboardHeightProvider;
	private KeyboardHeightProvider.KeyboardListener listener = new KeyboardHeightProvider.KeyboardListener() {
		@Override
		public void onHeightChanged(int height) {
			Display display = getWindowManager().getDefaultDisplay();
			Point size = new Point();
			display.getSize(size);
			executeShell(String.format(Locale.ENGLISH, "con_fHeightFactor = %.6f", (size.y - height) / (float) size.y));
		}
	};

	private boolean isNetworkConnected() {
		ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
		return cm.getActiveNetworkInfo() != null;
	}

    public String getWifiIP() {
        try {
            Enumeration<NetworkInterface> networkInterfaces = NetworkInterface.getNetworkInterfaces();
            while (networkInterfaces.hasMoreElements()) {
                NetworkInterface networkInterface = networkInterfaces.nextElement();
                if (networkInterface.getName().equals("wlan0") || networkInterface.getName().equals("ap0")) {
                    if (networkInterface.isUp()) {
                        Enumeration<InetAddress> addresses = networkInterface.getInetAddresses();
                        while (addresses.hasMoreElements()) {
                            InetAddress address = addresses.nextElement();
                            if (!address.isLoopbackAddress() && address instanceof Inet4Address) {
                                return address.getHostAddress();
                            }
                        }
                    }
                }
            }
        } catch (SocketException e) {
            return "Error occurred while retrieving IP: " + e.getMessage();
        }
        return "Wifi or AP not started";
    }

	private void checkUpdate() {
		if(isNetworkConnected())
		{
			try {
				PackageInfo packageInfo = getPackageManager().getPackageInfo(this.getPackageName(), 0);
				curVersionCode = packageInfo.versionCode;
			} catch (NameNotFoundException e) {
				   // TODO Auto-generated catch block
			}
			
			Updater updater = new Updater(this, curVersionCode);
		
			Updater.ReadFileTask tsk = updater.new ReadFileTask();
			tsk.execute();
		}
	}
	
	private void copyFile(String filename)  throws IOException {
		AssetManager assetManager = getAssets();
		try (InputStream in = assetManager.open(filename);
			 OutputStream out = new FileOutputStream(new File(homeDir, filename))) {
			byte[] buffer = new byte[1024];
			int read;
			while ((read = in.read(buffer)) != -1) {
				out.write(buffer, 0, read);
			}
		}
	}

	private void copyFolderOrFile(String name) throws IOException {
		AssetManager assetManager = getAssets();
		String[] files = assetManager.list(name);
		if (files == null) {
			return;
		}
		
		if (files.length == 0) {
			copyFile(name);
		} else {
			File outputFolder = new File(homeDir, name);
			outputFolder.mkdirs();

			for (String filename : files) {
				try (InputStream in = assetManager.open(name + "/" + filename);
					 OutputStream out = new FileOutputStream(new File(outputFolder, filename))) {
					byte[] buffer = new byte[1024];
					int read;
					while ((read = in.read(buffer)) != -1) {
						out.write(buffer, 0, read);
					}
				}
			}
		}
	}

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

		Button settingsBtn = findViewById(R.id.settingsBtn);
		settingsBtn.setOnLongClickListener(new View.OnLongClickListener() {
			@Override
			public boolean onLongClick(View v) {
			if (ButtonsMapping == false){
				ButtonsMapping = true;
				updateSoftKeyboardVisible();
				isTracking = false;
			}
				return true;
			}
		});
		
		ButtonView use = findViewById(R.id.input_use);
		use.setKeycode("Use");
		
		ButtonView crouch = findViewById(R.id.input_crouch);
		crouch.setKeycode("Crouch");
		
		ButtonView jump = findViewById(R.id.input_jump);
		jump.setKeycode("Jump");
		
		ButtonView prev = findViewById(R.id.buttonPrev);
		prev.setKeycode("PrevWeapon");
		
		ButtonView next = findViewById(R.id.buttonNext);
		next.setKeycode("NextWeapon");
		
		ButtonView fire = findViewById(R.id.input_fire);
		fire.setKeycode("Fire");
		
		ButtonView bomb = findViewById(R.id.input_SeriousBomb);
		bomb.setKeycode("SeriousBomb");
		
		findViewById(R.id.bgTrackerView).setOnTouchListener(new MyBtnListener());

		JoystickView joystick = findViewById(R.id.input_overlay);
		joystick.setListener(new Listener() {
			@Override
			public void onMove(float deltaX, float deltaY, MotionEvent ev) {
			if (gameState == GameState.NORMAL) {
				JoystickView joystick = findViewById(R.id.input_overlay);
					if (ButtonsMapping) {
							String fullName = getResources().getResourceName(joystick.getId());
							String name = fullName.substring(fullName.lastIndexOf("/") + 1);
							joystick.ButtonsMapping = true;
							float X = ev.getRawX() + -Utils.convertPixelsToDp(joystick.padPosX, MainActivity.this) - joystick.radius;
							float Y = ev.getRawY() + -Utils.convertPixelsToDp(joystick.padPosY, MainActivity.this) - joystick.radius;
							joystick.setX(X);
							joystick.setY(Y);
						} else {
							joystick.ButtonsMapping = false;
							setAxisValue(AXIS_MOVE_LR, deltaX);
							setAxisValue(AXIS_MOVE_FB, deltaY);
					}
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
		Log.wtf(TAG, "HomeDir: " + homeDir);
		Log.wtf(TAG, "LibDir: " + getLibDir(this));

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
		
		checkUpdate();

		if (!hasStoragePermission(this)) {
			ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_WRITE_STORAGE);
		} else {
			CacheDownloader showalarm = new CacheDownloader(this);
			showalarm.checkFolderAndDownloadFile(new CacheDownloader.DownloadCallback() {
				@Override
				public void onDownloadComplete(Boolean downloaded) {
					if (downloaded) {
						final ProgressDialog dialog = new ProgressDialog(MainActivity.this);
						runOnUiThread(new Runnable() {
							public void run() {
								dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
								dialog.setTitle("Serious Sam Android Edition ");
								dialog.setMessage("Распаковка кэша, ждите...");
								dialog.setCancelable(false);
								dialog.show();
							}
						});
						new Thread(new Runnable() {
							public void run() {
								try {
									File zipFile = new File(Environment.getExternalStorageDirectory(),BuildConfig.home + ".zip");
									InputStream inputStream = new FileInputStream(zipFile);
									ZipInputStream zipInputStream = new ZipInputStream(new BufferedInputStream(inputStream));
									byte[] buffer = new byte[8192];
									while (true) {
										ZipEntry nextEntry = zipInputStream.getNextEntry();
										if (nextEntry == null) {
											zipInputStream.close();
											inputStream.close();
											dialog.dismiss();
											startGame();
											return;
										}
										final File file = new File(Environment.getExternalStorageDirectory(), nextEntry.getName());
										if (!file.exists()) {
											runOnUiThread(new Runnable() {
												public void run() {
													Toast.makeText(MainActivity.this, "Распаковка: " + file.getName(),Toast.LENGTH_SHORT).show();
												}
											});
											File parentFile = nextEntry.isDirectory() ? file : file.getParentFile();
											if (!parentFile.isDirectory() && !parentFile.mkdirs()) {
												throw new FileNotFoundException("Failed to ensure directory: " + parentFile.getAbsolutePath());
											} else if (!nextEntry.isDirectory()) {
												FileOutputStream fileOutputStream = new FileOutputStream(file);
												while (true) {
													int read = zipInputStream.read(buffer);
													if (read == -1) {
														break;
													}
													fileOutputStream.write(buffer, 0, read);
												}
												fileOutputStream.close();
											}
										}
									}
								} catch (Throwable th) {
									final String mess = th.getMessage();
									th.printStackTrace();
									Log.wtf(TAG, "Error: " + mess);
									runOnUiThread(new Runnable() {
										public void run() {
											dialog.setMessage("Ошибка: " + mess);
											Toast.makeText(MainActivity.this, "Ошибка: " + mess,Toast.LENGTH_LONG).show();
										}
									});
								}
							}
						}).start();
					} else {
						startGame();
					}
				}
			});
		}
		
		updateSoftKeyboardVisible();

		keyboardHeightProvider = new KeyboardHeightProvider(this);

		executeShell("net_WifiIP=\""+getWifiIP()+"\"");
		//setupTouchControls();
//		getWindow().getDecorView().getViewTreeObserver().addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
//			@Override
//			public boolean onPreDraw() {
//				return true;
//			}
//		});

//		getWindow().getDecorView().getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
//			@Override
//			public void onGlobalLayout() {
//
//			}
//		});
	}

	public void AimAssistState() {
		if (!useAimAssist) {
			executeShell("plr_fAutoAimSensitivity=0.0f;");		
			Log.wtf(TAG, "AimAssist disabled");
		}
	}

	public void DinamicUI() {
		if ("On".equalsIgnoreCase(din_uiScale)) {
			uiScale = Utils.convertDpToPixel(1.0f, this) * glSurfaceView.getScale();
			Log.wtf(TAG, "Dinamic UI Enabled");
		} else if ("Off".equalsIgnoreCase(din_uiScale)) {
			uiScale = 1.0f;
			Log.wtf(TAG, "Dinamic UI Disabled");
		} else {
			uiScale = Utils.convertDpToPixel(1.0f, this) * glSurfaceView.getScale();
			Log.wtf(TAG, "Dinamic UI Enabled");
		}
	}

	public void drawBanner() {
		if ("On".equalsIgnoreCase(ui_drawBanner)) {
			executeShell("ui_drawBanner=1;");
			Log.wtf(TAG, "DrawBanner Enabled");
		} else if ("Off".equalsIgnoreCase(ui_drawBanner)) {
			executeShell("ui_drawBanner=0;");
			Log.wtf(TAG, "DrawBanner Disabled");
		} else {
			executeShell("ui_drawBanner=1;");
			Log.wtf(TAG, "DrawBanner Enabled");
		}
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

		ConstraintLayout constraintView = findViewById(R.id.constraint_content);
		ViewGroup parent = (ViewGroup) constraintView;

		if (parent != null) {
			for(int i=0; i < parent.getChildCount(); i++) {
				View child = parent.getChildAt(i);
				if (gameState != GameState.NORMAL) {
					child.setVisibility(keyboardVisibility);
				} else {
					child.setVisibility(View.VISIBLE);
					child.setAlpha(enableTouchController ? 255 - transparency : 0);
				}
				if (child instanceof Button && !(child instanceof ButtonView)) {
					if (child.getId() == R.id.buttonApply 
						|| child.getId() == R.id.buttonPlus 
							|| child.getId() == R.id.buttonMinus) {
						child.setVisibility(enableTouchController && ButtonsMapping ? View.VISIBLE : View.GONE);
					}
				} 
			}
		}
		
		try {
			ButtonView bomb = findViewById(R.id.input_SeriousBomb);
			if (bomb != null) {
				bomb.setVisibility(View.INVISIBLE);
			}
		} finally {}
		
		if (parent != null) {
			for(int i=0; i < parent.getChildCount(); i++) {
				View child = parent.getChildAt(i);
				if (child instanceof ButtonView) {
					ButtonView btn = (ButtonView) child;
					if (btn != null) {
						String key = btn.getKeycode();
						int currentBombs = bombs;
						if (key.equals("SeriousBomb")) {
							btn.setVisibility(enableTouchController && currentBombs != 0 ? View.VISIBLE : View.GONE);
							break;
						}
					}
				}
			}
		} 
		
		Button settingsBtn = findViewById(R.id.settingsBtn);
		settingsBtn.getBackground().setAlpha(255 - transparency);
		
		Button buttonConsole = findViewById(R.id.buttonConsole);
		buttonConsole.getBackground().setAlpha(255 - transparency);
		
		Button buttonLoad = findViewById(R.id.buttonLoad);
		buttonLoad.getBackground().setAlpha(255 - transparency);
		
		Button buttonSave = findViewById(R.id.buttonSave);
		buttonSave.getBackground().setAlpha(255 - transparency);
		
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		if (gameState == GameState.MENU || gameState == GameState.CONSOLE) {
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
	public void onFatalError(final ErrorEvent event) {
		AlertDialog.Builder dlgAlert = new AlertDialog.Builder(this);
		dlgAlert.setMessage(event.message);
		dlgAlert.setTitle("Fatal Error");
		dlgAlert.setPositiveButton("Close", new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				if (event.fatal) {
					System.exit(1);
				}
			}
		});
		dlgAlert.setCancelable(false);
		dlgAlert.create().show();
	}

	@Subscribe(sticky = true, threadMode = ThreadMode.MAIN)
	public void onConsoleVisibilityChange(StateChangeEvent event) {
		gameState = event.state;
		if (event.bombs > 0) {
			bombs = event.bombs;
		} else {
			bombs = 0;
		}
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

		glSurfaceView.syncOptions();
		syncOptions();
		keyboardHeightProvider.onResume();
	}

	@Override
	protected void onPause() {
		super.onPause();
		glSurfaceView.stop();
		executeShell("HideConsole();");
		executeShell("HideComputer();");
		executeShell("SaveOptions();");
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
				PendingIntent mPendingIntent = PendingIntent.getActivity(MainActivity.this, 1000, getIntent(), PendingIntent.FLAG_CANCEL_CURRENT);
				AlarmManager mgr = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
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
		switch(ev.getAction()) {
		case MotionEvent.ACTION_MOVE:
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
			break;
		default:
			return false;
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
			SeriousSamSurface.initializeLibrary(homeDir.getAbsolutePath(), getLibDir(context));
		}
	}

	@NonNull
	private static String getLibDir(Context context) {
		return context.getApplicationInfo().dataDir + "/lib";
	}

	@NonNull
	private static File getHomeDir() {
		return new File(Environment.getExternalStorageDirectory(), BuildConfig.home).getAbsoluteFile();
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		int keyCode = event.getKeyCode();	
		if (keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
			if (!useVolumeKeys) {
				return false;			
			} else {
				switch(event.getAction()) {
					case KeyEvent.ACTION_DOWN:
						nTouchKeyEvent(volumeUpKey, 1);
						return true;
					case KeyEvent.ACTION_UP:
						nTouchKeyEvent(volumeUpKey, 0);
						return true;
				}
			}
		}
		
		if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
			if (!useVolumeKeys) {
				return false;			
			} else {
				switch(event.getAction()) {
					case KeyEvent.ACTION_DOWN:
						nTouchKeyEvent(volumeDownKey, 1);
						return true;
					case KeyEvent.ACTION_UP:
						nTouchKeyEvent(volumeDownKey, 0);
						return true;
				}
			}
		}
		
		if (gameState == GameState.MENU || gameState == GameState.CONSOLE) {
			executeShell("input_iIsShiftPressed = " + (event.isShiftPressed() ? 1 : 0));
			if (event.getAction() == KeyEvent.ACTION_DOWN) {

				System.out.println(" KEYCODE=" +  keyCode);

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
					case KeyEvent.KEYCODE_PAGE_UP:
						executeShell("MenuEvent(" + VK_PRIOR + ")");
						break;
					case KeyEvent.KEYCODE_PAGE_DOWN:
						executeShell("MenuEvent(" + VK_NEXT + ")");
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
		int unicodeChar = event.getUnicodeChar();
		if (event.getAction() == KeyEvent.ACTION_DOWN && unicodeChar > 0) {
			executeShell("MenuChar(" + unicodeChar + ")");
		}
		return true;
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
		if (requestCode == REQUEST_WRITE_STORAGE) {
			if (grantResults.length > 0) {
				if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
				Log.wtf(TAG,"Permission is granted");
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
		executeShell("net_WifiIP=\""+getWifiIP()+"\"");
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
	
	public void changeButtonSize(String mode) {
		try {
			ConstraintLayout constraintView = findViewById(R.id.constraint_content);
			ViewGroup parent = (ViewGroup) constraintView;
			
			if (parent != null) {
				for(int i=0; i < parent.getChildCount(); i++) {
					View child = parent.getChildAt(i);
					if (child instanceof ButtonView) {
						int count = 5;
						if (mode.equals("+")) {
							count = 5;
						} else if (mode.equals("-")) {
							count = -5;
						}
						ButtonView btn = (ButtonView) child;
						btn.getLayoutParams().width = btn.getLayoutParams().width + count;
						btn.getLayoutParams().height = btn.getLayoutParams().height + count;
						btn.setVisibility(View.GONE);
						btn.setVisibility(View.VISIBLE);	
					}
				}
			}
		} catch (Exception e) {
			Toast toast = Toast.makeText(MainActivity.this, "Change button size error: " + e,Toast.LENGTH_SHORT);
			toast.show();
		}
	}

	public void btnApply(View view) {
		generateTouchControlsJson();
		ButtonsMapping = false;
		updateSoftKeyboardVisible();
	}

	void removeLayoutControls() {
		ConstraintLayout constraintView = findViewById(R.id.constraint_content);
		ViewGroup parent = (ViewGroup) constraintView;
		int[] id = new int[] {R.id.input_use, R.id.input_crouch, R.id.input_jump, R.id.buttonPrev, R.id.buttonNext,R.id.input_fire, R.id.input_SeriousBomb};
		if (parent != null) {
			for(int i=0; i < id.length; i++) {
				View child = findViewById(id[i]);
				parent.removeView(child);
			}
		}
	}

	void restoreControls() {
		File homeDir = getHomeDir();
		File controlsFolder = new File(homeDir + "/MobileControls");
		String jsonF = controlsFolder  + "/TouchControls.json";
		File jsonFile = new File(jsonF);
		
		if (jsonFile.exists()) {
			ConstraintLayout constraintView = findViewById(R.id.constraint_content);
			ViewGroup parent = (ViewGroup) constraintView;
			
			StringBuilder text = new StringBuilder();
			try {
				BufferedReader br = new BufferedReader(new FileReader(jsonFile));
				String line;

				while ((line = br.readLine()) != null) {
					text.append(line);
					text.append('\n');
				}
				br.close();

				Gson gson = new Gson();

				ButtonSet[] listItems = gson.fromJson(String.valueOf(text), ButtonSet[].class);

				for (ButtonSet item : listItems)
				{
					switch (item.type) {
						case "BitmapButton":
							InitBitmapButton(item.bitmap, item.x, item.y, item.h, item.w, item.action);
							break;
						case "Joystick":
							JoystickView joystick = findViewById(R.id.input_overlay);
							joystick.setX(item.x);
							joystick.setY(item.y);
							break;
					}
				}

			} catch (Exception e) {
				Toast toast = Toast.makeText(MainActivity.this, "Restore controls error:" + e,Toast.LENGTH_SHORT);
				toast.show();
			}
		}
		return;
	}

	public List<View> getAllButtons(ViewGroup layout){
			List<View> btn = new ArrayList<>();
			for(int i =0; i< layout.getChildCount(); i++){
				View v =layout.getChildAt(i);
				btn.add((View) v);
			}
			return btn;
	}

	void generateTouchControlsJson() {
		ConstraintLayout constraintView = findViewById(R.id.constraint_content);
		ViewGroup parent = (ViewGroup) constraintView;

		File homeDir = getHomeDir();
		File controlsFolder = new File(homeDir + "/MobileControls");
		
		String jsonF = controlsFolder + "/TouchControls.json";
		File jsonFile = new File(jsonF);
		Gson gson = new Gson();
		ArrayList<ButtonSet> buttons = new ArrayList<>();
		
		List<View> btn_list = getAllButtons(constraintView);
		
		try (Writer writer = new FileWriter(jsonF)) {
			if (btn_list != null && parent != null) {
				for(int i=0; i < btn_list.size(); i++) {
					View child = btn_list.get(i);
					if (child instanceof ButtonView) {
						ButtonView btn = (ButtonView) child;
						if (btn.getKeycode() == "SeriousBomb" && BuildConfig.home.endsWith("TFE")){
							continue;
						} else {
							int bitmapId = btn.getBitmap();
							String fullBitmapName = getResources().getResourceName(bitmapId);
							String bitmapName = fullBitmapName.substring(fullBitmapName.lastIndexOf("/") + 1);
							buttons.add(new ButtonSet("BitmapButton", btn.getX(), btn.getY(), btn.getLayoutParams().height, 
							btn.getLayoutParams().width, bitmapName, btn.getKeycode()));
							parent.removeView(child);	
						}
					}
					if (child instanceof JoystickView) {
						JoystickView joystick = (JoystickView) child;
						buttons.add(new ButtonSet("Joystick", joystick.getX(), joystick.getY(), 0, 0, "", ""));
					}
				}
			}
			Gson gsonWrite = new GsonBuilder().setPrettyPrinting().create();
			gsonWrite.toJson(buttons, writer);
			Toast toast = Toast.makeText(MainActivity.this, "Touch Controls saved!",Toast.LENGTH_SHORT);
			toast.show();
		} catch (IOException e) {
			Toast toast = Toast.makeText(MainActivity.this, "Generate default controls error: " + e,Toast.LENGTH_SHORT);
			toast.show();
		}
		if (!ControlsInitialized || ButtonsMapping) {
			restoreControls();	
		}
	}

	void setupTouchControls() {
		File homeDir = getHomeDir();
		File controlsFolder = new File(homeDir + "/MobileControls");
		if (!controlsFolder.exists()) controlsFolder.mkdirs();
		
		String jsonF = controlsFolder  + "/TouchControls.json";
		File jsonFile = new File(jsonF);
		
		if (!jsonFile.exists()) {
			generateTouchControlsJson();
		} else {
			removeLayoutControls();
			restoreControls();
		}
		ControlsInitialized = true;
		updateSoftKeyboardVisible();
	}

	public void createTextButton(String text, float x, float y, String keyCode, int id) {
		ConstraintLayout constraintView = findViewById(R.id.constraint_content);
		Button btn = new Button(this);
		btn.setLayoutParams(new ConstraintLayout.LayoutParams(ConstraintLayout.LayoutParams.WRAP_CONTENT, ConstraintLayout.LayoutParams.WRAP_CONTENT));
		btn.setX(x);
		btn.setY(y);
		btn.setText(text);
		btn.setId(id);
		constraintView.addView(btn);
		btn.setVisibility(View.VISIBLE);
		btn.setOnTouchListener(new MyBtnListener(keyCode));
	}

	public void InitBitmapButton(String bitmap, float x, float y, int h, int w, String keyCode) {
		ConstraintLayout constraintView = findViewById(R.id.constraint_content);
		try {
			ButtonView btn = new ButtonView(this);
			btn.setKeycode(keyCode);
			btn.setOnTouchListener(new MyBtnListener(keyCode));
			int bitmapId = getResources().getIdentifier(bitmap, "drawable",this.getPackageName());
			ConstraintLayout.LayoutParams layoutParams = new ConstraintLayout.LayoutParams(140, 140);
			layoutParams.setMargins(5, 3, 0, 0); // left, top, right, bottom
			btn.setLayoutParams(layoutParams);
			btn.setX(x);
			btn.setY(y);
			btn.getLayoutParams().height = h;
			btn.getLayoutParams().width = w;
			if (bitmapId > 0) {
				btn.setBitmap(bitmapId);
			}
			btn.getBackground().setAlpha(255 - transparency);
			constraintView.addView(btn);
			btn.setVisibility(View.VISIBLE);
		} catch (Exception e) {
			
		}
	}
	
	public void btnPlus(View view) {
		changeButtonSize("+");
		//ConstraintLayout constraintView = findViewById(R.id.constraint_content);
		//float  centreX=constraintView.getX() + constraintView.getWidth()  / 2;
		//float centreY=constraintView.getY() + constraintView.getHeight() / 2;
		//InitBitmapButton(123, "ic_fire", centreX, centreY, 140, 140, "Tab");
		//createTextButton("Test", centreX, centreY, KeyEvent.KEYCODE_BUTTON_R1);
	}
	
	public void btnMinus(View view) {
		changeButtonSize("-");
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

		try {
			copyFolderOrFile("Scripts/Menu");
			copyFolderOrFile("Scripts/NetSettings");
			copyFolderOrFile("Classes/AdvancedItemClasses");
			copyFolderOrFile("Classes/AdvancedMonsterClasses");
		} catch (IOException e) {
			Log.e(TAG, "Error while copying resources", e);
		}

		SeriousSamSurface.initializeLibrary(homeDir.getAbsolutePath(), getLibDir(this));
		isGameStarted = true;
		glSurfaceView.start();
	}

	public void syncOptions() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		useGyroscope = preferences.getBoolean("use_gyroscope", false);
		showTouchController = preferences.getString("showTouchController", "Auto");
		gyroSensibility = preferences.getInt("gyro_sensibility", 50) / 100.f;
		aimViewSensibility = preferences.getInt("aimView_sensibility", 100) / 100.f;
		ctrlAimSensibility = preferences.getInt("ctrl_aimSensibility", 100) / 100.f;
		deadZone = preferences.getInt("ctrl_deadZone", 20) / 100.f;
		din_uiScale = preferences.getString("din_uiScale", "On");
		ui_drawBanner = preferences.getString("ui_drawBanner", "On");
		useAimAssist = preferences.getBoolean("useAimAssist", true);
		autoAimSens = preferences.getInt("autoAimSens", 100) / 100.f;
		useVolumeKeys = preferences.getBoolean("useVolumeKeys", false);
		volumeUpKey = preferences.getString("volumeUpAction", "PrevWeapon");
		volumeDownKey = preferences.getString("volumeDownAction", "NextWeapon");
		transparency = preferences.getInt("input_opacity", 50) * 255 / 100;
		DinamicUI();
		drawBanner();
		executeShell("plr_fAutoAimSensitivity=" + autoAimSens + ";");
		AimAssistState();		
		executeShell("hud_iStats=" + (preferences.getBoolean("hud_iStats", false) ? 2 : 0) + ";");
		executeShell("input_uiScale=" + uiScale + ";");
		Log.wtf(TAG, "uiScale: " + uiScale);
		updateSoftKeyboardVisible();
	}
	
	@Subscribe(threadMode = ThreadMode.MAIN)
	public void updateUI(UpdateUIEvent event) {
		final MainActivity context = MainActivity.this;
		runOnUiThread(new Runnable() {
			public void run() {				
				if (gameState == GameState.NORMAL && !ControlsInitialized) { 
					ButtonView Fire = findViewById(R.id.input_fire);
					if ((Fire.getX() != 0.0f && Fire.getY() != 0.0f) && !ControlsInitialized) {
						setupTouchControls();
					}
				}
			}
		});
	}
	
	private class MyBtnListener implements View.OnTouchListener {
		float lastX, lastY;
		private String btnToBind;
	
		public MyBtnListener() {
			this.btnToBind = "";
		}

		public MyBtnListener(String btnToBind) {
			this.btnToBind = btnToBind;
		}
		
		float dX, dY;

		@Override
		public boolean onTouch(View v, MotionEvent event) {
			switch (event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					if (ButtonsMapping && v instanceof Button){
						isTracking = false;
						dX = v.getX() - event.getRawX();
						dY = v.getY() - event.getRawY();
					} else {
						isTracking = true;
						lastX = event.getX();
						lastY = event.getY();
						if (this.btnToBind != "") {
							nTouchKeyEvent(btnToBind, 1);
						}
					}
					break;
				case MotionEvent.ACTION_UP:
					if (this.btnToBind != "") {
						nTouchKeyEvent(btnToBind, 0);
					}
					break;
				case MotionEvent.ACTION_POINTER_UP:
					isTracking = false;
					break;
				case MotionEvent.ACTION_MOVE:
					if (isTracking) {
						float rawX = event.getX();
						float rawY = event.getY();
						shiftAxisValue(AXIS_LOOK_LR, -Utils.convertPixelsToDp(rawX - lastX, MainActivity.this) * MULT_VIEW_TRACKER * aimViewSensibility);
						shiftAxisValue(AXIS_LOOK_UD, -Utils.convertPixelsToDp(rawY - lastY, MainActivity.this) * MULT_VIEW_TRACKER * aimViewSensibility);
						lastX = rawX;
						lastY = rawY;
					} else if (ButtonsMapping) {
						if ((v instanceof ButtonView) || (v instanceof Button)) {
							float X = 0.0f;
							float Y = 0.0f;
							if (v instanceof ButtonView) {
								ButtonView btn = (ButtonView) v;
								X = event.getRawX() + dX - -Utils.convertPixelsToDp(v.getWidth() / 2, MainActivity.this) - btn.radius;
								Y = event.getRawY() + dY - -Utils.convertPixelsToDp(v.getHeight() / 2, MainActivity.this) - btn.radius;
							} else if (v instanceof Button) {
								X = event.getRawX() + dX - -Utils.convertPixelsToDp(v.getWidth() / 2, MainActivity.this);
								Y = event.getRawY() + dY - -Utils.convertPixelsToDp(v.getHeight() / 2, MainActivity.this);	
							}
							v.setX(X);
							v.setY(Y);
						}
					}
					break;
				default:
					return false;
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
	private static native void nTouchKeyEvent(String key, int isPressed);
	private static native void nConfirmEditText(String newText);
	private static native void nCancelEditText();
}
