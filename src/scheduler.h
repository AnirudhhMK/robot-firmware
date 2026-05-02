#include <stdint.h>

typedef struct Task Task;
struct Task {
  volatile uint8_t ready;
  volatile uint8_t priority; // lower number means lower priority
  volatile uint32_t next_time;
  void (*next)(Task *t);
};

void schedule_timed_task(Task *t);
