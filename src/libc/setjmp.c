#include <setjmp.h>
#include "../shim.h"

int shim_setjmp(jmp_buf env) {
  return 0;
}

int shim__setjmp(jmp_buf env) {
  return 0;
}

int shim___sigsetjmp(jmp_buf env, int savemask) {
  return 0;
}

void shim_longjmp_impl(jmp_buf env, int val) {
  UNIMPLEMENTED();
}

void shim__longjmp_impl(jmp_buf env, int val) {
  UNIMPLEMENTED();
}

void shim_siglongjmp_impl(sigjmp_buf env, int val) {
  UNIMPLEMENTED();
}
