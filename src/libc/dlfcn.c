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

#define LINUX_RTLD_DI_LINKMAP 2

static int shim_dladdr1_impl(void* address, Dl_info* info, void** extra_info, int extra_info_type) {

  int dladdr_err = dladdr(address, info);
  if (dladdr_err != 0) {

    LOG("%s: \"%s\", %p, \"%s\", %p", __func__, info->dli_fname, info->dli_fbase, info->dli_sname, info->dli_saddr);

    switch (extra_info_type) {
      case LINUX_RTLD_DI_LINKMAP:

        assert(info->dli_fname != NULL);

        void* handle = dlopen(info->dli_fname, RTLD_NOLOAD);
        if (handle != NULL) {
          int dlinfo_err = dlinfo(handle, RTLD_DI_LINKMAP, extra_info);
          dlclose(handle); // ?
          if (dlinfo_err != 0) {
            return 0;
          }
        } else {
          *extra_info = NULL; // ?
        }

        break;

      default:
        assert(0);
    }
  }

  return dladdr_err;
}

static void* shim_dlopen_impl(const char* path, int linux_mode) {

  assert((linux_mode & KNOWN_LINUX_DLOPEN_MODE_FLAGS) == linux_mode);

  int mode = linux_mode & ~LINUX_RTLD_NOLOAD;

  if (linux_mode & LINUX_RTLD_NOLOAD) {
    mode |= RTLD_NOLOAD;
  }

  void* p = dlopen(path, mode);

#ifdef DEBUG
  if (p == NULL) {
    LOG("%s: %s", __func__, dlerror());
  }
#endif

  return p;
}

typedef long int Lmid_t;

static void* shim_dlmopen_impl(Lmid_t lmid, const char* path, int mode) {
  return shim_dlopen_impl(path, mode);
}

#define GLIBC_RTLD_NEXT    ((void*)-1)
#define GLIBC_RTLD_DEFAULT ((void*) 0)

static void* _shim_dlvsym(void* linux_handle, const char* symbol, const char* version) {

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

  void* handle;
  switch ((uintptr_t)linux_handle) {
    case  (uintptr_t)GLIBC_RTLD_DEFAULT: handle = RTLD_DEFAULT; break;
    case  (uintptr_t)GLIBC_RTLD_NEXT:    handle = RTLD_NEXT;    break;
    default:
      handle = linux_handle;
  }

  dlerror(); // reset error

  if (version == NULL) {
    return dlsym(handle, symbol);
  } else {
    return dlvsym(handle, symbol, version);
  }
}

static void* shim_dlsym_impl(void* handle, const char* symbol) {
  return _shim_dlvsym(handle, symbol, NULL);
}

static void* shim_dlvsym_impl(void* handle, const char* symbol, const char* version) {
  return _shim_dlvsym(handle, symbol, version);
}

SHIM_WRAP(dladdr1);
SHIM_WRAP(dlopen);
SHIM_WRAP(dlmopen);
SHIM_WRAP(dlsym);
SHIM_WRAP(dlvsym);

static int shim_dlinfo_impl(void* restrict handle, int request, void* restrict p) {
  UNIMPLEMENTED();
}

SHIM_WRAP(dlinfo);
