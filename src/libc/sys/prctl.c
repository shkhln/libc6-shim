#include <assert.h>
#include <errno.h>
#include "../../shim.h"

int shim_prctl_impl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5) {
  errno = EACCES;
  return -1;
}

SHIM_WRAP(prctl);
