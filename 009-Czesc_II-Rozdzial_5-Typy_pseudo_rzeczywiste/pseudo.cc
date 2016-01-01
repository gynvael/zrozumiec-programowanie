#include <stdio.h>
#include <string.h>

int main(void) {
  long double x, y;

  // 3.3621e-4932 encoded as Pseudo-Denormal value.
  unsigned char x_bytes_le[] = {
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, // Significand.
    0x00, 0x00, // Exponent (-16382 denormal) + Sign bit.
  };

  // 3.3621e-4932 encoded as Normal value.
  unsigned char y_bytes_le[] = {
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, // Significand.
    0x01, 0x00, // Exponent (-16382) + Sign bit.
  };

  // Copy byte representation.
  memcpy(&x, x_bytes_le, 10);
  memcpy(&y, y_bytes_le, 10);

  // Are they equal? They should be.
  printf("x==y --> %i\n", x == y);

  return 0;
}
