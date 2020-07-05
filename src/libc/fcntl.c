#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "../shim.h"
#include "fcntl.h"

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

  if (cmd == LINUX_F_SETOWN) {
#ifdef DEBUG
    int pid = va_arg(args, int);
    LOG("%s: cmd = F_SETOWN, arg = 0x%x", __func__, pid);
#endif
    return -1;
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

  if (linux_flags & LINUX_O_WRONLY)    flags |= O_WRONLY;
  if (linux_flags & LINUX_O_RDWR)      flags |= O_RDWR;
  if (linux_flags & LINUX_O_CREAT)     flags |= O_CREAT;
  if (linux_flags & LINUX_O_EXCL)      flags |= O_EXCL;
  if (linux_flags & LINUX_O_TRUNC)     flags |= O_TRUNC;
  if (linux_flags & LINUX_O_NONBLOCK)  flags |= O_NONBLOCK;
  if (linux_flags & LINUX_O_DIRECTORY) flags |= O_DIRECTORY;
  if (linux_flags & LINUX_O_CLOEXEC)   flags |= O_CLOEXEC;

  if (linux_flags & LINUX_O_TMPFILE) {
    //~ fprintf(stderr, "%s: O_TMPFILE\n", __func__);
    return -1; // ?
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

int shim_open64_impl(const char* path, int linux_flags, va_list args) {
  return shim_open_impl(path, linux_flags, args);
}

int shim_posix_fallocate64_impl(int fd, linux_off64_t offset, linux_off64_t len) {
  return posix_fallocate(fd, offset, len);
}

SHIM_WRAP(fcntl);
SHIM_WRAP(open);
SHIM_WRAP(open64);
SHIM_WRAP(posix_fallocate64);
