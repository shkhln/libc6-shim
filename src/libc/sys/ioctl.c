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

#define LINUX_TIOCGWINSZ     0x5413
#define LINUX_FIONREAD       0x541b
#define LINUX_FIONBIO        0x5421
#define LINUX_SIOCGIFCONF    0x8912
#define LINUX_SNDCTL_SYSINFO 0x84f85801

#define LINUX_IOC_DIR(x) (((x) >> 30) & 0x0003)
#define LINUX_IOC_LEN(x) (((x) >> 16) & 0x3fff)
#define LINUX_IOC_GRP(x) (((x) >>  8) & 0x00ff)
#define LINUX_IOC_CMD(x) ( (x)        & 0x00ff)

static unsigned long linux_to_native_dir(int linux_dir) {
  switch (linux_dir) {
    case 0: return IOC_VOID;
    case 1: return IOC_IN;
    case 2: return IOC_OUT;
    case 3: return IOC_IN | IOC_OUT;
    default:
      assert(0);
  }
}

static int check_len(int len) {
  assert(len < (1 << 13));
  return len;
}

#define LINUX_TO_NATIVE_REQ(x) (_IOC(linux_to_native_dir(LINUX_IOC_DIR(x)), LINUX_IOC_GRP(x), LINUX_IOC_CMD(x), check_len(LINUX_IOC_LEN(x))))

static int shim_ioctl_impl(int fd, unsigned long request, va_list args) {

  int command = request & 0xffff;
  if ((command >= 0x4600 && command <= 0x46ff) || /* nvidia */
       command == 0x6d00                       || /* nvidia-modeset */
      // Linuxulator has slightly more elaborate OSS handling, so this is somewhat off
      (command >= 0x4d00 && command <= 0x510e) || /* SOUND_MIXER_WRITE_VOLUME to SNDCTL_SYNTH_MEMAVL */
      (command >= 0x6440 && command <= 0x64a0))   /* [DRM_IOCTL_BASE, DRM_COMMAND_BASE to DRM_COMMAND_END] */
  {
    return ioctl(fd, LINUX_TO_NATIVE_REQ(request), va_arg(args, void*));
  }

  if (request == LINUX_TIOCGWINSZ) {
    return ioctl(fd, TIOCGWINSZ, va_arg(args, void*));
  }

  if (request == LINUX_FIONREAD) {
    return ioctl(fd, FIONREAD, va_arg(args, int*));
  }

  if (request == LINUX_FIONBIO) {
    return ioctl(fd, FIONBIO, va_arg(args, int*));
  }

  // steamclient.so wants this, but it already calls getifaddrs,
  // which presumably returns the same information
  if (request == LINUX_SIOCGIFCONF) {
    errno = EINVAL;
    return -1;
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
