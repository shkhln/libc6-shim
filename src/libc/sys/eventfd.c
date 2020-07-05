#include <unistd.h>
#include "../../shim.h"
#include "fcntl.h"

static int fake_efd = -1;

__attribute__((constructor))
static void init_fake_eventfd() {

  int inout[2];
  {
    int err = pipe(inout);
    assert(err == 0);
  }

  fake_efd = inout[1];
}

int shim_eventfd_impl(unsigned int initval, int linux_flags) {
  return fake_efd;
}

SHIM_WRAP(eventfd);
