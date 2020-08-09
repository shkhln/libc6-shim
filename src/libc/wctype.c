#include <wctype.h>
#include "../shim.h"

wctype_t shim___wctype_l_impl(const char* property, locale_t locale) {
  return wctype_l(property, locale);
}

SHIM_WRAP(__wctype_l);
