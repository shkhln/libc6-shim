#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "../shim.h"
#include "locale.h"

extern const unsigned short** shim___ctype_b_loc();
extern int32_t** shim___ctype_tolower_loc();
extern int32_t** shim___ctype_toupper_loc();

linux_locale_t shim___newlocale_impl(int category_mask, const char* locale, linux_locale_t base) {

  assert(category_mask == 64); // LC_ALL_MASK
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

SHIM_WRAP(__newlocale);
SHIM_WRAP(__duplocale);
SHIM_WRAP(__uselocale);
SHIM_WRAP(__freelocale);

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

SHIM_WRAP(newlocale);
SHIM_WRAP(duplocale);
SHIM_WRAP(uselocale);
SHIM_WRAP(freelocale);
