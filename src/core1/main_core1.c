#include "config.h"
#include "math.h"
#include "regs.h"
#include "scheduler.h"
#include "tasks.h"
#include "uart_comms.h"
#include <stdint.h>

#define QUEUE_SIZE 4

uint32_t y = 1;

void send_code(Task *t);
__attribute__((section(".ramcode"))) void computesqrt(Task *t);

Task tasklist[QUEUE_SIZE] = {{1, 1, 0, &send_code},
                             {1, 1, 0, &telemetry_debug},
                             {1, 1, 0, &telemetry_fast},
                             {1, 1, 0, &process_commands_task}};

__attribute__((section(".ramcode"))) void main_core1(void) {
  NVIC->ISER[0] = (1 << 0) | // enable timer_irq_0 (alarm0)
                  (1 << 11); // enable dma_irq_0
  init_uart_comms();
  SYSTICK->RVR = (1 << 24) - 1;
  SYSTICK->CSR =
      (1 << 0) | (1 << 2); // enable systick and clck source is processor clock
  TIMER->INTE = (1 << 0);
  for (;;) {
    for (uint8_t i = 0; i < QUEUE_SIZE; i++) {
      if (tasklist[i].ready != 0) {
        tasklist[i].next(&tasklist[i]);
      }
    }
  }
}

void send_code(Task *t) {
  t->next_time = TIMER->TIMERAWL + 1000 * 1000;
  t->next = send_code;
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

void __attribute__((interrupt, section(".ramcode"))) TIMER_IRQ_0(void) {
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

void __attribute__((interrupt)) DMA_IRQ_0(void) {
  uint32_t status = DMA->INTS0;
  if (status & (1 << 0)) {
    DMA->INTS0 = 1 << 0;
    uart_tx_DMA_handler();
  }
  if (status & (1 << 1)) {
    DMA->INTS0 = 1 << 1;
    uart_rx_DMA_handler();
  }
}
