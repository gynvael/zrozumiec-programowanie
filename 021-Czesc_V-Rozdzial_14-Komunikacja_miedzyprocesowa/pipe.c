#define _GNU_SOURCE  // Program korzysta z funkcji specyficznych dla GNU/Linux.
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
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

  // Poczekaj aż proces potomny zakończy działanie. Wszystkie dane wypisane na
  // standardowe wyjście i wejście nadal będą w buforach jądra przynależnych
  // do odpowiednich potoków.
  waitpid(child, NULL, 0);

  // Wypisz dane ze standardowych wyjść. Korzystam w tym celu m.in. z funkcji
  // splice, która kopiuje dane bezpośrednio między dwoma potokami, bez potrzeby
  // manualnego wczytania ich do pamięci procesu ze źródłowego potoku i zapisu
  // do docelowego potoku. Ponieważ splice korzysta z niskopoziomowych
  // deskryptorów (a nie obiektu FILE*), korzystam z funkcji fileno aby otrzymać
  // niskopoziomowy deskryptor standardowego wyjścia obecnego procesu.
  // Alternatywnie mógłbym użyć stałej 1, ponieważ standardowe wyjście w tym
  // procesie na pewno będzie na tym deskryptorze.
  puts("Child's STDOUT:"); fflush(stdout);
  splice(pstdio.out, NULL, fileno(stdout), NULL, 1024, 0);

  puts("\nChild's STDERR:"); fflush(stdout);  
  splice(pstdio.err, NULL, fileno(stdout), NULL, 1024, 0);

  puts("");  

  // Zamknij potoki.
  close(pstdio.in);
  close(pstdio.out);
  close(pstdio.err);
  
  return 0;
}


