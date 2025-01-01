#include <iconv.h>
#include "../shim.h"

extern iconv_t (*libiconv_iconv_open) (const char*, const char*);
extern int     (*libiconv_iconv_close)(iconv_t);
extern size_t  (*libiconv_iconv)      (iconv_t, char** restrict, size_t* restrict, char** restrict, size_t* restrict);

iconv_t shim_iconv_open_impl(const char* dstname, const char* srcname) {
  return libiconv_iconv_open(dstname, srcname);
}

SHIM_WRAP(iconv_open);

int shim_iconv_close_impl(iconv_t cd) {
  return libiconv_iconv_close(cd);
}

SHIM_WRAP(iconv_close);

size_t shim_iconv_impl(iconv_t cd, char** restrict src, size_t* restrict srcleft, char** restrict dst, size_t* restrict dstleft) {
  return libiconv_iconv(cd, src, srcleft, dst, dstleft);
}

SHIM_WRAP(iconv);
