#include "multicore.h"
#include "regs.h"
#include "scheduler.h"
#include "uart_comms.h"
#include <stddef.h>
#include <stdint.h>

void send_imu_payload(imu_payload_t *imu_data) {
  packet_header_t header = (packet_header_t){
      0x55aa, PID_IMU_DATA, sizeof(imu_payload_t), TIMER->TIMERAWL};
  uart_tx_send(&header, (uint8_t *)imu_data);
}

void send_angle_estimate_payload(angle_estimate_payload_t *angle) {
  packet_header_t header =
      (packet_header_t){0x55aa, PID_ANGLE_ESTIMATE,
                        sizeof(angle_estimate_payload_t), TIMER->TIMERAWL};
  uart_tx_send(&header, (uint8_t *)angle);
}

void send_msg_payload(char *msg, uint16_t len) {
  packet_header_t header = (packet_header_t){
      0x55aa, PID_MSG, sizeof(uint8_t) * len / sizeof(char), TIMER->TIMERAWL};
  uart_tx_send(&header, (uint8_t *)msg);
}

void telemetry_debug(Task *t) {
  char msg[] = "Hello World!\r\n";
  send_msg_payload(msg, sizeof(msg));
  // command_packet_t pack = {0x55aa, CMD_DEBUG, 0};
  // uart_tx_send((uint8_t *)&pack, sizeof(command_packet_t));

  t->next_time = TIMER->TIMERAWL + 1000 * 1000;
  schedule_timed_task(t);
}

void telemetry_fast(Task *t) {
  send_imu_payload(&imu_payload_dbuf.buf[imu_payload_dbuf.writer ^ 1]);
  send_angle_estimate_payload(
      &angle_estimate_dbuf.buf[angle_estimate_dbuf.writer ^ 1]);
  t->next_time = TIMER->TIMERAWL + 1000 * 100;
  schedule_timed_task(t);
}

void *memcpy(void *dst, const void *src, size_t n) {
  uint8_t *d = (uint8_t *)dst;
  const uint8_t *s = (uint8_t *)src;

  while (n--)
    *d++ = *s++;

  return dst;
}
