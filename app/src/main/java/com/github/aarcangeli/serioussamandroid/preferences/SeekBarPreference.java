package com.github.aarcangeli.serioussamandroid.preferences;

import android.app.AlertDialog;
import android.content.Context;
import android.os.Bundle;
import android.os.Parcelable;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

public class SeekBarPreference extends DialogPreference {
    private static final String ANDROID_NS = "http://schemas.android.com/apk/res/android";

    private SeekBar mSeekBar;
    private TextView mValueText;
    private Context mContext;

    private String mDialogMessage, mSuffix;
    private int mDefault, mMax;
    private int mValue = 0;
    private OnPositiveButtonClick onPositiveButtonClick = new OnPositiveButtonClick();
    private MyOnSeekBarChangeListener onSeekBarChangeListener = new MyOnSeekBarChangeListener();

    public SeekBarPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;

        int mDialogMessageId = attrs.getAttributeResourceValue(ANDROID_NS, "dialogMessage", 0);
        if (mDialogMessageId == 0) {
            mDialogMessage = attrs.getAttributeValue(ANDROID_NS, "dialogMessage");
        } else {
            mDialogMessage = mContext.getString(mDialogMessageId);
        }

        int mSuffixId = attrs.getAttributeResourceValue(ANDROID_NS, "text", 0);
        if (mSuffixId == 0) {
            mSuffix = attrs.getAttributeValue(ANDROID_NS, "text");
        } else {
            mSuffix = mContext.getString(mSuffixId);
        }

        mDefault = attrs.getAttributeIntValue(ANDROID_NS, "defaultValue", 0);
        mMax = attrs.getAttributeIntValue(ANDROID_NS, "max", 100);

        setProgress(getPersistedInt(mDefault));
    }

    @Override
    protected View onCreateDialogView() {
        LinearLayout layout = new LinearLayout(mContext);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(6, 6, 6, 6);

        TextView mSplashText = new TextView(mContext);
        mSplashText.setPadding(30, 10, 30, 10);
        if (mDialogMessage != null)
            mSplashText.setText(mDialogMessage);
        layout.addView(mSplashText);

        mValueText = new TextView(mContext);
        mValueText.setGravity(Gravity.CENTER_HORIZONTAL);
        mValueText.setTextSize(32);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
        layout.addView(mValueText, params);

        mSeekBar = new SeekBar(mContext);
        mSeekBar.setOnSeekBarChangeListener(onSeekBarChangeListener);
        layout.addView(mSeekBar, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

        mSeekBar.setMax(mMax);
        mSeekBar.setProgress(mValue);

        return layout;
    }

    @Override
    public CharSequence getSummary() {
        CharSequence summary = super.getSummary();
        if (summary == null) {
            return null;
        } else {
            return String.format(summary.toString(), mValue);
        }
    }

    @Override
    protected void onBindDialogView(View v) {
        super.onBindDialogView(v);
        mSeekBar.setMax(mMax);
        mSeekBar.setProgress(mValue);
    }

    @Override
    protected void onSetInitialValue(boolean restore, Object defaultValue) {
        super.onSetInitialValue(restore, defaultValue);
        int value;
        if (restore)
            value = shouldPersist() ? getPersistedInt(mDefault) : 0;
        else
            value = defaultValue instanceof Integer ? (Integer) defaultValue : 0;
        if (mValue != value) {
            mValue = value;
            notifyChanged();
        }
    }

    public void setMax(int max) {
        mMax = max;
    }

    public int getMax() {
        return mMax;
    }

    public void setProgress(int progress) {
        if (mValue != progress) {
            mValue = progress;
            notifyChanged();
            if (mSeekBar != null)
                mSeekBar.setProgress(progress);
        }
    }

    public int getProgress() {
        return mValue;
    }

    @Override
    public void showDialog(Bundle state) {
        super.showDialog(state);
        ((AlertDialog) getDialog()).getButton(AlertDialog.BUTTON_POSITIVE).setOnClickListener(onPositiveButtonClick);
    }

    private class OnPositiveButtonClick implements OnClickListener {
        @Override
        public void onClick(View v) {
            int progress = mSeekBar.getProgress();
            if (mValue != progress) {
                mValue = progress;
                persistInt(progress);
                callChangeListener(progress);
                notifyChanged();
            }
            getDialog().dismiss();
        }
    }

    private class MyOnSeekBarChangeListener implements SeekBar.OnSeekBarChangeListener {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            String t = String.valueOf(progress);
            mValueText.setText(mSuffix == null ? t : t.concat(" " + mSuffix));
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
        }
    }
}
