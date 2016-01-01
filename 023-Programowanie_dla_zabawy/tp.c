#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>

int main(void) {
  unsigned long long kStart = 0x10000000ULL;
  unsigned long long kEnd =   0x700000000000ULL;

  // Allocate a page at an "unknown" address to look for it.
  mmap((void *)0x45551341a000, 0x1000, PROT_EXEC | PROT_READ | PROT_WRITE,
              MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
  
  // Look where the data was allocated.
  mmap((void *)kStart, 0x1000, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE |
                 MAP_FIXED | MAP_NORESERVE, -1, 0);
  
  unsigned long long sz = kEnd - kStart;
  unsigned long long left = 0;  
  unsigned long long right = sz;
  unsigned long long old_size = 0x1000;

  while (1) {
    unsigned long long attempt = (left + (right - left) / 2) & ~0xfffULL;

    if (right - left <= 0x1000) {
      // At this point right and left should be equal.
      puts("the end");
      printf("probably at: %llx\n", kStart + left);
      break;
    }

    void *n = mremap((void *)kStart, old_size, attempt, 0);

    int ok = (n != (void *)(~0ULL));

    printf("[%llx] %llx - %llx --> %i [%p]\n", old_size, kStart + left,
           kStart + attempt, ok, n);

    if (ok) {
      old_size = attempt;
      left = attempt;
    } else {
      right = attempt;
    }
  }

  return 0;
}

