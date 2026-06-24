#include "config.h"
#include "regs.h"
#include <stdint.h>

#define MPU_6050_ADDR 0x68
#define SMPLRT_DIV 0x19
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

#include "i2c_imu.h"

void init_i2c_imu(void) {
  ACCESS_OFFSET(PADS_BANK0->GPIO[I2C0_SDA_IO], hw_xor) =
      (1 << 3) | (1 << 2); // xor the pullup/pulldown bits, at reset its pulled
                           // down after xor it becomes pull up enabled

  ACCESS_OFFSET(PADS_BANK0->GPIO[I2C0_SCL_IO], hw_xor) = (1 << 3) | (1 << 2);

  IO_BANK0->GPIO[I2C0_SDA_IO].CTRL = 3;
  IO_BANK0->GPIO[I2C0_SCL_IO].CTRL = 3;

  I2C0->TAR = MPU_6050_ADDR;

  I2C0->ENABLE = 1;

  I2C0->DATA_CMD = PWR_MGMT_1 | (0 << 8);
  I2C0->DATA_CMD = (0x00) | (1 << 9);
  I2C0->DATA_CMD = GYRO_CONFIG;
  I2C0->DATA_CMD = (0x00) | (1 << 9);
  I2C0->DATA_CMD = ACCEL_CONFIG;
  I2C0->DATA_CMD = (0x00) | (1 << 9);
}

enum imu_error read_imu(imu_readings *imu_struct) {
  uint32_t start = TIMER->TIMERAWL;
  uint32_t timeout = 1000;

  I2C0->DATA_CMD = ACCEL_XOUT_H;

  while (I2C0->TXFLR > (16 - 14))
    if (TIMER->TIMERAWL - start > timeout)
      return IMU_TIMEOUT;

  for (uint8_t i = 0; i < 13; i++)
    I2C0->DATA_CMD = (1 << 8);
  I2C0->DATA_CMD = (1 << 8) | (1 << 9);

  while (I2C0->RXFLR < 14)
    if (TIMER->TIMERAWL - start > timeout)
      return IMU_TIMEOUT;

  uint8_t *ptr =
      (uint8_t *)imu_struct + sizeof(*imu_struct); // end of imu_struct

  while (ptr != (uint8_t *)imu_struct)
    *--ptr = (uint8_t)I2C0->DATA_CMD; // write the struct in reverse order (to
                                      // account for big endiannes of imu data)

  return IMU_OK;
}
