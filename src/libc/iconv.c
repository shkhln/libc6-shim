#include <dlfcn.h>
#include <iconv.h>
#include <sys/elf.h>
#include "../shim.h"

static iconv_t (*libiconv_iconv_open) (const char*, const char*);
static int     (*libiconv_iconv_close)(iconv_t);
static size_t  (*libiconv_iconv)      (iconv_t, char** restrict, size_t* restrict, char** restrict, size_t* restrict);

__attribute__((constructor))
static void init() {
  void* libiconv = dlopen("libiconv.so.2", RTLD_LAZY);
  if (libiconv != NULL) {
    libiconv_iconv_open  = dlsym(libiconv, "libiconv_open");  assert(libiconv_iconv_open  != NULL);
    libiconv_iconv_close = dlsym(libiconv, "libiconv_close"); assert(libiconv_iconv_close != NULL);
    libiconv_iconv       = dlsym(libiconv, "libiconv");       assert(libiconv_iconv       != NULL);
  } else {
    Link_map* link_map = NULL;

    int err = dlinfo(RTLD_SELF, RTLD_DI_LINKMAP, &link_map);
    assert(err == 0);

    fprintf(stderr, "%s: unable to load libiconv.so.2 (%s)\n", link_map->l_name, dlerror());

    libiconv_iconv_open  = iconv_open;
    libiconv_iconv_close = iconv_close;
    libiconv_iconv       = iconv;
  }
}

static iconv_t shim_iconv_open_impl(const char* dstname, const char* srcname) {
  return libiconv_iconv_open(dstname, srcname);
}

SHIM_WRAP(iconv_open);

static int shim_iconv_close_impl(iconv_t cd) {
  return libiconv_iconv_close(cd);
}

SHIM_WRAP(iconv_close);

static size_t shim_iconv_impl(iconv_t cd, char** restrict src, size_t* restrict srcleft, char** restrict dst, size_t* restrict dstleft) {
  return libiconv_iconv(cd, src, srcleft, dst, dstleft);
}

SHIM_WRAP(iconv);
