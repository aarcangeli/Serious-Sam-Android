package com.github.aarcangeli.serioussamandroid;

import android.Manifest;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

import java.io.File;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity {
    public static final String TAG = "SeriousSamJava";
    private final int REQUEST_WRITE_STORAGE = 1;

    private GLSurfaceView glSurfaceView;
    private File homeDir;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        System.loadLibrary("SeriousSamNatives");
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        homeDir = new File(Environment.getExternalStorageDirectory(), "SeriousSam");

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

    private void setup() {
        if (!homeDir.exists()) homeDir.mkdirs();
        Log.i(TAG, "HomeDir: " + homeDir.getAbsolutePath());
        setHomeDir(homeDir.getAbsolutePath());
        startEngine();

        glSurfaceView = new GLSurfaceView(this);
//        glSurfaceView.getHolder().setFixedSize(100, 100);
        glSurfaceView.setEGLContextClientVersion(2);
        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                init();
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                resize(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                doGame();
            }
        });
        setContentView(glSurfaceView);
    }

    public native void setHomeDir(String homeDir);

    public native void init();

    private native void startEngine();

    private native void resize(int width, int height);

    private native void doGame();

}
