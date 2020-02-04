#include "time.h"
#include "../shim.h"

int shim_clock_gettime_impl(linux_clockid_t clock_id, linux_timespec* tp) {
  switch (clock_id) {
    case LINUX_CLOCK_MONOTONIC:     return clock_gettime(CLOCK_MONOTONIC, tp);
    case LINUX_CLOCK_MONOTONIC_RAW: return clock_gettime(CLOCK_MONOTONIC, tp);
    default:
      UNIMPLEMENTED_ARGS("%d, %p", clock_id, tp);
  }
}

SHIM_WRAP(clock_gettime);
