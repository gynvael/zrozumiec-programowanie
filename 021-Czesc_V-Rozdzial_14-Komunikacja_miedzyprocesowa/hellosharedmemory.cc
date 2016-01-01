#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
  // Stwórz nową sekcję (pamięć dzieloną) i ją podmapuj w przestrzeni procesu.
  HANDLE h = CreateFileMapping(
      INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, "Local\\HelloWorld");
  if (h == NULL) {
    printf("error: failed to create mapping (%u)\n", 
           (unsigned int)GetLastError());
    return 1;
  }

  void *p = MapViewOfFile(h, FILE_MAP_WRITE, 0, 0, 0);
  if (p == NULL) {
    printf("error: failed to map view (%u)\n",
           (unsigned int)GetLastError());
    CloseHandle(h);
    return 1;
  }

  // Skopiuj wiadomość na początek podmapowanej sekcji.
  const char *message = "Hello World!\x1A";
  memcpy(p, message, strlen(message));

  // Poczekaj na zakończenie programu.
  puts("Press ENTER to leave...");
  getchar();

  UnmapViewOfFile(p);
  CloseHandle(h);
  return 0;
}


