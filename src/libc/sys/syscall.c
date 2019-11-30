#include <assert.h>
#include <pthread_np.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
//#include <sys/syscall.h>

#include "../../shim.h"

#ifdef __i386__
#define LINUX_GETPID         20
#define LINUX_GETTID        224
#define LINUX_CLOCK_GETTIME 265
#endif

#ifdef __x86_64__
#define LINUX_GETPID         39
#define LINUX_GETTID        186
#define LINUX_CLOCK_GETTIME 228
#endif

#define LINUX_CLOCK_MONOTONIC 1

long shim_syscall_impl(long number, va_list args) {

  if (number == LINUX_GETPID) {

    LOG("%s: getpid()", __func__);

    pid_t pid = getpid();
    LOG("%s: getpid -> %d", __func__, pid);

    return pid;
  }

  if (number == LINUX_GETTID) {

    LOG("%s: gettid()", __func__);

    int tid = pthread_getthreadid_np();
    LOG("%s: gettid -> %d", __func__, tid);

    return tid;
  }

  if (number == LINUX_CLOCK_GETTIME) {

    clockid_t        clock_id = va_arg(args, clockid_t);
    struct timespec* tp       = va_arg(args, struct timespec*);

    LOG("%s: clock_gettime(%d, %p)", __func__, clock_id, tp);

    if (clock_id == LINUX_CLOCK_MONOTONIC) {

      int time = clock_gettime(CLOCK_MONOTONIC, tp);
      LOG("%s: clock_gettime -> %d", __func__, time);

      return time;

    } else {
      assert(0);
    }
  }

  UNIMPLEMENTED_ARGS("%ld, ...", number);
}
