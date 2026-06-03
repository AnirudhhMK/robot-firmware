#pragma "once"
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
    ringbuf_handle_t name = &(ringbuf_t){                                      \
        .data = (uint8_t[64]){0}, .writer = 0, .reader = 0, .mask = size - 1}; \
  } while (0);

inline int32_t Rbuf_write_blocking(ringbuf_handle_t buf, uint8_t *data);
inline int32_t Rbuf_write_overriding(ringbuf_handle_t buf, uint8_t *data);
inline int32_t Rbuf_read(ringbuf_handle_t buf, uint8_t *data);
int32_t Rbuf_write_bulk(ringbuf_handle_t buf, uint8_t *data, uint32_t size);
