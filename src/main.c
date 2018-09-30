#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shim.h"

__asm__(".symver shim_stdin,stdin@GLIBC_2.0");
__asm__(".symver shim_stdin,stdin@GLIBC_2.2.5");
__asm__(".symver shim_stdin,_IO_stdin_@GLIBC_2.0");
__asm__(".symver shim_stdin,_IO_2_1_stdin_@GLIBC_2.1");
__asm__(".symver shim_stdin,_IO_2_1_stdin_@GLIBC_2.2.5");

__asm__(".symver shim_stdout,stdout@GLIBC_2.0");
__asm__(".symver shim_stdout,stdout@GLIBC_2.2.5");
__asm__(".symver shim_stdout,_IO_stdout_@GLIBC_2.0");
__asm__(".symver shim_stdout,_IO_2_1_stdout_@GLIBC_2.1");
__asm__(".symver shim_stdout,_IO_2_1_stdout_@GLIBC_2.2.5");

__asm__(".symver shim_stderr,stderr@GLIBC_2.0");
__asm__(".symver shim_stderr,stderr@GLIBC_2.2.5");
__asm__(".symver shim_stderr,_IO_stderr_@GLIBC_2.0");
__asm__(".symver shim_stderr,_IO_2_1_stderr_@GLIBC_2.1");
__asm__(".symver shim_stderr,_IO_2_1_stderr_@GLIBC_2.2.5");

static FILE std[3] = {};

FILE* shim_stdin  = &std[0];
FILE* shim_stdout = &std[1];
FILE* shim_stderr = &std[2];

__asm__(".symver shim_env,environ@GLIBC_2.0");
__asm__(".symver shim_env,environ@GLIBC_2.2.5");
__asm__(".symver shim_env,_environ@GLIBC_2.0");
__asm__(".symver shim_env,_environ@GLIBC_2.2.5");
__asm__(".symver shim_env,__environ@GLIBC_2.0");
__asm__(".symver shim_env,__environ@GLIBC_2.2.5");

#define MAX_SHIM_ENV_ENTRIES 100

static char* _shim_env[MAX_SHIM_ENV_ENTRIES];
char** shim_env = _shim_env;

__asm__(".symver shim_progname,__progname@GLIBC_2.0");
__asm__(".symver shim_progname,__progname@GLIBC_2.2.5");
char* shim_progname = "<progname>";

int    shim_argc = 0;
char** shim_argv = NULL;

__attribute__((constructor))
void shim_init(int argc, char** argv, char** env) {

  fprintf(stderr, "shim init\n");

  memcpy(shim_stdin,  stdin,  sizeof(FILE));
  memcpy(shim_stdout, stdout, sizeof(FILE));
  memcpy(shim_stderr, stderr, sizeof(FILE));

  for (int i = 0;; i++) {
    if (env[i] == NULL) {
      assert(i < MAX_SHIM_ENV_ENTRIES);
      memcpy(shim_env, env, sizeof(char*) * (i + 1));
      break;
    }
  }

  shim_argc = argc;
  shim_argv = argv;
}

__asm__(".symver shim_libc_start_main,__libc_start_main@GLIBC_2.0");
__asm__(".symver shim_libc_start_main,__libc_start_main@GLIBC_2.2.5");
int shim_libc_start_main(
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
    LOG("%s: init\n", __func__);
    init(shim_argc, shim_argv, shim_env);
  }

  LOG("%s: main\n", __func__);
  exit(main(shim_argc, shim_argv, shim_env));
}

char* shim_gnu_get_libc_version_impl() {
  return "2.17";
}

void shim___cxa_finalize_impl(void* d) {
  // do nothing
}

int shim___cxa_atexit_impl(void (*cb)(void*), void* arg, void* dso_handle) {
  return 0;
}

void shim___stack_chk_fail_impl() {
  assert(0);
}

/*#include <link.h>

int shim_dl_iterate_phdr_impl(int (*callback)(struct dl_phdr_info*, size_t, void*), void* data) {
  UNIMPLEMENTED();
}*/
