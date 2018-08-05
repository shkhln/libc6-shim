#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "../shim.h"

#define LINUX_F_GETFD 1
#define LINUX_F_SETFD 2
#define LINUX_F_GETFL 3
#define LINUX_F_SETFL 4

#define LINUX_O_RDWR     0x002
#define LINUX_O_NONBLOCK 0x800

int shim_fcntl_impl(int fd, int cmd, va_list args) {

  if (cmd == LINUX_F_GETFD) {
    LOG("%s: cmd = F_GETFD\n", __func__);
    return fcntl(fd, F_GETFD);
  }

  if (cmd == LINUX_F_SETFD) {
    int arg = va_arg(args, int);
    LOG("%s: cmd = F_SETFD, arg = 0x%x\n", __func__, arg);
    assert(arg == 1);
    return fcntl(fd, F_SETFD, FD_CLOEXEC);
  }

  if (cmd == LINUX_F_GETFL) {
    LOG("%s: cmd = F_GETFL\n", __func__);
    int flags       = fcntl(fd, F_GETFL);
    int linux_flags =
      (flags & O_RDWR     ? LINUX_O_RDWR     : 0) |
      (flags & O_NONBLOCK ? LINUX_O_NONBLOCK : 0);

    return linux_flags;
  }

  if (cmd == LINUX_F_SETFL) {
    int linux_flags = va_arg(args, int);
    LOG("%s: cmd = F_SETFL, arg = 0x%x\n", __func__, linux_flags);

    assert((linux_flags & (LINUX_O_RDWR | LINUX_O_NONBLOCK)) == linux_flags);

    int flags =
      (linux_flags & LINUX_O_RDWR     ? O_RDWR     : 0) |
      (linux_flags & LINUX_O_NONBLOCK ? O_NONBLOCK : 0);

    return fcntl(fd, F_SETFL, flags);
  }

  UNIMPLEMENTED_ARGS("%d, %d, ...", fd, cmd);
}

int shim_open_impl(const char* path, int flags, va_list args) {

  if (str_starts_with(path, "/proc/") || str_starts_with(path, "/sys/")) {
    errno = EACCES;
    return -1;
  }

  mode_t mode = 0;

  if (flags & O_CREAT) {
    mode = va_arg(args, int);
  }

  if (strcmp("/dev/nvidia-uvm", path) == 0) {
    return open("/dev/null", flags, mode);
  }

  return open(path, flags, mode);
}
