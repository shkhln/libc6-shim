#include <assert.h>
#include <unistd.h>
#include "../../shim.h"

int shim_get_nprocs(void) {
  LOG("%s()\n", __func__);
  int nprocs = sysconf(_SC_NPROCESSORS_CONF);
  LOG_RES("%d", nprocs);
  return nprocs;
}
