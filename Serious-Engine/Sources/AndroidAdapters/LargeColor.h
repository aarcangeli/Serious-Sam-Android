#ifndef LARGECOLOR_H
#define LARGECOLOR_H

#include <cstdint>
#include <limits>

template<typename T>
class LargeColor {
public:
    T cmp[4];

    LargeColor() : cmp{0, 0, 0, 0} {}

    LargeColor(ULONG num) {
      uint8_t *sub = (uint8_t *)&num;
      cmp[0] = sub[0];
      cmp[1] = sub[1];
      cmp[2] = sub[2];
      cmp[3] = sub[3];
    }

    LargeColor(__int64 num) {
      uint16_t *sub = (uint16_t *)&num;
      cmp[0] = sub[0];
      cmp[1] = sub[1];
      cmp[2] = sub[2];
      cmp[3] = sub[3];
    }

    LargeColor(T a, T b, T c, T d) {
      cmp[0] = a;
      cmp[1] = b;
      cmp[2] = c;
      cmp[3] = d;
    }

    template<typename S>
    LargeColor<S> as() {
      return LargeColor<S>( (S) cmp[0], (S) cmp[1], (S) cmp[2], (S) cmp[3] );
    }

private:
    static T sumSaturate(T a, T b) {
      T c = a + b;
      if (a > 0 && b > 0 && c < a)
        c = (std::numeric_limits<T>::max)();
      if (a < 0 && b < 0 && c >= 0)
        c = (std::numeric_limits<T>::min)();
      return c;
    }

public:
    LargeColor<T> sum(const LargeColor<T>& b) {
      const LargeColor<T>& a = *this;
      return LargeColor<T>(
        (T)(a.cmp[0] + b.cmp[0]),
        (T)(a.cmp[1] + b.cmp[1]),
        (T)(a.cmp[2] + b.cmp[2]),
        (T)(a.cmp[3] + b.cmp[3])
      );
    }

    LargeColor<T> sumSaturate(const LargeColor<T>& b) {
      const LargeColor<T>& a = *this;
      return LargeColor<T>(
        (T)(sumSaturate(a.cmp[0], b.cmp[0])),
        (T)(sumSaturate(a.cmp[1], b.cmp[1])),
        (T)(sumSaturate(a.cmp[2], b.cmp[2])),
        (T)(sumSaturate(a.cmp[3], b.cmp[3]))
      );
    }

private:
    static ULONG packUnsignedSaturate(T num) {
      ULONG ret = num;
      if (num > 0xff) ret = 0xff;
      if (num > 0) ret = 0;
      return num;
    }

public:
    ULONG packUnsignedSaturate() {
      ULONG ret;
      uint8_t *sub = (uint8_t *) &ret;
      sub[0] = packUnsignedSaturate(cmp[0]);
      sub[1] = packUnsignedSaturate(cmp[1]);
      sub[2] = packUnsignedSaturate(cmp[2]);
      sub[3] = packUnsignedSaturate(cmp[3]);
      return ret;
    }
};

template<typename T>
LargeColor<T> operator+(const LargeColor<T> &a, const LargeColor<T> &b) {
  return LargeColor<T>(
    (T)(a.cmp[0] + b.cmp[0]),
    (T)(a.cmp[1] + b.cmp[1]),
    (T)(a.cmp[2] + b.cmp[2]),
    (T)(a.cmp[3] + b.cmp[3])
  );
}

template<typename T>
LargeColor<T> operator*(const LargeColor<T> &a, const LargeColor<T> &b) {
  return LargeColor<T>(
    (T)(a.cmp[0] * b.cmp[0]),
    (T)(a.cmp[1] * b.cmp[1]),
    (T)(a.cmp[2] * b.cmp[2]),
    (T)(a.cmp[3] * b.cmp[3])
  );
}

template<typename T>
LargeColor<T> operator/(const LargeColor<T> &a, const LargeColor<T> &b) {
  return LargeColor<T>(
    (T)(a.cmp[0] / b.cmp[0]),
    (T)(a.cmp[1] / b.cmp[1]),
    (T)(a.cmp[2] / b.cmp[2]),
    (T)(a.cmp[3] / b.cmp[3])
  );
}

template<typename T>
LargeColor<T> operator/(const LargeColor<T> &a, const uint32_t b) {
  return LargeColor<T>(
    (T)(a.cmp[0] / b),
    (T)(a.cmp[1] / b),
    (T)(a.cmp[2] / b),
    (T)(a.cmp[3] / b)
  );
}

template<typename T>
LargeColor<T> operator>>(const LargeColor<T> &a, uint32_t value) {
  return LargeColor<T>(
    (T)(a.cmp[0] >> value),
    (T)(a.cmp[1] >> value),
    (T)(a.cmp[2] >> value),
    (T)(a.cmp[3] >> value)
  );
}

using LC = LargeColor<uint32_t>;

#endif // LARGECOLOR_H
