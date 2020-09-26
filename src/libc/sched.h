#include <sched.h>
#include <spawn.h>

typedef struct sched_param linux_sched_param;

enum {
  LINUX_SCHED_NORMAL  = 0,
  LINUX_SCHED_FIFO    = 1,
  LINUX_SCHED_RR      = 2,
  LINUX_SCHED_BATCH   = 3,
  LINUX_SCHED_IDLE    = 5
};

int linux_to_native_sched_policy(int linux_policy);
int native_to_linux_sched_policy(int linux_policy);
