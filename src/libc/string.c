#include <errno.h>
#include <limits.h>
#include <string.h>
#include <xlocale.h>
#include "../shim.h"
#include "locale.h"

static void* shim___rawmemchr_impl(const void* s, int c) {
  return memchr(s, c, INT_MAX);
}

static void shim_perror_impl(const char* string) {
  errno = linux_to_native_errno(errno);
  perror(string);
}

static char* shim_strerror_impl(int errnum) {
  return strerror(linux_to_native_errno(errnum));
}

static char* shim_strerror_r_impl(int errnum, char* strerrbuf, size_t buflen) {
  strerror_r(linux_to_native_errno(errnum), strerrbuf, buflen);
  return strerrbuf;
}

SHIM_WRAP(__rawmemchr);
SHIM_WRAP(perror);
SHIM_WRAP(strerror);
SHIM_WRAP(strerror_r);

static void* shim___memset_chk_impl(void* dest, int c, size_t len, size_t destlen) {
  assert(len <= destlen);
  return memset(dest, c, len);
}

SHIM_WRAP(__memset_chk);

static char* shim___strcat_chk_impl(char* dest, const char* src, size_t destlen) {
  assert(strlen(src) + strlen(dest) < destlen);
  return strcat(dest, src);
}

static char* shim___strncat_chk_impl(char* dest, const char* src, size_t n, size_t destlen) {
  assert(strlen(src) + strlen(dest) < destlen);
  return strncat(dest, src, n);
}

SHIM_WRAP(__strcat_chk);
SHIM_WRAP(__strncat_chk);

static void* shim___memcpy_chk_impl(void* dst, const void* src, size_t len, size_t destlen) {
  assert(len <= destlen);
  return memcpy(dst, src, len);
}

static char* shim___strcpy_chk_impl(char* dest, const char* src, size_t destlen) {
  assert(strlen(src) < destlen);
  return strcpy(dest, src);
}

static char* shim___stpcpy_chk_impl(char* dest, const char* src, size_t destlen) {
  assert(strlen(src) < destlen);
  return stpcpy(dest, src);
}

static char* shim___strncpy_chk_impl(char* s1, const char* s2, size_t n, size_t s1len) {
  assert(n <= s1len);
  return strncpy(s1, s2, n);
}

SHIM_WRAP(__memcpy_chk);
SHIM_WRAP(__strcpy_chk);
SHIM_WRAP(__stpcpy_chk);
SHIM_WRAP(__strncpy_chk);

static int shim_strcasecmp_l_impl(const char* s1, const char* s2, linux_locale_t loc) {
  return strcasecmp_l(s1, s2, loc->native_locale);
}

static int shim_strcoll_l_impl(const char* s1, const char* s2, linux_locale_t loc) {
  return strcoll_l(s1, s2, loc->native_locale);
}

static int shim_strncasecmp_l_impl(const char* s1, const char* s2, size_t len, linux_locale_t loc) {
  return strncasecmp_l(s1, s2, len, loc->native_locale);
}

static size_t shim_strxfrm_l_impl(char* restrict dst, const char* restrict src, size_t n, linux_locale_t loc) {
  return strxfrm_l(dst, src, n, loc->native_locale);
}

SHIM_WRAP(strcasecmp_l);
SHIM_WRAP(strcoll_l);
SHIM_WRAP(strncasecmp_l);
SHIM_WRAP(strxfrm_l);

static int shim___xpg_strerror_r_impl(int errnum, char* buf, size_t buflen) {
  return strerror_r(linux_to_native_errno(errnum), buf, buflen);
}

SHIM_WRAP(__xpg_strerror_r);

static void* shim___memmove_chk_impl(void* dest, const void* src, size_t len, size_t destlen) {
  assert(len <= destlen);
  return memmove(dest, src, len);
}

SHIM_WRAP(__memmove_chk);
