#pragma once
#include "math.h"
#include "uart_comms.h"
#include <stdint.h>
void launch_core1();

typedef struct {
  int16_t pwm_l;
  int16_t pwm_r;
} pwm_data;

typedef struct {
  pwm_data buf[2];
  volatile uint8_t writer;
} pwm_data_dbuf_t;
extern pwm_data_dbuf_t pwm_data_dbuf;

typedef struct {
  angle_estimate_payload_t buf[2];
  volatile uint8_t writer;
} angle_estimate_dbuf_t;
extern angle_estimate_dbuf_t angle_estimate_dbuf;

typedef struct {
  imu_payload_t buf[2];
  volatile uint8_t writer;
} imu_payload_dbuf_t;
extern imu_payload_dbuf_t imu_payload_dbuf;

typedef struct {
  loop_time_payload_t buf[2];
  volatile uint8_t writer;
} loop_time_payload_dbuf_t;
extern loop_time_payload_dbuf_t loop_time_payload_dbuf;
