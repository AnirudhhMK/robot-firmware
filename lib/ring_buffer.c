#include "ring_buffer.h"
#include <stdint.h>

// return value if operation fails is always negative
// incase of write_overriding if some data is overwriten,-(1+data) is returned
inline int32_t Rbuf_write_blocking(ringbuf_handle_t buf, uint8_t *data) {
  if (buf->writer - buf->reader > buf->mask)
    return -1;
  buf->data[buf->writer++ & buf->mask] = *data;
  return 0;
}

inline int32_t Rbuf_write_overriding(ringbuf_handle_t buf, uint8_t *data) {
  int32_t ret = 0;
  if (buf->writer - buf->reader > buf->mask)
    ret = ~(uint32_t)(buf->data[buf->reader++ & buf->mask]);
  buf->data[buf->writer++ & buf->mask] = *data;
  return ret;
}

inline int32_t Rbuf_read(ringbuf_handle_t buf, uint8_t *data) {
  if (buf->writer - buf->reader <= 0)
    return -1;
  *data = buf->data[buf->reader++ & buf->mask];
  return 0;
}
