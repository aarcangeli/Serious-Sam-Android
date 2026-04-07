# Serious Sam Android [![Build status](https://ci.appveyor.com/api/projects/status/pycufpcyo9i4axl8?svg=true)](https://ci.appveyor.com/project/Skyrimus/serious-sam-android/branch/master)

# Serious Sam Classic The First Encounter
## Running the game
1. Locate the game directory for "Serious Sam Classic The First Encounter" ([steam](https://store.steampowered.com/app/41050/Serious_Sam_Classic_The_First_Encounter/))
1. [Download](https://github.com/aarcangeli/Serious-Sam-Android/releases/latest) and install the latest version of SeriousSam-TFE-Release.apk
1. Create a directory on your sdcard called "SeriousSamTFE"
1. Copy all *.gro files and Levels folder from the game directory to SeriousSamTFE directory.
   At the current time the files are:
   * Levels (folder)
   * 1_00_ExtraTools.gro
   * 1_00_music.gro
   * 1_00c.gro
   * 1_00c_Logo.gro
   * 1_00c_scripts.gro
   * 1_04_patch.gro
1. Start the game
   * The first time will ask you permission to read from external storage

# Serious Sam Classic The Second Encounter
## Running the game
1. Locate the game directory for "Serious Sam Classic The Second Encounter" ([steam](https://store.steampowered.com/app/41060/Serious_Sam_Classic_The_Second_Encounter/))
1. [Download](https://github.com/aarcangeli/Serious-Sam-Android/releases/latest) and install the latest version of SeriousSam-TSE-Release.apk
1. Create a directory on your sd card called "SeriousSamTSE"
1. Copy all *.gro files from the game directory to SeriousSamTSE directory.
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

## Compile from source

### Using Android Studio
1. Clone or download the repository in a directory
1. Open the project in Android Studio
1. If necessary install the suggested packages
1. Connect an android device with debugging enabled
1. Compile and run the game

### Using command line (without Android Studio)
1. Clone or download the repository in a directory
2. Download Android SDK (Command line tools only) https://developer.android.com/studio
![alt text](https://image.prntscr.com/image/ztZ-0HbhRCSRhNwNScoJ-A.png)
3. Unzip sdk-tools-windows-*.zip to C:\androidsdk (You can change path in local.properties)
3. Create a file named 'local.properties' in the project root (near settings.gradle) with the following content:
```
sdk.dir=C:\\androidsdk
ndk.dir=C:\\androidsdk\\ndk-bundle
```
5. Download and install [Java SE](https://www.oracle.com/java/technologies/javase/javase-jdk8-downloads.html)
6. In cmd set JAVA_HOME use command
```cmd
    set JAVA_HOME="C:\jdk\"
```
7. Download tools and NDK. Open cmd in C:\androidsdk\tools\bin\ folder and use command
```cmd
    sdkmanager.bat "cmake;3.10.2.4988404" "platform-tools" "platforms;android-28"
```
8. Open cmd in C:\jdk\bin\ and generate your keystore
```cmd
    keytool.exe -genkey -v -keystore release.keystore -alias release -keyalg RSA -keysize 2048 -validity 10000
```
9. Move release.keystore to root path of Serious-Sam-Android source
10. Configure signing.properties file
11. Open cmd in Serious-Sam-Android folder and start compilation
```cmd
    gradlew assembleRelease
```
