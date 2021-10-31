#ifndef STRINGFORMATTER_H
#define STRINGFORMATTER_H

#include <Engine/Base/CTString.h>
#include <Engine/Base/FileName.h>
#include <Engine/Entities/EntityPointer.h>

namespace stringFormatter {

  template<typename SRC>
  static SRC convertParam(SRC p) {
    return p;
  }

  static char *convertParam(CTString &p) {
    return p.str_String;
  }

  static char *convertParam(CTFileName &p) {
    return p.str_String;
  }

  static char *convertParam(CEntityPointer &p);

  // no parameters
  template<typename ...generic>
  static CTString format(const char *strPattern) {
    return strPattern;
  }

  template<typename T1>
  static CTString format(const char *strPattern, T1 p1) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1));
    return buffer;
  }

  template<typename T1, typename T2>
  static CTString format(const char *strPattern, T1 p1, T2 p2) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2));
    return buffer;
  }

  template<typename T1, typename T2, typename T3>
  static CTString format(const char *strPattern, T1 p1, T2 p2, T3 p3) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2),
             convertParam(p3));
    return buffer;
  }

  template<typename T1, typename T2, typename T3, typename T4>
  static CTString format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4));
    return buffer;
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5>
  static CTString format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5));
    return buffer;
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  static CTString format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6));
    return buffer;
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7));
    return buffer;
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7),
             convertParam(p8));
    return buffer;
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7),
             convertParam(p8), convertParam(p9));
    return buffer;
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9,
         T10 p10) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7),
             convertParam(p8), convertParam(p9), convertParam(p10));
    return buffer;
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9,
         T10 p10, T11 p11) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7),
             convertParam(p8), convertParam(p9), convertParam(p10), convertParam(p11));
    return buffer;
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9,
         T10 p10, T11 p11, T12 p12) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7),
             convertParam(p8), convertParam(p9), convertParam(p10), convertParam(p11),
             convertParam(p12));
    return buffer;
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9,
         T10 p10, T11 p11, T12 p12, T13 p13) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7),
             convertParam(p8), convertParam(p9), convertParam(p10), convertParam(p11),
             convertParam(p12), convertParam(p13));
    return buffer;
  }
  
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13 , typename T14>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9,
         T10 p10, T11 p11, T12 p12, T13 p13, T14 p14) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7),
             convertParam(p8), convertParam(p9), convertParam(p10), convertParam(p11),
             convertParam(p12), convertParam(p13), convertParam(p14));
    return buffer;
  }
  
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13 , typename T14, typename T15>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9,
         T10 p10, T11 p11, T12 p12, T13 p13, T14 p14, T15 p15) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7),
             convertParam(p8), convertParam(p9), convertParam(p10), convertParam(p11),
             convertParam(p12), convertParam(p13), convertParam(p14), convertParam(p15));
    return buffer;
  }
  
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13 , typename T14, typename T15, typename T16>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9,
         T10 p10, T11 p11, T12 p12, T13 p13, T14 p14, T15 p15, T16 p16) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7),
             convertParam(p8), convertParam(p9), convertParam(p10), convertParam(p11),
             convertParam(p12), convertParam(p13), convertParam(p14), convertParam(p15), convertParam(p16));
    return buffer;
  }
  
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13 , typename T14, typename T15, typename T16, typename T17>
  static CTString
  format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9,
         T10 p10, T11 p11, T12 p12, T13 p13, T14 p14, T15 p15, T16 p16, T17 p17) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3),
             convertParam(p4), convertParam(p5), convertParam(p6), convertParam(p7),
             convertParam(p8), convertParam(p9), convertParam(p10), convertParam(p11),
             convertParam(p12), convertParam(p13), convertParam(p14), convertParam(p15), convertParam(p16), convertParam(p17));
    return buffer;
  }
}

// forward declaration
template<typename ... Types>
void CTString::PrintF(const char *strPattern, Types... t) {
  PrintF(stringFormatter::format(strPattern, t...));
}

template<typename ... Types>
CTString::CTString(INDEX iDummy, const char *strPattern, Types... t) {
  str_String = StringDuplicate("");
  *this = stringFormatter::format(strPattern, t...);
}

#endif // STRINGFORMATTER_H
