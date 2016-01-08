%include "vm.inc"

; Reset alarm.
  vset r0, 0
  voutb 0x71, r0
  voutb 0x71, r0

; Put 1 sec in alarm clock.
  vset r0, 1000
  vmov r1, r0
  vset r2, 8
  vshr r1, r2
  vset r2, 0xff
  vand r0, r2
  vand r1, r2
  voutb 0x71, r1
  voutb 0x71, r0

; Set up interrupt.
  vset r0, int_8
  vcrl 0x108, r0

; Turn on maskable interrupts and start PIT.
  vset r0, 1
  vcrl 0x110, r0
  voutb 0x70, r0

; Enter an infinite loop.
infloop:
  vjmp infloop

; Interrupt.
int_8:
  vset r0, 'O'
  vset r1, 'K'
  vset r2, 0xa
  voutb 0x20, r0
  voutb 0x20, r1
  voutb 0x20, r2
  voff

