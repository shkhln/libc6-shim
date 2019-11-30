#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include "../shim.h"

FILE* shim_fopen_impl(const char* path, const char* mode) {

  if (str_starts_with(path, "/proc/")) {

    if (strcmp("/proc/driver/nvidia/params", path) == 0) {

      assert(strcmp(mode, "r") == 0);

      char str[] = "ModifyDeviceFiles: 0\n";

      FILE* mem = fmemopen(0, sizeof(str), "r+");
      fwrite(str, sizeof(char), sizeof(str), mem);
      rewind(mem);

      return mem;
    }

    errno = EACCES;
    return NULL;
  }

  if (str_starts_with(path, "/sys/")) {
    errno = EACCES;
    return NULL;
  }

  return fopen(path, mode);
}

int shim_remove_impl(const char* path) {
  assert(!str_starts_with(path, "/dev/"));
  return remove(path);
}

int shim___isoc99_fscanf(FILE* restrict stream, const char* restrict format, ...) {
  LOG_ENTRY("%p, \"%.100s\", ...", stream, format);
  va_list args;
  va_start(args, format);
  int nitems = vfscanf(stream, format, args);
  va_end(args);
  LOG_EXIT("%d", nitems);
  return nitems;
}
