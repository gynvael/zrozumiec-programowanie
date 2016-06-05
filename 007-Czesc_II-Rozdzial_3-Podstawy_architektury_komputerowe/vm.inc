[org 0x0]

; Registers.
%define r0 0
%define r1 1
%define r2 2
%define r3 3
%define r4 4
%define r5 5
%define r6 6
%define r7 7
%define r8 8
%define r9 9
%define r10 10
%define r11 11
%define r12 12
%define r13 13
%define r14 14
%define r15 15
%define pc 15
%define sp 14

%macro vmov 2
db 0x00, %1, %2
%endmacro

%macro vset 2
db 0x01, %1
dd %2
%endmacro 

%macro vld 2
db 0x02, %1, %2
%endmacro

%macro vst 2
db 0x03, %1, %2
%endmacro

%macro vldb 2
db 0x04, %1, %2
%endmacro

%macro vstb 2
db 0x05, %1, %2
%endmacro


%macro vadd 2
db 0x10, %1, %2
%endmacro

%macro vsub 2
db 0x11, %1, %2
%endmacro

%macro vmul 2
db 0x12, %1, %2
%endmacro

%macro vdiv 2
db 0x13, %1, %2
%endmacro

%macro vmod 2
db 0x14, %1, %2
%endmacro

%macro vor 2
db 0x15, %1, %2
%endmacro

%macro vand 2
db 0x16, %1, %2
%endmacro

%macro vxor 2
db 0x17, %1, %2
%endmacro

%macro vnot 1
db 0x18, %1
%endmacro

%macro vshl 2
db 0x19, %1, %2
%endmacro

%macro vshr 2
db 0x1a, %1, %2
%endmacro

%macro vcmp 2
db 0x20, %1, %2
%endmacro

%macro vjz 1
db 0x21
dw (%1 - ($ + 2))
%endmacro
%define vje vjz

%macro vjnz 1
db 0x22
dw (%1 - ($ + 2))
%endmacro
%define vjne vjnz

%macro vjc 1
db 0x23
dw (%1 - ($ + 2))
%endmacro
%define vjl vjc  ; Deprecated, use vjb.
%define vjb vjc

%macro vjnc 1
db 0x24
dw (%1 - ($ + 2))
%endmacro
%define vjge vjnc  ; Deprecated, use vjae.
%define vjae vjnc

%macro vjbe 1
db 0x25
dw (%1 - ($ + 2))
%endmacro
%define vjle vjbe  ;  Deprecated, use vjbe.

%macro vja 1
db 0x26
dw (%1 - ($ + 2))
%endmacro
%define vjg vja  ; Deprecated, use vja.

%macro vpush 1
db 0x30, %1
%endmacro

%macro vpop 1
db 0x31, %1
%endmacro


%macro vjmp 1
db 0x40
dw (%1 - ($ + 2))
%endmacro

%macro vjmpr 1
db 0x41, %1
%endmacro

%macro vcall 1
db 0x42
dw (%1 - ($ + 2))
%endmacro

%macro vcallr 1
db 0x43, %1
%endmacro

%macro vret 0
db 0x44
%endmacro


%macro vcrl 2
db 0xf0, %2
dw %1
%endmacro

%macro vcrs 2
db 0xf1, %2
dw %1
%endmacro

%macro voutb 2
db 0xf2, %2, %1
%endmacro

%macro vinb 2
db 0xf3, %2, %1
%endmacro

%macro viret 0
db 0xf4
%endmacro

%macro vcrsh 0
db 0xfe
%endmacro

%macro voff 0
db 0xff
%endmacro

