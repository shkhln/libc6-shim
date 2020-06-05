#include "../shim.h"
#include "netdb.h"

int* shim___h_errno_location_impl() {
  return &h_errno;
}

SHIM_WRAP(__h_errno_location);
