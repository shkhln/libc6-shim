#include <sys/types.h>
#include "../shim.h"

typedef void cpu_set_t;

int shim_sched_getaffinity_impl(pid_t pid, size_t cpusetsize, cpu_set_t* mask) {
  return -1;
}

int shim_sched_setaffinity_impl(pid_t pid, size_t cpusetsize, cpu_set_t* mask) {
  return 0;
}

SHIM_WRAP(sched_getaffinity);
SHIM_WRAP(sched_setaffinity);
