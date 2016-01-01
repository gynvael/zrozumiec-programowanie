#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define UNUSED(a) ((void)a)

int child_main(int s, unsigned int n, unsigned int mod) {
  // Wylicz N-ty wyraz ciągu fibonacciego modulo mod i odeślij odpowiedź. Co
  // jakiś czas sprawdzaj, czy rodzic nie przysłał bajtu FF, który oznacza
  // sygnał do zakończenia. Zakończ również, jeśli połączenie zostanie
  // przerwane.
  const int MSG_CHECK_INTERVAL = 10000;

  printf("child(%u, %u): ready; starting the math\n", n, mod);
  fflush(stdout);

  unsigned int check_counter = MSG_CHECK_INTERVAL;
  unsigned int result = 1; 
  unsigned int i;
  for (i = 1; i < n; i++) {
    // Wykonaj obliczenia.
    result = (result + i) % mod;

    // Ew. sprawdź, czy są nowe dane.
    if (--check_counter == 0) {
      check_counter = MSG_CHECK_INTERVAL;  // Reset.
      
      // Odbierz bajt danych jeśli jest dostępny.
      unsigned char data;
      ssize_t ret = recv(s, &data, 1, 0);
      if (ret == 0) {
        // Połączenie zostało zakończone, można zakończyć proces.
        printf("child(%u, %u): connection closed, exiting\n", n, mod);
        return 1;
      }

      if (ret == 1) {
        if (data == 0xff) {
          printf("child(%u, %u): received FF packet, exiting\n", n, mod);
          return 1;
        } else {
          printf("child(%u, %u): received %.2X packet, ignoring\n",
                 n, mod, data);
          fflush(stdout);
        }
      }

      // Sprawdź czy wystąpił błąd, z którym nie można nic zrobić. Błędy, które
      // są normalnymi zdarzeniami to:
      //   EAGAIN i EWOULDBLOCK - brak danych
      if (ret == -1 && !(errno == EAGAIN || errno == EWOULDBLOCK)) {
        // Wystąpił błąd związany z gniazdem.
        printf("child(%u, %u): connection failed, exiting\n", n, mod);
        return 1;
      }

      // Brak danych.
    }    
  }

  printf("child(%u, %u): done!\n", n, mod);
  fflush(stdout);

  // Sprawdź, czy połączenie jest nadal aktywne, jeśli tak, wyślij dane.
  unsigned char data;
  ssize_t ret = recv(s, &data, 1, 0);
  if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    // Połączenie jest nadal aktywne. Wyślij dane.
    // Funkcja send, szczególnie w trybie asynchronicznym (ale w trybie
    // blokującym również) może nie wysłać wszystkich danych od razu, natomiast
    // zwróci informacje o tym ile bajtów udało się wysłać.
    // W praktyce dla 4 bajtów nie powinno się to nigdy zdarzyć.
    ssize_t to_send = sizeof(result);
    unsigned char *presult = (unsigned char*)&result;
    while(to_send != 0) {
      ret = send(s, presult, to_send, 0);
      if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          // Wywołanie by blokowało (gniazdo jest w trybie asynchronicznym).
          continue;
        }

        // Połączenie zostało zerwane lub wystąpił inny błąd.
        return 1;
      }

      to_send -= ret;
      presult += ret;
    }

    return 0;
  }

  printf("child(%u, %u): result sent failed\n", n, mod);
  return 1;
}

int spawn_worker(unsigned int n, unsigned int mod) {
  // Stwórz asynchroniczne gniazdo strumieniujące, typu Unix Domain Socket.
  int sv[2];
  int ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sv);
  if (ret == -1) {
    return -1;
  }

  // Utworz proces potomny.
  pid_t child = fork();
  if (child == 0) {
    close(sv[0]);  // Zamknij zduplikowane gniazdo rodzica (proces potomny z
                   // niego nie korzysta).

    // Przejdź do kodu procesu potomnego.
    ret = child_main(sv[1], n, mod);
    close(sv[1]);
    fflush(stdout);  // W normalnej sytuacji odpowiednia procedura
                     // zarejestrowana przy pomocy atexit by wywołała fflush
                     // dla stdout, natomiast poniżej korzystamy z _exit, ktora
                     // pomija wywołanie wszelkich procedur zarejestrowanych
                     // przy pomocy atexit, wiec należy fflush wywołać
                     // manualnie.
    _exit(ret);  // W przypadku procesów „sklonowanych” przy pomocy fork,
                 // których dokładne działanie i stan w danym momencie nie jest
                 // znane, zaleca się pominięcie wywołania procedur
                 // zarejestrowanych przy pomocy atexit, ponieważ ich wywołoanie
                 // mogło by spowodować np. oczekiwanie na spinlock
                 // odziedziczony po rodzicu, który rodzic odblokował po fork we
                 // własnym kontekście, ale proces potomny (będący kopią rodzica
                 // z momentu wywołania fork) mógłby mieć ową blokadę nadal 
                 // zajętą, co doprowadziło by do zablokowania tego procesu.
  }
  // Kontynuacja procesu rodzica.

  // Zamknij gniazdo dziecka (proces potomny posiada jego kopię).
  close(sv[1]);

  return sv[0];  
}

int main(void) {
  // Stwórz 10 procesów potomnych. Poczekaj na wyniki od połowy z nich i zakończ
  // pracę.
  struct child_into_st {
    int s;
    unsigned char data[sizeof(unsigned int)];    
    ssize_t data_received;
  } workers[10];
  int results = 0;

  int i;
  for (i = 0; i < 10; i++) {
    workers[i].data_received = 0;    
    workers[i].s = -1;
  }

  // Stwórz procesy.
  for (i = 0; i < 10; i++) {
    int s = spawn_worker(1000000 * (1 + rand() % 100), 2 + rand() % 12345);
    // Celowo nie wywołuje srand (dla powtarzalności wyników).

    if (s == -1) {
      printf("main: failed to create child %i; aborting\n", i);
      fflush(stdout);
      goto err;
    }

    workers[i].s = s;
  }

  // Aktywnie czekaj aż pojawi się przynajmniej 5 wyników.
  while (results < 5) {
    for (i = 0; i < 10; i++) {
      if (workers[i].s == -1) {
        continue;
      }

      // Spróbuj odebrać dane.
      ssize_t ret = recv(workers[i].s, 
                         workers[i].data + workers[i].data_received,
                         sizeof(workers[i].data) - workers[i].data_received,
                         0);

      // Zinterpretuj wynik.
      int close_socket = 0;
      if (ret == 0) {
        printf("main: huh, child %i died\n", i);
        fflush(stdout);
        close_socket = 1;
      } else if (ret == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          printf("main: child %i connection error\n", i);
          fflush(stdout);
          close_socket = 1;
        }
        // W innym wypadku po prostu nie ma danych do odebrania.
      } else if (ret > 0) {
        // Udało się odebrać dane. Czy są już 4 bajty?
        workers[i].data_received += ret;
        if (workers[i].data_received == sizeof(workers[i].data)) {
          // Tak, został odebrany cały wynik.
          unsigned int res;
          memcpy(&res, workers[i].data, sizeof(unsigned int));
          results++;
          printf("main: got result from %i: %u (we have %i/5 results now)\n",
                 i, res, results);
          fflush(stdout);
          close_socket = 1;
        }
      }

      // Zamknij gniazdo jeśli to jest wymagane.
      if (close_socket) {
        close(workers[i].s);
        workers[i].s = -1;
      }
    }
  }

  printf("main: we have met the required number of results; finishing\n");
  fflush(stdout);

  // Poinformuj procesy o zakończeniu i wyjdź.
  for (i = 0; i < 10; i++) {
    if (workers[i].s != -1) {
      unsigned char byte = 0xff;
      send(workers[i].s, &byte, 1, 0);
      close(workers[i].s);
    }
  }
  return 0;

err:
  // Pozamykaj gniazda. Procesy potomne same wyjdą po wykryciu zamknięcia
  // gniazd.
  for (i = 0; i < 10; i++) {
    if (workers[i].s != -1) {
      close(workers[i].s);
    }
  }

  return 1;
}




