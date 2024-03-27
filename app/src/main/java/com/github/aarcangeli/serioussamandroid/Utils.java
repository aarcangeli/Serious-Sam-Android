package com.github.aarcangeli.serioussamandroid;

import android.content.Context;
import android.util.DisplayMetrics;
import android.view.InputDevice;

public class Utils {
	public static float convertDpToPixel(float dp, Context context) {
		return dp * ((float) context.getResources().getDisplayMetrics().densityDpi / DisplayMetrics.DENSITY_DEFAULT);
	}

	public static float convertPixelsToDp(float px, Context context) {
		return px / ((float) context.getResources().getDisplayMetrics().densityDpi / DisplayMetrics.DENSITY_DEFAULT);
	}

	static boolean isThereControllers() {
		for (int id : InputDevice.getDeviceIds()) {
			InputDevice dev = InputDevice.getDevice(id);
			int sources = dev.getSources();
			if (((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD)
                || ((sources & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)) {
				return true;
			}
		}
		return false;
	}
}
