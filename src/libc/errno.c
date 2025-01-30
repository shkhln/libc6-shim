#include <errno.h>
#include "../shim.h"

static int* shim___errno_location_impl() {
  return __error();
}

SHIM_WRAP(__errno_location);
