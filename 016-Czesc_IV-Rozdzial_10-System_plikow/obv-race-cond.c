#include <stdio.h>
#include <unistd.h>

void copy_file(const char *fnameout, const char *fnamein) {
  FILE *fin = fopen(fnamein, "rb");
  FILE *fout = fopen(fnameout, "wb");
  unsigned char buf[1024];
  size_t ret;

  if (fin == NULL || fout == NULL) {
    goto error;
  }

  do {
    ret = fread(buf, 1, 1024, fin);
    fwrite(buf, 1, ret, fout);
  } while(ret != 0);

  fclose(fin);
  fclose(fout);
    
  return;

error:
  if (fin != NULL) {
    fclose(fin);
  }

  if (fout != NULL) {
    fclose(fout);
  }  
}

void puts_file(const char *fnameout, const char *string) {
  FILE *f = fopen(fnameout, "w");
  if (f == NULL) {
    return;
  }
  fputs(string, f);
  fclose(f);
}

int main(void) {
  puts("Welcome to Obvious Race Condition 2000!");

  puts("Please press ENTER to create /tmp/file1.");
  getchar();

  puts_file("/tmp/file1", "I am the master of this file!\n");
  puts("File /tmp/file1 created!\n");

  puts("Please press ENTER to duplicate file1 as file2.");
  getchar();

  copy_file("/tmp/file2", "/tmp/file1");
  puts("File /tmp/file2 created!\n");

  puts("Please press ENTER to remove both files.");
  getchar();

  unlink("/tmp/file1");
  unlink("/tmp/file2");
  puts("Done! Thank you for using this super safe app!");

  return 0;
}

