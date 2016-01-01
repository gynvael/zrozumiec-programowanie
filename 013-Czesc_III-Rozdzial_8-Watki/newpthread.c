#include <stdio.h>
#include <pthread.h>

void *run_me_in_a_new_thread(void *data) {
  printf("I was run in a new thread with %p as data.\n", data);
  return (void*)0xC0DE;
}


int main(void) {
  // Stworzenie nowego wątku.
  pthread_t th;
  int result = pthread_create(
      &th, NULL, run_me_in_a_new_thread, (void*)0x12345678);

  if (result != 0) {
    fprintf(stderr, "error: creating second thread failed\n");
    return 1;
  }

  // Oczekiwanie na zakończenie nowego wątku.
  void *retval = NULL;
  pthread_join(th, &retval);
  printf("Second thread returned: %p\n", retval);

  return 0;
}


