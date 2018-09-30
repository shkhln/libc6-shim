#include <errno.h>
#include "../shim.h"

int* shim___errno_location_impl() {
  return __error();
}
