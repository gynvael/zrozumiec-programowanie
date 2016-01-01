#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(void) {
  FILE *f = fopen("example.bin", "rb");
  if (!f) {
    return 1;
  }

  // Sparsuj plik wypisując pozyskane informacje.
  // Uwaga: poniższy kod nie zawiera żadnej walidacji poprawności i dostępności
  // zadeklarowanej ilości danych względem faktycznej wielkości pliku.

  // Wczytaj ilość zestawów danych.
  uint16_t n;
  fread(&n, sizeof(n), 1, f);
  printf("Data set count: %u\n", n);

  // Wczytaj zestawy.
  for (size_t i = 0; i < n; i++) {
    uint8_t m;
    fread(&m, sizeof(m), 1, f);

    printf("Set %u (size: %u): ", (unsigned int)i, m);

    uint32_t *data = malloc(m * sizeof(uint32_t));
    fread(data, sizeof(uint32_t), m, f);

    for (size_t j = 0; j < m; j++) {
      printf("%u ", data[j]);
    }
    putchar('\n');
    free(data);    
  }

  fclose(f);
  return 0;
}

