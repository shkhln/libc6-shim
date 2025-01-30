#include <unistd.h>
#include <sys/eventfd.h>
#include "../../shim.h"
#include "fcntl.h"

#define LINUX_EFD_NONBLOCK 0x00800
#define LINUX_EFD_CLOEXEC  0x80000

static int shim_eventfd_impl(unsigned int initval, int linux_flags) {

  assert((linux_flags & ~(LINUX_EFD_CLOEXEC | LINUX_EFD_NONBLOCK)) == 0);

  int flags = 0;
  if (linux_flags & LINUX_EFD_NONBLOCK) flags |= EFD_NONBLOCK;
  if (linux_flags & LINUX_EFD_CLOEXEC)  flags |= EFD_CLOEXEC;

  return eventfd(initval, flags);
}

SHIM_WRAP(eventfd);
