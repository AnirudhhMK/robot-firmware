#include "multicore.h"
#include "regs.h"
#include <stdint.h>

#define __sev asm volatile("sev")

extern const uint32_t _core1vectorTable[];
extern void main_core1(void);
void __attribute__((weak)) main_core1(void) {
  for (;;)
    ;
}
void launch_core1() {
  static const uint32_t cmd_seq[] = {0,          0,
                                     1,          (uintptr_t)(_core1vectorTable),
                                     0x21030000, (uintptr_t)&main_core1};
  uint32_t seq = 0;
  do {
    const uint32_t cmd = cmd_seq[seq];
    if (!cmd) {
      while (SIO->FIFO_ST & 1) {
        uint32_t x = SIO->FIFO_RD;
        asm volatile("sev");
      }
    }
    SIO->FIFO_WR = (uintptr_t)cmd;
    __sev;
    while (!(SIO->FIFO_ST & 1))
      ;
    seq = SIO->FIFO_RD == (uintptr_t)cmd ? seq + 1 : 0;

  } while (seq < 6);
}

pwm_data_dbuf_t pwm_data_dbuf;
