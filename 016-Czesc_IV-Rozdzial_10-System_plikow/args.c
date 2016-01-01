#include <stdio.h>
int _CRT_glob = 0;
int main(int argc, char **argv) {
  while(argc-->0) puts(*argv++);
  return 0;
}

