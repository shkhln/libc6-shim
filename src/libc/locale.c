#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "../shim.h"
#include "locale.h"

extern const unsigned short** shim___ctype_b_loc();
extern int32_t** shim___ctype_tolower_loc();
extern int32_t** shim___ctype_toupper_loc();

static struct linux_lconv _fake_linux_lconv = {0};

linux_locale_t shim___newlocale_impl(int category_mask, const char* locale, linux_locale_t base) {

  assert(category_mask == 64 || category_mask == 8127); // LC_ALL_MASK
  assert(strcmp(locale, "C") == 0);

  linux_locale_t linux_locale = malloc(sizeof(struct linux_locale));

  linux_locale->__ctype_b       = *shim___ctype_b_loc();
  linux_locale->__ctype_tolower = *shim___ctype_tolower_loc();
  linux_locale->__ctype_toupper = *shim___ctype_toupper_loc();

  for (int i = 0; i < 13; i++) {
    linux_locale->__locales[i] = (void*)(uintptr_t)(0xdead0000 + i);
    linux_locale->__names[i]   = "<dummy locale name>";
  }

  linux_locale->native_locale = newlocale(LC_ALL_MASK, locale, base != NULL ? base->native_locale : NULL);

  return linux_locale;
}

linux_locale_t shim___duplocale_impl(linux_locale_t locale) {
  linux_locale_t duplicate = malloc(sizeof(struct linux_locale));
  memcpy(duplicate, locale, sizeof(struct linux_locale));
  duplicate->native_locale = duplocale(locale->native_locale);
  return duplicate;
}

__thread linux_locale_t thread_locale = LINUX_LC_GLOBAL_LOCALE;

linux_locale_t shim___uselocale_impl(linux_locale_t locale) {

  linux_locale_t prev = thread_locale;

  if (locale != NULL) {

    thread_locale = locale;

    if (locale == LINUX_LC_GLOBAL_LOCALE) {
      uselocale(LC_GLOBAL_LOCALE);
    } else {
      uselocale(locale->native_locale);
    }
  }

  return prev;
}

void shim___freelocale_impl(linux_locale_t locale) {
  freelocale(locale->native_locale);
  free(locale);
}

linux_lconv_t shim___localeconv_impl(void) {
  struct lconv * _freebsd_lconv = localeconv();

  _fake_linux_lconv.decimal_point = _freebsd_lconv->decimal_point;
  _fake_linux_lconv.thousands_sep = _freebsd_lconv->thousands_sep;
  _fake_linux_lconv.grouping = _freebsd_lconv->grouping;
  _fake_linux_lconv.mon_decimal_point = _freebsd_lconv->mon_decimal_point;
  _fake_linux_lconv.mon_thousands_sep = _freebsd_lconv->mon_thousands_sep;
  _fake_linux_lconv.mon_grouping = _freebsd_lconv->mon_grouping;
  _fake_linux_lconv.positive_sign = _freebsd_lconv->positive_sign;
  _fake_linux_lconv.negative_sign = _freebsd_lconv->negative_sign;
  _fake_linux_lconv.currency_symbol = _freebsd_lconv->currency_symbol;
  _fake_linux_lconv.frac_digits = _freebsd_lconv->frac_digits;
  _fake_linux_lconv.p_cs_precedes = _freebsd_lconv->p_cs_precedes;
  _fake_linux_lconv.n_cs_precedes = _freebsd_lconv->n_cs_precedes;
  _fake_linux_lconv.p_sep_by_space = _freebsd_lconv->p_sep_by_space;
  _fake_linux_lconv.n_sep_by_space = _freebsd_lconv->n_sep_by_space;
  _fake_linux_lconv.p_sign_posn = _freebsd_lconv->p_sign_posn;
  _fake_linux_lconv.n_sign_posn = _freebsd_lconv->n_sign_posn;
  _fake_linux_lconv.int_curr_symbol = _freebsd_lconv->int_curr_symbol;
  _fake_linux_lconv.int_frac_digits = _freebsd_lconv->int_frac_digits;
  _fake_linux_lconv.int_p_cs_precedes = _freebsd_lconv->int_p_cs_precedes;
  _fake_linux_lconv.int_n_cs_precedes = _freebsd_lconv->int_n_cs_precedes;
  _fake_linux_lconv.int_p_sep_by_space = _freebsd_lconv->int_p_sep_by_space;
  _fake_linux_lconv.int_n_sep_by_space = _freebsd_lconv->int_n_sep_by_space;
  _fake_linux_lconv.int_p_sign_posn = _freebsd_lconv->int_p_sign_posn;
  _fake_linux_lconv.int_n_sign_posn = _freebsd_lconv->int_n_sign_posn;

  return (linux_lconv_t)&_fake_linux_lconv;
}

SHIM_WRAP(__newlocale);
SHIM_WRAP(__duplocale);
SHIM_WRAP(__uselocale);
SHIM_WRAP(__freelocale);
SHIM_WRAP(__localeconv);

linux_locale_t shim_newlocale_impl(int mask, const char* locale, linux_locale_t base) {
  return shim___newlocale_impl(mask, locale, base);
}

linux_locale_t shim_duplocale_impl(linux_locale_t locale) {
  return shim___duplocale_impl(locale);
}

linux_locale_t shim_uselocale_impl(linux_locale_t locale) {
  return shim___uselocale_impl(locale);
}

void shim_freelocale_impl(linux_locale_t locale) {
  shim___freelocale_impl(locale);
}

linux_lconv_t shim_localeconv_impl(void) {
  return shim___localeconv_impl();
}

SHIM_WRAP(newlocale);
SHIM_WRAP(duplocale);
SHIM_WRAP(uselocale);
SHIM_WRAP(freelocale);
SHIM_WRAP(localeconv);
