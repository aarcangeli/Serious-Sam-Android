# Serious Sam Android
## Running the game
1. Locate the game directory for "Serious Sam Classic The Second Encounter" ([steam](https://store.steampowered.com/app/41060/Serious_Sam_Classic_The_Second_Encounter/))
1. [Download](https://github.com/aarcangeli/Serious-Sam-Android/releases/latest) and install the latest version of SeriousSamRelease.apk
1. Create a directory on your sd card called "SeriousSam"
1. Copy all *.gro files from the game directory to SeriousSam directory.
   At the current time the files are:
   * SE1_00.gro
   * SE1_00_Extra.gro
   * SE1_00_ExtraTools.gro
   * SE1_00_Levels.gro
   * SE1_00_Logo.gro
   * SE1_00_Music.gro
   * 1_04_patch.gro
   * 1_07_tools.gro
1. Start the game
   * The first time will ask you permission to read from external storage

Currently the game is playable only with a xbox 360 controller connected via usb

NB: In some devices the OTG storage must be enabled (settings -> advanced -> OTG storage)

## Compile from source
### Using Gradle script without Android Studio
1. Clone or download the repository in a directory
2. Download Android SDK (Command line tools only) https://developer.android.com/studio
![alt text](https://image.prntscr.com/image/ztZ-0HbhRCSRhNwNScoJ-A.png)
3. Unzip sdk-tools-windows-*.zip to C:\androidsdk (You can change path in local.properties)
4. Download OpenJDK https://github.com/Skyrimus/openjdk-1.8.0-win/tree/master
5. Unzip JDK to С:\jdk
6. In cmd use command
##### Set JAVA_HOME variable
    set JAVA_HOME="C:\jdk\"
7. Open cmd in C:\androidsdk\tools\bin\ folder and use command
##### Download tools and NDK
    sdkmanager.bat "cmake;3.10.2.4988404" "ndk-bundle" "platform-tools" "build-tools;29.0.0" "platforms;android-28" "platforms;android-27" "platforms;android-26" "platforms;android-25" "platforms;android-24" "platforms;android-23" "platforms;android-22" "platforms;android-21" "platforms;android-20" "platforms;android-19"
8. Open cmd in C:\jdk\bin\ and generate your keystore
##### Generate keystore, change password to keystore
    keytool.exe -genkey -v -keystore release.keystore -alias release -keyalg RSA -keysize 2048 -validity 10000
9. Move release.keystore to root path of Serious-Sam-Android source
10. Configure signing.properties file
11. Open cmd in Serious-Sam-Android folder and start compilation
##### Compile NOW!
    gradlew.bat assembleRelease
### Using Android Studio
1. Clone or download the repository in a directory
1. Open the project in Android Studio
1. Connect an android device with debugging enabled
1. Compile and run the game

If you have a precompiled version of ecc.exe you can put it inside the directory /Serious-Engine/Sources/Tools.Win32 and
resync the project (File -> Sync with Gradle Files)

## Porting notes:

### Graphic Engine

Serious Sam has been designed to work on a desktop computer in OpenGL or DirectX.<br>
This project is based on OpenGL ES, a subset of OpenGL, and many features that are necessary to run
Serious Sam are not avaiable on android.

The problem is solved by implementing an [adapter] that allows you to run OpenGL calls on OpenGL ES.

### Instruction set

[X86 assembly] is quite offen used in Serious Engine.
Since the main hardware platform for Android is [ARM],
in some cases it was necessary to convert asm codes into compatible equivalents.

CroTeam has provided (in most cases) a constant that enable to switch from asm to an equivalent C code.

There was another problem with some undefineds behaviors from the [specs](http://c0x.coding-guidelines.com/6.3.1.4.html)

For example:

```C
float source = 1e20f; // a logically large number for a 16-bit signed integer
int32_t destination = (int32_t) source;
printf("destination: %i", destination);
```

Running this snippet on Windows or Android we got two different results:

- Windows: destination: -2147483648
- Android: destination: 2147483647

This difference caused some troubles.

### Compiler

Serious Engine must be compiled with MSVC while Android ndk with Clang.
These two programs have differences.

These are just a few examples:
- ISO C++ forbids forward references to 'enum' types
- CTString instances are not casted automatically into char* when putted inside printf()
- error: non-const lvalue reference to type 'Vector<...>' cannot bind to a temporary of type 'Vector<...>'
- error: cannot initialize a parameter of type 'void *' with an rvalue of type 'BOOL (*)()'

[adapter]: Serious-Engine/Sources/AndroidAdapters/gles_adapter.cpp
[X86 assembly]: https://en.wikipedia.org/wiki/X86_assembly_language
[arm]: https://en.wikipedia.org/wiki/ARM_architecture

## Acknowledgements
* @Skyrimus - 4PDA