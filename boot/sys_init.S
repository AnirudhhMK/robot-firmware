#include "regs.h"
.global clock_init
.thumb_func
clock_init:
//sets up xosc@12MHz, clk_ref@12MHz, PLL-sys@100MHz, watchdog tick@1MHz, transfers system clock to PLL-sys
  push {lr}
  bl xosc_set_up
  
//set up the clk_ref clock generator
  ldr r0, =clocks_base
  mov r1, #2
  str r1, [r0, #0x30]
  //poll selected register
  1:
  ldr r1, [r0, #0x38]
  sub r1, #4
  bne 1b
//done with clk_ref setup

//start PLL-sys setup
//pull PLL-sys out of reset
  ldr r0, =(rst_base + hw_clr)
  mov r1, #(1)
  lsl r1, r1, #12
  str r1, [r0]
  
  ldr r0, =rst_base
  1:
  ldr r2, [r0,#8]
  and r2, r2, r1
  beq 1b
//pll sys is out of reset, now we set fbdiv=100, postdiv1=6, postdiv2=2 for freq of 100MHz
  ldr r0, =(0x40028000)
  mov r1, #100 //fbdiv
  str r1, [r0, #0x08]

  mov r1, #(6<<4) + 2 //post div1 and post div 2 
  lsl r1, r1, #12
  str r1, [r0, #0x0c]
  //power on pll and vco then wait for vco to lock, then set postdiv power
  mov r1, #1<<3
  str r1, [r0, #0x4]
  1:
  ldr r2, [r0, #0]
  lsr r2, r2, #31
  beq 1b
  //vco is locked, now we can power on postdiv to finally get the output
  mov r1, #0
  str r1, [r0, #0x04]
//done with pll sys

//lets transfer clk_sys to the pll sys now
  ldr r0, =clocks_base
  mov r1, #1
  str r1, [r0,#0x3c]
  
  1:
  ldr r1, [r0, #0x44]
  sub r1, #0b10
  bne 1b
//done with clk_sys clock source switch, it should now run from pll sys
//enable clk_peri
  ldr r0, =(clocks_base+0x48)
  mov r1, #1
  lsl r1, #11
  str r1,[r0]

//start watchdog tick at 1us (clk_ref/12)
  ldr r0, =watchdog_base
  ldr r1, =(1<<9|12)
  str r1, [r0,#0x2c]
  mov r1, #1
  lsl r1, #10
  1:
  ldr r2, [r0,#0x2c]
  tst r1, r2
  beq 1b //wait for tick to run?
//done watchdog_tick
  pop {pc}

xosc_set_up:
  ldr r0, =xosc_base
  mov r1, #47
  str r1, [r0, #0x0c]
  
  ldr r1, =(0xfab<<12 + 0xaa0)
  str r1, [r0, #0x00]

  1:
    ldr r1, [r0,#0x04]
    lsr r1, #30 
  beq 1b

  bx lr


