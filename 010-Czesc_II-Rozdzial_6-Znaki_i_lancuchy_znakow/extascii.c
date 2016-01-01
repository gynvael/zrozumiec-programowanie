#include <stdio.h>
#include <windows.h>

void print_nice_ext_ascii(void) {
  int i, j, ch;

  printf("    | ");
  for(i = 0; i < 0x10; i++) {
    printf("+%x ", i);
  }

  puts("\n----+-------------------------------------------------");

  for (j = 0; j < 0x100; j += 0x10) {
    printf(" %.2x | ", j);
    for (i = 0; i < 0x10; i++) {
      ch = j + i;
      if (ch >= 0x20 && ch != 0x7f) {
        printf(" %c ", ch);
      } else if (ch == '\n') {
        printf("\\n ");
      } else if (ch == '\t') {
        printf("\\t ");
      } else if (ch == '\r') {
        printf("\\r ");
      } else {
        printf("   ");
      }
    }
    putchar('\n');
  }
}

int main(int argc, char **argv) {
  if (argc >= 2) {
    int codepage = atoi(argv[1]);
    SetConsoleOutputCP((UINT)codepage);
  }

  print_nice_ext_ascii();

  return 0;
}

