#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define THREAD_COUNT 8
#define MAX_COUNTER (1 * 1000)

// Prawdopodobnie największa kolekcja zmiennych globalnych jaką stworzyłem
// od wielu wielu lat.
static volatile LONG g_spinlock;
static int g_counter[THREAD_COUNT];
static uint64_t g_max_wait[THREAD_COUNT];
static uint64_t g_min_wait[THREAD_COUNT];
static uint64_t g_total[THREAD_COUNT];
static uint32_t g_round_time[THREAD_COUNT][MAX_COUNTER];
static volatile int g_finish;
static volatile LONG g_poor_mans_lock;

inline void my_spinlock_lock(LONG volatile *lock) {
  while(InterlockedExchange(lock, 1) == 1);
}

inline void my_spinlock_unlock(LONG volatile *lock) {
  InterlockedExchange(lock, 0);
}

inline uint64_t rdtsc() {
  uint64_t timestamp;
  // Używam RDTSCP zamiast RDTSC, ponieważ ta pierwsza jest instrukcją
  // serializującą, tj. czeka aż wszystkie instrukcje faktycznie się wykonają -
  // RDTSC mógłby zostać wykonany poza kolejnością, co by mogło dać trochę
  // korzystniejsze wyniki.
  asm volatile ("rdtscp" 
      : "=A" (timestamp)  // EDX:EAX mają zostać zapisane do zmiennej timestamp
      :                   // Brak argumentów wejściowych.
      : "ecx");           // Zawartość rejestru ECX jest nadpisywana (jeśli
                          // kompilator uzna wartość w ECX za istotną, 
  return timestamp;
}

DWORD WINAPI BruteTest(LPVOID param) {
  uint64_t t_start, t_end, t_final;
  uint64_t t_total_start, t_total_end;
  int thread_id = (int)param;
  g_min_wait[thread_id] = ~0ULL;

  // Próba zsynchronizowania wątków, by wystartowały naraz (jak w poprzednim
  // przykładzie).
  InterlockedIncrement(&g_poor_mans_lock);
  while (g_poor_mans_lock != THREAD_COUNT);

  t_total_start = rdtsc();
  while (!g_finish) {
    // Zajmij spinlock (i zmierz czas operacji w cyklach procesora).
    t_start = rdtsc();
    my_spinlock_lock(&g_spinlock);
    t_end = rdtsc();

    // Zanotuj czas trwania operacji.
    t_final = t_end - t_start;
    g_round_time[thread_id][g_counter[thread_id]] = t_end - t_total_start;

    if (t_final > g_max_wait[thread_id]) {
      g_max_wait[thread_id] = t_final;
    }

    if (t_final < g_min_wait[thread_id]) {
      g_min_wait[thread_id] = t_final;
    }    

    if (++g_counter[thread_id] == MAX_COUNTER) {
      g_finish = 1;
    }

    my_spinlock_unlock(&g_spinlock);
  }
  t_total_end = rdtsc();

  // Zanotuj całkowity czas trwania pętli.
  g_total[thread_id] = t_total_end - t_total_start;
  return 0;
}

int main(void) {
  int i, j;
  int non_zero_threads = 0;
  HANDLE h[THREAD_COUNT];

  // Uruchomienie THREAD_COUNT wątków.
  g_poor_mans_lock = 0;
  for (i = 0; i < THREAD_COUNT; i++) {
    h[i] = CreateThread(NULL, 0, BruteTest, (LPVOID)i, 0, NULL);
  }
  WaitForMultipleObjects(THREAD_COUNT, h, TRUE, INFINITE);

  // Zamknij uchwyty i wypisz podstawowe informacje.
  for (i = 0; i < THREAD_COUNT; i++) {
    CloseHandle(h[i]);
    if (g_counter[i] > 0) {
      printf("Counter %2i: %10i [%10I64u -- %10I64u]  %I64u\n", 
          i, g_counter[i], g_min_wait[i], g_max_wait[i], g_total[i]);
      non_zero_threads++;      
    }
  }

  printf("Total: %i threads\n", non_zero_threads);

  // Zapisz 
  uint64_t tsc_sum[THREAD_COUNT] = { 0 };
  FILE *f = fopen("starv.txt", "w");
  for (i = 0; i < MAX_COUNTER; i++) {
    fprintf(f, "%i\t", i);
    for (j = 0; j < THREAD_COUNT; j++) {
      if (g_round_time[j][i] == 0) {
        g_round_time[j][i] = tsc_sum[j];
      }
      tsc_sum[j] = g_round_time[j][i];

      fprintf(f, "%u\t", g_round_time[j][i]);
    }
    fprintf(f, "\n");
  }
  fclose(f);

  return 0;
}

