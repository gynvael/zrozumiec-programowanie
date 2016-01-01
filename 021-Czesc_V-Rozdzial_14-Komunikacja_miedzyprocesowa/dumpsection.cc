#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    puts("usage: dumpsection <SectionName>");
    puts("note : for \\Sessions\\N\\BaseNamedObjects\\ "
         "objects use Local\\ prefix");
    puts("       for \\BaseNamedObjects\\ objects use "
         "Global\\ prefix");
    return 1;
  }

  // Podmapuj sekcjê do przestrzeni pamiêci procesu.
  HANDLE h = OpenFileMapping(FILE_MAP_READ, FALSE, argv[1]);
  if (h == NULL) {
    printf("error: failed to open mapping (%u)\n", 
           (unsigned int)GetLastError());
    return 1;
  }

  void *p = MapViewOfFile(h, FILE_MAP_READ, 0, 0, 0);
  if (p == NULL) {
    printf("error: failed to map view (%u)\n",
           (unsigned int)GetLastError());
    CloseHandle(h);
    return 1;
  }

  // Pobierz wielkoœæ sekcji.
  MEMORY_BASIC_INFORMATION mbi;
  VirtualQuery(p, &mbi, sizeof(mbi));
  
  // Stwórz nazwê pliku
  char fname[256 + 8] = {0};
  strncpy(fname, argv[1], 255);
  for (char *ch = fname; *ch; ch++) {
    if (*ch == '\\') {
      *ch = '_';
    }
  }
  strcat(fname, ".bin");

  // Zapisz zawartoœæ sekcji do pliku.
  FILE *f = fopen(fname, "wb");
  if (f == NULL) {
    perror("error: failed to open output file");
    UnmapViewOfFile(p);
    CloseHandle(h);
    return 1;
  }

  fwrite(p, 1, mbi.RegionSize, f);
  fclose(f);

  UnmapViewOfFile(p);
  CloseHandle(h);
  return 0;
}

