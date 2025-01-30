#include <setjmp.h>
#include "../shim.h"

#ifdef __i386__
  _Static_assert(sizeof(jmp_buf)    <= 156, "");
  _Static_assert(sizeof(sigjmp_buf) <= 156, "");
#endif

#ifdef __x86_64__
  _Static_assert(sizeof(jmp_buf)    <= 200, "");
  _Static_assert(sizeof(sigjmp_buf) <= 200, "");
#endif

int shim_setjmp(jmp_buf env) {
  UNIMPLEMENTED();
}

__attribute__((naked))
int shim__setjmp(jmp_buf env) {
  __asm__("jmp _setjmp");
}

__attribute__((naked))
int shim___sigsetjmp(jmp_buf env, int savemask) {
  __asm__("jmp sigsetjmp");
}

SHIM_EXPORT(setjmp);
SHIM_EXPORT(_setjmp);
SHIM_EXPORT(__sigsetjmp);

static void shim_longjmp_impl(jmp_buf env, int val) {
  //~ fprintf(stderr, "[[%s]]\n", __func__);
  _longjmp(env, val);
}

static void shim__longjmp_impl(jmp_buf env, int val) {
  //~ fprintf(stderr, "[[%s]]\n", __func__);
  _longjmp(env, val);
}

static void shim_siglongjmp_impl(sigjmp_buf env, int val) {
  //~ fprintf(stderr, "[[%s]]\n", __func__);
  siglongjmp(env, val);
}

SHIM_WRAP(longjmp);
SHIM_WRAP(_longjmp);
SHIM_WRAP(siglongjmp);
