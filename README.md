# Serious Sam Android

## Running the game
1. Locate the game directory for "Serious Sam Classic The Second Encounter" ([steam](https://store.steampowered.com/app/41060/Serious_Sam_Classic_The_Second_Encounter/))
1. [Download](https://github.com/aarcangeli/Serious-Sam-Android/releases) and install SeriousSamRelease.apk
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

### Compile from source
If you want to compile the apk file from the source, proceed as follows:
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
