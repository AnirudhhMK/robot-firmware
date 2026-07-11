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

typedef struct {
  int16_t offx;
  int16_t offy;
  int16_t offz;
} gyro_offset_t;
gyro_offset_t gyro_offset;

static inline enum imu_error wait_for_transaction(uint32_t timeout) {
  uint32_t start = TIMER->TIMERAWL;
  while (I2C0->STATUS &
         (1 << 5)) { // checks for i2c master activity, idle:0, active:1
    if (TIMER->TIMERAWL - start > timeout) {
      SIO->GPIO_OUT_XOR = (1 << DEBUG_LED_IO);
      return IMU_TIMEOUT;
    }
  }
  if (I2C0->TX_ABRT_SOURCE) {
    SIO->GPIO_OUT_XOR = (1 << DEBUG_LED_IO);
    return IMU_ABORT;
  }
  return IMU_OK; // IMU_OK = 0
}

enum imu_error calibrate_imu(void) {
  uint32_t i = 0;
  int32_t x = 0, y = 0, z = 0;
  const uint8_t samplelog2 = 8;
  uint32_t start_time = TIMER->TIMERAWL;
  imu_readings reading;
  while (i < (1 << samplelog2)) {
    uint32_t time = TIMER->TIMERAWL;
    if (time - start_time > 1000 * 1000)
      return IMU_TIMEOUT;

    if (read_imu(&reading) == IMU_OK) {
      i++;
      x += reading.gyro_x;
      y += reading.gyro_y;
      z += reading.gyro_z;
      while (TIMER->TIMERAWL - time < 1000)
        ;
    } else
      (void)I2C0->CLR_TX_ABRT;
  }
  gyro_offset.offx = (x >> samplelog2);
  gyro_offset.offy = (y >> samplelog2);
  gyro_offset.offz = (z >> samplelog2);
  return IMU_OK;
}
void init_i2c(void) {
  ACCESS_OFFSET(PADS_BANK0->GPIO[I2C0_SDA_IO], hw_xor) =
      (1 << 3) | (1 << 2); // xor the pullup/pulldown bits, at reset its pulled
                           // down after xor it becomes pull up enabled

  ACCESS_OFFSET(PADS_BANK0->GPIO[I2C0_SCL_IO], hw_xor) = (1 << 3) | (1 << 2);

  I2C0->TAR = MPU_6050_ADDR;
  I2C0->FS_SCL_HCNT = 100;
  I2C0->FS_SCL_LCNT = 150;
  I2C0->FS_SPKLEN = 9;
  I2C0->SDA_HOLD = 31;
  I2C0->ENABLE = 1;
}

enum imu_error init_imu(void) {
  // now we try talking to mpu6050, but since it might take a while to finish
  // setting up, we delay till its ready
  for (uint32_t i = 0; i < 200; i++) {
    (void)I2C0->CLR_TX_ABRT;
    I2C0->DATA_CMD = PWR_MGMT_1;
    uint32_t time = TIMER->TIMERAWL;
    while (TIMER->TIMERAWL - time < 1 * 1000)
      ;
    if (!I2C0->TX_ABRT_SOURCE)
      break;
  }
  if (I2C0->TX_ABRT_SOURCE) {
    return IMU_ABORT;
  }

  I2C0->DATA_CMD = (0x00) | (1 << 9);
  I2C0->DATA_CMD = GYRO_CONFIG;
  I2C0->DATA_CMD = (0x00) | (1 << 9);
  I2C0->DATA_CMD = ACCEL_CONFIG;
  I2C0->DATA_CMD = (0x00) | (1 << 9);
  static uint32_t first_iter = 1;
  if (first_iter--) { // for the first iteration of initializing add a 30ms
                      // delay to get gyro to settle after powering on
    uint32_t time = TIMER->TIMERAWL;
    while (TIMER->TIMERAWL - time < 30 * 1000)
      ;
  }
  return wait_for_transaction(1000);
}

enum imu_error read_imu(imu_readings *imu_struct) {
  (void)I2C0->CLR_TX_ABRT;
  uint32_t time = TIMER->TIMERAWL;
  while (I2C0->TXFLR > 1)
    if (TIMER->TIMERAWL - time > 100)
      return IMU_TIMEOUT;

  I2C0->DATA_CMD = ACCEL_XOUT_H;
  for (uint8_t i = 0; i < 13; i++)
    I2C0->DATA_CMD = (1 << 8);
  I2C0->DATA_CMD = (1 << 8) | (1 << 9);
  enum imu_error ret = wait_for_transaction(900);
  if (ret != IMU_OK)
    return ret;

  // SIO->GPIO_OUT_XOR = (1 << DEBUG_LED_IO);
  if (I2C0->RXFLR < 14)
    return IMU_TIMEOUT;

  uint8_t *ptr =
      (uint8_t *)imu_struct + sizeof(*imu_struct); // end of imu_struct
  uint8_t val = 0;
  while (ptr != (uint8_t *)imu_struct) {
    *--ptr = (uint8_t)I2C0->DATA_CMD; // write the struct in reverse order (to
                                      // account for big endiannes of imu data)
    val |= *ptr;
  }
  if (!val)
    return IMU_ASLEEP;

  if (I2C0->TX_ABRT_SOURCE)
    return IMU_ABORT;

  return IMU_OK;
}
