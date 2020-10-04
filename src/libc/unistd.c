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
#define GLIBC_SC_NPROCESSORS_ONLN  84
#define GLIBC_SC_PHYS_PAGES        85
#define GLIBC_SC_MONOTONIC_CLOCK  149

long shim_sysconf_impl(int name) {

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

int shim_ftruncate64_impl(int fd, linux_off64_t length) {
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

linux_off64_t shim_lseek64_impl(int fd, linux_off64_t offset, int whence) {
  return lseek(fd, offset, whence);
}

SHIM_WRAP(lseek64);

static char* shim_optarg = NULL;
static int   shim_optind = 1;
static int   shim_optopt = 0;
static int   shim_opterr = 1;

SHIM_EXPORT(optarg);
SHIM_EXPORT(optind);
SHIM_EXPORT(optopt);
SHIM_EXPORT(opterr);

static char** _optarg = NULL;
static int*   _optind = NULL;
static int*   _optopt = NULL;
static int*   _opterr = NULL;

int shim_getopt_impl(int argc, char* const argv[], const char* optstring) {

  if (!_optarg) _optarg = look_up_global_var("optarg");
  if (!_optind) _optind = look_up_global_var("optind");
  if (!_optopt) _optopt = look_up_global_var("optopt");
  if (!_opterr) _opterr = look_up_global_var("opterr");

  optind = *_optind;
  opterr = *_opterr;

  int err = getopt(argc, argv, optstring);

  *_optarg = optarg;
  *_optind = optind;
  *_optopt = optopt;

  return err;
}

SHIM_WRAP(getopt);
