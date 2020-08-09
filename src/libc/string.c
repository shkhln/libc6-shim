#include <errno.h>
#include <limits.h>
#include <string.h>
#include "../shim.h"

void* shim___rawmemchr_impl(const void* s, int c) {
  return memchr(s, c, INT_MAX);
}

void shim_perror_impl(const char* string) {
  errno = linux_to_native_errno(errno);
  perror(string);
}

char* shim_strerror_impl(int errnum) {
  return strerror(linux_to_native_errno(errnum));
}

char* shim_strerror_r_impl(int errnum, char* strerrbuf, size_t buflen) {
  strerror_r(linux_to_native_errno(errnum), strerrbuf, buflen);
  return strerrbuf;
}

SHIM_WRAP(__rawmemchr);
SHIM_WRAP(perror);
SHIM_WRAP(strerror);
SHIM_WRAP(strerror_r);

char* shim___strcpy_chk_impl(char* dest, const char* src, size_t destlen) {
  assert(strlen(src) < destlen);
  return strcpy(dest, src);
}

SHIM_WRAP(__strcpy_chk);
