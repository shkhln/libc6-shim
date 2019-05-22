#include <math.h>
#include <stdint.h>
#include "../shim.h"

int shim___isinf_impl(double arg) {
  return isinf(arg);
}

int shim___isinff_impl(float arg) {
  return isinf(arg);
}

int shim___isnan_impl(double arg) {
  return isnan(arg);
}

int shim___isnanf_impl(float arg) {
  return isnanf(arg);
}

int shim_finite_impl(double x) {
  return isfinite(x);
}

#ifdef __i386__

__asm__(".symver shim___divdi3,__divdi3@GLIBC_2.0");
int64_t shim___divdi3(int64_t a, int64_t b) {
  LOG_ARGS("%lld, %lld", a, b);
  int64_t ret = a / b;
  LOG_RES("%lld", ret);
  return ret;
}

__asm__(".symver shim___udivdi3,__udivdi3@GLIBC_2.0");
uint64_t shim___udivdi3(uint64_t a, uint64_t b) {
  LOG_ARGS("%llu, %llu", a, b);
  uint64_t ret = a / b;
  LOG_RES("%llu", ret);
  return ret;
}

__asm__(".symver shim___umoddi3,__umoddi3@GLIBC_2.0");
uint64_t shim___umoddi3(uint64_t a, uint64_t b) {
  LOG_ARGS("%llu, %llu", a, b);
  uint64_t ret = a % b;
  LOG_RES("%llu", ret);
  return ret;
}

#endif
