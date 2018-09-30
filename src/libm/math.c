#include <math.h>
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
