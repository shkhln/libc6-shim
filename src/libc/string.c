#include <limits.h>
#include <string.h>
#include "../shim.h"

void* shim___rawmemchr_impl(const void* s, int c) {
  return memchr(s, c, INT_MAX);
}

char* shim_strerror_r_impl(int errnum, char* strerrbuf, size_t buflen) {
  strerror_r(errnum, strerrbuf, buflen);
  return strerrbuf;
}

SHIM_WRAP(__rawmemchr);
SHIM_WRAP(strerror_r);
