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

int shim___isoc99_fscanf_impl(FILE* restrict stream, const char* restrict format, va_list args) {
  return vfscanf(stream, format, args);
}

FILE* shim_fopen_impl(const char* path, const char* mode) {

  if (str_starts_with(path, "/proc/")) {

    if (strcmp(path, "/proc/driver/nvidia/params") == 0) {

      assert(strcmp(mode, "r") == 0);

      char str[] = "ModifyDeviceFiles: 0\n";

      FILE* mem = fmemopen(0, sizeof(str), "r+");
      fwrite(str, sizeof(char), sizeof(str), mem);
      rewind(mem);

      return mem;
    }

    if (strcmp(path, "/proc/self/maps") == 0) {
      return fopen("/dev/null", mode); // necessary for CUDA init
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

FILE* shim_fopen64_impl(const char* path, const char* mode) {
  return shim_fopen_impl(path, mode);
}

int shim_remove_impl(const char* path) {
  assert(!str_starts_with(path, "/dev/"));
  return remove(path);
}

SHIM_WRAP(__isoc99_fscanf);
SHIM_WRAP(fopen);
SHIM_WRAP(fopen64);
SHIM_WRAP(remove);

int shim___printf_chk_impl(int flag, const char* format, va_list args) {
  return vprintf(format, args);
}

SHIM_WRAP(__printf_chk);
