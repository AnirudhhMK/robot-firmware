#pragma once
#include "math.h"
#include "uart_comms.hpp"
#include <stdint.h>
void launch_core1();

class Dbuf_ptr;
template <typename T>
class DoubleBuffer { // under compiler optimization, the methods of the class
                     // get inlined costing no extra runtime overhead compared
                     // to a regular C implementation
                     //
                     // NOTE: this implementation doesn't implement any locks on
                     // the induvidual buffer so it is entirely possible to
                     // write to a buffer that is currently being read from,
                     // however aslong as the rate at which we swap is much
                     // slower than how long it takes to finish reading a
                     // buffer, there will be no issues.
  T buf[2];
  uint32_t front = 0; // read from front, write on back
  inline void swap() { front ^= 1; }
  inline T *writer() { return &buf[front ^ 1]; }
  inline const T *reader() { return &buf[front]; }

public:
  class Producer;
  class Consumer;
  Producer producer(); // producer and consumer handle to enforce SPSC style
  const Consumer consumer();
  friend Dbuf_ptr; // used if a type-erased pointer to a general Double buffer
                   // is required.
};

template <typename T> class DoubleBuffer<T>::Producer {
  DoubleBuffer<T> *dbuf;
  explicit Producer(DoubleBuffer<T> *dbuf) : dbuf(dbuf) {}

public:
  friend DoubleBuffer<T>::Producer DoubleBuffer<T>::producer();
  T *acquire() { return dbuf->writer(); }
  void publish() { dbuf->swap(); }
};

template <typename T> class DoubleBuffer<T>::Consumer {
  DoubleBuffer<T> *dbuf;

  explicit Consumer(DoubleBuffer<T> *dbuf) : dbuf(dbuf) {}

public:
  friend DoubleBuffer<T>::Consumer DoubleBuffer<T>::consumer();
  const T *acquire() const { return dbuf->reader(); }
};

template <typename T>
typename DoubleBuffer<T>::Producer DoubleBuffer<T>::producer() {
  return DoubleBuffer<T>::Producer(this);
}
template <typename T>
const typename DoubleBuffer<T>::Consumer DoubleBuffer<T>::consumer() {
  return DoubleBuffer<T>::Consumer(this);
}
class Dbuf_ptr {
  const char *buf;
  const size_t size;
  const uint8_t *front;

public:
  template <typename T>
  Dbuf_ptr(const DoubleBuffer<T> &dbuf)
      : buf(dbuf.buf), size(sizeof(T)), front(&dbuf.front) {}
  inline const char *reader() const { return buf + size * (*front); }
};

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

typedef struct {
  q16_16_t Kp, Kd, Ki;
} pid_gains_t;
extern pid_gains_t pid_gains;
