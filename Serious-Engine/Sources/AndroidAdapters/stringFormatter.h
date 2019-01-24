#ifndef STRINGFORMATTER_H
#define STRINGFORMATTER_H

#include <Engine/Base/CTString.h>

namespace stringFormatter {

    template<typename SRC>
    static SRC convertParam(SRC p) {
        return p;
    }

    static char *convertParam(CTString p) {
        return p.str_String;
    }

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
        snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3));
        return buffer;
    }

    template<typename T1, typename T2, typename T3, typename T4>
    static CTString format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4) {
        const int bufferSize = 2048;
        char buffer[bufferSize];
        snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3), convertParam(p4));
        return buffer;
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    static CTString format(const char *strPattern, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
        const int bufferSize = 2048;
        char buffer[bufferSize];
        snprintf(buffer, bufferSize, strPattern, convertParam(p1), convertParam(p2), convertParam(p3), convertParam(p4), convertParam(p5));
        return buffer;
    }
}

#endif // STRINGFORMATTER_H
