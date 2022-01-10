package com.github.aarcangeli.serioussamandroid;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;

public class SplashScreen extends Activity {
	public static final String TAG = "SeriousSamJava";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// speed up bootstrap if we have already storage permission
		MainActivity.tryPremain(this);

		super.onCreate(savedInstanceState);
		setContentView(R.layout.splash_layout);

		getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);

		new Handler().postDelayed(new Runnable() {
			public void run() {
				startSeriousSam();
			}
		}, 2000);
	}

	private void startSeriousSam() {
		startActivity(new Intent(this, MainActivity.class));
		finish();
	}

}
