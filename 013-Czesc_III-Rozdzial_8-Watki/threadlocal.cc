#include <stdio.h>

#include <thread>

thread_local int thread_var;  // C11, C++11
// Starsze wersje GCC, nieobsługujące w pełni C11 i C++11, posiadały inne
// słowo kluczowe oznaczające zmienne lokalne dla wątku, dostępne w ramach
// rozszerzenia języka:
//   __thread int thread_var;
// Analogicznie było w przypadku Microsoft Visual C++, który udostępniał
// następujący mechanizm oznaczenia zmiennych lokalnych wątku:
//   __declspec(thread) int thread_var;

void print_thread_var() {
  printf("[2] Initial thread_var value: %u\n", thread_var);
  thread_var = 8765;
  printf("[2] Changed thread_var value: %u\n", thread_var);
}

int main(void) {
  thread_var = 1234;
  printf("[1] Initial thread_var value: %u\n", thread_var);

  // Uruchomienie nowego wątku, tym razem korzystając z klasy std::thread z
  // C++11.
  std::thread second_thread(print_thread_var);
  second_thread.join();

  printf("[1] Final thread_var value: %u\n", thread_var); 
  return 0;
}

