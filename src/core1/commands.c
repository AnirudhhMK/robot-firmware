#include "multicore.h"
#include "regs.h"
#include "scheduler.h"
#include "uart_comms.h"

void process_commands_task(Task *t) {
  command_packet_t pack;
  while (uart_rx_get(&pack) == 0) {
    switch (pack.command) {
    case CMD_DEBUG:
      // SIO->GPIO_OUT_XOR = (1 << 25);
      break;
    case CMD_SET_P:
      pid_gains.Kp = pack.value;
      break;
    case CMD_SET_D:
      pid_gains.Kd = pack.value;
      break;

    default:
      break;
    }
  }
  t->next_time = TIMER->TIMERAWL + 4 * 1000;
  schedule_timed_task(t);
}
