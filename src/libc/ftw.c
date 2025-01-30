#include "../shim.h"
#include "sys/stat.h"

typedef struct FTW linux_FTW;

static int shim_ftw_impl(const char* path, int (*fn)(const char*, const struct linux_stat*, int), int maxfds) {
  return -1;
}

static int shim_nftw_impl(const char* path, int (*fn)(const char*, const struct linux_stat*, int, linux_FTW*), int maxfds, int flags) {
  return -1;
}

SHIM_WRAP(ftw);
SHIM_WRAP(nftw);
