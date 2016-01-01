// sudo su -
// XPID=`pidof print_and_wait`
// dd bs=1 skip=$[0x400634] count=24 if=/proc/${XPID}/mem 2>/dev/null; echo
#include <stdio.h>

int main(void) {
  printf("Address: %p\n", "This is a sample string.");

  // Najprostsza metoda wstrzymania wykonania.
  getchar();

  return 0;
}
