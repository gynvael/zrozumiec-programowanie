#include <windows.h>
#include <string.h>
#include <stdio.h>

static char buffer[1024 * 1024 * 128];  // 128 MB

int main(void) {
  HANDLE h = CreateFile(
      "overlapped.file", 
      GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
      FILE_FLAG_OVERLAPPED, // Zaznaczenie, iż wykonywane będą operacje
                            // asynchroniczne.
      NULL);

  if (h == INVALID_HANDLE_VALUE) {
    printf("error: failed to open file (%u)\n",
        (unsigned int)GetLastError());
    return 1;
  }

  OVERLAPPED ov;
  memset(&ov, 0, sizeof(ov));
  // Opcjonalnie można podać obiekt zdarzenia, które zostanie zasygnalizowane
  // gdy operacja się zakończy.

  if (!WriteFile(h, buffer, sizeof(buffer), NULL, &ov)) {
    if (GetLastError() == ERROR_IO_PENDING) {
      puts("Asynchronous write scheduled.");
    } else {
      printf("error: scheduling write failed (%u)\n",
          (unsigned int)GetLastError());
      CloseHandle(h);
      return 2;
    }
  }

  printf("Writting: Zz");
  for (;;) {
    DWORD bytes_written = 0;    
    BOOL ret = GetOverlappedResult(h, &ov, &bytes_written, FALSE);
    // Ew. możnaby skorzystać z makra HasOverlappedIoCompleted.
    if (ret) {
      break;
    }
    printf("z"); fflush(stdout);

    // Wykonaj jakąś inną, bardzo ciężką pracę. Ew. zaśnij, tylko udając, że
    // pracujesz.
    Sleep(10);
  }

  puts("\nDone!");
  CloseHandle(h);
  return 0;
}

