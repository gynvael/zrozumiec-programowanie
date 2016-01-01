#include <stdio.h>
#include "md5.h"

int calc_file_md5(const char *fname, 
                  unsigned char result[16]) {
  FILE *f;
  MD5_CTX md5;
  char buffer[1024];
  size_t ret;

  f = fopen(fname, "rb");
  if (!f) {
    return -1;
  }

  MD5_Init(&md5);

  while(1) {
    ret = fread(buffer, 1, sizeof(buffer), f);
    if (ret == 0) {
      break;
    }

    MD5_Update(&md5, buffer, ret);
  }

  fclose(f);
  MD5_Final(result, &md5);
  return 0;
}

int main(void) {
  unsigned char res[16];
  int i;

  calc_file_md5("fib.cpp", res);
  for(i = 0; i < 16; i++) {
    printf("%.2x", res[i]);
  }

  putchar('\n');  
  return 0;
}

