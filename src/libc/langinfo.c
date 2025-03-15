#include <langinfo.h>
#include "../shim.h"
#include "locale.h"

#define LINUX_CODESET          14
#define LINUX_D_T_FMT     0x20028
#define LINUX_D_FMT       0x20029
#define LINUX_T_FMT       0x2002a
#define LINUX_T_FMT_AMPM  0x2002b
#define LINUX_AM_STR      0x20026
#define LINUX_PM_STR      0x20027
#define LINUX_DAY_1       0x20007
#define LINUX_DAY_2       0x20008
#define LINUX_DAY_3       0x20009
#define LINUX_DAY_4       0x2000a
#define LINUX_DAY_5       0x2000b
#define LINUX_DAY_6       0x2000c
#define LINUX_DAY_7       0x2000d
#define LINUX_ABDAY_1     0x20000
#define LINUX_ABDAY_2     0x20001
#define LINUX_ABDAY_3     0x20002
#define LINUX_ABDAY_4     0x20003
#define LINUX_ABDAY_5     0x20004
#define LINUX_ABDAY_6     0x20005
#define LINUX_ABDAY_7     0x20006
#define LINUX_MON_1       0x2001a
#define LINUX_MON_2       0x2001b
#define LINUX_MON_3       0x2001c
#define LINUX_MON_4       0x2001d
#define LINUX_MON_5       0x2001e
#define LINUX_MON_6       0x2001f
#define LINUX_MON_7       0x20020
#define LINUX_MON_8       0x20021
#define LINUX_MON_9       0x20022
#define LINUX_MON_10      0x20023
#define LINUX_MON_11      0x20024
#define LINUX_MON_12      0x20025
#define LINUX_ABMON_1     0x2000e
#define LINUX_ABMON_2     0x2000f
#define LINUX_ABMON_3     0x20010
#define LINUX_ABMON_4     0x20011
#define LINUX_ABMON_5     0x20012
#define LINUX_ABMON_6     0x20013
#define LINUX_ABMON_7     0x20014
#define LINUX_ABMON_8     0x20015
#define LINUX_ABMON_9     0x20016
#define LINUX_ABMON_10    0x20017
#define LINUX_ABMON_11    0x20018
#define LINUX_ABMON_12    0x20019
#define LINUX_ERA         0x2002c
#define LINUX_ERA_D_FMT   0x2002e
#define LINUX_ERA_D_T_FMT 0x20030
#define LINUX_ERA_T_FMT   0x20031
#define LINUX_ALT_DIGITS  0x2002f
#define LINUX_RADIXCHAR   0x10000
#define LINUX_THOUSEP     0x10001
#define LINUX_YESEXPR     0x50000
#define LINUX_NOEXPR      0x50001
#define LINUX_CRNCYSTR    0x4000f

static nl_item linux_to_native_nl_item(nl_item linux_item) {
  switch (linux_item) {
    case LINUX_CODESET:     return CODESET;
    case LINUX_D_T_FMT:     return D_T_FMT;
    case LINUX_D_FMT:       return D_FMT;
    case LINUX_T_FMT:       return T_FMT;
    case LINUX_T_FMT_AMPM:  return T_FMT_AMPM;
    case LINUX_AM_STR:      return AM_STR;
    case LINUX_PM_STR:      return PM_STR;
    case LINUX_DAY_1:       return DAY_1;
    case LINUX_DAY_2:       return DAY_2;
    case LINUX_DAY_3:       return DAY_3;
    case LINUX_DAY_4:       return DAY_4;
    case LINUX_DAY_5:       return DAY_5;
    case LINUX_DAY_6:       return DAY_6;
    case LINUX_DAY_7:       return DAY_7;
    case LINUX_ABDAY_1:     return ABDAY_1;
    case LINUX_ABDAY_2:     return ABDAY_2;
    case LINUX_ABDAY_3:     return ABDAY_3;
    case LINUX_ABDAY_4:     return ABDAY_4;
    case LINUX_ABDAY_5:     return ABDAY_5;
    case LINUX_ABDAY_6:     return ABDAY_6;
    case LINUX_ABDAY_7:     return ABDAY_7;
    case LINUX_MON_1:       return MON_1;
    case LINUX_MON_2:       return MON_2;
    case LINUX_MON_3:       return MON_3;
    case LINUX_MON_4:       return MON_4;
    case LINUX_MON_5:       return MON_5;
    case LINUX_MON_6:       return MON_6;
    case LINUX_MON_7:       return MON_7;
    case LINUX_MON_8:       return MON_8;
    case LINUX_MON_9:       return MON_9;
    case LINUX_MON_10:      return MON_10;
    case LINUX_MON_11:      return MON_11;
    case LINUX_MON_12:      return MON_12;
    case LINUX_ABMON_1:     return ABMON_1;
    case LINUX_ABMON_2:     return ABMON_2;
    case LINUX_ABMON_3:     return ABMON_3;
    case LINUX_ABMON_4:     return ABMON_4;
    case LINUX_ABMON_5:     return ABMON_5;
    case LINUX_ABMON_6:     return ABMON_6;
    case LINUX_ABMON_7:     return ABMON_7;
    case LINUX_ABMON_8:     return ABMON_8;
    case LINUX_ABMON_9:     return ABMON_9;
    case LINUX_ABMON_10:    return ABMON_10;
    case LINUX_ABMON_11:    return ABMON_11;
    case LINUX_ABMON_12:    return ABMON_12;
    case LINUX_ERA:         return ERA;
    case LINUX_ERA_D_FMT:   return ERA_D_FMT;
    case LINUX_ERA_D_T_FMT: return ERA_D_T_FMT;
    case LINUX_ERA_T_FMT:   return ERA_T_FMT;
    case LINUX_ALT_DIGITS:  return ALT_DIGITS;
    case LINUX_RADIXCHAR:   return RADIXCHAR;
    case LINUX_THOUSEP:     return THOUSEP;
    case LINUX_YESEXPR:     return YESEXPR;
    case LINUX_NOEXPR:      return NOEXPR;
    case LINUX_CRNCYSTR:    return CRNCYSTR;
    default:
      PANIC("Unknown linux nl_item: %d", linux_item);
  }
}

static char* shim_nl_langinfo_impl(nl_item linux_item) {
  return nl_langinfo(linux_to_native_nl_item(linux_item));
}

SHIM_WRAP(nl_langinfo);

static char* shim_nl_langinfo_l_impl(nl_item item, linux_locale_t loc) {
  UNIMPLEMENTED();
}

SHIM_WRAP(nl_langinfo_l);
