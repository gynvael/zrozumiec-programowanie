#include <stdio.h>
#include <windows.h>

#define UNUSED(a) (void)a
HANDLE ev_thread_ready;

DWORD WINAPI InfiniteLoop(LPVOID unused) {
  UNUSED(unused);
  puts("[2] Second thread ready! Entering infinite loop."); 
  fflush(stdout);

  // Dajmy znać pierwszemu wątkowi, że ten wątek wystartował.
  SetEvent(ev_thread_ready);

  // Nieskończona pętla.
#ifdef __GNUC__
  __asm(
      ".intel_syntax noprefix\n"
      "mov eax, 0x12345678\n"
      "1:\n"
      "  cmp eax, 0x12345678\n"
      "  je 1b\n"
      ".att_syntax\n");
#endif
  // Powyższy kod odpowiada następującemu pseudo-kodowi:
  //   eax = 0x12345678;
  //   while(eax == 0x12345678);

  // Wersja dla Microsoft Visual C++
#ifdef _MSC_VER
  __asm {
    mov eax, 0x12345678
    infloop:
      cmp eax, 0x12345678
      je infloop
  }
#endif

  puts("[2] The infinite loop has ended!");
  fflush(stdout);  
  return 0;
}

int main(void) {
  ev_thread_ready = CreateEvent(NULL, FALSE, FALSE, NULL);

  puts("[1] Creating a new thread.");  
  fflush(stdout);  
  HANDLE h = CreateThread(NULL, 0, InfiniteLoop, NULL, 0, NULL);

  // Poczekajmy aż drugi wątek jest gotowy (da nam znać za pomocą zdarzenia
  // ev_thread_ready);
  WaitForSingleObject(ev_thread_ready, INFINITE);
  CloseHandle(ev_thread_ready);

  puts("[1] Second thread said it's ready. Suspending it...");
  fflush(stdout);
  bool retry = true;
  do {
    SuspendThread(h);

    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_FULL;  // Pobierz cały kontekst.

    // Wątek jeszcze nie jest wstrzymany, więc funkcja GetThreadContext może
    // za pierwszym razem się niepowieść.
    while (!GetThreadContext(h, &ctx)) {
      Sleep(0);
    }

    // Jeśli w rejestrze EAX nie ma wartości 0x12345678, wstrzymaliśmy wątek
    // zbyt wcześnie. W przeciwnym wypadku można przystąpić do zmiany
    // wartości EAX.
    if (ctx.Eax == 0x12345678) {
      ctx.Eax = 0xDEADC0D3;  // Dowolna inna wartość.
      SetThreadContext(h, &ctx);
      retry = false;
      puts("[1] Changed EAX to 0xDEADC0D3!");
      fflush(stdout);      
    } else {
      puts("[1] Suspended thread too early. Retrying.");
      fflush(stdout);      
    }

    ResumeThread(h);
  } while(retry);

  // Poczekajmy aż drugi wątek skończy działanie.
  WaitForSingleObject(h, INFINITE);
  CloseHandle(h);

  puts("[1] The end.");
  fflush(stdout);      
  return 0;
}


