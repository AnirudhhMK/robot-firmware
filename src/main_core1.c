#include "config.h"
#include "math.h"
#include "regs.h"
#include "scheduler.h"
#include <stdint.h>

#define QUEUE_SIZE 2

uint32_t y = 1;
void schedule_timed_task(Task *t);

void send_code(Task *t);
void send_code1(Task *t);
__attribute__((section(".ramcode"))) void computesqrt(Task *t);
Task tasklist[QUEUE_SIZE] = {};
Task code = {1, 1, 0, &send_code};
Task tsqrt = {1, 1, 0, &computesqrt};

__attribute__((section(".ramcode"))) void main_core1(void) {
  SYSTICK->RVR = (1 << 24) - 1;
  SYSTICK->CSR =
      (1 << 0) | (1 << 2); // enable systick and clck source is processor clock
  tasklist[0] = code;
  tasklist[1] = tsqrt;
  NVIC->ISER[0] = 1 << 0; // enable timer_irq_0 (alarm0)
  TIMER->INTE = (1 << 0);
  for (;;) {
    for (uint8_t i = 0; i < QUEUE_SIZE; i++) {
      if (tasklist[i].ready != 0) {
        tasklist[i].next(&tasklist[i]);
      }
    }
  }
}
__attribute__((section(".ramcode"))) void computesqrt(Task *t) {
  static q16_16_t num = 1;
  q16_16_t x = inv_sqrt(90);
  SYSTICK->CVR = 0;
  x = inv_sqrt(num);
  uint32_t tdiff = SYSTICK->CVR;
  tdiff = (1 << 24) - 1 - tdiff;

  SYSTICK->CVR = 0;
  uint32_t ndiff = SYSTICK->CVR;
  ndiff = (1 << 24) - 1 - ndiff;

  y = tdiff - ndiff;
  SPI0->SSPDR = num >> 16;
  SPI0->SSPDR = num >> 8;
  SPI0->SSPDR = num >> 0;
  x = ((x >> 8) * ((x >> 8) * (num >> 8)) >> 8);
  SPI0->SSPDR = x >> 24;
  SPI0->SSPDR = x >> 16;
  SPI0->SSPDR = x >> 8;
  SPI0->SSPDR = x >> 0;
  SPI0->SSPDR = tdiff - ndiff;
  num += 30;
  num |= (1 << 23) - 1;
  // t->next_time = TIMER->TIMERAWL + 1000 * 10000;
  t->ready = 0;
  // schedule_timed_task(t);
}
void send_code(Task *t) {
  if (y & 1) {
    SIO->FIFO_WR = 1234;
  }
  y = y >> 1;
  t->next_time = TIMER->TIMERAWL + 1000 * 1000;
  t->next = send_code;
  // t->next remains the same
  schedule_timed_task(t);
}
void send_code1(Task *t) {
  SIO->FIFO_WR = 1234;
  t->next_time = TIMER->TIMERAWL + 1000 * 1000 / 4;
  t->next = send_code;
  //  t->next remains the same
  schedule_timed_task(t);
}

void schedule_timed_task(Task *t) {
  uint32_t cur_time = TIMER->TIMERAWL;
  if ((uint32_t)(cur_time - t->next_time) <= 1) {
    t->ready = 1;
    return;
  }
  // insert task into whatever datastructure timer uses as an atomic operation
  // since we're just using the same tasklist here, no need for that

  if (((uint32_t)(t->next_time - cur_time) <
       (uint32_t)(TIMER->ALARM[0] - cur_time)) ||
      !(TIMER->ARMED & (1 << 0))) {

    TIMER->ALARM[0] = t->next_time;
  }
  t->ready = 0;
  return;
}

void __attribute__((interrupt, section(".ramcode"))) _TIMER_IRQ_0(void) {
  uint32_t cur_time = TIMER->TIMERAWL;
  TIMER->INTR = 1 << 0;
  uint32_t nexttime = TIMER->ALARM[0] - 1;
  for (uint32_t i = 0; i < QUEUE_SIZE; i++) {
    if ((uint32_t)(cur_time - tasklist[i].next_time) <= 1) {
      tasklist[i].ready = 1;
    } else if (tasklist[i].ready == 0) {

      if ((uint32_t)(tasklist[i].next_time - cur_time) <
          (uint32_t)(nexttime - cur_time)) {
        nexttime = tasklist[i].next_time;
      }
    }
  }
  TIMER->ALARM[0] = nexttime;
}
