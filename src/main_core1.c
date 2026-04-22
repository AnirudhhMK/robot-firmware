#include "config.h"
#include "regs.h"
#include "scheduler.h"
#include <stdint.h>

void schedule_timed_task(Task *t);

void send_code(Task *t);
void send_code1(Task *t);
Task tasklist[1] = {};
Task code = {1, 1, 0, &send_code};

__attribute__((section(".ramcode"))) void main_core1(void) {
  tasklist[0] = code;
  NVIC->ISER[0] = 1 << 0; // enable timer_irq_0 (alarm0)
  TIMER->INTE = (1 << 0);
  for (;;) {
    for (uint8_t i = 0; i < sizeof(tasklist) / sizeof(Task); i++) {
      if (tasklist[i].ready != 0) {
        tasklist[i].next(&tasklist[i]);
      }
    }
  }
}

void send_code(Task *t) {
  SIO->FIFO_WR = 1234;
  t->next_time = TIMER->TIMERAWL + 1000 * 1000;
  t->next = send_code1;
  // t->next remains the same
  schedule_timed_task(t);
}
void send_code1(Task *t) {
  SIO->FIFO_WR = 1234;
  t->next_time = TIMER->TIMERAWL + 1000 * 1000 / 4;
  t->next = send_code;
  // t->next remains the same
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
    t->ready = 0;
  }
  return;
}

void __attribute__((interrupt, section(".ramcode"))) _TIMER_IRQ_0(void) {
  uint32_t cur_time = TIMER->TIMERAWL;
  TIMER->INTR = 1 << 0;
  uint32_t nexttime = TIMER->ALARM[0] - 1;
  for (uint32_t i = 0; i < sizeof(tasklist) / sizeof(Task); i++) {
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
