package com.github.aarcangeli.serioussamandroid;

import android.opengl.GLSurfaceView;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity {
    public static final String TAG = "MainActivity";

    private GLSurfaceView glSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        glSurfaceView = new GLSurfaceView(this);
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
                draw();
            }
        });
        setContentView(glSurfaceView);
    }

    public native void init();

    private native void resize(int width, int height);

    private native void draw();

    static {
        System.loadLibrary("SeriousSamNatives");
    }
}
