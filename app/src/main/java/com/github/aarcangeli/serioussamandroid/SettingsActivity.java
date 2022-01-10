package com.github.aarcangeli.serioussamandroid;

import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;

public class SettingsActivity extends PreferenceActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		getFragmentManager().beginTransaction().replace(android.R.id.content, new MyPreferenceFragment()).commit();
	}

	public static class MyPreferenceFragment extends PreferenceFragment {
		@Override
		public void onCreate(final Bundle savedInstanceState) {
			super.onCreate(savedInstanceState);
			addPreferencesFromResource(R.xml.preferences);

			final CheckBoxPreference useGyroscope = (CheckBoxPreference) findPreference("use_gyroscope");
			final Preference gyroSensibility = findPreference("gyro_sensibility");
			final CheckBoxPreference useAimAssist = (CheckBoxPreference) findPreference("useAimAssist");
			final Preference autoAimSens = findPreference("autoAimSens");
			
			if (useGyroscope != null && gyroSensibility != null) {
				gyroSensibility.setEnabled(useGyroscope.isChecked());
				useGyroscope.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
					@Override
					public boolean onPreferenceChange(Preference preference, Object newValue) {
						gyroSensibility.setEnabled((Boolean) newValue);
						return true;
					}
				});
			}
			
			if (useAimAssist != null && autoAimSens != null) {
				autoAimSens.setEnabled(useAimAssist.isChecked());
				useAimAssist.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
					@Override
					public boolean onPreferenceChange(Preference preference, Object newValue) {
						autoAimSens.setEnabled((Boolean) newValue);
						return true;
					}
				});
			}
		}
	}
}
