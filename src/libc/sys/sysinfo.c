#include <assert.h>
#include <unistd.h>
#include "../../shim.h"

static int shim_get_nprocs_impl() {
  return sysconf(_SC_NPROCESSORS_CONF);
}

SHIM_WRAP(get_nprocs);

static int shim_sysinfo_impl(void* info) {
  return -1;
}

SHIM_WRAP(sysinfo);
