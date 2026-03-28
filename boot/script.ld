MEMORY
{
    FLASH(rx) : ORIGIN = 0x10000000, LENGTH = 2048k
    RAM(rwx) : ORIGIN =  0x20000000, LENGTH = 256k
    SCRATCH_X(rwx) : ORIGIN = 0x20040000, LENGTH = 4k
    SCRATCH_Y(rwx) : ORIGIN = 0x20041000, LENGTH = 4k
}

SECTIONS
{
  .boot2 : { 
    __boot2_start__ = . ;
    KEEP (*(.boot2))
    __boot2_end__ = . ;
  }>FLASH

  .text : {
    KEEP (*(.vectors))
    KEEP (*(.text*))
    KEEP (*(.text))
  }>FLASH
  . = ALIGN(4);

  .ramcode : {
    PROVIDE (_sramcode = .);
    *(.data)
    *(.ramcode)
    . = ALIGN(4);
    PROVIDE (_eramcode = .);
  }>RAM AT>FLASH

  PROVIDE (_siramcode = LOADADDR(.ramcode));
  
  .rodata : {
    KEEP (*(.rodata))
  } > FLASH
}
