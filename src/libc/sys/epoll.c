#include <unistd.h>
#include "../../shim.h"
#include "fcntl.h"

static int fake_epoll_fd = -1;

__attribute__((constructor))
static void init_fake_epoll_fd() {

  int inout[2];
  {
    int err = pipe(inout);
    assert(err == 0);
  }

  fake_epoll_fd = inout[1];
}

int shim_epoll_create_impl(int size) {
  return fake_epoll_fd;
}

typedef void linux_epoll_event;

int shim_epoll_ctl_impl(int epfd, int op, int fd, linux_epoll_event* event) {
  return 0; // ?
}

int shim_epoll_wait_impl(int epfd, linux_epoll_event*events, int maxevents, int timeout) {
  return -1; // ?
}

#define LINUX_EFD_NONBLOCK 0x00800
#define LINUX_EFD_CLOEXEC  0x80000

int shim_epoll_create1_impl(int linux_flags) {

  assert((linux_flags & ~(LINUX_EFD_CLOEXEC | LINUX_EFD_NONBLOCK)) == 0);

  int flags = 0;
  if (linux_flags & LINUX_EFD_NONBLOCK) flags |= O_NONBLOCK;
  if (linux_flags & LINUX_EFD_CLOEXEC)  flags |= O_CLOEXEC;

  int inout[2];
  {
    int err = pipe2(inout, flags);
    assert(err == 0);
  }

  return inout[1];
}

SHIM_WRAP(epoll_create1);
SHIM_WRAP(epoll_create);
SHIM_WRAP(epoll_ctl);
SHIM_WRAP(epoll_wait);
