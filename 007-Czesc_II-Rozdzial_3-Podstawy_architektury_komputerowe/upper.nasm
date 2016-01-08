%include "vm.inc"

  ; Read one char from stdin
  vinb 0x20, r0

  ; If it's not in range 'a'-'z', jump out.
  vset r1, 'a'
  vcmp r0, r1
  vjl not_lower
  vset r1, 'z'
  vcmp r0, r1
  vjg not_lower

  ; Otherwise convert it to uppercase.
  vset r1, 0x20
  vsub r0, r1

not_lower:

  ; Display it
  voutb 0x20, r0

  ; Done.
  voff

