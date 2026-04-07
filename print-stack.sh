#!/bin/bash
adb logcat -c
adb logcat | /root/sdk/ndk-bundle/ndk-stack -sym app/build/intermediates/cmake/release/obj/armeabi-v7a > stack.txt
