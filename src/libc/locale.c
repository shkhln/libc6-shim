#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "../shim.h"
#include "locale.h"

extern const unsigned short** shim___ctype_b_loc();
extern int32_t** shim___ctype_tolower_loc();
extern int32_t** shim___ctype_toupper_loc();

static linux_locale_t shim___newlocale_impl(int category_mask, const char* locale, linux_locale_t base) {

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

static linux_locale_t shim___duplocale_impl(linux_locale_t locale) {
  linux_locale_t duplicate = malloc(sizeof(struct linux_locale));
  memcpy(duplicate, locale, sizeof(struct linux_locale));
  duplicate->native_locale = duplocale(locale->native_locale);
  return duplicate;
}

__thread linux_locale_t thread_locale = LINUX_LC_GLOBAL_LOCALE;

static linux_locale_t shim___uselocale_impl(linux_locale_t locale) {

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

static void shim___freelocale_impl(linux_locale_t locale) {
  freelocale(locale->native_locale);
  free(locale);
}

SHIM_WRAP(__newlocale);
SHIM_WRAP(__duplocale);
SHIM_WRAP(__uselocale);
SHIM_WRAP(__freelocale);

static linux_locale_t shim_newlocale_impl(int mask, const char* locale, linux_locale_t base) {
  return shim___newlocale_impl(mask, locale, base);
}

static linux_locale_t shim_duplocale_impl(linux_locale_t locale) {
  return shim___duplocale_impl(locale);
}

static linux_locale_t shim_uselocale_impl(linux_locale_t locale) {
  return shim___uselocale_impl(locale);
}

static void shim_freelocale_impl(linux_locale_t locale) {
  shim___freelocale_impl(locale);
}

SHIM_WRAP(newlocale);
SHIM_WRAP(duplocale);
SHIM_WRAP(uselocale);
SHIM_WRAP(freelocale);

struct shim_lconv {
  char *decimal_point;
  char *thousands_sep;
  char *grouping;
  char *int_curr_symbol;
  char *currency_symbol;
  char *mon_decimal_point;
  char *mon_thousands_sep;
  char *mon_grouping;
  char *positive_sign;
  char *negative_sign;
  char int_frac_digits;
  char frac_digits;
  char p_cs_precedes;
  char p_sep_by_space;
  char n_cs_precedes;
  char n_sep_by_space;
  char p_sign_posn;
  char n_sign_posn;
  char int_p_cs_precedes;
  char int_p_sep_by_space;
  char int_n_cs_precedes;
  char int_n_sep_by_space;
  char int_p_sign_posn;
  char int_n_sign_posn;
};

typedef struct shim_lconv linux_lconv;

static linux_lconv process_lconv;

static void set_process_lconv() {
  struct lconv* native_lconv = localeconv();
  assert(native_lconv != NULL);
  process_lconv.decimal_point      = native_lconv->decimal_point;
  process_lconv.thousands_sep      = native_lconv->thousands_sep;
  process_lconv.grouping           = native_lconv->grouping;
  process_lconv.int_curr_symbol    = native_lconv->int_curr_symbol;
  process_lconv.currency_symbol    = native_lconv->currency_symbol;
  process_lconv.mon_decimal_point  = native_lconv->mon_decimal_point;
  process_lconv.mon_thousands_sep  = native_lconv->mon_thousands_sep;
  process_lconv.mon_grouping       = native_lconv->mon_grouping;
  process_lconv.positive_sign      = native_lconv->positive_sign;
  process_lconv.negative_sign      = native_lconv->negative_sign;
  process_lconv.int_frac_digits    = native_lconv->int_frac_digits;
  process_lconv.frac_digits        = native_lconv->frac_digits;
  process_lconv.p_cs_precedes      = native_lconv->p_cs_precedes;
  process_lconv.p_sep_by_space     = native_lconv->p_sep_by_space;
  process_lconv.n_cs_precedes      = native_lconv->n_cs_precedes;
  process_lconv.n_sep_by_space     = native_lconv->n_sep_by_space;
  process_lconv.p_sign_posn        = native_lconv->p_sign_posn;
  process_lconv.n_sign_posn        = native_lconv->n_sign_posn;
  process_lconv.int_p_cs_precedes  = native_lconv->int_p_cs_precedes;
  process_lconv.int_p_sep_by_space = native_lconv->int_p_sep_by_space;
  process_lconv.int_n_cs_precedes  = native_lconv->int_n_cs_precedes;
  process_lconv.int_n_sep_by_space = native_lconv->int_n_sep_by_space;
  process_lconv.int_p_sign_posn    = native_lconv->int_p_sign_posn;
  process_lconv.int_n_sign_posn    = native_lconv->int_n_sign_posn;
}

__attribute__((constructor))
static void init() {
  set_process_lconv();
}

static linux_lconv* shim_localeconv_impl() {
  return &process_lconv;
}

#define LINUX_LC_CTYPE    0
#define LINUX_LC_NUMERIC  1
#define LINUX_LC_TIME     2
#define LINUX_LC_COLLATE  3
#define LINUX_LC_MONETARY 4
#define LINUX_LC_MESSAGES 5
#define LINUX_LC_ALL      6

static int linux_to_native_category(int linux_category) {
  switch (linux_category) {
    case LINUX_LC_CTYPE:    return LC_CTYPE;
    case LINUX_LC_NUMERIC : return LC_NUMERIC;
    case LINUX_LC_TIME:     return LC_TIME;
    case LINUX_LC_COLLATE:  return LC_COLLATE;
    case LINUX_LC_MONETARY: return LC_MONETARY;
    case LINUX_LC_MESSAGES: return LC_MESSAGES;
    case LINUX_LC_ALL:      return LC_ALL;
    default:
      assert(0);
  }
}

static char* shim_setlocale_impl(int linux_category, const char* locale) {
  char* ret = setlocale(linux_to_native_category(linux_category), locale);
  if (ret != NULL) {
    set_process_lconv();
  }
  return ret;
}

SHIM_WRAP(localeconv);
SHIM_WRAP(setlocale);
