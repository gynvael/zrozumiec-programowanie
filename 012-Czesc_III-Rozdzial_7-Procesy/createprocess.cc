#include <windows.h>
#include <stdio.h>

int main() {
  // Uruchom proces.
  PROCESS_INFORMATION process_info;
  STARTUPINFO startup_info = { sizeof(STARTUPINFO) };

  BOOL result = CreateProcess(
      "C:\\Windows\\System32\\notepad.exe",
      NULL, NULL, NULL, FALSE, 0, NULL, NULL, 
      &startup_info, &process_info);

  if (!result) {
    fprintf(stderr, "CreateProcess failed.\n");
    return 1;
  }

  // Przekazujac funkcji CreateProcess instancje struktury PROCESS_INFORMATION
  // do wypelnienia, naszemu procesowi przypisywane sa dwa uchwyty (do procesu,
  // oraz do pierwszego watku). O ile bedziemy korzystac jeszcze z uchwytu do
  // procesu, to uchwyt do watku mozna od razu zamknac.
  CloseHandle(process_info.hThread);

  printf("New process started (pid: %u).\n",
      static_cast<unsigned int>(process_info.dwProcessId));

  // Poczekaj, az proces zakonczy dzialanie. Uchwyt procesu, ktory posiadamy
  // dzieki strukturze PROCESS_INFORMATION, umozliwia rowniez synchronizacje
  // z procesem potomnym (tj. uzyta ponizej funkcja oczekujaca na "sygnal"
  // wroci dopiero, gdy proces potomny zakonczy dzialanie).
  WaitForSingleObject(process_info.hProcess, INFINITE);
  CloseHandle(process_info.hProcess);

  puts("The child process has exited.");
  return 0;
}

