#include "../../shim.h"

typedef void linux_statvfs64;

int shim_statvfs64_impl(const char* restrict path, linux_statvfs64* restrict buf) {
  return -1;
}

SHIM_WRAP(statvfs64);
