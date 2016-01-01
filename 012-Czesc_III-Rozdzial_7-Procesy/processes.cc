#include<windows.h>
#include<stdio.h>
#include<tlhelp32.h>

int main() {
  // Zrob "zdjecie" obecnej listy procesow w systemie.
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "CreateToolhelp32Snapshot failed.\n");
    return -1;
  }

  puts("   PID  Thread Count   Executable\n"
       "----------------------------------");

  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  // Dla kazdego procesu na skopiowanej liscie, wypisz informacje o nim.
  BOOL result = Process32First(snapshot, &entry);
  while (result) {
    printf("%6u  %12u   %s\n",
        static_cast<unsigned int>(entry.th32ProcessID),
        static_cast<unsigned int>(entry.cntThreads),
        entry.szExeFile);
    result = Process32Next(snapshot, &entry);
  }  

  // Usun skopiowana liste procesow.
  CloseHandle(snapshot);
  return 0;
}

