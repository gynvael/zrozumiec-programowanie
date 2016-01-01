#include <windows.h>
#include <stdio.h>

int main(void) {
  puts(GetCommandLine());
  // Patrz rowniez: CommandLineToArgvW
  return 0;
}

