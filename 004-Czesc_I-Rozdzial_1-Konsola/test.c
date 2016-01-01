// Kompilacja (MinGW): gcc test.c -mwindows
#include <stdio.h>
#include <windows.h>

int main(void) {
  char buffer[128] = "You have entered: ";
  scanf("%109[^\n]", buffer + 18);
  MessageBox(0, buffer, "Example", MB_OK);
  return 0;
}

