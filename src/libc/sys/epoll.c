#define EPOLL_SHIM_DISABLE_WRAPPER_MACROS

#include <unistd.h>
#include <sys/epoll.h>
#include "../../shim.h"
#include "../signal.h"

#define LINUX_EPOLL_CLOEXEC 0x80000

typedef struct epoll_event linux_epoll_event;

extern int (*libepoll_epoll_create) (int);
extern int (*libepoll_epoll_create1)(int);
extern int (*libepoll_epoll_ctl)    (int, int, int, struct epoll_event*);
extern int (*libepoll_epoll_wait)   (int, struct epoll_event*, int, int);
extern int (*libepoll_epoll_pwait)  (int, struct epoll_event*, int, int, const sigset_t*);

static int shim_epoll_create_impl(int size) {
  return libepoll_epoll_create(size);
}

static int shim_epoll_create1_impl(int linux_flags) {

  assert((linux_flags & ~LINUX_EPOLL_CLOEXEC) == 0);

  int flags = 0;

  if (linux_flags & LINUX_EPOLL_CLOEXEC) {
    flags |= EPOLL_CLOEXEC;
  }

  return libepoll_epoll_create1(flags);
}

static int shim_epoll_ctl_impl(int epfd, int linux_op, int fd, linux_epoll_event* linux_event) {
  return libepoll_epoll_ctl(epfd, linux_op /* same encoding */, fd, linux_event /* same encoding */);
}

static int shim_epoll_wait_impl(int epfd, linux_epoll_event* events, int maxevents, int timeout) {
  return libepoll_epoll_wait(epfd, events /* same encoding */, maxevents, timeout);
}

static int shim_epoll_pwait_impl(int epfd, struct epoll_event* events, int maxevents, int timeout, const linux_sigset_t* sigmask) {
  return libepoll_epoll_pwait(epfd, events /* same encoding */, maxevents, timeout, (sigset_t*)sigmask);
}

SHIM_WRAP(epoll_create);
SHIM_WRAP(epoll_create1);
SHIM_WRAP(epoll_ctl);
SHIM_WRAP(epoll_wait);
SHIM_WRAP(epoll_pwait);
