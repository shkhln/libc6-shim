#include <assert.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include "../../shim.h"

int shim_sysinfo_impl(struct sysinfo* info) {
  UNIMPLEMENTED();
}

__asm__(".symver shim_get_nprocs,get_nprocs@GLIBC_2.0");
__asm__(".symver shim_get_nprocs,get_nprocs@GLIBC_2.2.5");
int shim_get_nprocs(void) {
  LOG("%s()\n", __func__);
  int nprocs = sysconf(_SC_NPROCESSORS_CONF);
  LOG_RES("%d", nprocs);
  return nprocs;
}
