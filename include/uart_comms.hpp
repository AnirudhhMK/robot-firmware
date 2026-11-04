#pragma once
#include "i2c_imu.h"
#include "math.h"
#include "regs.h"
#include <stddef.h>
#include <stdint.h>

enum class PacketId : uint16_t {
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
static_assert(sizeof(angle_estimate_payload_t) == 3 * 4,
               "size of angle_estimate_payload_t is wrong");

typedef struct {
  uint32_t min_time;
  uint32_t max_time;
} loop_time_payload_t;
static_assert(sizeof(loop_time_payload_t) == 2 * 4,
               "size of loop_time_payload_t is wrong");

struct __attribute__((packed)) packet_header_t {
  uint16_t sync;
  PacketId pid;
  uint16_t payload_len;
  uint32_t timestamp;
  packet_header_t(PacketId id, uint16_t len = 0) {
    sync = 0x55aa;
    pid = id;
    switch (id) {
    case PacketId::PID_ANGLE_ESTIMATE:
      payload_len = sizeof(angle_estimate_payload_t);
      break;
    case PacketId::PID_CONTROL:
      payload_len = sizeof(control_payload_t);
      break;
    case PacketId::PID_IMU_DATA:
      payload_len = sizeof(imu_payload_t);
      break;
    case PacketId::PID_MSG:
      payload_len = len;
      break;
    }
    timestamp = TIMER->TIMERAWL;
  }
};

namespace uart {
void init(void);
int32_t send(packet_header_t *packet_header, uint8_t *payload);
int32_t send(const uint8_t *const message[], const size_t sizes[],
             size_t length);
void tx_DMA_handler(void);

int32_t get(command_packet_t *packet);
void rx_DMA_handler();
} // namespace uart
