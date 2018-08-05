#include <math.h>
#include "../shim.h"

int shim_isinf(double arg) {
  return isinf(arg);
}

int shim_isinff(float arg) {
  return isinf(arg);
}

int shim_isnan(double arg) {
  return isnan(arg);
}

int shim_isnanf(float arg) {
  return isnanf(arg);
}

SYM_EXPORT(shim_isinf,  __isinf);
SYM_EXPORT(shim_isinff, __isinff);
SYM_EXPORT(shim_isnan,  __isnan);
SYM_EXPORT(shim_isnanf, __isnanf);

int shim_finite(double x) {
  return isfinite(x);
}

SYM_EXPORT(shim_finite, finite);
