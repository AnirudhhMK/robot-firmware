#include <stdint.h>

typedef struct __attribute__((packed)) {
  uint16_t sync;
  uint16_t pid;
  uint16_t payload_len;
  uint32_t timestamp;
} packet_header_t;

typedef struct __attribute__((packed)) {
  uint16_t sync;
  uint16_t command;
  uint32_t value;
} command_packet_t;

typedef struct __attribute__((packed)) {
  int16_t dtheta_gyro;
  int16_t theta_accel;
  int16_t theta_filtered;
  int16_t pid_output;
} control_payload_t;

typedef struct __attribute__((packed)) {
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;

  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;
} sensor_payload_t;

typedef struct __attribute__((packed)) {
  int16_t pwm_l;
  int16_t pwm_r;
} telemetry_fast_payload_t;

enum {
  PID_CONTROL = 1,
  PID_SENSOR_DATA = 2,
  PID_FAST = 3,
  PID_MSG = 4,
};

enum {
  CMD_DEBUG = 1,
};

void init_uart_comms(void);
void uart_tx_kick(void);
void uart_tx_update_reader(void);
int32_t uart_tx_send(uint8_t *data, uint32_t size);
void uart_tx_DMA_handler(void);

int32_t uart_rx_get(command_packet_t *packet);
void uart_rx_DMA_handler();
