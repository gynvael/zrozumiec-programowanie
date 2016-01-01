#include <windows.h>
#include <stdio.h>

static const char kWindowClass[] = "MsgTestWindowClass";
static const char kUniqueMessageClass[] = "MsgTestMessageClass";
enum {
  kMsgTestHi,
  kMsgTestBye
};

static DWORD thread_main;

BOOL WINAPI CleanExitHandler(DWORD) {
  puts("info: received shutdown signal");

  // Wyślij wiadomość o zakańczaniu aplikacji do głównego wątku.
  // Zazwyczaj robi się to za pomocą funkcji PostQuitMessage, jednak funkcja
  // obsługująca sygnały CTRL+C oraz podobne działa w innym wątku, więc nie
  // można posłużyć się PostQuitMessage która wysyła wiadomość WM_QUIT tylko
  // do obecnego wątku.
  PostThreadMessage(thread_main, WM_QUIT, 0, 0);
  return TRUE;
}

int main()
{
  // Pobranie adresu obrazu pliku wykonywalnego aplikacji, który jest często
  // używany w "okienkowej" części WinAPI.
  HINSTANCE h_instance = static_cast<HINSTANCE>(GetModuleHandle(NULL));  

  // Rejestracja bardzo prostej klasy okna.
  WNDCLASSEX wc;
  memset(&wc, 0, sizeof(wc));
  wc.cbSize         = sizeof(WNDCLASSEX);
  wc.lpfnWndProc    = DefWindowProc;
  wc.hInstance      = h_instance;
  wc.lpszClassName  = kWindowClass;

  if(!RegisterClassEx(&wc))
  {
    printf("error: failed to register window class (%u)\n", 
        static_cast<unsigned int>(GetLastError()));
    return 1;
  }

  // Stworzenie okna w systemie. Samo okno nie zostanie wyświetlone (w tym celu
  // należałoby wywołać funkcje ShowWindow i UpdateWindow), ale nie przeszkadza
  // to w odbieraniu wiadomości rozgłoszeniowych.
  HWND hwnd = CreateWindowEx(
      0, kWindowClass, NULL, 0, 0, 0, 0, 0, NULL, NULL, h_instance, NULL);
  if(!hwnd)
  {
    printf("error: failed to create a window (%u)\n",
        static_cast<unsigned int>(GetLastError()));
    return 1;
  }

  // Rejestracja unikalnego numeru rodzaju wiadomości w systemie (wszystkie inne
  // procesy, które zarejestrują tą samą nazwę wiadomości, otrzymają dokładnie
  // ten sam identyfikator).
  UINT msgtest_msg_class = RegisterWindowMessage(kUniqueMessageClass);
  printf("info: unique message ID is %u\n", msgtest_msg_class);

  // Wyślij wiadomość powitalną do innych procesów.
  printf("info: my process ID is %u\n", 
      static_cast<unsigned int>(GetCurrentProcessId()));

  PostMessage(HWND_BROADCAST, msgtest_msg_class,
     static_cast<WPARAM>(GetCurrentProcessId()), 
     static_cast<LPARAM>(kMsgTestHi));

  // Zarejestruj sygnału CTRL+C, który da znać pętli z wiadomościami, iż należy
  // zakończyć proces w kontrolowany sposób.
  thread_main = GetCurrentThreadId();
  SetConsoleCtrlHandler(CleanExitHandler, TRUE);

  // Odbieraj wiadomości aż do otrzymania informacji o zakańczaniu procesu.
  // Technicznie GetMessage zwróci FALSE po otrzymaniu wiadomości WM_QUIT.
  MSG msg;
  while(GetMessage(&msg, NULL, 0, 0))
  {
    if (msg.message == msgtest_msg_class) {
      int m = static_cast<int>(msg.lParam);
      printf("info: process %u says '%s'\n", 
          static_cast<unsigned int>(msg.wParam),
          m == kMsgTestHi ? "Hi!" :
          m == kMsgTestBye ? "Bye!" : "???");
    } else {
      DispatchMessage(&msg);
    }
  }

  // Koniec.
  PostMessage(HWND_BROADCAST, msgtest_msg_class,
     static_cast<WPARAM>(GetCurrentProcessId()), 
     static_cast<LPARAM>(kMsgTestBye));
  DestroyWindow(hwnd);
  UnregisterClass(kWindowClass, h_instance);
  puts("info: bye!");
  return 0;
}

