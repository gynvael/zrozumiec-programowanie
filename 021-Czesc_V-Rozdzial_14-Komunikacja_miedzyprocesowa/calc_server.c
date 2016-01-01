#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "calc.h"

// Ponowna deklaracja dwóch funkcji, w razie gdyby MinGW Platform SDK w danej
// wersji ich nie posiadał.

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365440.aspx
BOOL WINAPI GetNamedPipeClientProcessId(
  HANDLE Pipe,
  PULONG ClientProcessId
);

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365442.aspx
BOOL WINAPI GetNamedPipeClientSessionId(
  HANDLE Pipe,
  PULONG ClientSessionId
);

HANDLE WaitForNewClient(void) {
  // Stwórz nowy potok.
  HANDLE h = CreateNamedPipe(
      CALCSERVER_PIPE,
      PIPE_ACCESS_DUPLEX,
      PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
      PIPE_UNLIMITED_INSTANCES,
      sizeof(calc_response),
      sizeof(calc_request) + sizeof(unsigned int) * 48,
      0,
      NULL);

  if (h == INVALID_HANDLE_VALUE) {
    printf("error: failed to create pipe (%u)\n", (unsigned int)GetLastError());
    return INVALID_HANDLE_VALUE;
  }

  // Poczekaj na połączenie z drugiej strony potoku.
  BOOL result = ConnectNamedPipe(h, NULL);
  if (result == TRUE) {
    return h;
  }

  // W powyższym przypadku ma miejsce sytuacja wyścigu, tj. klient może się
  // podłączyć do potoku pomiędzy wywołaniem CreateNamedPipe a ConnectNamedPipe.
  // W takim wypadku GetLastError zwróci ERROR_PIPE_CONNECTED.
  if (GetLastError() == ERROR_PIPE_CONNECTED) {
    return h;
  }

  printf("error: failed to connect pipe (%u)\n", (unsigned int)GetLastError());
  CloseHandle(h);
  return INVALID_HANDLE_VALUE;
}

calc_request *GetCalcRequest(HANDLE h, size_t *item_count) {
  // Zablokuj aż do otrzymania danych.
  DWORD bytes_read;
  BOOL result = ReadFile(h, NULL, 0, &bytes_read, NULL);
  if (!result) {
    printf("error: failed on initial read (%u)\n",
        (unsigned int)GetLastError());
    return NULL;
  }

  // Sprawdź wielkość danych.
  DWORD bytes_avail;
  result = PeekNamedPipe(h, NULL, 0, NULL, &bytes_avail, NULL);

  if (bytes_avail < sizeof(calc_request)) {
    printf("protocol error (packet too small)\n");
    return NULL;
  }

  if ((bytes_avail - sizeof(calc_request)) % sizeof(unsigned int) != 0) {
    printf("protocol error (data misaligned)\n");
    return NULL;
  }

  // Zaalokuj pamięć na strukturę i ją odbierz.
  calc_request *creq = malloc(bytes_avail);
  result = ReadFile(h, creq, bytes_avail, &bytes_read, NULL);

  switch (creq->op) {
    case OP_SUM: case OP_MUL: case OP_DIV: break;
    default:
      printf("protocol error (invalid operation)\n");
      free(creq);
      return NULL;                                        
  }

  *item_count = (bytes_avail - sizeof(calc_request)) / sizeof(unsigned int);
  return creq;
}

int Calc(unsigned int *result, calc_request *cr, size_t count) {
  size_t i;
  switch(cr->op) {  
    case OP_SUM:
      *result = 0;
      for (i = 0; i < count; i++) { *result += cr->data[i]; }
      break;

    case OP_MUL:
      *result = 1;
      for (i = 0; i < count; i++) { *result *= cr->data[i]; }
      break;   

    case OP_DIV:
      if (count == 0) {
        return -1;
      }
      *result = cr->data[0];
      for (i = 1; i < count; i++) { 
        if (cr->data[i] == 0) {
          return -1;
        }
        *result /= cr->data[i]; 
      }
      break;

    default:
      return -1;
  }

  return 0;
}

int SendCalcResponse(HANDLE h, calc_response *cresp) {
  DWORD bytes_sent;
  if (!WriteFile(h, cresp, sizeof(*cresp), &bytes_sent, NULL)) {
    printf("error: failed to send data over pipe (%u)\n",
        (unsigned int)GetLastError());
    return -1;
  }

  if (bytes_sent != sizeof(*cresp)) {
    printf("error: failed to send all of the data over pipe (%u, %u)\n",
        (unsigned int)bytes_sent, (unsigned int)GetLastError());    
    return -1;
  }

  return 0;
}

int main() {
  // Jednowątkowy serwer pseudo-kalkulatora, obsługujący jednego klienta naraz.
  for (;;) {
    // Stwórz potok i poczekaj na połączenie.
    HANDLE h = WaitForNewClient();
    if (h == INVALID_HANDLE_VALUE) {
      continue;
    }

    // Wypisz informacje o kliencie.
    ULONG pid, sid;
    GetNamedPipeClientProcessId(h, &pid);
    GetNamedPipeClientSessionId(h, &sid);
    printf("info: client connected! (pid=%u, sid=%u)\n", 
        (unsigned int)pid, (unsigned int)sid);

    // Odbierz żądanie.
    size_t item_count = 0;
    calc_request *creq = GetCalcRequest(h, &item_count);
    if (creq == NULL) {
      CloseHandle(h);
      continue;     
    }

    // Wylicz wynik.
    unsigned int value = 0;
    calc_response cresp;
    memset(&cresp, 0, sizeof(cresp));
    if (Calc(&value, creq, item_count) == 0) {
      cresp.res = RES_OK;
      cresp.value = value;
    } else {
      cresp.res = RES_ERROR;
    }
    printf("info: result for the client is %u\n", value);
    free(creq);

    // Odeślij wynik i rozlącz klienta.
    SendCalcResponse(h, &cresp);
    DisconnectNamedPipe(h);
    CloseHandle(h);
  }

  return 0;
}


