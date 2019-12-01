#include <assert.h>
#include <unistd.h>
#include "../../shim.h"

int shim_get_nprocs_impl() {
  return sysconf(_SC_NPROCESSORS_CONF);
}

SHIM_WRAP(get_nprocs);
