package com.github.aarcangeli.serioussamandroid.views;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.view.View;
import android.widget.Button;

import com.github.aarcangeli.serioussamandroid.R;
import com.github.aarcangeli.serioussamandroid.Utils;

@SuppressLint("AppCompatCustomView")
public class ButtonView extends Button {
    public static final int COLOR_PAD = 0xFF5a5a5a;
    private Bitmap buttonBitmap;
    private Paint paintPadFill = new Paint();
    private Paint paintPadStroke = new Paint();
    private float centerX, centerY, radius;

    // temporaney canvas for opacity
    Bitmap tempBmp;
    Canvas c = new Canvas();
    private Paint alphaPaint = new Paint();

    private final SharedPreferences preferences;
    private int opacity;

    private SharedPreferences.OnSharedPreferenceChangeListener preferenceListener = new SharedPreferences.OnSharedPreferenceChangeListener() {
        @Override
        public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
            readPreferences();
        }
    };

    public ButtonView(Context context) {
        this(context, null);
    }

    public ButtonView(Context context, AttributeSet attrs) {
        super(context, attrs);

        setBackgroundColor(Color.TRANSPARENT);

        if (attrs != null) {
            TypedArray typedArray = context.obtainStyledAttributes(attrs, R.styleable.ButtonView);
            if (typedArray != null) {
                int buttonResId = typedArray.getResourceId(R.styleable.ButtonView_btnBitmap, -1);
                if (buttonResId > 0) {
                    buttonBitmap = BitmapFactory.decodeResource(getResources(), buttonResId);
                }

                typedArray.recycle();
            }
        }

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
        centerX = width / 2;
        centerY = height / 2;
        radius = Math.min(width, height) / 2 - Utils.convertDpToPixel(4, getContext());

        paintPadFill.setStyle(Paint.Style.FILL);
        paintPadFill.setAntiAlias(true);
        paintPadFill.setFilterBitmap(true);
        paintPadFill.setColor(COLOR_PAD);

        paintPadStroke.setStyle(Paint.Style.STROKE);
        paintPadStroke.setAntiAlias(true);
        paintPadStroke.setFilterBitmap(true);
        paintPadStroke.setColor(Color.BLACK);
        paintPadStroke.setStrokeWidth(Utils.convertDpToPixel(4, getContext()));
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

        // draw button
        c.translate(centerX, centerY);

        paintPadStroke.setShadowLayer(Utils.convertDpToPixel(2, getContext()), 0, 0, Color.BLACK);
       // c.drawCircle(0, 0, radius, paintPadStroke);
       // c.drawCircle(0, 0, radius, paintPadFill);

        paintPadStroke.setShadowLayer(0, 0, 0, Color.BLACK);
        c.drawCircle(0, 0, radius - Utils.convertDpToPixel(3, getContext()), paintPadStroke);
        c.drawCircle(0, 0, radius - Utils.convertDpToPixel(3, getContext()), paintPadFill);

        if (buttonBitmap != null) {
            c.drawBitmap(buttonBitmap, -buttonBitmap.getWidth() / 2, -buttonBitmap.getHeight() / 2, null);
        }

        // copy to destination canvas
        c.restore();
        alphaPaint.setAlpha(opacity);
        canvas.drawBitmap(tempBmp, 0, 0, alphaPaint);
    }
}
