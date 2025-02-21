#include <ucontext.h>
#include "../shim.h"

// Breakpad
int shim_getcontext_impl(ucontext_t* ucp) {
  return -1;
}

SHIM_WRAP(getcontext);
