%include "vm.inc"

; Set up interrupt.
  vset r0, int_9
  vcrl 0x109, r0

; Turn on maskable interrupts and start PIT.
  vset r0, 1
  vcrl 0x110, r0
  voutb 0x22, r0

; Enter an infinite loop.
infloop:
  vjmp infloop

; Interrupt.
int_9:
  vset r0, 'C'
  vset r1, ':'
  vset r2, ' '
  voutb 0x20, r0  ; Write to STDOUT
  voutb 0x20, r1  ; Write to STDOUT
  voutb 0x20, r2  ; Write to STDOUT
  vset r1, 0

while_data:
  vinb 0x21, r0  ; Any data on STDIN?
  vcmp r0, r1
  vjz end_of_data  ; Nope, go away.

  vinb 0x20, r0  ; Read STDIN
  voutb 0x20, r0 ; Write to STDOUT
  vjmp while_data

end_of_data:

; Turn on interrupts again.
  vset r0, 1
  voutb 0x22, r0

  vcrl 0x110, r0
  viret

