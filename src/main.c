#include <assert.h>
#include <dlfcn.h>
#include <iconv.h>
#include <libgen.h>
#include <limits.h>
#include <link.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/elf.h>
#include <sys/epoll.h>

#include "shim.h"

// necessary for the rtld's direct execution mode
char** environ    = NULL;
char*  __progname = "<progname>";

struct globals globals;

static char** shim_env  = NULL;
static int    shim_argc = 0;
static char** shim_argv = NULL;

// we don't route fread/fwrite through libepoll-shim, should we?
int     (*libepoll_epoll_create)    (int);
int     (*libepoll_epoll_create1)   (int);
int     (*libepoll_epoll_ctl)       (int, int, int, struct epoll_event*);
int     (*libepoll_epoll_wait)      (int, struct epoll_event*, int, int);
int     (*libepoll_epoll_pwait)     (int, struct epoll_event*, int, int, const sigset_t*);
ssize_t (*libepoll_epoll_shim_read) (int, void*, size_t);
ssize_t (*libepoll_epoll_shim_write)(int, const void*, size_t);
int     (*libepoll_epoll_shim_close)(int);
int     (*libepoll_epoll_shim_poll) (struct pollfd[], nfds_t, int);
int     (*libepoll_epoll_shim_ppoll)(struct pollfd[], nfds_t, const struct timespec* restrict, const sigset_t* restrict);
int     (*libepoll_epoll_shim_fcntl)(int, int, ...);

static int dummy() {
  return -1;
}

__attribute__((constructor(101)))
static void shim_init(int argc, char** argv, char** env) {

  Link_map* link_map = NULL;

  int err = dlinfo(RTLD_SELF, RTLD_DI_LINKMAP, &link_map);
  assert(err == 0);

  fprintf(stderr, "%s: shim init\n", link_map->l_name);

  char* short_program_name = basename(argv[0]);

#define G(var, version, value)\
   globals.var = dlvsym(RTLD_DEFAULT, #var, version); assert(globals.var != NULL); \
  *globals.var = value;

#ifdef __amd64__
  G(_IO_2_1_stderr_,               "GLIBC_2.2.5", stderr);
  G(_IO_2_1_stdin_,                "GLIBC_2.2.5", stdin);
  G(_IO_2_1_stdout_,               "GLIBC_2.2.5", stdout);
  G(__environ,                     "GLIBC_2.2.5", env);
  G(_environ,                      "GLIBC_2.2.5", env);
  G(environ,                       "GLIBC_2.2.5", env);
  G(stderr,                        "GLIBC_2.2.5", stderr);
  G(stdin,                         "GLIBC_2.2.5", stdin);
  G(stdout,                        "GLIBC_2.2.5", stdout);
  G(optarg,                        "GLIBC_2.2.5", NULL);
  G(opterr,                        "GLIBC_2.2.5", 1);
  G(optind,                        "GLIBC_2.2.5", 1);
  G(optopt,                        "GLIBC_2.2.5", 0);
  G(__progname,                    "GLIBC_2.2.5", short_program_name);
  G(__progname_full,               "GLIBC_2.2.5", argv[0]);
  G(program_invocation_name,       "GLIBC_2.2.5", argv[0]);
  G(program_invocation_short_name, "GLIBC_2.2.5", short_program_name);
#endif

#ifdef __i386__
  G(_IO_stderr_,                   "GLIBC_2.0", stderr);
  G(_IO_stdin_,                    "GLIBC_2.0", stdin);
  G(_IO_stdout_,                   "GLIBC_2.0", stdout);
  G(_IO_2_1_stderr_,               "GLIBC_2.1", stderr);
  G(_IO_2_1_stdin_,                "GLIBC_2.1", stdin);
  G(_IO_2_1_stdout_,               "GLIBC_2.1", stdout);
  G(__environ,                     "GLIBC_2.0", env);
  G(_environ,                      "GLIBC_2.0", env);
  G(environ,                       "GLIBC_2.0", env);
  G(stderr,                        "GLIBC_2.0", stderr);
  G(stdin,                         "GLIBC_2.0", stdin);
  G(stdout,                        "GLIBC_2.0", stdout);
  G(optarg,                        "GLIBC_2.0", NULL);
  G(opterr,                        "GLIBC_2.0", 1);
  G(optind,                        "GLIBC_2.0", 1);
  G(optopt,                        "GLIBC_2.0", 0);
  G(__progname,                    "GLIBC_2.0", short_program_name);
  G(__progname_full,               "GLIBC_2.0", argv[0]);
  G(program_invocation_name,       "GLIBC_2.0", argv[0]);
  G(program_invocation_short_name, "GLIBC_2.0", short_program_name);
#endif

#undef G

  shim_env  = env;
  shim_argc = argc;
  shim_argv = argv;

  void* libepoll = dlopen("libepoll-shim.so.0", RTLD_LAZY);
  if (libepoll != NULL) {
#define E(sym) libepoll_##sym = dlsym(libepoll, #sym); assert(libepoll_##sym != NULL);
    E(epoll_create);
    E(epoll_create1);
    E(epoll_ctl);
    E(epoll_wait);
    E(epoll_pwait);
    E(epoll_shim_read);
    E(epoll_shim_write);
    E(epoll_shim_close);
    E(epoll_shim_poll);
    E(epoll_shim_ppoll);
    E(epoll_shim_fcntl);
#undef E
  } else {
    fprintf(stderr, "%s: unable to load libepoll-shim.so.0 (%s)\n", link_map->l_name, dlerror());
    libepoll_epoll_create     = dummy;
    libepoll_epoll_create1    = dummy;
    libepoll_epoll_ctl        = dummy;
    libepoll_epoll_wait       = dummy;
    libepoll_epoll_pwait      = dummy;
    libepoll_epoll_shim_read  = read;
    libepoll_epoll_shim_write = write;
    libepoll_epoll_shim_close = close;
    libepoll_epoll_shim_poll  = poll;
    libepoll_epoll_shim_ppoll = ppoll;
    libepoll_epoll_shim_fcntl = fcntl;
  }
}

extern int __cxa_atexit(void (*)(void*), void*, void*);

#ifdef __i386__
struct wrapper_args {
  void (*cb)(void*);
  void* arg;
};

// helps with stack alignment crashes in steamclient.so
static void __cxa_atexit_cb_wrapper(void* arg) {
  LOG_ENTRY("%p", arg);
  struct wrapper_args* wargs = (struct wrapper_args*)arg;
  LOG("cb = %p, arg = %p", wargs->cb, wargs->arg);
  wargs->cb(wargs->arg);
  free(wargs);
  LOG_EXIT();
}

int shim___cxa_atexit_impl(void (*cb)(void*), void* arg, void* dso) {
  struct wrapper_args* wargs = malloc(sizeof(struct wrapper_args));
  wargs->cb  = cb;
  wargs->arg = arg;
  return __cxa_atexit(__cxa_atexit_cb_wrapper, wargs, dso);
}
#else
int shim___cxa_atexit_impl(void (*cb)(void*), void* arg, void* dso) {
  return __cxa_atexit(cb, arg, dso);
}
#endif

extern void __cxa_finalize(void*);

void shim___cxa_finalize_impl(void* dso) {
  __cxa_finalize(dso);
}

int shim___libc_start_main_impl(
  int (*main)(int, char**, char**),
  int argc,
  char** ubp_av,
  void (*init)(int, char**, char**),
  void (*fini)(void),
  void (*rtld_fini)(void),
  void* stack_end
) {

  // _init_tls?

  if (fini != NULL) {
    //shim___cxa_atexit_impl(fini, NULL, NULL);
    atexit(fini);
  }

  if (init != NULL) {
    LOG("%s: init", __func__);
    init(shim_argc, shim_argv, shim_env);
  }

  LOG("%s: main", __func__);
  exit(main(shim_argc, shim_argv, shim_env));
}

void shim___stack_chk_fail_impl() {
  assert(0);
}

int shim___register_atfork_impl(void (*prepare)(void), void (*parent)(void), void (*child)(void), void* dso_handle) {
  return pthread_atfork(prepare, parent, child);
}

char* shim_gnu_get_libc_version_impl() {
  return "2.17";
}

SHIM_WRAP(__cxa_atexit);
SHIM_WRAP(__cxa_finalize);
SHIM_WRAP(__libc_start_main);
SHIM_WRAP(__stack_chk_fail);
SHIM_WRAP(__register_atfork);
SHIM_WRAP(gnu_get_libc_version);

extern void* __tls_get_addr(void*);

__attribute__((__regparm__(1)))
void* shim___tls_get_addr(void* ti) {
  return __tls_get_addr(ti);
}

extern __typeof(shim___tls_get_addr) shim____tls_get_addr __attribute__((alias("shim___tls_get_addr")));

__asm__(".symver shim___tls_get_addr,__tls_get_addr@GLIBC_2.3");
__asm__(".symver shim____tls_get_addr,___tls_get_addr@GLIBC_2.3");

#define LINUX_AT_SYSINFO_EHDR 33

unsigned long shim_getauxval_impl(unsigned long type) {
  assert(type == LINUX_AT_SYSINFO_EHDR);
  return 0;
}

SHIM_WRAP(getauxval);
