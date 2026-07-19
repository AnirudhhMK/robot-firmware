#pragma once
#include "i2c_imu.h"
#include "math.h"
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

typedef imu_readings imu_payload_t;

typedef struct {
  q16_16_t theta_a;
  q16_16_t theta_g;
  q16_16_t theta;
} angle_estimate_payload_t;
_Static_assert(sizeof(angle_estimate_payload_t) == 3 * 4,
               "size of angle_estimate_payload_t is wrong");

typedef struct {
  uint32_t min_time;
  uint32_t max_time;
} loop_time_payload_t;
_Static_assert(sizeof(loop_time_payload_t) == 2 * 4,
               "size of loop_time_payload_t is wrong");

enum {
  PID_CONTROL = 1,
  PID_IMU_DATA = 2,
  PID_ANGLE_ESTIMATE = 3,
  PID_MSG = 4,
};

enum {
  CMD_DEBUG = 1,
  CMD_SET_P = 2,
  CMD_SET_D = 3,
};

void init_uart_comms(void);
void uart_tx_kick(void);
void uart_tx_update_reader(void);
int32_t uart_tx_send(packet_header_t *packet_header, uint8_t *payload);
void uart_tx_DMA_handler(void);

int32_t uart_rx_get(command_packet_t *packet);
void uart_rx_DMA_handler();
