#pragma once
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
