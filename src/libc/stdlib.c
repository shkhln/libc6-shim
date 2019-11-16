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

long shim___strtol_internal_impl(char* nptr, char** endptr, int base, int group) {
  return strtol(nptr, endptr, base);
}

long shim___strtoll_internal_impl(char* nptr, char** endptr, int base, int group) {
  return strtoll(nptr, endptr, base);
}

unsigned long shim___strtoul_internal_impl(char* nptr, char** endptr, int base, int group) {
  return strtoul(nptr, endptr, base);
}

unsigned long shim___strtoull_internal_impl(char* nptr, char** endptr, int base, int group) {
  return strtoull(nptr, endptr, base);
}

float shim___strtof_internal_impl(char* nptr, char** endptr, int group) {
  return strtof(nptr, endptr);
}

double shim___strtod_internal_impl(char* nptr, char** endptr, int group) {
  return strtod(nptr, endptr);
}

long double shim___strtold_internal_impl(char* nptr, char** endptr, int group) {
  return strtold(nptr, endptr);
}
