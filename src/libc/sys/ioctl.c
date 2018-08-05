#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "../../shim.h"

#define NV_UVM_INITIALIZE   0x30000001
#define NV_UVM_DEINITIALIZE 0x30000002

#define NV_ESC_SYS_PARAMS 214
#define NV_ESC_NUMA_INFO  215

int shim_ioctl_impl(int fd, unsigned long request, va_list args) {

  if (request == NV_UVM_INITIALIZE) {
    return -1;
  }

  if (request == NV_UVM_DEINITIALIZE) {
    return 0;
  }

  switch (request & 0xff) {

    case NV_ESC_SYS_PARAMS:
      return 0;

    case NV_ESC_NUMA_INFO:
      return 0;

    default: {

      int err = ioctl(fd, request, va_arg(args, void*));
      if (err == -1) {
        perror(__func__);
      }

      return err;
    }
  }
}
