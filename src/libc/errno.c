#include <errno.h>
#include "../shim.h"

int* shim_errno_location() {
  return __error();
}

SYM_EXPORT(shim_errno_location, __errno_location);
