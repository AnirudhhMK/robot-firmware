#include "config.h"
#include "regs.h"
#include "scheduler.h"
#include <stdint.h>
#define W_REGISTER(addr) ((0b001) << 5 | addr)
#define W_TX_PAYLOAD 0b10100000

void radio_standby1(Task *t);
void radio_pwr_up(Task *t) {
  SPI0->SSPDR = W_REGISTER(0x00);
  SPI0->SSPDR = (0 << 0) | (1 << 1); // ptx mode, pwr up

  SPI0->SSPDR = W_REGISTER(0x06);
  SPI0->SSPDR = (1 << 0) | (01 << 1) |
                (0 << 3); // set LNA, tx pwr = -12db, air data rate = 1Mbps

  t->next = radio_standby1;
  t->next_time = TIMER->TIMERAWL + (15 * 100); // 1.5ms = 1.5 * 1000 us
  schedule_timed_task(t);
}

void radio_standby1(Task *t) {
  SPI0->SSPDR = W_TX_PAYLOAD;
  SPI0->SSPDR = 0x41;
  SIO->GPIO_OUT_SET = (1 << CE);
}
