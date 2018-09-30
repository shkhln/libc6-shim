#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "shim.h"

__asm__(".symver shim_stdin,stdin@GLIBC_2.0");
__asm__(".symver shim_stdin,stdin@GLIBC_2.2.5");
__asm__(".symver shim_stdin,_IO_stdin_@GLIBC_2.0");
__asm__(".symver shim_stdin,_IO_2_1_stdin_@GLIBC_2.1");
__asm__(".symver shim_stdin,_IO_2_1_stdin_@GLIBC_2.2.5");
FILE* shim_stdin  = NULL;

__asm__(".symver shim_stdout,stdout@GLIBC_2.0");
__asm__(".symver shim_stdout,stdout@GLIBC_2.2.5");
__asm__(".symver shim_stdout,_IO_stdout_@GLIBC_2.0");
__asm__(".symver shim_stdout,_IO_2_1_stdout_@GLIBC_2.1");
__asm__(".symver shim_stdout,_IO_2_1_stdout_@GLIBC_2.2.5");
FILE* shim_stdout = NULL;

__asm__(".symver shim_stderr,stderr@GLIBC_2.0");
__asm__(".symver shim_stderr,stderr@GLIBC_2.2.5");
__asm__(".symver shim_stderr,_IO_stderr_@GLIBC_2.0");
__asm__(".symver shim_stderr,_IO_2_1_stderr_@GLIBC_2.1");
__asm__(".symver shim_stderr,_IO_2_1_stderr_@GLIBC_2.2.5");
FILE* shim_stderr = NULL;

__attribute__((constructor))
void shim_init() {

  fprintf(stderr, "shim init\n");

  shim_stdin  = stdin;
  shim_stdout = stdout;
  shim_stderr = stderr;
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

/*#include <link.h>

int shim_dl_iterate_phdr_impl(int (*callback)(struct dl_phdr_info*, size_t, void*), void* data) {
  UNIMPLEMENTED();
}*/
