#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include "../shim.h"

int shim_chown_impl(const char* path, uid_t owner, gid_t group) {
  assert(!str_starts_with(path, "/dev/"));
  return chown(path, owner, group);
}

ssize_t shim_readlink_impl(const char* path, char* buf, size_t bufsize) {

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

      int name[] = {
        CTL_KERN,
        KERN_PROC,
        KERN_PROC_PATHNAME,
        pid
      };

      int err = sysctl(name, nitems(name), buf, &bufsize, NULL, 0);
      assert(err == 0);

      free(p);

      return bufsize;

    } else {

      free(p);

      errno = EACCES;
      return -1;
    }
  }

  if (str_starts_with(path, "/sys/")) {
    errno = EACCES;
    return -1;
  }

  return readlink(path, buf, bufsize);
}

int shim_execl_impl(const char* path, const char* arg, va_list args) {
  UNIMPLEMENTED();
}

int shim_execle_impl(const char* path, const char* arg, va_list args) {
  UNIMPLEMENTED();
}

int shim_execlp_impl(const char* file, const char* arg, va_list args) {
  UNIMPLEMENTED();
}

#define GLIBC_SC_PAGESIZE         30
#define GLIBC_SC_NPROCESSORS_ONLN 84

long shim_sysconf_impl(int name) {

  if (name == GLIBC_SC_NPROCESSORS_ONLN) {
    return sysconf(_SC_NPROCESSORS_ONLN);
  }

  if (name == GLIBC_SC_PAGESIZE) {
    return sysconf(_SC_PAGESIZE);
  }

  UNIMPLEMENTED_ARGS("%d", name);
}

int shim_ftruncate64_impl(int fd, linux_off64_t length) {
  return ftruncate(fd, length);
}

int shim_ftruncate64(int fd, linux_off64_t length) {
  LOG_ENTRY("%d, %jd", fd, length);
  int err = shim_ftruncate64_impl(fd, length);
  LOG_EXIT("%d", err);
  return err;
}
