#include <locale.h>
#include "../shim.h"

locale_t shim___newlocale_impl(int category_mask, const char* locale, locale_t base) {
  assert(category_mask == 64); // wtf?
  return newlocale(LC_ALL_MASK, locale, base);
}

SHIM_WRAP(__newlocale);

locale_t shim___uselocale_impl(locale_t locale) {
  return uselocale(locale);
}

SHIM_WRAP(__uselocale);
