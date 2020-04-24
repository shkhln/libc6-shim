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

int64_t shim___divdi3_impl(int64_t a, int64_t b) {
  //~ LOG_ENTRY("%lld, %lld", a, b);
  int64_t ret = a / b;
  //~ LOG_EXIT("%lld", ret);
  return ret;
}

uint64_t shim___udivdi3_impl(uint64_t a, uint64_t b) {
  //~ LOG_ENTRY("%llu, %llu", a, b);
  uint64_t ret = a / b;
  //~ LOG_EXIT("%llu", ret);
  return ret;
}

uint64_t shim___umoddi3_impl(uint64_t a, uint64_t b) {
  //~ LOG_ENTRY("%llu, %llu", a, b);
  uint64_t ret = a % b;
  //~ LOG_EXIT("%llu", ret);
  return ret;
}

SHIM_WRAP(__divdi3);
SHIM_WRAP(__udivdi3);
SHIM_WRAP(__umoddi3);

#endif

SHIM_WRAP(__isinf);
SHIM_WRAP(__isinff);

#ifdef SHIM_SCAN
#undef __isnan
#undef __isnanf
SHIM_WRAP(__isnan);
SHIM_WRAP(__isnanf);
#else
SHIM_WRAPPER___isnan
SHIM_WRAPPER___isnanf
#endif

SHIM_WRAP(finite);
