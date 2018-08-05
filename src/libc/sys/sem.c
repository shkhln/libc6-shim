#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../shim.h"

int shim_semctl_impl(int semid, int semnum, int cmd, ...) {
  UNIMPLEMENTED();
}
