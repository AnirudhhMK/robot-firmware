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
  
  .vectors : {
    KEEP (*(.vectors))
  }
  .text : {
    KEEP (*(.text*))
    KEEP (*(.text))
  }>FLASH
  . = ALIGN(4);
  
 .ramcode : {
    *(.ramcode)
    . = ALIGN(4);
  }>RAM AT>FLASH

  .rodata : {
    KEEP (*(.rodata))
  } > FLASH
  . = ALIGN(4);
  .data : {
    *(.data)
  }>RAM AT>FLASH

  . = ALIGN(4);
  .bss : {
    *(.bss)
  }>RAM

  . = ALIGN(4);
  .copy_table :{
    __copy_table_start__ = . ;
    LONG(LOADADDR(.data)) ;
    LONG(ADDR(.data)) ;
    LONG(SIZEOF(.data)/4) ;
    
    LONG(LOADADDR(.ramcode)) ;
    LONG(ADDR(.ramcode)) ;
    LONG(SIZEOF(.ramcode)/4) ;

    __copy_table_end__ = . ;
  }>FLASH
  
  . = ALIGN(4);
  .zero_table :{
    __zero_table_start__ = . ;
    LONG(ADDR(.bss))
    LONG(SIZEOF(.bss)/4)
    __zero_table_end__ = . ;
  } > FLASH

 

}
