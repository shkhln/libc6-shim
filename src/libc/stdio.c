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

  if (strcmp(path, "/proc/driver/nvidia/params") == 0) {

    assert(strcmp(mode, "r") == 0);

    char str[] = "ModifyDeviceFiles: 0\n";

    FILE* mem = fmemopen(0, sizeof(str), "r+");
    fwrite(str, sizeof(char), sizeof(str), mem);
    rewind(mem);

    return mem;
  }

  // CUDA
  if (strncmp(path, "/proc/self/task/", sizeof("/proc/self/task/") - 1) == 0) {
    char* tail = strchr(path + sizeof("/proc/self/task/"), '/');
    if (tail != NULL && strcmp(tail, "/comm") == 0) {
      assert(strcmp(mode, "wb") == 0);
      return fopen("/dev/null", mode);
    }
  }

  char* p = redirect(path);
  if (p == NULL) {
    errno = EACCES;
    return NULL;
  }

  return fopen(p, mode);
}

FILE* shim_fopen64_impl(const char* path, const char* mode) {
  return shim_fopen_impl(path, mode);
}

int shim_remove_impl(const char* path) {
  assert(str_starts_with(path, "/dev/char/195:") || !str_starts_with(path, "/dev/"));
  return remove(path);
}

SHIM_WRAP(__isoc99_fscanf);
SHIM_WRAP(fopen);
SHIM_WRAP(fopen64);
SHIM_WRAP(remove);

int shim___printf_chk_impl(int flag, const char* format, va_list args) {
  return vprintf(format, args);
}

int shim___snprintf_chk_impl(char* str, size_t maxlen, int flag, size_t strlen, const char* format, va_list args) {
  assert(flag == 1 && strlen >= maxlen);
  return vsnprintf(str, maxlen, format, args);
}

int shim___vsnprintf_chk_impl(char* str, size_t maxlen, int flag, size_t strlen, const char* format, va_list args) {
  assert(flag == 1 && strlen >= maxlen);
  return vsnprintf(str, maxlen, format, args);
}

int shim___fprintf_chk_impl(FILE* stream, int flag, const char* format, va_list args) {
  return vfprintf(stream, format, args);
}

int shim___sprintf_chk_impl(char* str, int flag, size_t strlen, const char* format, va_list args) {
  return vsprintf(str, format, args);
}

int shim___vasprintf_chk_impl(char** ret, int flags, const char* format, va_list args) {
  return vasprintf(ret, format, args);
}

int shim___vfprintf_chk_impl(FILE* stream, int flag, const char* format, va_list args) {
  return vfprintf(stream, format, args);
}

SHIM_WRAP(__printf_chk);
SHIM_WRAP(__snprintf_chk);
SHIM_WRAP(__vsnprintf_chk);
SHIM_WRAP(__fprintf_chk);
SHIM_WRAP(__sprintf_chk);
SHIM_WRAP(__vasprintf_chk);
SHIM_WRAP(__vfprintf_chk);

int shim_fgetpos64_impl(FILE* stream, linux_fpos64_t* pos) {
  return fgetpos(stream, pos);
}

SHIM_WRAP(fgetpos64);
