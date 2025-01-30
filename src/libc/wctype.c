#include <wctype.h>
#include "../shim.h"
#include "locale.h"

static wctype_t shim___wctype_l_impl(const char* property, linux_locale_t locale) {
  return wctype_l(property, locale->native_locale);
}

SHIM_WRAP(__wctype_l);

static int shim_iswalnum_l_impl(wint_t wc, linux_locale_t loc) {
  return iswalnum_l(wc, loc->native_locale);
}

static int shim_iswalpha_l_impl(wint_t wc, linux_locale_t loc) {
  return iswalpha_l(wc, loc->native_locale);
}

static int shim_iswblank_l_impl(wint_t wc, linux_locale_t loc) {
  return iswblank_l(wc, loc->native_locale);
}

static int shim_iswcntrl_l_impl(wint_t wc, linux_locale_t loc) {
  return iswcntrl_l(wc, loc->native_locale);
}

static int shim_iswdigit_l_impl(wint_t wc, linux_locale_t loc) {
  return iswdigit_l(wc, loc->native_locale);
}

static int shim_iswgraph_l_impl(wint_t wc, linux_locale_t loc) {
  return iswgraph_l(wc, loc->native_locale);
}

static int shim_iswlower_l_impl(wint_t wc, linux_locale_t loc) {
  return iswlower_l(wc, loc->native_locale);
}

static int shim_iswprint_l_impl(wint_t wc, linux_locale_t loc) {
  return iswprint_l(wc, loc->native_locale);
}

static int shim_iswpunct_l_impl(wint_t wc, linux_locale_t loc) {
  return iswpunct_l(wc, loc->native_locale);
}

static int shim_iswspace_l_impl(wint_t wc, linux_locale_t loc) {
  return iswspace_l(wc, loc->native_locale);
}

static int shim_iswupper_l_impl(wint_t wc, linux_locale_t loc) {
  return iswupper_l(wc, loc->native_locale);
}

static int shim_iswxdigit_l_impl(wint_t wc, linux_locale_t loc) {
  return iswxdigit_l(wc, loc->native_locale);
}

SHIM_WRAP(iswalnum_l);
SHIM_WRAP(iswalpha_l);
SHIM_WRAP(iswblank_l);
SHIM_WRAP(iswcntrl_l);
SHIM_WRAP(iswdigit_l);
SHIM_WRAP(iswgraph_l);
SHIM_WRAP(iswlower_l);
SHIM_WRAP(iswprint_l);
SHIM_WRAP(iswpunct_l);
SHIM_WRAP(iswspace_l);
SHIM_WRAP(iswupper_l);
SHIM_WRAP(iswxdigit_l);

static wint_t shim_towctrans_l_impl(wint_t wc, wctrans_t _arg_2, linux_locale_t loc) {
  return towctrans_l(wc, _arg_2, loc->native_locale);
}

static wint_t shim_towlower_l_impl(wint_t wc, linux_locale_t loc) {
  return towlower_l(wc, loc->native_locale);
}

static wint_t shim_towupper_l_impl(wint_t wc, linux_locale_t loc) {
  return towupper_l(wc, loc->native_locale);
}

static wctrans_t shim_wctrans_l_impl(const char* _arg_1, linux_locale_t loc) {
  return wctrans_l(_arg_1, loc->native_locale);
}

SHIM_WRAP(towctrans_l);
SHIM_WRAP(towlower_l);
SHIM_WRAP(towupper_l);
SHIM_WRAP(wctrans_l);
