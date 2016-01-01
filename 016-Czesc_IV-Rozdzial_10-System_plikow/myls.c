#include <stdio.h>
#include <windows.h>

//int _CRT_glob = 0;

void list_files(const char *pattern);

int main(int argc, char **argv) {
  int i;

  if (argc == 1) {
    list_files(".\\*");
  } else {
    for (i = 1; i < argc; i++) {
      list_files(argv[i]);
    }
  }
  return 0;
}

void list_files(const char *pattern) {
  HANDLE h;
  WIN32_FIND_DATA entry;
  printf("--- Listing files for: %s\n", pattern);

  // 32-bitowe procesy na 64-bitowych systemach z rodziny Windows widzą
  // częściowo zwirtualizowany system plików - dotyczy to w szczególności
  // katalogu C:\Windows\system32, który w przypadku 32-bitowych procesów
  // przekierowuje do C:\Windows\sysWOW64.
  // Aby chwilowo wyłączyć wirtualizację można poslużyć się funkcją:
  //   PVOID old;
  //   Wow64DisableWow64FsRedirection(&old);
  // Po wylistowaniu plików można włączyć ponownie wirtualizacje:
  //   Wow64RevertWow64FsRedirection(old);

  h = FindFirstFile(pattern, &entry);
  if (h == INVALID_HANDLE_VALUE) {
    DWORD last_error = GetLastError();
    if (last_error == ERROR_FILE_NOT_FOUND) {
      puts("(no files found)");
      return;
    }

    printf("(error: %u)\n", (unsigned int)last_error);
    return;
  }

  do {
    // Ustal wypisany typ pliku.
    const char *type = "FILE";
    if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      type = "DIR ";
    }

    if ((entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
      type = "LINK";
    }

    // Wypisz typ i nazwę pliku.
    // (w strukturze WIN32_FIND_DATA jest również dużo innych ciekawych
    // informacji)
    printf("[%s] %s\n", type, entry.cFileName);    
  } while (FindNextFile(h, &entry));

  FindClose(h);
}

