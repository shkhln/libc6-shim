#include <dlfcn.h>
#include <errno.h>
#include <sys/elf.h>
#include "../../shim.h"

#define LINUX_ENFILE 23

static int dummy() {
  errno = LINUX_ENFILE;
  return -1;
}

static int (*libinotify_inotify_init)     (void);
static int (*libinotify_inotify_init1)    (int);
static int (*libinotify_inotify_add_watch)(int, const char*, uint32_t);
static int (*libinotify_inotify_rm_watch) (int, int);

__attribute__((constructor))
static void init() {
  void* libinotify = dlopen("libinotify.so.0", RTLD_LAZY);
  if (libinotify != NULL) {
    libinotify_inotify_init      = dlsym(libinotify, "inotify_init");      assert(libinotify_inotify_init      != NULL);
    libinotify_inotify_init1     = dlsym(libinotify, "inotify_init1");     assert(libinotify_inotify_init1     != NULL);
    libinotify_inotify_add_watch = dlsym(libinotify, "inotify_add_watch"); assert(libinotify_inotify_add_watch != NULL);
    libinotify_inotify_rm_watch  = dlsym(libinotify, "inotify_rm_watch");  assert(libinotify_inotify_rm_watch  != NULL);
  } else {
    Link_map* link_map = NULL;

    int err = dlinfo(RTLD_SELF, RTLD_DI_LINKMAP, &link_map);
    assert(err == 0);

    fprintf(stderr, "%s: unable to load libinotify.so.0 (%s)\n", link_map->l_name, dlerror());

    libinotify_inotify_init      = dummy;
    libinotify_inotify_init1     = dummy;
    libinotify_inotify_add_watch = dummy;
    libinotify_inotify_rm_watch  = dummy;
  }
}

static int shim_inotify_init_impl() {
  return libinotify_inotify_init();
}

SHIM_WRAP(inotify_init);

static int shim_inotify_init1_impl(int flags) {
  return libinotify_inotify_init1(flags);
}

SHIM_WRAP(inotify_init1);

static int shim_inotify_add_watch_impl(int fd, const char* path, uint32_t mask) {
  return libinotify_inotify_add_watch(fd, path, mask);
}

SHIM_WRAP(inotify_add_watch);

static int shim_inotify_rm_watch_impl(int fd, int wd) {
  return libinotify_inotify_rm_watch(fd, wd);
}

SHIM_WRAP(inotify_rm_watch);
