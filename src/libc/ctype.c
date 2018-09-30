#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include "../shim.h"

__asm__(".symver shim_ctype_b,__ctype_b@GLIBC_2.0");
__asm__(".symver shim_ctype_b,__ctype_b@GLIBC_2.2.5");
const unsigned short* shim_ctype_b = (void*)0x4242; // ?

size_t shim___ctype_get_mb_cur_max_impl() {
  UNIMPLEMENTED();
}
