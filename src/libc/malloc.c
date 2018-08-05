#include "../shim.h"

void* shim_mallinfo() {
  UNIMPLEMENTED();
}

SYM_EXPORT(shim_mallinfo, mallinfo);
