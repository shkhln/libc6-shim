#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "../../shim.h"

int shim_setrlimit_impl(int resource, const linux_rlimit* rlp) {
  UNIMPLEMENTED();
}

SHIM_WRAP(setrlimit);
