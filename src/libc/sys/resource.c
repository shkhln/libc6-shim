#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "../../shim.h"

typedef void linux_rlimit;
typedef void linux_rlimit64;

static int shim_getrlimit_impl(int resource, linux_rlimit* rlp) {
  errno = native_to_linux_errno(EPERM);
  return -1;
}

static int shim_getrlimit64_impl(int resource, linux_rlimit64* rlp) {
  errno = native_to_linux_errno(EPERM);
  return -1;
}

static int shim_setrlimit_impl(int resource, const linux_rlimit* rlp) {
  UNIMPLEMENTED();
}

SHIM_WRAP(getrlimit);
SHIM_WRAP(getrlimit64);
SHIM_WRAP(setrlimit);
