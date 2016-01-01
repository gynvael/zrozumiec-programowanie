#include<windows.h>
#include<stdio.h>
#include<string.h>
#include<tlhelp32.h>
#include<ntdef.h>

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms684280.aspx
extern "C" LONG WINAPI NtQueryInformationProcess(
  HANDLE ProcessHandle,
  DWORD ProcessInformationClass,
  PVOID ProcessInformation,
  ULONG ProcessInformationLength,
  PULONG ReturnLength
);

// Źródło:
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms684280.aspx
typedef struct _PROCESS_BASIC_INFORMATION {
  PVOID Reserved1;
  PVOID PebBaseAddress;
  PVOID Reserved2[2];
  ULONG_PTR UniqueProcessId;
  PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa813741.aspx
typedef struct _RTL_USER_PROCESS_PARAMETERS {
  BYTE           Reserved1[16];
  PVOID          Reserved2[10];
  UNICODE_STRING ImagePathName;
  UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa813706.aspx
typedef struct _PEB {
  BYTE Reserved1[2];
  BYTE BeingDebugged;
  BYTE Reserved2[1];
  PVOID Reserved3[2];
  PVOID Ldr;
  PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
  BYTE Reserved4[104];
  PVOID Reserved5[52];
  PVOID PostProcessInitRoutine;
  BYTE Reserved6[128];
  PVOID Reserved7[1];
  ULONG SessionId;
} PEB, *PPEB;

void fetch_cmdline(DWORD pid, char *buffer, size_t max_length) {
  // Jeśli bufor jest zbyt mały, nie trzeba nic robić.
  if (buffer == NULL || max_length == 0) {
    return;
  }
  buffer[0] = '\0';

  // Otwórz zdalny proces tak, aby można pobrać o nim informacje oraz czytać
  // jego pamięć.
  HANDLE h = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
                         FALSE, pid);
  if (h == NULL) {
    return;
  }

  // Pobierz podstawowe informacje o procesie.
  PROCESS_BASIC_INFORMATION pqi;
  LONG status = NtQueryInformationProcess(h, 0 /* ProcessBasicInformation */,
                                          &pqi, sizeof(pqi), NULL);
  if (status < 0) {
    CloseHandle(h);
    return;
  }

  // PEB jest równy NULL oznacza, iż prawdopodobnie mamy do czynienia z
  // 64-bitowym procesem, którego PEB znajduje się poza zasięgiem 32-bitowej
  // aplikacji (co również oznacza, iż nasza aplikacja jest 32-bitowa).
  // W takim wypadku ReadProcessMemory i tak nie byłby w stanie odczytać
  // odpowiedniego fragmentu pamięci, gdyż jako adres zdalny przyjmuje 32-bitową
  // liczbę (a najwyraźniej adres PEB wykracza 32-bitowy zakres). Ten problem
  // można obejść np. korzystając z biblioteki wow64ext autorstwa ReWolfa:
  //
  //   https://github.com/rwfpl/rewolf-wow64ext
  //
  // Niemniej jednak temat przeskakiwania pomiędzy trybami procesora wykracza
  // poza tematykę niniejszej książki, więc ograniczę się tutaj do zwrócenia
  // pustej linii poleceń.
  if (pqi.PebBaseAddress == NULL) {
    CloseHandle(h);
    return;
  }

  // Pobierz strukturę Process Environment Block ze zdalnego procesu.
  PEB peb;
  SIZE_T read_bytes;
  BOOL result = ReadProcessMemory(h, pqi.PebBaseAddress,
                                  &peb, sizeof(peb), &read_bytes);
  if (!result || read_bytes != sizeof(peb)) {
    CloseHandle(h);
    return;
  }

  // Pobierz RTL_USER_PROCESS_PARAMETERS zdalnego procesu.
  RTL_USER_PROCESS_PARAMETERS upp;
  result = ReadProcessMemory(h, peb.ProcessParameters,
                             &upp, sizeof(upp), &read_bytes);
  if (!result || read_bytes != sizeof(upp)) {
    CloseHandle(h);
    return;
  }

  // Zaalokuj odpowiedni bufor na pobranie linii poleceń.
  size_t length = upp.CommandLine.Length + 2;
  WCHAR *command_line_utf16 = new WCHAR[length];
  memset(command_line_utf16, 0, length);

  // Pobierz linię poleceń do bufora.
  result = ReadProcessMemory(h, upp.CommandLine.Buffer,
                             command_line_utf16, length - 2, &read_bytes);
  if (!result || read_bytes != length - 2) {
    CloseHandle(h);
    return;
  }

  // Skonwertuj UTF-16 na rozszerzone ASCII którego używamy.
  memset(buffer, 0, max_length);
  WideCharToMultiByte(CP_ACP, 0, command_line_utf16, -1, 
                      buffer, max_length - 1, NULL, NULL);

  CloseHandle(h);
}

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
    char cmdline[4096];
    fetch_cmdline(entry.th32ProcessID, cmdline, sizeof(cmdline));

    printf("%6u  %12u   %-20s  %s\n",
        static_cast<unsigned int>(entry.th32ProcessID),
        static_cast<unsigned int>(entry.cntThreads),
        entry.szExeFile, cmdline);
    result = Process32Next(snapshot, &entry);
  }  

  // Usun skopiowana liste procesow.
  CloseHandle(snapshot);
  return 0;
}

