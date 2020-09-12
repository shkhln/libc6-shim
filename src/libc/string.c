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

void* shim___memset_chk_impl(void* dest, int c, size_t len, size_t destlen) {
  assert(len <= destlen);
  return memset(dest, c, len);
}

SHIM_WRAP(__memset_chk);

void* shim___memcpy_chk_impl(void* dst, const void* src, size_t len, size_t destlen) {
  assert(len <= destlen);
  return memcpy(dst, src, len);
}

char* shim___strcpy_chk_impl(char* dest, const char* src, size_t destlen) {
  assert(strlen(src) < destlen);
  return strcpy(dest, src);
}

char* shim___strncpy_chk_impl(char* s1, const char* s2, size_t n, size_t s1len) {
  assert(n <= s1len);
  return strncpy(s1, s2, n);
}

SHIM_WRAP(__memcpy_chk);
SHIM_WRAP(__strcpy_chk);
SHIM_WRAP(__strncpy_chk);
