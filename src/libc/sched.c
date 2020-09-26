#include <sys/types.h>
#include "../shim.h"
#include "sched.h"

typedef void cpu_set_t;

int shim_sched_getaffinity_impl(pid_t pid, size_t cpusetsize, cpu_set_t* mask) {
  return -1;
}

int shim_sched_setaffinity_impl(pid_t pid, size_t cpusetsize, cpu_set_t* mask) {
  return 0;
}

SHIM_WRAP(sched_getaffinity);
SHIM_WRAP(sched_setaffinity);

int linux_to_native_sched_policy(int linux_policy) {
  switch (linux_policy) {
    case LINUX_SCHED_NORMAL: return SCHED_OTHER;
    case LINUX_SCHED_FIFO:   return SCHED_FIFO;
    case LINUX_SCHED_RR:     return SCHED_RR;
    default:
      assert(0);
  }
}

int native_to_linux_sched_policy(int linux_policy) {
  switch (linux_policy) {
    case SCHED_OTHER: return LINUX_SCHED_NORMAL;
    case SCHED_FIFO:  return LINUX_SCHED_FIFO;
    case SCHED_RR:    return LINUX_SCHED_RR;
    default:
      assert(0);
  }
}

int shim_sched_get_priority_max_impl(int linux_policy) {
  return sched_get_priority_max(linux_to_native_sched_policy(linux_policy));
}

int shim_sched_get_priority_min_impl(int linux_policy) {
  return sched_get_priority_min(linux_to_native_sched_policy(linux_policy));
}

SHIM_WRAP(sched_get_priority_max);
SHIM_WRAP(sched_get_priority_min);

int shim_sched_getscheduler_impl(pid_t pid) {
  return native_to_linux_sched_policy(sched_getscheduler(pid));
}

int shim_sched_setscheduler_impl(pid_t pid, int policy, const linux_sched_param* param) {
  return sched_setscheduler(pid, linux_to_native_sched_policy(policy), param);
}

SHIM_WRAP(sched_getscheduler);
SHIM_WRAP(sched_setscheduler);

int shim_posix_spawnattr_getschedpolicy_impl(const posix_spawnattr_t* restrict attr, int* restrict linux_policy) {
  int policy;
  int err = posix_spawnattr_getschedpolicy(attr, &policy);
  if (err == 0) {
    *linux_policy = native_to_linux_sched_policy(policy);
  }
  return err;
}

int shim_posix_spawnattr_setschedpolicy_impl(posix_spawnattr_t* attr, int linux_policy) {
  return posix_spawnattr_setschedpolicy(attr, linux_to_native_sched_policy(linux_policy));
}

SHIM_WRAP(posix_spawnattr_getschedpolicy);
SHIM_WRAP(posix_spawnattr_setschedpolicy);
