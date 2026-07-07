#include "multicore.h"
#include "regs.h"
#include "scheduler.h"
#include "uart_comms.h"
#include <stddef.h>
#include <stdint.h>

void send_imu_payload(imu_payload_t *imu_data) {
  struct __attribute__((packed)) {
    packet_header_t header;
    imu_payload_t payload;
  } packet;
  packet.header = (packet_header_t){0x55aa, PID_IMU_DATA, sizeof(imu_payload_t),
                                    TIMER->TIMERAWL};
  packet.payload = *imu_data;
  uart_tx_send((uint8_t *)&packet, sizeof(packet));
}
void telemetry_debug(Task *t) {

  char msg[] = "Hello World!";
  struct __attribute__((packed)) {
    packet_header_t header;
    char buf[sizeof(msg)];
  } msg_pack = {{0x55aa, PID_MSG, sizeof(msg), TIMER->TIMERAWL}, 0};
  for (uint8_t i = 0; i < sizeof(msg); i++) {
    msg_pack.buf[i] = msg[i];
  }
  // uart_tx_send((uint8_t *)&msg_pack, sizeof(msg_pack));
  command_packet_t pack = {0x55aa, CMD_DEBUG, 0};
  uart_tx_send((uint8_t *)&pack, sizeof(command_packet_t));

  t->next_time = TIMER->TIMERAWL + 1000 * 1000;
  schedule_timed_task(t);
}

void telemetry_fast(Task *t) {
  send_imu_payload(&imu_payload_dbuf.buf[imu_payload_dbuf.writer ^ 1]);
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
