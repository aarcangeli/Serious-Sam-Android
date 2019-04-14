adb logcat -c
adb logcat | ndk-stack -sym app\build\intermediates\cmake\release\obj\armeabi-v7a > stack.txt