#include <string.h>
#include <xlocale.h>
#include "time.h"
#include "../shim.h"
#include "locale.h"

long shim_timezone = 0;
SHIM_EXPORT(timezone);

extern long shim___timezone __attribute__((alias("shim_timezone")));
SHIM_EXPORT(__timezone);

int shim_daylight = 0;
SHIM_EXPORT(daylight);

extern int shim___daylight __attribute__((alias("shim_daylight")));
SHIM_EXPORT(__daylight);

static char shim_tzname_0[] = "GMT\0\0";
static char shim_tzname_1[] = "GMT\0\0";

char* shim_tzname[2] = {shim_tzname_0, shim_tzname_1};
SHIM_EXPORT(tzname);

extern char** shim___tzname __attribute__((alias("shim_tzname")));
SHIM_EXPORT(__tzname);

static bool find_offset(int current_year, int current_month, bool is_dst, long* offset) {

  struct tm date = {0};
  struct tm out;

  for (int year = current_year; year >= 70; year--) {
    for (int month = 11; month >= 5; month -= 6) {

      if (year == current_year && month > current_month) {
        continue;
      }

      date.tm_mday = 21;
      date.tm_mon  = month;
      date.tm_year = year;

      time_t t = timegm(&date);

      if (localtime_r(&t, &out) == NULL) {
        PANIC("localtime_r");
      }

      if (out.tm_isdst == is_dst) {
        if (offset != NULL) {
          *offset = out.tm_gmtoff;
        }
        return true;
      }
    }
  }

  return false;
}

static void shim_tzset_impl() {

  tzset();

  strlcpy(shim_tzname_0, tzname[0], sizeof(shim_tzname_0));
  strlcpy(shim_tzname_1, tzname[1], sizeof(shim_tzname_1));

  time_t t = time(NULL);

  struct tm lt;
  if (localtime_r(&t, &lt) == NULL) {
    PANIC("localtime_r");
  }

  int current_year  = lt.tm_year;
  int current_month = lt.tm_mon;

  long timezone;
  if (find_offset(current_year, current_month, 0, &timezone)) {
    timezone = -timezone;
  } else {
    timezone = 0;
  }

  *globals.timezone   = timezone;
  *globals.__timezone = timezone;

  int daylight;
  if (find_offset(current_year, current_month, 1, NULL)) {
    daylight = 1;
  } else {
    daylight = 0;
  }

  *globals.daylight   = daylight;
  *globals.__daylight = daylight;
}

SHIM_WRAP(tzset);

#ifndef CLOCK_BOOTTIME
#define CLOCK_BOOTTIME CLOCK_UPTIME
#endif

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

static size_t shim___strftime_l_impl(char* restrict buf, size_t maxsize, const char* restrict format, const linux_tm* restrict timeptr, linux_locale_t loc) {
  return strftime_l(buf, maxsize, format, timeptr, loc->native_locale);
}

static size_t shim_strftime_l_impl(char* restrict buf, size_t maxsize, const char* restrict format, const linux_tm* restrict timeptr, linux_locale_t loc) {
  return strftime_l(buf, maxsize, format, timeptr, loc->native_locale);
}

static char* shim_strptime_l_impl(const char* restrict buf, const char* restrict format, linux_tm* restrict timeptr, linux_locale_t loc) {
  return strptime_l(buf, format, timeptr, loc->native_locale);
}

SHIM_WRAP(__strftime_l);
SHIM_WRAP(strftime_l);
SHIM_WRAP(strptime_l);
