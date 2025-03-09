#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/sysctl.h>

#include "../shim.h"

#define STRTONUM_INTERNAL_I(ret_type, name) \
  static ret_type shim_ ## __ ## name ## _internal_impl(char* nptr, char** endptr, int base, int group) {\
    return name(nptr, endptr, base);\
  }

#define STRTONUM_INTERNAL_F(ret_type, name) \
  static ret_type shim_ ## __ ## name ## _internal_impl(char* nptr, char** endptr, int group) {\
    return name(nptr, endptr);\
  }

STRTONUM_INTERNAL_I(long,               strtol);
STRTONUM_INTERNAL_I(long long,          strtoll);
STRTONUM_INTERNAL_I(unsigned long,      strtoul);
STRTONUM_INTERNAL_I(unsigned long long, strtoull);
STRTONUM_INTERNAL_F(float,              strtof);
STRTONUM_INTERNAL_F(double,             strtod);
STRTONUM_INTERNAL_F(long double,        strtold);

SHIM_WRAP(__strtol_internal);
SHIM_WRAP(__strtoll_internal);
SHIM_WRAP(__strtoul_internal);
SHIM_WRAP(__strtoull_internal);
SHIM_WRAP(__strtof_internal);
SHIM_WRAP(__strtod_internal);
SHIM_WRAP(__strtold_internal);

static void* shim_memalign_impl(size_t alignment, size_t size) {
  void* p = NULL;
  posix_memalign(&p, alignment, size);
  return p;
}

SHIM_WRAP(memalign);

static char* shim___realpath_chk_impl(const char* path, char* resolved_path, size_t resolved_len) {
  assert(resolved_len >= PATH_MAX);
  return realpath(path, resolved_path);
}

SHIM_WRAP(__realpath_chk);

static char* shim_secure_getenv_impl(const char* name) {
  return issetugid() == 0 ? getenv(name) : NULL;
}

SHIM_WRAP(secure_getenv);

static char* shim_realpath_impl(const char* restrict path, char* restrict resolved_path) {

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

      char* out = resolved_path != NULL ? resolved_path : malloc(PATH_MAX);

      if ((pid == -1 || pid == getpid()) && proc_self_exe_override != NULL) {
        size_t nchars = strlcpy(out, proc_self_exe_override, PATH_MAX);
        assert(nchars < PATH_MAX);
      } else {
        int name[] = {
          CTL_KERN,
          KERN_PROC,
          KERN_PROC_PATHNAME,
          pid
        };

        size_t len = PATH_MAX;
        int err = sysctl(name, nitems(name), out, &len, NULL, 0);
        if (err == -1) {
          errno = native_to_linux_errno(errno);
          return NULL;
        }
      }

      return out;

    } else {

      free(p);

      errno = native_to_linux_errno(EACCES);
      return NULL;
    }
  }

  if (str_starts_with(path, "/sys/")) {
    errno = native_to_linux_errno(EACCES);
    return NULL;
  }

  return realpath(path, resolved_path);
}

SHIM_WRAP(realpath);
