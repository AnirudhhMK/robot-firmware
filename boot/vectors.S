#include "regs.h"
.cpu cortex-m0plus
.thumb

.section .vectors, "ax"
.word	0x20042000	//start of stack pointer
.word	_reset//reset
.word	0x100001c3	//isr_nmi
.word	_isr_hardfault	//isr_hardfault
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c7	//isr_svcall
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c9	//isr_pendsv
.word	0x100001cb	//isr_systick
.word	_TIMER_IRQ_0//interrupt_0
.word	0x100001cd	//interrupt_1
.word	0x100001cd	//interrupt_2
.word	0x100001cd	//interrupt_3
.word	0x100001cd	//interrupt_4
.word	0x100001cd	//interrupt_5
.word	0x100001cd	//interrupt_6
.word	_PIO0_IRQ_0//interrupt_7
.word	0x100001cd	//interrupt_8
.word	0x100001cd	//interrupt_9
.word	0x100001cd	//interrupt_10
.word	0x100001cd	//interrupt_11
.word	0x100001cd	//interrupt_12
.word	0x100001cd	//interrupt_13
.word	0x100001cd	//interrupt_14
.word	0x100001cd	//interrupt_15
.word	0x100001cd	//interrupt_16
.word	0x100001cd	//interrupt_17
.word	0x100001cd	//interrupt_18
.word	0x100001cd	//interrupt_19
.word	0x100001cd	//interrupt_20
.word	0x100001cd	//interrupt_21
.word	0x100001cd	//interrupt_22
.word	0x100001cd	//interrupt_23
.word	0x100001cd	//interrupt_24
.word	0x100001cd	//interrupt_25
.word	0x100001cd	//interrupt_26
.word	0x100001cd	//interrupt_27
.word	0x100001cd	//interrupt_28
.word	0x100001cd	//interrupt_29
.word	0x100001cd	//interrupt_30
.word	0x100001cd	//interrupt_31


.global _core1vectorTable
_core1vectorTable:
.word	0x20041000	//start of stack pointer
.word	_isr_hardfault//for now ####
.word	0x100001c3	//isr_nmi
.word	_isr_hardfault	//isr_hardfault
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c7	//isr_svcall
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c1	//isr_invalid; Reserved, should never fire
.word	0x100001c9	//isr_pendsv
.word	0x100001cb	//isr_systick
.word	_TIMER_IRQ_0//interrupt_0
.word	0x100001cd	//interrupt_1
.word	0x100001cd	//interrupt_2
.word	0x100001cd	//interrupt_3
.word	0x100001cd	//interrupt_4
.word	0x100001cd	//interrupt_5
.word	0x100001cd	//interrupt_6
.word	_PIO0_IRQ_0//interrupt_7
.word	0x100001cd	//interrupt_8
.word	0x100001cd	//interrupt_9
.word	0x100001cd	//interrupt_10
.word	0x100001cd	//interrupt_11
.word	0x100001cd	//interrupt_12
.word	0x100001cd	//interrupt_13
.word	0x100001cd	//interrupt_14
.word	0x100001cd	//interrupt_15
.word	0x100001cd	//interrupt_16
.word	0x100001cd	//interrupt_17
.word	0x100001cd	//interrupt_18
.word	0x100001cd	//interrupt_19
.word	0x100001cd	//interrupt_20
.word	0x100001cd	//interrupt_21
.word	0x100001cd	//interrupt_22
.word	0x100001cd	//interrupt_23
.word	0x100001cd	//interrupt_24
.word	0x100001cd	//interrupt_25
.word	0x100001cd	//interrupt_26
.word	0x100001cd	//interrupt_27
.word	0x100001cd	//interrupt_28
.word	0x100001cd	//interrupt_29
.word	0x100001cd	//interrupt_30
.word	0x100001cd	//interrupt_31

.weak _TIMER_IRQ_0
.set _TIMER_IRQ_0, __badisr__

.weak _PIO0_IRQ_0
.set _PIO0_IRQ_0, __badisr__

.thumb_func
__badisr__:
  b __badisr__

.thumb_func
.global _isr_hardfault
_isr_hardfault:
  ldr r0, =(rst_base + hw_clr)// atomic register for clearing reset controller (0x4000c000+0x3000) 
	mov r1, #32      	// load a 1 into bit 5
	str r1, [r0, #0] 	// store the bitmask into the atomic register to clear register

// check if reset is done
rst:     
  ldr r0, =rst_base	// base address for reset controller
	ldr r1, [r0, #8] 	// offset to get to the reset_done register
	mov r2, #32			// load 1 in bit 5 of register 2 (...0000000000100000)
	and r1, r1, r2		// isolate bit 5
	beq rst				// if bit five is 0 then check again, if not, reset is done

  ldr r0, =(io_bank0+0xcc)		// control register for GPIO25
	mov r1, #5			// Function 5, select SIO for GPIO25 2.19.2
	str r1, [r0]  		// Store function_5 in GPIO25 control register
//shifts over "1" the number of bits of GPIO pin	
	mov r1, #1			// load a 1 into register 1
	lsl r1, r1, #25 	// move the bit over to align with GPIO25
	ldr r0, =sio_base	// SIO base 
	str r1, [r0, #36]  	// 0x20 GPIO output enable


led_loop:
  ldr r2, =(1000*1000)
  str r1,[r0,#28]
  1:
  sub r2, #1
  bcs 1b
  b led_loop			// do the loop again


.thumb_func
_reset:
  bl crt0_init
  bl clock_init
  bl main
  1:
  b 1b
