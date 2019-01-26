# Serious Sam Android
========================

This project wants to make Serious Sam running natively on Android.

# General problems:
  - CTString instances are not casted automatically into char* when putted inside printf()
  - ISO C++ forbids forward references to 'enum' types
  - inline asm
  - error: non-const lvalue reference to type 'Vector<...>' cannot bind to a temporary of type 'Vector<...>'
  - error: cannot initialize a parameter of type 'void *' with an rvalue of type 'BOOL (*)()'
