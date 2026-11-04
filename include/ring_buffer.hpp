#pragma once
#include <stddef.h>
#include <stdint.h>

template <size_t size> class Ring_buffer {
  static_assert((size > 0) & ((size & (size - 1)) == 0),
                "ring buffer size must be a power of 2");

public:
  static constexpr size_t mask = size - 1;
  uint32_t writer = 0, reader = 0;
  uint8_t data[size];
  Ring_buffer() {}
  int32_t write(const uint8_t *msg, size_t length) {
    if ((writer + length) - reader > mask + 1)
      return -1; // maybe return the amount of excess space needed?
    uint32_t w = writer;
    for (size_t i = 0; i < length; i++) {
      data[w++ & mask] = msg[i];
    }
    writer = w;
    return 0;
  }
  int32_t read(uint8_t *data, size_t length) {
    if ((reader + length) > writer)
      return -1;
    uint32_t r = reader;
    for (uint32_t i = 0; i < length; i++) {
      data[i] = data[r++ & mask];
    }
    reader = r;
    return 0;
  }
  uint32_t const space() { return size + (writer - reader); }
};
/*typedef struct {
  uint8_t *data;
  uint32_t writer, reader;
  uint32_t mask;
} ringbuf_t;

typedef ringbuf_t *ringbuf_handle_t;

#define CREATE_RINGBUF(name, size)                                             \
  static_assert(size & (size - 1) != 0,                                       \
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
*/
