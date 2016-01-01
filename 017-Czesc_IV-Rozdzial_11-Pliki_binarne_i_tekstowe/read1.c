#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(void) {
  FILE *f = fopen("example.bin", "rb");
  if (!f) {
    return 1;
  }

  // Ustal wielkość pliku (alternatywnie możnaby użyć np. fstat).
  fseek(f, 0, SEEK_END);  // Przesuń kursor na koniec pliku.
  size_t file_size = ftell(f);  // Odczytaj pozycję kursora.
  fseek(f, 0, SEEK_SET);  // Przesuń kursora na początek pliku.

  // Zaalokuj odpowiedniej wielkości bufor pamięci i wczytaj plik.
  uint8_t *buffer = malloc(file_size);
  fread(buffer, 1, file_size, f);
  fclose(f);

  // Sparsuj plik wypisując pozyskane informacje.
  // Uwaga: poniższy kod nie zawiera żadnej walidacji poprawności i dostępności
  // zadeklarowanej ilości danych względem faktycznej wielkości pliku.

  // Wczytaj ilość zestawów danych.
  uint8_t *p = buffer;
  uint16_t n = *(uint16_t*)p;
  p += sizeof(uint16_t);  // 2

  printf("Data set count: %u\n", n);

  // Wczytaj zestawy.
  for (size_t i = 0; i < n; i++) {
    uint8_t m = *p++;
    printf("Set %u (size: %u): ", i, m);

    for (size_t j = 0; j < m; j++) {
      // Odczyt niewyrównanych 32-bitowych wartości z pamięci - ten kod nie jest
      // przenośny, ale zadziała poprawnie na wybranej platformie (x86,
      // Windows 7 oraz Ubuntu).
      printf("%u ", *(uint32_t*)p);
      p += sizeof(uint32_t);  // 4
    }

    putchar('\n');
  }

  free(buffer);
  return 0;
}

