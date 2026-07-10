#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) {
  int16_t gyro_z, gyro_y, gyro_x, temp, accel_z, accel_y, accel_x;
} imu_readings;

enum imu_error {
  IMU_OK = 0,
  IMU_ABORT,
  IMU_TIMEOUT,
  IMU_ASLEEP,
};
_Static_assert(sizeof(imu_readings) == 14,
               "imu_readings struct is of wrong size");

void init_i2c(void);
enum imu_error init_imu(void);
enum imu_error read_imu(imu_readings *imu_struct);
enum imu_error calibrate_imu(void);
