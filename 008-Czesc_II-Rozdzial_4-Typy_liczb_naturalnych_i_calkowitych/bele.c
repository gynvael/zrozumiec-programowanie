#include <stdio.h>
#include <stdint.h>

uint32_t byte_swap(uint32_t n) {
  uint32_t bytes[4] = { 
    (n & 0x000000ff),
    (n & 0x0000ff00) >> 8,
    (n & 0x00ff0000) >> 16,
    (n & 0xff000000) >> 24
  };

  bytes[0] <<= 24;
  bytes[1] <<= 16;
  bytes[2] <<= 8;

  return bytes[0] | bytes[1] | bytes[2] | bytes[3];
}

uint32_t byte_swap2(uint32_t n) {
  return ((n & 0x000000ff) << 24) |
         ((n & 0x0000ff00) << 8) |
         ((n & 0x00ff0000) >> 8) |
         ((n & 0xff000000) >> 24);
}

int main(void) {
  uint32_t a = 0x12345678;

  printf("%.8x %.8x %.8x\n", a, byte_swap(a), byte_swap2(a));


  return 0;
}


