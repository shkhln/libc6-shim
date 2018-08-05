#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include "../shim.h"

FILE* shim_fopen_impl(const char* path, const char* mode) {

  if (str_starts_with(path, "/proc/")) {

    if (strcmp("/proc/driver/nvidia/params", path) == 0) {

      assert(strcmp(mode, "r") == 0);

      uint32_t value = 0;
      size_t   sz    = sizeof(value);

      int err = sysctlbyname("hw.nvidia.registry.ModifyDeviceFiles", &value, &sz, NULL, 0);
      assert(err == 0);

      LOG("shim_fopen: ModifyDeviceFiles = %d\n", value);

      char buf[250];
      int pos = 0;

      pos += snprintf(&buf[pos], sizeof(buf) - pos, "ModifyDeviceFiles: %d\n", value);

      FILE* mem = fmemopen(0, pos, "r+");
      fwrite(buf, sizeof(char), pos, mem);
      rewind(mem);

      return mem;
    }

    errno = EACCES;
    return NULL;
  }

  if (str_starts_with(path, "/sys/")) {

    if (strcmp("/sys/devices/system/node/node0/meminfo", path) == 0) {

      assert(strcmp(mode, "r") == 0);

      uint64_t phys_mem = 0;
      {
        size_t sz = sizeof(phys_mem);

        int err = sysctlbyname("hw.physmem", &phys_mem, &sz, NULL, 0);
        assert(err == 0);
      }

      uint32_t wire_count = 0;
      {
        size_t sz = sizeof(wire_count);

        int err = sysctlbyname("vm.stats.vm.v_wire_count", &wire_count, &sz, NULL, 0);
        assert(err == 0);
      }

      uint32_t page_size = 0;
      {
        size_t sz = sizeof(page_size);

        int err = sysctlbyname("vm.stats.vm.v_page_size", &page_size, &sz, NULL, 0);
        assert(err == 0);
      }

      uint64_t free_mem = phys_mem - wire_count * page_size;

      LOG("%s: MemTotal = %9llu kB\n", __func__, phys_mem / 1024LL);
      LOG("%s: MemFree  = %9llu kB\n", __func__, free_mem / 1024LL);

      char buf[250];
      int pos = 0;

      pos += snprintf(&buf[pos], sizeof(buf) - pos, "Node 0 %-10s%9llu kB\n", "MemTotal:", phys_mem / 1024LL);
      pos += snprintf(&buf[pos], sizeof(buf) - pos, "Node 0 %-10s%9llu kB\n", "MemFree:",  free_mem / 1024LL);

      FILE* mem = fmemopen(0, pos, "r+");
      fwrite(buf, sizeof(char), pos, mem);
      rewind(mem);

      return mem;
    }

    errno = EACCES;
    return NULL;
  }

  return fopen(path, mode);
}

int shim_remove_impl(const char* path) {
  assert(!str_starts_with(path, "/dev/"));
  return remove(path);
}
