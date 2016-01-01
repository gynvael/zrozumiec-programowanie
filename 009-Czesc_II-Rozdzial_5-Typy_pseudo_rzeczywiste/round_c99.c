#include <fenv.h>
#include <stdio.h>
#include <string.h>

// This pragma is said to be required by the C99 standard, but some compilers
// don't seem to know it.
//#pragma STDC FENV_ACCESS ON

const char *get_fraction(double x) {
  // Assuming little endian.
  unsigned char x_bytes[8];
  memcpy(x_bytes, &x, 8);

  // This is multithread unsafe.
  static char text[56] = "1.";
  char *p = text + 2;
  for (int i = 51; i >= 0; i--, p++) {
    int byte = i / 8;
    int bit = i % 8;
    *p = "01"[((x_bytes[byte] >> bit) & 1)];
  }
  *p = '\0';

  return text;
}

int main(void) {
  volatile double a = 2.0;
  volatile double b = 10.0;
  volatile double x;

  struct {
    int i;
    const char *s;
  } rounding[] = {
#define R(a) { a, #a }
    R(FE_DOWNWARD),
    R(FE_TONEAREST),
    R(FE_TOWARDZERO),
    R(FE_UPWARD)
#undef R
  };

  for (size_t i = 0; i < sizeof(rounding) / sizeof(*rounding); i++) {
    fesetround(rounding[i].i);
    x = a / b;
    printf("--- %s:\n    x: %.17f\n frac: %s\n\n", 
        rounding[i].s, x, get_fraction(x));
  }

  return 0;
}


