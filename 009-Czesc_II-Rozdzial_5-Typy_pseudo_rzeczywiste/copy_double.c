#include <stdio.h>
#include <string.h>
int main(void) {
  unsigned char number[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x3F };
  double x;
  memcpy(&x, number, 8);
  printf("%f\n", x);
  return 0;
}

