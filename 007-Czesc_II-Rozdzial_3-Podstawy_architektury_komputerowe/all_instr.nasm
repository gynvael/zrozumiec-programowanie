%include "vm.inc"

  ; First group.
  vmov r0, r1
  vset r0, 1234
  vld r1, r0
  vst r0, r1
  vldb r1, r0
  vstb r0, r1

  ; Second group.
  vadd r1, r1
  vsub r1, r1
  vmul r1, r1
  vdiv r1, r0  ; Counting it's still 1234
  vmod r1, r0
  vor r1, r1
  vand r1, r1
  vxor r1, r1
  vnot r1
  vshl r1, r1
  vshr r1, r1

  ; Third group.
  vcmp r0, r0
  vjz n0
n0:
  vje n1
n1:
  vjnz n2
n2:
  vjne n3
n3:
  vjc n4
n4:
  vjl n5
n5:
  vjnc n6
n6:
  vjge n7
n7:
  vjle n8
n8:
  vjg n9
n9:

  ; Forth group.
  vpush r0
  vpop r0

  ; Fifth group.
  vjmp n10
n10:
  vset r0, n11
  vjmpr n11
n11:
  vcall n12
n12:
  vpop r0
  vset r0, n13
  vpush r0
  vret
n13:
  vset r0, n14
  vcallr r0
n14:
  vpop r0
  
  ; Last group.
  vcrl 0x100, r0
  vcrs 0x100, r0
  voutb 0x71, r0
  vinb 0x71, r0  

  ; Done.
  voff

