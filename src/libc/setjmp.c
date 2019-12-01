#include <setjmp.h>
#include "../shim.h"

int shim___sigsetjmp_impl(jmp_buf env, int savemask) {
  return 0;
}

void shim__longjmp_impl(jmp_buf env, int val) {
  UNIMPLEMENTED();
}

int shim__setjmp_impl(jmp_buf env) {
  return 0;
}

void shim_longjmp_impl(jmp_buf env, int val) {
  UNIMPLEMENTED();
}

int shim_setjmp_impl(jmp_buf env) {
  return 0;
}

void shim_siglongjmp_impl(sigjmp_buf env, int val) {
  UNIMPLEMENTED();
}

SHIM_WRAP(__sigsetjmp);
SHIM_WRAP(_longjmp);
SHIM_WRAP(_setjmp);
SHIM_WRAP(longjmp);
SHIM_WRAP(setjmp);
SHIM_WRAP(siglongjmp);
