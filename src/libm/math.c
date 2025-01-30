#include <math.h>
#include <stdint.h>
#include "../shim.h"

static int shim___isinf_impl(double arg) {
  return isinf(arg);
}

static int shim___isinff_impl(float arg) {
  return isinf(arg);
}

static int shim___isnan_impl(double arg) {
  return isnan(arg);
}

static int shim___isnanf_impl(float arg) {
  return isnanf(arg);
}

static int shim___finitef_impl(float x) {
  return isfinite(x);
}

static int shim_finite_impl(double x) {
  return isfinite(x);
}

#ifdef __i386__

static int64_t shim___divdi3_impl(int64_t a, int64_t b) {
  return a / b;
}

static uint64_t shim___udivdi3_impl(uint64_t a, uint64_t b) {
  return a / b;
}

static int64_t shim___moddi3_impl(int64_t a, int64_t b) {
  return a % b;
}

static uint64_t shim___umoddi3_impl(uint64_t a, uint64_t b) {
  return a % b;
}

SHIM_WRAP(__divdi3);
SHIM_WRAP(__udivdi3);
SHIM_WRAP(__moddi3);
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

SHIM_WRAP(__finitef);
SHIM_WRAP(finite);
