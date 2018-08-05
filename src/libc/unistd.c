#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../shim.h"

int shim_chown_impl(const char* path, uid_t owner, gid_t group) {
  assert(!str_starts_with(path, "/dev/"));
  return chown(path, owner, group);
}

ssize_t shim_readlink_impl(const char* path, char* buf, size_t bufsize) {

  if (str_starts_with(path, "/proc/") || str_starts_with(path, "/sys/")) {
    errno = EACCES;
    return -1;
  }

  return readlink(path, buf, bufsize);
}

int shim_execle(const char* path, const char* arg, ...) {
  UNIMPLEMENTED();
}

SYM_EXPORT(shim_execle, execle);

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

int shim_execl(const char *path, const char *arg, ...) {
  UNIMPLEMENTED();
}

SYM_EXPORT(shim_execl, execl);
