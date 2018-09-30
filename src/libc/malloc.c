#include "../shim.h"

__asm__(".symver shim_mallinfo,mallinfo@GLIBC_2.0");
__asm__(".symver shim_mallinfo,mallinfo@GLIBC_2.2.5");
void* shim_mallinfo() {
  UNIMPLEMENTED();
}
