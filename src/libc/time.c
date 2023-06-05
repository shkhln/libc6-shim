#include <string.h>
#include <xlocale.h>
#include "time.h"
#include "../shim.h"
#include "locale.h"

long int shim_timezone = 0;
SHIM_EXPORT(timezone);

extern long int shim___timezone __attribute__((alias("shim_timezone")));
SHIM_EXPORT(__timezone);

static char shim_tzname_0[] = "GMT";
static char shim_tzname_1[] = "GMT";

char* shim_tzname[2] = {shim_tzname_0, shim_tzname_1};
SHIM_EXPORT(tzname);

extern char** shim___tzname __attribute__((alias("shim_tzname")));
SHIM_EXPORT(__tzname);

void shim_tzset_impl() {
  tzset();
  strlcpy(shim_tzname_0, tzname[0], sizeof(shim_tzname_0));
  strlcpy(shim_tzname_1, tzname[1], sizeof(shim_tzname_1));
}

SHIM_WRAP(tzset);

static clockid_t linux_to_native_clockid(linux_clockid_t linux_clock_id) {
  switch (linux_clock_id) {
    case LINUX_CLOCK_REALTIME:         return CLOCK_REALTIME;
    case LINUX_CLOCK_MONOTONIC:        return CLOCK_MONOTONIC;
    case LINUX_CLOCK_MONOTONIC_RAW:    return CLOCK_MONOTONIC_FAST;
    case LINUX_CLOCK_REALTIME_COARSE:  return CLOCK_REALTIME_FAST;
    case LINUX_CLOCK_MONOTONIC_COARSE: return CLOCK_MONOTONIC_FAST;
    case LINUX_CLOCK_BOOTTIME:         return CLOCK_BOOTTIME;
    default:
      UNIMPLEMENTED_ARGS("%d", linux_clock_id);
  }
}

int shim_clock_gettime_impl(linux_clockid_t linux_clock_id, linux_timespec* tp) {
  return clock_gettime(linux_to_native_clockid(linux_clock_id),  tp);
}

SHIM_WRAP(clock_gettime);

size_t shim_strftime_l_impl(char* restrict buf, size_t maxsize, const char* restrict format, const linux_tm* restrict timeptr, linux_locale_t loc) {
  return strftime_l(buf, maxsize, format, timeptr, loc->native_locale);
}

char* shim_strptime_l_impl(const char* restrict buf, const char* restrict format, linux_tm* restrict timeptr, linux_locale_t loc) {
  return strptime_l(buf, format, timeptr, loc->native_locale);
}

SHIM_WRAP(strftime_l);
SHIM_WRAP(strptime_l);
