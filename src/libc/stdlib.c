#include <stdlib.h>
#include <string.h>
#include "../shim.h"

void* shim_memalign(size_t alignment, size_t size) {
  LOG_ARGS("%zu, %zu", alignment, size);
  void* p = NULL;
  posix_memalign(&p, alignment, size);
  LOG_RES("%p",  p);
  return p;
}

SYM_EXPORT(shim_memalign, memalign);

long shim_glibc_strtol(char* nptr, char** endptr, int base, int group) {
  return strtol(nptr, endptr, base);
}

long shim_glibc_strtoll(char* nptr, char** endptr, int base, int group) {
  return strtoll(nptr, endptr, base);
}

unsigned long shim_glibc_strtoul(char* nptr, char** endptr, int base, int group) {
  return strtoul(nptr, endptr, base);
}

unsigned long shim_glibc_strtoull(char* nptr, char** endptr, int base, int group) {
  return strtoull(nptr, endptr, base);
}

float shim_glibc_strtof(char* nptr, char** endptr, int group) {
  return strtof(nptr, endptr);
}

double shim_glibc_strtod(char* nptr, char** endptr, int group) {
  return strtod(nptr, endptr);
}

long double shim_glibc_strtold(char* nptr, char** endptr, int group) {
  return strtold(nptr, endptr);
}

SYM_EXPORT(shim_glibc_strtol,   __strtol_internal);
SYM_EXPORT(shim_glibc_strtoll,  __strtoll_internal);
SYM_EXPORT(shim_glibc_strtoul,  __strtoul_internal);
SYM_EXPORT(shim_glibc_strtoull, __strtoull_internal);
SYM_EXPORT(shim_glibc_strtof,   __strtof_internal);
SYM_EXPORT(shim_glibc_strtod,   __strtod_internal);
SYM_EXPORT(shim_glibc_strtold,  __strtold_internal);
