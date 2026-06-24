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

FUNCTION __aeabi_lmu
	push	{r4, lr}
	mul	r1, r2
	mul	r3, r0
	add	r1, r3
	
	lsr	r3, r0, #16
	lsr	r4, r2, #16
	mul	r3, r4
	add	r1, r3

	lsr	r3, r0, #16
	uxth	r0, r0
	uxth	r2, r2
	mul	r3, r2
	mul	r4, r0
	mul	r0, r2
	
	mov	r2, #0
	add	r3, r4
	adc	r2, r2
	lsl	r2, #16
	add	r1, r2
	
	lsl	r2, r3, #16
	lsr	r3, #16
	add	r0, r2
	adc	r1, r3
	pop	{r4, pc}

ENDFUNC __aeabi_lmu
