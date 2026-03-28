.extern _sdata
.extern _edata
.extern _sidata //start of initialized data (flash data)

.extern _sramcode
.extern _eramcode
.extern _siramcode

.global crt0_init
.thumb_func
crt0_init:
  push {lr}
  //initialize ramcode
  ldr r1, =(_sramcode)
  ldr r0, =(_siramcode)
  ldr r2, =(_eramcode)
  bl init_loop
  pop {pc}

init_loop: //r0 init start, r1 ram start, r2 ram end
  1:
    cmp r1, r2
    bcs 1f
    ldr r3, [r0]
    str r3, [r1]
    add r1, #4
    add r0, #4
    b 1b
  1:
  bx lr
.ltorg
 
