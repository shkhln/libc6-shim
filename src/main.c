#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <link.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/elf.h>

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

static int    shim_argc = 0;
static char** shim_argv = NULL;

__attribute__((constructor(101)))
static void shim_init(int argc, char** argv, char** env) {

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

__attribute__((constructor(102)))
static void shim_libgl_init(int argc, char** argv, char** env) {

  void* libgl = dlopen("libGL.so.1", RTLD_LAZY);
  assert(libgl != NULL);

  Link_map* map = NULL;

  int err = dlinfo(libgl, RTLD_DI_LINKMAP, &map);
  assert(err == 0);

  while (map != NULL) {

    int dt_fini_count = 0;
    for (const Elf_Dyn* dyn = map->l_ld; dyn->d_tag != DT_NULL; dyn++) {
      if (dyn->d_tag == DT_FINI) {
        dt_fini_count++;
      }
    }

    if (dt_fini_count == 2) {

      for (const Elf_Dyn* dyn = map->l_ld; dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_FINI) {

          LOG("%s: calling init function for %s\n", __func__, map->l_name);

          void (*init)(int, char**, char**) = (void*)(map->l_addr + dyn->d_un.d_ptr);
          init(argc, argv, env);

          break;
        }
      }
    }

    map = map->l_next;
  }
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

int shim___register_atfork_impl(void (*prepare)(void), void (*parent)(void), void (*child)(void), void* dso_handle) {
  return pthread_atfork(prepare, parent, child);
}
