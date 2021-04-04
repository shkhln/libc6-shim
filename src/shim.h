#pragma once

#include <assert.h>
#include <execinfo.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define __HEAD(head, ...) head
#define __TAIL(head, ...) __VA_ARGS__

#ifdef DEBUG

#include <errno.h>
#include <pthread_np.h>

extern __thread int what_was_that_error;

#define LOG(...) (\
 what_was_that_error = errno,\
 fprintf(stderr, "[%d:%d] " __HEAD(__VA_ARGS__) "\n", getpid(), pthread_getthreadid_np(), __TAIL(__VA_ARGS__)),\
 errno = what_was_that_error\
)

#define LOG_ENTRY(fmt, ...) __builtin_choose_expr(__builtin_strcmp("" fmt, "") == 0, LOG("%s()",       __func__), LOG("%s("    fmt ")", __func__, ## __VA_ARGS__))
#define LOG_EXIT( fmt, ...) __builtin_choose_expr(__builtin_strcmp("" fmt, "") == 0, LOG("%s -> void", __func__), LOG("%s -> " fmt,     __func__, ## __VA_ARGS__))

#else

#define LOG(...)
#define LOG_ENTRY(fmt, ...)
#define LOG_EXIT( fmt, ...)

#endif

#define UNIMPLEMENTED()         {\
  fprintf(stderr, "%s is not implemented\n", __func__);\
  void* buffer[100];\
  int nframes = backtrace(buffer, 100);\
  backtrace_symbols_fd(buffer, nframes, STDERR_FILENO);\
  assert(0);\
}

#define UNIMPLEMENTED_ARGS(...) {\
  fprintf(stderr, "%s(" __HEAD(__VA_ARGS__) ") is not implemented\n", __func__, __TAIL(__VA_ARGS__));\
  void* buffer[100];\
  int nframes = backtrace(buffer, 100);\
  backtrace_symbols_fd(buffer, nframes, STDERR_FILENO);\
  assert(0);\
}

#ifndef SHIM_EXPORT
#define SHIM_EXPORT(sym) SHIM_EXPORT_ ##sym
#endif

#ifndef SHIM_WRAP
#define SHIM_WRAP(fun, ...) SHIM_WRAPPER_ ##fun
#endif

bool str_starts_with(const char* str, const char* substr);

int native_to_linux_errno(int error);
int linux_to_native_errno(int error);

const char* redirect(const char* path);

struct globals {
#ifdef __i386__
  FILE**  _IO_stderr_;
  FILE**  _IO_stdin_;
  FILE**  _IO_stdout_;
#endif
  FILE**  _IO_2_1_stderr_;
  FILE**  _IO_2_1_stdin_;
  FILE**  _IO_2_1_stdout_;
  char*** __environ;
  char*** _environ;
  char*** environ;
  FILE**  stdin;
  FILE**  stderr;
  FILE**  stdout;
  char**  optarg;
  int*    opterr;
  int*    optind;
  int*    optopt;
  char**  __progname;
  char**  __progname_full;
  char**  program_invocation_name;
  char**  program_invocation_short_name;
};

extern struct globals globals;

typedef int64_t linux_off64_t;

#ifdef __i386__
typedef int32_t linux_off_t;
#endif

#ifdef __x86_64__
typedef int64_t linux_off_t;
#endif

#include <getopt.h>

typedef struct option linux_option;

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

typedef struct rusage linux_rusage;

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

typedef struct sembuf linux_sembuf;

#include <rpc/rpc.h>

typedef struct pollfd linux_pollfd;

struct linux_sysinfo;

typedef struct linux_sysinfo linux_sysinfo;

#include <link.h>

typedef struct dl_phdr_info linux_dl_phdr_info;

_Static_assert(sizeof(fpos_t) <= 12 /* sizeof(fpos_t) on glibc/Linux with i386 */, "");

typedef fpos_t linux_fpos_t;
typedef fpos_t linux_fpos64_t;
