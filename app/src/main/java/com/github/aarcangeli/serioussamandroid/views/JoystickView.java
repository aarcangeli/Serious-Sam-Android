package com.github.aarcangeli.serioussamandroid.views;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.RadialGradient;
import android.graphics.Shader;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import com.github.aarcangeli.serioussamandroid.Utils;

public class JoystickView extends View {
    public static float sizePad;
    public static float sizePadBg;
    public static float sizePadBg2;

    public static final int COLOR_PAD = 0xFF343434;
    public static final int COLOR_PAD2 = 0xFF404040;
    public static final int COLOR_BG = 0xFF1b1b1b;
    public static final int COLOR_BG2 = 0xFF343434;
    private Paint paintBgFill = new Paint();
    private Paint paintBgStroke = new Paint();
    private Paint paintPadFill = new Paint();
    private Paint paintPadStroke = new Paint();
    public float padPosX, padPosY;
    public boolean ButtonsMapping = false;
    private float deltaX;
    private float deltaY;
    private Listener listener;
    private RadialGradient padShader;

    private final SharedPreferences preferences;
    private int opacity;

    private SharedPreferences.OnSharedPreferenceChangeListener preferenceListener = new SharedPreferences.OnSharedPreferenceChangeListener() {
        @Override
        public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
            readPreferences();
        }
    };

    // temporaney canvas for opacity
    Bitmap tempBmp;
    Canvas c = new Canvas();
    private Paint alphaPaint = new Paint();

    public JoystickView(Context context) {
        this(context, null);
    }

    public JoystickView(Context context, AttributeSet attrs) {
        super(context, attrs);

        preferences = PreferenceManager.getDefaultSharedPreferences(getContext());
        preferences.registerOnSharedPreferenceChangeListener(preferenceListener);
        readPreferences();
    }

    private void readPreferences() {
        opacity = preferences.getInt("input_opacity", 70) * 255 / 100;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        float width = MeasureSpec.getSize(widthMeasureSpec);
        float height = MeasureSpec.getSize(heightMeasureSpec);
        padPosX = width / 2;
        padPosY = height / 2;

        sizePadBg2 = Math.min(width, height) * 0.3f - Utils.convertDpToPixel(4, getContext());
        sizePadBg = sizePadBg2 - Utils.convertDpToPixel(10, getContext());
        sizePad = sizePadBg - Utils.convertDpToPixel(14, getContext());

        paintBgFill.setStyle(Paint.Style.FILL);
        paintBgFill.setAntiAlias(true);
        paintBgFill.setFilterBitmap(true);

        paintPadFill.setStyle(Paint.Style.FILL);
        paintPadFill.setAntiAlias(true);
        paintPadFill.setFilterBitmap(true);
        paintPadFill.setColor(COLOR_PAD);

        paintBgStroke.setStyle(Paint.Style.STROKE);
        paintBgStroke.setAntiAlias(true);
        paintBgStroke.setFilterBitmap(true);
        paintBgStroke.setColor(Color.BLACK);
        paintBgStroke.setStrokeWidth(Utils.convertDpToPixel(3, getContext()));

        paintPadStroke.setStyle(Paint.Style.STROKE);
        paintPadStroke.setAntiAlias(true);
        paintPadStroke.setFilterBitmap(true);
        paintPadStroke.setColor(Color.BLACK);
        paintPadStroke.setStrokeWidth(Utils.convertDpToPixel(4, getContext()));
        paintPadStroke.setShadowLayer(Utils.convertDpToPixel(4, getContext()), 0, 0, Color.BLACK);

        float radius = Utils.convertDpToPixel(sizePad, getContext());
        padShader = new RadialGradient(0, 0, radius, new int[]{COLOR_PAD, COLOR_PAD2, COLOR_PAD}, new float[]{0, 0.7f, 1}, Shader.TileMode.MIRROR);
    }

    @Override
    @SuppressLint("CanvasSize")
    protected void onDraw(Canvas canvas) {
        if (canvas == null || canvas.getWidth() <= 0 || canvas.getHeight() <= 0) return;
        if (tempBmp == null || tempBmp.isRecycled() || tempBmp.getWidth() != canvas.getWidth() || tempBmp.getHeight() != canvas.getHeight()) {
            if (tempBmp != null) tempBmp.recycle();
            tempBmp = Bitmap.createBitmap(canvas.getWidth(), canvas.getHeight(), Bitmap.Config.ARGB_8888);
            c.setBitmap(tempBmp);
        }
        setLayerType(LAYER_TYPE_SOFTWARE, paintPadFill);

        // clear previous drawings
        c.save();
        c.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);

        // pad bg
        c.translate(padPosX, padPosY);
       // paintBgFill.setColor(COLOR_BG2);
       // c.drawCircle(0, 0, sizePadBg2, paintBgFill);
       // paintBgFill.setColor(COLOR_BG);
       // c.drawCircle(0, 0, sizePadBg, paintBgFill);
       // c.drawCircle(0, 0, sizePadBg2, paintBgStroke);
       // c.drawCircle(0, 0, sizePadBg, paintBgStroke);

        // pad
        if (!ButtonsMapping) {
        c.translate(deltaX, deltaY);
        }
        paintPadFill.setShader(padShader);
        c.drawCircle(0, 0, sizePad, paintPadStroke);
        c.drawCircle(0, 0, sizePad, paintPadFill);

        // copy to destination canvas
        c.restore();
        alphaPaint.setAlpha(opacity);
        canvas.drawBitmap(tempBmp, 0, 0, alphaPaint);
    }

    @Override
    @SuppressLint("ClickableViewAccessibility")
    public boolean onTouchEvent(MotionEvent event) {
        float sizePx = sizePad;
        float posX = event.getX();
        float posY = event.getY();
        int action = event.getAction();

        switch (action) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_MOVE:

                float deltaX = posX - padPosX;
                float deltaY = posY - padPosY;
                float distance = (float) Math.sqrt(deltaX * deltaX + deltaY * deltaY);

                if (action == MotionEvent.ACTION_DOWN && distance > sizePx * 2) {
                    return false;
                }

                if (distance > sizePx) {
                    deltaX = deltaX * sizePx / distance;
                    deltaY = deltaY * sizePx / distance;
                }

                this.deltaX = deltaX;
                this.deltaY = deltaY;
                listener.onMove(-deltaX / sizePx, -deltaY / sizePx, event);

                invalidate();
                return true;

            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                this.deltaX = 0;
                this.deltaY = 0;
                invalidate();
                listener.onMove(0, 0, event);
                return true;
        }
        return false;
    }

    public void setListener(Listener listener) {
        this.listener = listener;
    }

    public interface Listener {
        void onMove(float deltaX, float deltaY, MotionEvent ev);
    }
}
