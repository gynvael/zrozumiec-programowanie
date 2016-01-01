#pragma once

#define CALCSERVER_PIPE "\\\\.\\pipe\\calcserver"

// Ponieważ dane będą przesyłane w obrębie jednej architektury, dopuszczalne
// jest posłużenie się bezpośrednio strukturami.
// Uwaga: najlepiej jest wyzerować instancje struktur przed ich wysłaniem, w
// celu upewnienia się, że żadne niewyczyszczone dane nie zaplątały się pomiedzy
// pola (tj. w dopełnienia między polami). W tym wypadku pola będą
// prawdopodobnie ściśle dopasowane, niemniej jednak w przyszłości struktura
// potencjalnie może ulec zmianie
typedef struct {
  enum {
    OP_SUM, // Suma liczb.
    OP_MUL, // Iloczyn liczb.
    OP_DIV  // Pierwsza liczba podzielona przez pozostałe.
  } op;
  unsigned int data[]; // Wielkość tablicy zależy od dynamicznej alokacji.
} calc_request;

typedef struct {
  enum {
    RES_ERROR,
    RES_OK
  } res;
  unsigned int value;
} calc_response;

