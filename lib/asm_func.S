.macro FUNCTION name
.global \name
.type \name, %function
.func \name
.thumb_func
\name:
.endm

.macro ENDFUNC name
.ltorg
.size \name, . - \name
.endfunc
.endm

.macro check_bits bits
lsr r2, r0, #\bits
beq 1\@f
mov r0, r2
sub r1, #\bits
1\@:
.endm

FUNCTION _clz //r0 is the num
  mov r1, #32
  check_bits 16
  check_bits 8
  check_bits 4
  check_bits 2
  check_bits 1
  sub r0, r1,r0
  bx lr
ENDFUNC _clz
