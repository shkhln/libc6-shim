#include <limits.h>
#include <string.h>
#include "../shim.h"

void* shim_rawmemchr(const void* s, int c) {
  return memchr(s, c, INT_MAX);
}

SYM_EXPORT(shim_rawmemchr, __rawmemchr);
