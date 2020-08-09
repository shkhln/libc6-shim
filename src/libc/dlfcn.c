#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../shim.h"

#define LINUX_RTLD_LOCAL    RTLD_LOCAL
#define LINUX_RTLD_LAZY     RTLD_LAZY
#define LINUX_RTLD_NOW      RTLD_NOW
#define LINUX_RTLD_NOLOAD   0x4
#define LINUX_RTLD_GLOBAL   RTLD_GLOBAL
#define LINUX_RTLD_NODELETE RTLD_NODELETE

#define KNOWN_LINUX_DLOPEN_MODE_FLAGS ( \
 LINUX_RTLD_LOCAL    |                  \
 LINUX_RTLD_LAZY     |                  \
 LINUX_RTLD_NOW      |                  \
 LINUX_RTLD_NOLOAD   |                  \
 LINUX_RTLD_GLOBAL   |                  \
 LINUX_RTLD_NODELETE                    \
)

int shim_dladdr1_impl(void* address, Dl_info* info, void** extra_info, int flags) {
  return dladdr(address, info);
}

void* shim_dlopen_impl(const char* path, int linux_mode) {

  assert((linux_mode & KNOWN_LINUX_DLOPEN_MODE_FLAGS) == linux_mode);

  int mode = linux_mode & ~LINUX_RTLD_NOLOAD;

  if (linux_mode & LINUX_RTLD_NOLOAD) {
    mode |= RTLD_NOLOAD;
  }

  void* p = dlopen(path, mode);

#ifdef DEBUG
  if (p == NULL) {
    LOG("%s: %s\n", __func__, dlerror());
  }
#endif

  return p;
}

typedef long int Lmid_t;

void* shim_dlmopen_impl(Lmid_t lmid, const char* path, int mode) {
  return shim_dlopen_impl(path, mode);
}

#define GLIBC_RTLD_NEXT    ((void*)-1)
#define GLIBC_RTLD_DEFAULT ((void*) 0)

void* shim_dlsym_impl(void* handle, const char* symbol) {

  char buf[100];
  snprintf(buf, sizeof(buf), "shim_%s", symbol);

  void* shim_fn = dlsym(NULL, buf);
  if (shim_fn) {
    LOG("%s: substituting %s with %s", __func__, symbol, buf);
    return shim_fn;
  }

  // ?
  if (strcmp(symbol, "__malloc_hook")   == 0) return NULL;
  if (strcmp(symbol, "__realloc_hook")  == 0) return NULL;
  if (strcmp(symbol, "__free_hook")     == 0) return NULL;
  if (strcmp(symbol, "__memalign_hook") == 0) return NULL;

  if (handle == GLIBC_RTLD_DEFAULT)
    return dlsym(RTLD_DEFAULT, symbol);

  if (handle == GLIBC_RTLD_NEXT)
    return dlsym(RTLD_NEXT, symbol);

  return dlsym(handle, symbol);
}

void* shim_dlvsym_impl(void* handle, const char* symbol, const char* version) {
  return shim_dlsym_impl(handle, symbol);
}

SHIM_WRAP(dladdr1);
SHIM_WRAP(dlopen);
SHIM_WRAP(dlmopen);
SHIM_WRAP(dlsym);
SHIM_WRAP(dlvsym);

int shim_dlinfo_impl(void* restrict handle, int request, void* restrict p) {
  UNIMPLEMENTED();
}

SHIM_WRAP(dlinfo);
