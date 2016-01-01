#include <stdio.h>
#include <windows.h>

DWORD WINAPI RunMeInANewThread(LPVOID data) {
  printf("I was run in a new thread with %p as data.\n", data);
  return 0xC0DE;
}

int main(void) {
  // Stworzenie nowego wątku.
  HANDLE h = CreateThread(
      NULL, 0, 
      RunMeInANewThread, (LPVOID)0x12345678,
      0, NULL);

  if (h == NULL) {
    fprintf(stderr, "Creating the second thread failed.\n");
    return 1;
  }

  // Oczekiwanie na zakończenie nowego wątku.
  DWORD retval;
  WaitForSingleObject(h, INFINITE);
  GetExitCodeThread(h, &retval);
  printf("Second thread returned: %x\n", (unsigned int)retval);
  CloseHandle(h);
  return 0;
}



