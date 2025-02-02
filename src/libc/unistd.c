#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include "../shim.h"
#include "fcntl.h"

static int shim_chown_impl(const char* path, uid_t owner, gid_t group) {
  assert(!str_starts_with(path, "/dev/"));
  return chown(path, owner, group);
}

static char* proc_self_exe_override = NULL;

__attribute__((constructor))
static void init() {
  proc_self_exe_override = getenv("SHIM_PROC_SELF_EXE");
}

static ssize_t shim_readlink_impl(const char* path, char* buf, size_t bufsize) {

  if (str_starts_with(path, "/proc/")) {

    char* p = strdup(&path[sizeof("/proc/") - 1]);
    assert(p != NULL);

    char* s = p;
    char* node  = strsep(&s, "/");
    char* entry = strsep(&s, "/");

    if (strcmp(entry, "exe") == 0) {

      int pid;
      if (strcmp(node, "self") == 0) {
        pid = -1;
      } else {
        pid = strtoul(node, NULL, 10);
        assert(pid > 0);
      }

      free(p);

      if (proc_self_exe_override != NULL) {
        size_t nchars = strlen(proc_self_exe_override);
        strncpy(buf, proc_self_exe_override, nchars <= bufsize ? nchars : bufsize);
        return nchars <= bufsize ? nchars : bufsize;
      } else {
        int name[] = {
          CTL_KERN,
          KERN_PROC,
          KERN_PROC_PATHNAME,
          pid
        };

        int err = sysctl(name, nitems(name), buf, &bufsize, NULL, 0);
        if (err == -1) {
          // we don't care about emulating silent truncation here
          errno = native_to_linux_errno(errno);
          return -1;
        }

        return bufsize - 1; // length without a nul char
      }

    } else {

      free(p);

      errno = native_to_linux_errno(EACCES);
      return -1;
    }
  }

  if (str_starts_with(path, "/sys/")) {
    errno = native_to_linux_errno(EACCES);
    return -1;
  }

  return readlink(path, buf, bufsize);
}

//~ int shim_execl_impl(const char* path, const char* arg, va_list args) {
  //~ UNIMPLEMENTED();
//~ }

//~ int shim_execle_impl(const char* path, const char* arg, va_list args) {
  //~ UNIMPLEMENTED();
//~ }

//~ int shim_execlp_impl(const char* file, const char* arg, va_list args) {
  //~ UNIMPLEMENTED();
//~ }

#define GLIBC_SC_PAGESIZE          30
#define GLIBC_SC_NPROCESSORS_CONF  83
#define GLIBC_SC_NPROCESSORS_ONLN  84
#define GLIBC_SC_PHYS_PAGES        85
#define GLIBC_SC_MONOTONIC_CLOCK  149

static long shim_sysconf_impl(int name) {

  if (name == GLIBC_SC_NPROCESSORS_CONF) {
    return sysconf(_SC_NPROCESSORS_CONF);
  }

  if (name == GLIBC_SC_NPROCESSORS_ONLN) {
    return sysconf(_SC_NPROCESSORS_ONLN);
  }

  if (name == GLIBC_SC_PAGESIZE) {
    return sysconf(_SC_PAGESIZE);
  }

  if (name == GLIBC_SC_PHYS_PAGES) {
    return sysconf(_SC_PHYS_PAGES);
  }

  if (name == GLIBC_SC_MONOTONIC_CLOCK) {
    return sysconf(_SC_MONOTONIC_CLOCK);
  }

  UNIMPLEMENTED_ARGS("%d", name);
}

static int shim_ftruncate64_impl(int fd, linux_off64_t length) {
  return ftruncate(fd, length);
}

SHIM_WRAP(chown);
SHIM_WRAP(ftruncate64);
SHIM_WRAP(readlink);
SHIM_WRAP(sysconf);

int shim_pipe2_impl(int fildes[2], int linux_flags) {

  assert((linux_flags & (LINUX_O_CLOEXEC | LINUX_O_NONBLOCK)) == linux_flags);

  int flags = 0;

  if (linux_flags & LINUX_O_NONBLOCK) flags |= O_NONBLOCK;
  if (linux_flags & LINUX_O_CLOEXEC)  flags |= O_CLOEXEC;

  return pipe2(fildes, flags);
}

SHIM_WRAP(pipe2);

static linux_off64_t shim_lseek64_impl(int fd, linux_off64_t offset, int whence) {
  return lseek(fd, offset, whence);
}

SHIM_WRAP(lseek64);

static int shim_getopt_impl(int argc, char* const argv[], const char* optstring) {

  optind = *globals.optind;
  opterr = *globals.opterr;

  int err = getopt(argc, argv, optstring);

  *globals.optarg = optarg;
  *globals.optind = optind;
  *globals.optopt = optopt;

  return err;
}

SHIM_WRAP(getopt);

static int shim_access_impl(const char* path, int mode) {

  char* p = redirect(path);
  if (p == NULL) {
    errno = EACCES;
    return -1;
  }

  return access(p, mode);
}

SHIM_WRAP(access);

extern ssize_t (*libepoll_epoll_shim_read) (int, void*, size_t);
extern ssize_t (*libepoll_epoll_shim_write)(int, const void*, size_t);
extern int     (*libepoll_epoll_shim_close)(int);

static ssize_t shim_read_impl(int fd, void *buf, size_t nbytes) {
  return libepoll_epoll_shim_read(fd, buf, nbytes);
}

static ssize_t shim_write_impl(int fd, const void* buf, size_t nbytes) {
  return libepoll_epoll_shim_write(fd, buf, nbytes);
}

static int shim_close_impl(int fd) {
  return libepoll_epoll_shim_close(fd);
}

SHIM_WRAP(read);
SHIM_WRAP(write);
SHIM_WRAP(close);
