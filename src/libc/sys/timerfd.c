#include <assert.h>
#include <sys/timerfd.h>
#include "../time.h"
#include "../../shim.h"

typedef struct itimerspec linux_itimerspec; // same layout

#define LINUX_TFD_NONBLOCK  0x00800
#define LINUX_TFD_CLOEXEC   0x80000

#define LINUX_TFD_TIMER_ABSTIME       1
#define LINUX_TFD_TIMER_CANCEL_ON_SET 2

int shim_timerfd_create_impl(int linux_clockid, int linux_flags) {

  assert((linux_flags & ~(LINUX_TFD_NONBLOCK | LINUX_TFD_CLOEXEC)) == 0);

  int flags = 0;
  if ((linux_flags & LINUX_TFD_NONBLOCK) != 0) flags |= TFD_NONBLOCK;
  if ((linux_flags & LINUX_TFD_CLOEXEC)  != 0) flags |= TFD_CLOEXEC;

  return timerfd_create(linux_to_native_clockid(linux_clockid), flags);
}

int shim_timerfd_gettime_impl(int fd, linux_itimerspec* curr_value) {
  return timerfd_gettime(fd, curr_value);
}

int shim_timerfd_settime_impl(int fd, int linux_flags, const linux_itimerspec* new_value, linux_itimerspec* old_value) {
  assert((linux_flags & ~(LINUX_TFD_TIMER_ABSTIME | LINUX_TFD_TIMER_CANCEL_ON_SET)) == 0);
  return timerfd_settime(fd, linux_flags /* same values */, new_value, old_value);
}

SHIM_WRAP(timerfd_create);
SHIM_WRAP(timerfd_gettime);
SHIM_WRAP(timerfd_settime);
