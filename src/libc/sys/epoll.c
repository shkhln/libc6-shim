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

int shim_epoll_create1_impl(int flags) {
  return fake_epoll_fd;
}

typedef void linux_epoll_event;

int shim_epoll_ctl_impl(int epfd, int op, int fd, linux_epoll_event* event) {
  return 0; // ?
}

int shim_epoll_wait_impl(int epfd, linux_epoll_event*events, int maxevents, int timeout) {
  return -1; // ?
}

SHIM_WRAP(epoll_create);
SHIM_WRAP(epoll_create1);
SHIM_WRAP(epoll_ctl);
SHIM_WRAP(epoll_wait);
