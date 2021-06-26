#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "../shim.h"
#include "fcntl.h"

#ifdef __i386__

struct linux_flock {
  uint16_t l_type;
  uint16_t l_whence;
  uint32_t l_start;
  uint32_t l_len;
  uint32_t l_pid;
};

#endif

#ifdef __x86_64__

struct linux_flock {
  uint16_t l_type;
  uint16_t l_whence;
  uint64_t l_start;
  uint64_t l_len;
  uint32_t l_pid;
};

#endif

#define LINUX_F_RDLCK 0
#define LINUX_F_WRLCK 1
#define LINUX_F_UNLCK 2

static void copy_linux_flock(struct flock* dst, struct linux_flock* src) {

  switch (src->l_type) {
    case LINUX_F_RDLCK: dst->l_type = F_RDLCK; break;
    case LINUX_F_WRLCK: dst->l_type = F_WRLCK; break;
    case LINUX_F_UNLCK: dst->l_type = F_UNLCK; break;
    default:
      assert(0);
  }

  dst->l_whence = src->l_whence;
  dst->l_start  = src->l_start;
  dst->l_len    = src->l_len;
  dst->l_pid    = src->l_pid;
}

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

  if (cmd == LINUX_F_GETLK) {
#ifdef DEBUG
    void* lock = va_arg(args, void*);
    LOG("%s: cmd = F_GETLK, arg = %p", __func__, lock);
#endif
    assert(0);
  }

  if (cmd == LINUX_F_SETLK) {

    struct linux_flock* linux_lock = va_arg(args, struct linux_flock*);
    LOG("%s: cmd = F_SETLK, arg = %p", __func__, linux_lock);

    struct flock lock;
    copy_linux_flock(&lock, linux_lock);

    return fcntl(fd, F_SETLK, &lock);
  }

  if (cmd == LINUX_F_SETLKW) {

    struct linux_flock* linux_lock = va_arg(args, struct linux_flock*);
    LOG("%s: cmd = F_SETLKW, arg = %p", __func__, linux_lock);

    struct flock lock;
    copy_linux_flock(&lock, linux_lock);

    return fcntl(fd, F_SETLKW, &lock);
  }

  if (cmd == LINUX_F_SETOWN) {
#ifdef DEBUG
    int pid = va_arg(args, int);
    LOG("%s: cmd = F_SETOWN, arg = 0x%x", __func__, pid);
#endif
    assert(0);
  }

  if (cmd == LINUX_F_GETOWN) {
    LOG("%s: cmd = F_GETOWN", __func__);
    assert(0);
  }

  if (cmd == LINUX_F_ADD_SEALS) {
#if __FreeBSD_version >= 1300139
    int flags = va_arg(args, int);
    LOG("%s: cmd = LINUX_F_ADD_SEALS, arg = 0x%x", __func__, flags);
    return fcntl(fd, F_ADD_SEALS, flags);
#else
    return -1;
#endif
  }

  UNIMPLEMENTED_ARGS("%d, %d, ...", fd, cmd);
}

int shim_fcntl64_impl(int fd, int cmd, va_list args) {
  return shim_fcntl_impl(fd, cmd, args);
}

int shim_open_impl(const char* path, int linux_flags, va_list args) {

  char* p = redirect(path);
  if (p == NULL) {
    errno = EACCES;
    return -1;
  }

  assert((linux_flags & KNOWN_LINUX_OPEN_FLAGS) == linux_flags);

  int flags = 0;

  if (linux_flags & LINUX_O_WRONLY)    flags |= O_WRONLY;
  if (linux_flags & LINUX_O_RDWR)      flags |= O_RDWR;
  if (linux_flags & LINUX_O_CREAT)     flags |= O_CREAT;
  if (linux_flags & LINUX_O_EXCL)      flags |= O_EXCL;
  if (linux_flags & LINUX_O_NOCTTY)    flags |= O_NOCTTY;
  if (linux_flags & LINUX_O_TRUNC)     flags |= O_TRUNC;
  if (linux_flags & LINUX_O_APPEND)    flags |= O_APPEND;
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

  return open(p, flags, mode);
}

int shim_open64_impl(const char* path, int linux_flags, va_list args) {
  return shim_open_impl(path, linux_flags, args);
}

int shim_posix_fallocate64_impl(int fd, linux_off64_t offset, linux_off64_t len) {
  return posix_fallocate(fd, offset, len);
}

SHIM_WRAP(fcntl);
SHIM_WRAP(fcntl64);
SHIM_WRAP(open);
SHIM_WRAP(open64);
SHIM_WRAP(posix_fallocate64);

int shim_shm_open_impl(const char* path, int linux_flags, mode_t mode) {

  char buf[PATH_MAX];
  snprintf(buf, sizeof(buf), "/compat/linux/dev/shm%s", path);

  assert((linux_flags & (LINUX_O_RDONLY | LINUX_O_RDWR | LINUX_O_CREAT | LINUX_O_EXCL | LINUX_O_TRUNC)) == linux_flags);

  int flags = 0;

  if (linux_flags & LINUX_O_WRONLY) flags |= O_WRONLY;
  if (linux_flags & LINUX_O_RDWR)   flags |= O_RDWR;
  if (linux_flags & LINUX_O_CREAT)  flags |= O_CREAT;
  if (linux_flags & LINUX_O_EXCL)   flags |= O_EXCL;
  if (linux_flags & LINUX_O_TRUNC)  flags |= O_TRUNC;

  return open(buf, flags, mode | O_CLOEXEC);
}

SHIM_WRAP(shm_open);
