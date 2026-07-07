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
  q16_16_t theta_a;
  q16_16_t theta_g;
  q16_16_t theta;
} angle_estimate;

typedef struct {
  angle_estimate buf[2];
  volatile uint8_t writer;
} angle_estimate_dbuf_t;
extern angle_estimate_dbuf_t angle_estimate_dbuf;

typedef struct {
  imu_payload_t buf[2];
  volatile uint8_t writer;
} imu_payload_dbuf_t;
extern imu_payload_dbuf_t imu_payload_dbuf;
