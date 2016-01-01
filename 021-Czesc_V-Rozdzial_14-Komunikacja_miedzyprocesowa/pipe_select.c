#define _GNU_SOURCE  // Program korzysta z funkcji specyficznych dla GNU/Linux.
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>

#define PIPE_READ_END 0
#define PIPE_WRITE_END 1
#define UNUSED(a) ((void)a)

typedef struct {
  int in;
  int out;
  int err;
} process_standard_io;

// Dla zwięzłości kodu całkowicie pomijam sprawdzanie błędów.
pid_t spawn_process(process_standard_io *pipes,
                    char *program, char **argv, char **envp) {
  // Stwórz potoki dla standardowego wejścia, wyjścia i wyjścia błędów.
  int in[2], out[2], err[2];
  pipe(in);
  pipe(out);
  pipe(err);

  // Uruchom nowy proces.
  pid_t child = fork();
  if (child == 0) {
    // Gałąź procesu potomnego.

    // Zamknij nieużywane końcówki potoków, tj.:
    // - końcówkę od zapisu w przypadku in
    // - końcówkę od odczytu w przypadku out i err
    close(in[PIPE_WRITE_END]);
    close(out[PIPE_READ_END]);
    close(err[PIPE_READ_END]);

    // Przestaw deskryptory odpowiednich końcówek in, out i err na deskryptory
    // używane przez stdin (0), stdout (1) i stderr (2).
    // Funkcja dup2 wykonuje kopię deskryptora. Po wykonaniu kopii stary
    // deskryptor jest redundantny, wiec można go zamknąć.
    dup2(in[PIPE_READ_END], 0);
    dup2(out[PIPE_WRITE_END], 1);
    dup2(err[PIPE_WRITE_END], 2);
    close(in[PIPE_READ_END]);
    close(out[PIPE_WRITE_END]);
    close(err[PIPE_WRITE_END]);

    // W tym momencie odpowiednie końcówki potoków są podpięte jako stdin,
    // stdout oraz stderr procesu potomnego. Można więc uruchomić program
    // docelowy, który je odziedziczy.
    execve(program, argv, envp);

    // Jeśli wykonanie dotrze do tego miejsca, coś poszło żle.
    perror("child execve failed");
    _exit(1);  // Natychmiastowe zakończenie procesu dziecka.
  }

  // Gałąź rodzica.

  // Zamknij nieużywane końcówki potoków.
  close(in[PIPE_READ_END]);
  close(out[PIPE_WRITE_END]);
  close(err[PIPE_WRITE_END]);

  // Przepisz pozostałe deskryptory potoków do struktury wyjściowej i wyjdź.
  pipes->in = in[PIPE_WRITE_END];
  pipes->out = out[PIPE_READ_END];
  pipes->err = err[PIPE_READ_END];
  return child;
}

int main(int argc, char **argv, char **envp) {
  UNUSED(argc);
  UNUSED(argv);

  process_standard_io pstdio;
  char *process_path = "/usr/bin/find";
  char *process_argv[] = {
    process_path,
    "/etc",
    "-name", "passwd",
    NULL
  };
  pid_t child = spawn_process(&pstdio, process_path, process_argv, envp);

  int child_exited = 0;  // Flaga: proces potomny zakończył działanie.
  int no_more_out = 0;   // Flaga: potok pstdio.out wyczyszczony i zamknięty.
  int no_more_err = 0;   // Flaga: potok pstdio.err wyczyszczony i zamknięty.
  while (!(child_exited && no_more_out && no_more_err)) {
    // Sprawdź, czy proces potomny zakończył działanie.
    if (!child_exited && waitpid(child, NULL, WNOHANG) == child) {
      puts("Child exited.");
      child_exited = 1;
    }

    // Sprawdź, czy są gotowe dane w jednym z potoków. W tym celu użyj funkcji
    // select, która monitoruje określone deskryptory w oczekiwaniu na dane, ew.
    // wraca wcześniej w przypadku ich braku (po 25 milisekundach w tym
    // przypadku).

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 25 * 1000;  // 25 milisekund.

    // Utwórz fd_set i oznacz informacje o których deskryptorach nas interesują.
    fd_set read_fds;  
    FD_ZERO(&read_fds);
    FD_SET(pstdio.out, &read_fds);
    FD_SET(pstdio.err, &read_fds);

    // Funkcja select przyjmuje w drugim argumencie maksymalną wartość
    // deskryptora plus jeden (sic!). Sprawdź który z naszych deskryptorów ma
    // większą wartość liczbową.
    int max_fds = pstdio.out > pstdio.err ? pstdio.out : pstdio.err;

    // Sprawdź dostępność danych.
    if (select(max_fds + 1, &read_fds, NULL, NULL, &timeout) == 0) {
      continue;
    }

    // Prawdopodobnie są dostępne dane (choć również jest możliwe, że jeden lub
    // oba potoki zostały zamknięte, gdyż proces potomny kończy działanie).
    // Odbierz dostępne dane, jeśli jakieś są. W obu wypadkach operacje
    // w tym momencie będą nieblokujące (z powodu obecności danych lub
    // zamkniętego z drugiej strony potoku).
    char buffer[1024];
    int ret;

    if (FD_ISSET(pstdio.out, &read_fds)) {
      ret = read(pstdio.out, buffer, sizeof(buffer));
      if (ret != 0) {
        puts("Child's STDOUT:");
        fwrite(buffer, 1, ret, stdout);
        putchar('\n');        
      } else {
        no_more_out = 1;        
      }
    }

    if (FD_ISSET(pstdio.err, &read_fds)) {
      ret = read(pstdio.err, buffer, sizeof(buffer));
      if (ret != 0) {
        puts("Child's STDERR:");
        fwrite(buffer, 1, ret, stdout);
        putchar('\n');
      } else {
        no_more_err = 1;
      }
    }
  }

  puts("End of data.");

  // Zamknij potoki.
  close(pstdio.in);
  close(pstdio.out);
  close(pstdio.err);
  return 0;
}


