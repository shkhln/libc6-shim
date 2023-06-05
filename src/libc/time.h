#pragma once

#include <time.h>

#define LINUX_CLOCK_REALTIME         0
#define LINUX_CLOCK_MONOTONIC        1
#define LINUX_CLOCK_MONOTONIC_RAW    4
#define LINUX_CLOCK_REALTIME_COARSE  5
#define LINUX_CLOCK_MONOTONIC_COARSE 6
#define LINUX_CLOCK_BOOTTIME         7

typedef clockid_t linux_clockid_t;

typedef struct timespec linux_timespec;
typedef struct timeval  linux_timeval;
typedef struct timezone linux_timezone;
typedef struct tm       linux_tm;

int shim_clock_gettime_impl(linux_clockid_t clock_id, linux_timespec* tp);
