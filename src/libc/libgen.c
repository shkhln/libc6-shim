#include <libgen.h>
#include "../shim.h"

static char* shim___xpg_basename_impl(const char* path) {
  return basename(path);
}

SHIM_WRAP(__xpg_basename);
