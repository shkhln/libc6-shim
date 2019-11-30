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

#define LINUX_O_RDONLY   0x00000
#define LINUX_O_WRONLY   0x00001
#define LINUX_O_RDWR     0x00002
#define LINUX_O_CREAT    0x00040
#define LINUX_O_TRUNC    0x00200
#define LINUX_O_NONBLOCK 0x00800
#define LINUX_O_CLOEXEC  0x80000

#define KNOWN_LINUX_OPEN_FLAGS (LINUX_O_RDONLY | LINUX_O_WRONLY | LINUX_O_RDWR | LINUX_O_CREAT | LINUX_O_TRUNC | LINUX_O_NONBLOCK | LINUX_O_CLOEXEC)

int shim_fcntl_impl(int fd, int cmd, va_list args) {

  if (cmd == LINUX_F_GETFD) {
    LOG("%s: cmd = F_GETFD", __func__);
    return fcntl(fd, F_GETFD);
  }

  if (cmd == LINUX_F_SETFD) {
    int arg = va_arg(args, int);
    LOG("%s: cmd = F_SETFD, arg = 0x%x", __func__, arg);
    assert(arg == 1);
    return fcntl(fd, F_SETFD, FD_CLOEXEC);
  }

  if (cmd == LINUX_F_GETFL) {
    LOG("%s: cmd = F_GETFL", __func__);
    int flags       = fcntl(fd, F_GETFL);
    int linux_flags =
      (flags & O_RDWR     ? LINUX_O_RDWR     : 0) |
      (flags & O_NONBLOCK ? LINUX_O_NONBLOCK : 0);

    return linux_flags;
  }

  if (cmd == LINUX_F_SETFL) {
    int linux_flags = va_arg(args, int);
    LOG("%s: cmd = F_SETFL, arg = 0x%x", __func__, linux_flags);

    assert((linux_flags & (LINUX_O_RDWR | LINUX_O_NONBLOCK)) == linux_flags);

    int flags =
      (linux_flags & LINUX_O_RDWR     ? O_RDWR     : 0) |
      (linux_flags & LINUX_O_NONBLOCK ? O_NONBLOCK : 0);

    return fcntl(fd, F_SETFL, flags);
  }

  UNIMPLEMENTED_ARGS("%d, %d, ...", fd, cmd);
}

int shim_open_impl(const char* path, int linux_flags, va_list args) {

  if (str_starts_with(path, "/proc/") || str_starts_with(path, "/sys/")) {
    errno = EACCES;
    return -1;
  }

  assert((linux_flags & KNOWN_LINUX_OPEN_FLAGS) == linux_flags);

  int flags = 0;

  if (linux_flags & LINUX_O_WRONLY)   flags |= O_WRONLY;
  if (linux_flags & LINUX_O_RDWR)     flags |= O_RDWR;
  if (linux_flags & LINUX_O_CREAT)    flags |= O_CREAT;
  if (linux_flags & LINUX_O_TRUNC)    flags |= O_TRUNC;
  if (linux_flags & LINUX_O_NONBLOCK) flags |= O_NONBLOCK;
  if (linux_flags & LINUX_O_CLOEXEC)  flags |= O_CLOEXEC;

  mode_t mode = 0;

  if (flags & O_CREAT) {
    mode = va_arg(args, int);
  }

  if (strcmp("/dev/nvidia-uvm", path) == 0) {
    return open("/dev/null", flags, mode);
  }

  return open(path, flags, mode);
}

int shim_posix_fallocate64_impl(int fd, linux_off64_t offset, linux_off64_t len) {
  return posix_fallocate(fd, offset, len);
}
