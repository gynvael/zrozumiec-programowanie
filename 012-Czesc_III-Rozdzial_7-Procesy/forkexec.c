#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define UNUSED(a) ((void)a)

int main(int argc, char **argv, char **envp) {
  UNUSED(argc);
  UNUSED(argv);

  // Stworze kopię niniejszego procesu. Z punktu widzenia kodu można rozróżnić
  // proces dziecko od procesu rodzica za pomocą zwróconego przez fork() PIDu.
  // Proces rodzic otrzyma PID dziecka, natomiast w przypadku procesu dziecka
  // PID będzie wyzerowany.
  pid_t pid = fork();
  if (pid == 0) {
    // Kod procesu dziecka.
    char *new_argv[] = {
      "/usr/bin/vim",
      NULL
    };
    execve(new_argv[0], new_argv, envp);  // Ta funkcja nigdy nie wraca.
  }

  // Kod procesu rodzica.
  printf("Process started (pid: %u). Waiting for it to exit.\n",
      (unsigned int)pid);
  waitpid(pid, NULL, 0);
  puts("The child process has exited.");
  return 0;
}

