#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include "../../shim.h"

int shim_sysctl_impl(int* name, int namelen, void* oldp, size_t* oldlenp, void* newp, size_t newlen) {
  UNIMPLEMENTED();
}
