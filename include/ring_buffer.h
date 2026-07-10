#pragma once
#include <stdint.h>

typedef struct {
  uint8_t *data;
  uint32_t writer, reader;
  uint32_t mask;
} ringbuf_t;

typedef ringbuf_t *ringbuf_handle_t;

#define CREATE_RINGBUF(name, size)                                             \
  _Static_assert(size & (size - 1) != 0,                                       \
                 "Ring buffer size must be a power of 2");                     \
  do {                                                                         \
    ringbuf_handle_t name = &(ringbuf_t){.data = (uint8_t[size]){0},           \
                                         .writer = 0,                          \
                                         .reader = 0,                          \
                                         .mask = size - 1};                    \
  } while (0);

static inline int32_t Rbuf_write_blocking(ringbuf_handle_t buf, uint8_t *data) {
  if (buf->writer - buf->reader >
      buf->mask) // occupancy >= capacity, occupancy > capacity - 1
    return -1;
  buf->data[buf->writer++ & buf->mask] = *data;
  return 0;
}

static inline int32_t Rbuf_write_overriding(ringbuf_handle_t buf,
                                            uint8_t *data) {
  int32_t ret = 0;
  if (buf->writer - buf->reader > buf->mask)
    ret = ~(uint32_t)(buf->data[buf->reader++ & buf->mask]);
  buf->data[buf->writer++ & buf->mask] = *data;
  return ret;
}

static inline int32_t Rbuf_read(ringbuf_handle_t buf, uint8_t *data) {
  if (buf->writer - buf->reader <= 0)
    return -1;
  *data = buf->data[buf->reader++ & buf->mask];
  return 0;
}

int32_t Rbuf_write_bulk(ringbuf_handle_t buf, uint8_t *data, uint32_t size);
int32_t Rbuf_read_bulk(ringbuf_handle_t buf, uint8_t *data, uint32_t size);

int32_t Rbuf_check_availability(ringbuf_handle_t buf, uint32_t space);
