#include "regs.h"
#include <stdint.h>

namespace Timer {
inline uint32_t get_time32() { return TIMER->TIMERAWL; }
inline uint64_t get_time64() {
  uint32_t hi = TIMER->TIMERAWH;
  uint32_t lo;
  do {
    lo = TIMER->TIMERAWL;
    uint32_t next_hi = TIMER->TIMERAWH;
    if (next_hi == hi)
      break;
    hi = next_hi;
  } while (true);
  return ((uint64_t)hi << 32u) | lo;
}

} // namespace Timer
