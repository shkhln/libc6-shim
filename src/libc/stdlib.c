#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
