#include <locale.h>

struct linux_locale {
  void*                 __locales[13];
  const unsigned short* __ctype_b;
  const int*            __ctype_tolower;
  const int*            __ctype_toupper;
  const char*           __names[13];
  locale_t              native_locale;
};

typedef struct linux_locale* linux_locale_t;

#define LINUX_LC_GLOBAL_LOCALE ((linux_locale_t)-1)
