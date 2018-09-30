#include <limits.h>
#include <string.h>
#include "../shim.h"

void* shim___rawmemchr_impl(const void* s, int c) {
  return memchr(s, c, INT_MAX);
}
