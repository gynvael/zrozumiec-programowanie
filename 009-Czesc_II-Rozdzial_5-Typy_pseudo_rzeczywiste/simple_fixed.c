#include <stdio.h>
#include <stdint.h>

// Unsigned fixed point type with 8 bit integer and 8 bit fraction.
typedef uint16_t myfixed_t;

inline myfixed_t float_to_myfixed(float f) {
  return (uint16_t)(f * 256.0f);
}

inline float myfixed_to_float(myfixed_t m) {
  return (float)m / 256.0f;
}

inline myfixed_t myfixed_add(myfixed_t a, myfixed_t b) {
  return a + b;
}

inline myfixed_t myfixed_sub(myfixed_t a, myfixed_t b) {
  return a - b;
}

inline myfixed_t myfixed_mul(myfixed_t a, myfixed_t b) {
  return ((uint32_t)a * (uint32_t)b) >> 8;
}

inline myfixed_t myfixed_div(myfixed_t a, myfixed_t b) {
  return ((uint32_t)a << 8) / b;
}

int main() {
  myfixed_t a = float_to_myfixed(12.0f);
  myfixed_t b = float_to_myfixed(1.0f);
  myfixed_t c = float_to_myfixed(0.45f);
  myfixed_t d = float_to_myfixed(2.5f);

  printf("initial=%.2f\n", myfixed_to_float(a));
 
  a = myfixed_mul(a, c); // Exact result: 5.4
  printf(" result=%.2f\n", myfixed_to_float(a));  

  a = myfixed_sub(a, d); // Exact result: 2.9
  printf(" result=%.2f\n", myfixed_to_float(a));

  a = myfixed_div(a, c); // Exact result: 6.4(4)
  printf(" result=%.2f\n", myfixed_to_float(a));  

  a = myfixed_add(a, b); // Exact result: 7.4(4)
  printf(" result=%.2f\n", myfixed_to_float(a));  

  return 0;
}


