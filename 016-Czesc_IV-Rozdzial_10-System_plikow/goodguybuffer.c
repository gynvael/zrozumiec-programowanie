#include <time.h>
#include <stdio.h>
#include <stdbool.h>

void perform_test(const char *fname, bool buffered) {
  FILE *f = fopen(fname, "wb");
  if (!buffered) {
    // Wyłączenie buforowania.
    setvbuf(f, NULL, _IONBF, 0);
  }

  // Bardzo wiele małych zapisów.
  for (int i = 0; i < 1024 * 1024; ++i) {
    char c = 'A';
    fwrite(&c, 1, 1, f);
  }

  fclose(f);
}

int main(void) {
  clock_t a, b, c;

  a = clock();
  perform_test("buffered", true);
  b = clock();
  perform_test("unbuffered", false);
  c = clock();

  printf("Buffered  : %u clocks\n", b - a);
  printf("Unbuffered: %u clocks\n", c - b);

  return 0;
}

