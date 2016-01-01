#include <malloc.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// MinGW GCC może nieposiadać deklaracji funkcji z rodziny _aligned_*, więc
// na wszelki wypadek ponowie deklarację, korzystając z:
// - https://msdn.microsoft.com/en-us/library/8z34s9c6.aspx
// - https://msdn.microsoft.com/en-us/library/17b5h8td.aspx

#ifndef _CRT_ALLOCATION_DEFINED
void * _aligned_malloc(
    size_t size, 
    size_t alignment
);

void _aligned_free (
    void *memblock
);
#endif

// Z drugiej strony, Visual C++ może nieposiadać pliku nagłówkowego stdbool.h,
// jako, że ten został dodany w C99, dla którego wsparcie w Visual C++ jest
// znikome.

typedef int bool;
#define false 0
#define true 1

#define SECTOR_SIZE 4096
static char *buffer;

void perform_test(const char *fname, bool buffered) {
  HANDLE h = CreateFile(
      fname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
      buffered ? FILE_ATTRIBUTE_NORMAL 
               : FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
      NULL);

  // Bardzo wiele zapisów.
  static char buffer[SECTOR_SIZE];
  int i;
  for (i = 0; i < 1024 * 16; ++i) {
    DWORD written;
    WriteFile(h, buffer, SECTOR_SIZE, &written, NULL);
  }

  CloseHandle(h);
}

int main(void) {
  clock_t a, b, c;

  // W przypadku zapisu bez buforowania po stronie jądra, dane muszą być
  // przygotowane w buforze o wielkości, która jest wielokrotnością wielkości
  // pojedynczego sektora na dysku (liczba 4096 spełnia tą właściwość zarówno
  // dla starszych, jak i nowszych dysków). Zalecane jest również ulokowanie
  // ich na adresie wyrównanym do wielkości sektora.
  buffer = _aligned_malloc(SECTOR_SIZE, SECTOR_SIZE);
  memset(buffer, 0, SECTOR_SIZE);

  a = clock();
  perform_test("buffered", true);
  b = clock();
  perform_test("unbuffered", false);
  c = clock();

  printf("Cached  : %u clocks\n", b - a);
  printf("Uncached: %u clocks\n", c - b);

  _aligned_free(buffer);

  return 0;
}

