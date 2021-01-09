#include <unistd.h>
#include "../../shim.h"
#include "fcntl.h"

#define LINUX_EFD_NONBLOCK 0x00800
#define LINUX_EFD_CLOEXEC  0x80000

//TODO: wrap native eventfd
int shim_eventfd_impl(unsigned int initval, int linux_flags) {

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

SHIM_WRAP(eventfd);
