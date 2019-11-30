#include <assert.h>
#include <unistd.h>
#include "../../shim.h"

int shim_get_nprocs(void) {
  LOG_ENTRY();
  int nprocs = sysconf(_SC_NPROCESSORS_CONF);
  LOG_EXIT("%d", nprocs);
  return nprocs;
}
