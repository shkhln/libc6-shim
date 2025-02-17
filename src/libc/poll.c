#include <poll.h>
#include "../shim.h"
#include "signal.h"
#include "time.h"

extern int (*libepoll_epoll_shim_poll) (struct pollfd[], nfds_t, int);
extern int (*libepoll_epoll_shim_ppoll)(struct pollfd[], nfds_t, const struct timespec* restrict, const sigset_t* restrict);

static int shim_poll_impl(struct pollfd fds[], nfds_t nfds, int timeout) {
  return libepoll_epoll_shim_poll(fds, nfds, timeout);
}

static int shim_ppoll_impl(struct pollfd fds[], nfds_t nfds, const linux_timespec* restrict timeout, const linux_sigset_t* restrict newsigmask) {
  return libepoll_epoll_shim_ppoll(fds, nfds, timeout, (sigset_t*)newsigmask);
}

SHIM_WRAP(poll);
SHIM_WRAP(ppoll);
