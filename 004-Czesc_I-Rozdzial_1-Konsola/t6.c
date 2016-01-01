#include <stdio.h>
extern const char **__environ;
int main(int argc, char **argv) {
  for (int i = 0; __environ[i] != NULL; i++) {
    printf("%i: %s\n", i, __environ[i]);
  }
  return 0;
}

