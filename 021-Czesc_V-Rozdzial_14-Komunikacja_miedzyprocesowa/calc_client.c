#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>

#include "calc.h"

int DoMath(unsigned int *calc_result, int op, size_t numdata, ...) {
  // Zaalokuj pamięć na pakiet z żadaniem.
  size_t creq_size = sizeof(calc_request) + numdata * sizeof(unsigned int);
  calc_request *creq = malloc(creq_size);
  memset(creq, 0, creq_size);
  creq->op = op;

  // Skopiuj argumenty operacji.
  va_list vl;
  va_start(vl, numdata);
  size_t i;
  for (i = 0; i < numdata; i++) {
    creq->data[i] = va_arg(vl, unsigned int);
  }

  // Wykonaj operację.
  calc_response cresp;
  BOOL result;
  DWORD cresp_recv_size = 0;
  result = CallNamedPipe(
    CALCSERVER_PIPE,
    creq, creq_size,
    &cresp, sizeof(cresp),
    &cresp_recv_size,
    NMPWAIT_USE_DEFAULT_WAIT);

  free(creq);

  // Przeanalizuj wynik.
  if (result == TRUE && cresp_recv_size == sizeof(cresp)) {
    if (cresp.res == RES_ERROR) {
      return -1;
    }

    *calc_result = cresp.value;
    return 0;
  }

  printf("protocol error (%u)\n", (unsigned int)GetLastError());
  return -1;
}

int main(void) {
  unsigned int result;

  // Przetestuj kalkulator dla sumowania.
  if (DoMath(&result, OP_SUM, 27, 11, 22, 33, 44, 55, 66, 77, 88, 99, 111, 222,
             333, 444, 555, 666, 777, 888, 999, 1111, 2222, 3333, 4444, 5555,
             6666, 7777, 8888, 9999) == 0) {
    printf("result: %u (should be 55485)\n", result);
  } else {
    puts("failed");
  } 

  // Przetestuj kalkulator dla mnożenia.  
  if (DoMath(&result, OP_MUL, 29, 70, 108, 97, 103, 123, 77, 89, 45, 80, 73, 80,
             69, 45, 73, 83, 45, 67, 65, 76, 76, 69, 68, 45, 72, 69, 78, 82, 89,
             125) == 0) {
    printf("result: %u (should be 1270874112)\n", result);
  } else {
    puts("failed");
  }

  // Przetestuj kalkulator dla dzielenia.
  if (DoMath(&result, OP_DIV, 21, 1048576, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
             2, 2, 2, 2, 2, 2, 2) == 0) {
    printf("result: %u (shoule be 1)\n", result);
  } else {
    puts("failed");
  }   

  return 0;
}


