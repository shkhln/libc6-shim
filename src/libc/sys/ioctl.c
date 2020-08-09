#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include "../../shim.h"

#define NV_UVM_INITIALIZE   0x30000001
#define NV_UVM_DEINITIALIZE 0x30000002

#define NV_ERR_NOT_SUPPORTED 0x56

struct NvUvmInitParams
{
  uint64_t flags __attribute__((aligned(8)));
  uint32_t status;
};

#define LINUX_FIONBIO        0x5421
#define LINUX_SNDCTL_SYSINFO 0x84f85801

int shim_ioctl_impl(int fd, unsigned long request, va_list args) {

  int m = request & 0xffff;
  if ((m >= 0x4600 && m <= 0x46ff) /* nvidia */ || m == 0x6d00 /* nvidia-modeset */) {
    return ioctl(fd, request, va_arg(args, void*));
  }

  if (m >= 0x4d00 && m <= 0x510E /* SOUND_MIXER_WRITE_VOLUME to SNDCTL_SYNTH_MEMAVL */) {

    // Linuxulator has slightly more elaborate OSS handling, so this is somewhat off

    if ((request & 0xff000000) == 0x40000000) {
      return ioctl(fd, (0x80 << 24) + (request & 0xffffff), va_arg(args, void*));
    }

    if ((request & 0xff000000) == 0x80000000) {
      return ioctl(fd, (0x40 << 24) + (request & 0xffffff), va_arg(args, void*));
    }

    return ioctl(fd, request, va_arg(args, void*));
  }

  if (request == LINUX_FIONBIO) {
    return ioctl(fd, FIONBIO, va_arg(args, int*));
  }

  if (request == LINUX_SNDCTL_SYSINFO) {
    errno = EINVAL;
    return -1;
  }

  if (request == NV_UVM_INITIALIZE) {
    struct NvUvmInitParams* params = (struct NvUvmInitParams*)va_arg(args, void*);
    params->status = NV_ERR_NOT_SUPPORTED;
    return 0;
  }

  if (request == NV_UVM_DEINITIALIZE) {
    return 0;
  }

  UNIMPLEMENTED_ARGS("%d, 0x%lx, _", fd, request);
}

SHIM_WRAP(ioctl);
