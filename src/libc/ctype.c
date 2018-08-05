#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include "../shim.h"

const unsigned short* shim_b = (void*)0x4242; // ?

SYM_EXPORT(shim_b, __ctype_b);

size_t shim_get_mb_cur_max() {
  UNIMPLEMENTED();
}

SYM_EXPORT(shim_get_mb_cur_max, __ctype_get_mb_cur_max);
