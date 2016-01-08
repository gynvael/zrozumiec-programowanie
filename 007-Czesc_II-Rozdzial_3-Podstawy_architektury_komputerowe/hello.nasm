%include "vm.inc"

; Wypisz dane, znak po znaku.
vset r4, data
vxor r0, r0
vset r1, 1
print_loop:
  ; Pobierz bajt spod adresu z R0.
  vldb r2, r4

  ; Jesli to zero, wyjdź z pętli.
  vcmp r2, r0
  vjz .end

  ; W przeciwym wypadku, wypisz znak na konsoli.
  voutb 0x20, r2

  ; Przesuń r4 na kolejny znak i idź na początek pętli.
  vadd r4, r1
  vjmp print_loop

.end:

 ; Koniec.
voff

data:
  db "Hello World", 0xa, 0

