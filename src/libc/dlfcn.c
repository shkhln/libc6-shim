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

void* shim_dlopen_impl(const char* path, int linux_mode) {

  assert((linux_mode & KNOWN_LINUX_DLOPEN_MODE_FLAGS) == linux_mode);

  int mode = linux_mode & ~LINUX_RTLD_NOLOAD;

  if (linux_mode & LINUX_RTLD_NOLOAD) {
    mode |= RTLD_NOLOAD;
  }

  void* p = dlopen(path, mode);

#ifdef DEBUG
  if (p == NULL) {
    fprintf(stderr, "%s: %s\n", __func__, dlerror());
  }
#endif

  return p;
}

__asm__(".symver shim_dladdr1,dladdr1@GLIBC_2.3.3");
int shim_dladdr1(void* address, Dl_info* info, void** extra_info, int flags) {
  LOG_ARGS("%p, %p, %p, %d", address, info, extra_info, flags);
  int err = dladdr(address, info);
  LOG_RES("%d", err);
  return err;
}

#define GLIBC_RTLD_NEXT    ((void*)-1)
#define GLIBC_RTLD_DEFAULT ((void*) 0)

void* shim_memalign      (size_t alignment, size_t size);
int   shim_posix_memalign(void **ptr, size_t alignment, size_t size);

int shim_pthread_setname_np(pthread_t tid, const char* name);
int shim_pthread_getname_np(pthread_t tid, char* name, size_t len);

int shim_register_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void), void* dso_handle);

size_t shim_malloc_usable_size(const void* ptr);
void   shim_free              (void* ptr);
void*  shim_malloc            (size_t size);
void*  shim_realloc           (void* ptr, size_t size);

void* shim_dlsym_impl(void* handle, const char* symbol) {

  if (strcmp(symbol, "dladdr1")            == 0) return &shim_dladdr1;
  if (strcmp(symbol, "malloc_usable_size") == 0) return &shim_malloc_usable_size;
  if (strcmp(symbol, "malloc")             == 0) return &shim_malloc;
  if (strcmp(symbol, "realloc")            == 0) return &shim_realloc;
  if (strcmp(symbol, "free")               == 0) return &shim_free;
  if (strcmp(symbol, "memalign")           == 0) return &shim_memalign;
  if (strcmp(symbol, "__malloc_hook")      == 0) return NULL;
  if (strcmp(symbol, "__realloc_hook")     == 0) return NULL;
  if (strcmp(symbol, "__free_hook")        == 0) return NULL;
  if (strcmp(symbol, "__memalign_hook")    == 0) return NULL;
  if (strcmp(symbol, "posix_memalign")     == 0) return &shim_posix_memalign;
  if (strcmp(symbol, "__register_atfork")  == 0) return &shim_register_atfork;
  if (strcmp(symbol, "pthread_setname_np") == 0) return &shim_pthread_setname_np;
  if (strcmp(symbol, "pthread_getname_np") == 0) return &shim_pthread_getname_np;
  // if (strcmp(symbol, "xf86PciAccInfo")     == 0) return NULL;
  // if (strcmp(symbol, "drmGetVersion")      == 0) return NULL;
  // if (strcmp(symbol, "drmFreeVersion")     == 0) return NULL;
  // if (strcmp(symbol, "drmIoctl")           == 0) return NULL;
  // if (strcmp(symbol, "drmPrimeFDToHandle") == 0) return NULL;

  if (handle == GLIBC_RTLD_DEFAULT)
    return dlsym(RTLD_DEFAULT, symbol);

  if (handle == GLIBC_RTLD_NEXT)
    return dlsym(RTLD_NEXT, symbol);

  return dlsym(handle, symbol);
}

void* shim_dlvsym_impl(void* handle, const char* symbol, const char* version) {
  return shim_dlsym_impl(handle, symbol);
}
