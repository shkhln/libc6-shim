#include <assert.h>
#include <stdio.h>
#include <execinfo.h>
#include "../shim.h"

size_t shim_backtrace_impl(void** addrlist, size_t len) {
  UNIMPLEMENTED();
}
