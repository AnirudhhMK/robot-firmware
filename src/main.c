#include "config.h"
#include "multicore.h"
// #include "nRF.h"
#include "regs.h"

#include <stdint.h>

void main_core1(void);
void spi0_init();

int main() {
  // pull IOBANK0 out of reset
  RESETS_CLR->RESET = 1 << 5;
  while (!(RESETS->RESET_DONE & (1 << 5)))
    ;

  IO_BANK0->GPIO[25].CTRL = 5;

  IO_BANK0->GPIO[2].CTRL = 1;
  IO_BANK0->GPIO[3].CTRL = 1;
  IO_BANK0->GPIO[4].CTRL = 1;
  IO_BANK0->GPIO[5].CTRL = 1;

  SIO->GPIO_OE_SET = (1 << 25);
  spi0_init();

  SIO->GPIO_OE_SET = (1 << CE);
  // radio_pwr_up();
  RESETS_CLR->RESET = (1 << 21);
  while (!(RESETS->RESET_DONE & (1 << 21)))
    ;
  launch_core1();

  for (;;) {
    while (!(SIO->FIFO_ST & 1))
      ;
    if (SIO->FIFO_RD == 1234) {
      SIO->GPIO_OUT_XOR = (1 << 25);
      // radio_standby1();
    }
  }
}
void spi0_init() {
  // pull out of reset
  RESETS_CLR->RESET = (1 << 16); // SPI0
  while (!(RESETS->RESET_DONE & (1 << 16)))
    ;
  SPI0->SSPCPSR = 50; // cpsdvsr
  SPI0->SSPCR0 =
      (7 | (0 << 6) | (1 << 8)); //  8-bit data size, Motorala mode 0,scr=1

  SPI0->SSPCR1 = (1 << 1); // master mode, enable ssp
}
