#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
  int fd = open("/tmp/knownname", 
                O_CREAT | // Utwórz nowy plik, jeśli nie istnieje.
                O_EXCL | // Upewnij się, że na pewno zostanie utworzony nowy
                         // plik (tj. plik nie istniał wcześniej).
                O_WRONLY, // Otwarcie tylko do zapisu.
                0600 // S_IRUSR | S_IWUSR - czyli rw------- 
                );

  if (fd == -1) {
    perror("Failed to create file");
    return 1;
  }

  // W tym momencie tylko i wyłącznie obecny użytkownik posiada dostęp do nowego
  // pliku.

  // ...

  // Załóżmy, że po wykonaniu stosownych operacji na pliku chcemy dać 
  // użytkownikom z grupy www-data dostęp tylko do odczytu do pliku.

  struct group *www_data = getgrnam("www-data");
  if (www_data == NULL) {
    perror("Failed to get ID of group www-data");
    close(fd);
    return 2;
  }

  // Uwaga: aby móc zmienić grupę pliku, użytkownik musi być członkiem docelowej
  // grupy.
  if (fchown(fd, -1, www_data->gr_gid) == -1) {
    perror("Failed to change group");
    close(fd);
    return 3;
  }

  // Nowe uprawnienie: S_IRGRP, co daje rw-r-----.
  if (fchmod(fd, 0640) == -1) {
    perror("Failed to change permissions");
    close(fd);
    return 4;
  }
  
  close(fd);  
  return 0;
}

