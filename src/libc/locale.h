#include <locale.h>

struct linux_locale {
  void*                 __locales[13];
  const unsigned short* __ctype_b;
  const int*            __ctype_tolower;
  const int*            __ctype_toupper;
  const char*           __names[13];
  locale_t              native_locale;
};

struct linux_lconv {
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

typedef struct linux_locale* linux_locale_t;
typedef struct linux_lconv* linux_lconv_t;
#define LINUX_LC_GLOBAL_LOCALE ((linux_locale_t)-1)
