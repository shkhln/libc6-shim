#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "shim.h"

FILE* shim_stdin  = NULL;
FILE* shim_stdout = NULL;
FILE* shim_stderr = NULL;

SYM_EXPORT(shim_stdin,  stdin);
SYM_EXPORT(shim_stdout, stdout);
SYM_EXPORT(shim_stderr, stderr);

SYM_EXPORT(shim_stdout, _IO_2_1_stdout_);

__attribute__((constructor))
void shim_init() {

  fprintf(stderr, "shim init\n");

  shim_stdin  = stdin;
  shim_stdout = stdout;
  shim_stderr = stderr;
}

char* shim_gnu_get_libc_version() {
  LOG("%s()\n", __func__);
  return "2.17"; //"2.5.5";
}

SYM_EXPORT(shim_gnu_get_libc_version, gnu_get_libc_version);

void shim_cxa_finalize(void* d) {
  LOG_ARGS("%p", d);
}

int shim_cxa_atexit(void (*cb)(void*), void* arg, void* dso_handle) {
  LOG_ARGS("%p, %p, %p", cb, arg, dso_handle);
  return 0;
}

SYM_EXPORT(shim_cxa_finalize, __cxa_finalize);
SYM_EXPORT(shim_cxa_atexit,   __cxa_atexit);

/*#include <link.h>

int shim_dl_iterate_phdr_impl(int (*callback)(struct dl_phdr_info*, size_t, void*), void* data) {
  UNIMPLEMENTED();
}*/
