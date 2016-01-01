#if defined(__unix__)
#  include <unistd.h>
#  define SLEEP(x) usleep(x * 1000)
#elif defined(WIN32) || defined(_WIN32)
#  include <windows.h>
#  define SLEEP(x) Sleep(x)
#endif

int main() {
  while (true) { 
    SLEEP(10); 
  }
  return 0;
}

