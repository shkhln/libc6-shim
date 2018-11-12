#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include "../../shim.h"

#define NV_UVM_INITIALIZE   0x30000001
#define NV_UVM_DEINITIALIZE 0x30000002

#define NV_ERR_NOT_SUPPORTED 0x56

#define NV_ESC_SYS_PARAMS 214
#define NV_ESC_NUMA_INFO  215

struct NvUvmInitParams
{
  uint64_t flags __attribute__((aligned(8)));
  uint32_t status;
};

int shim_ioctl_impl(int fd, unsigned long request, va_list args) {

  if (request == NV_UVM_INITIALIZE) {
    struct NvUvmInitParams* params = (struct NvUvmInitParams*)va_arg(args, void*);
    params->status = NV_ERR_NOT_SUPPORTED;
    return 0;
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
