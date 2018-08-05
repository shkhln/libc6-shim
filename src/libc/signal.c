#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include "../shim.h"

int shim_sigaction(int sig, const struct sigaction* act, const struct sigaction* oact) {
  UNIMPLEMENTED();
}

char* shim_strsignal_impl(int sig) {
  UNIMPLEMENTED()
}

int shim_kill_impl(pid_t pid, int sig) {
  UNIMPLEMENTED();
}

int shim_raise_impl(int sig) {
  UNIMPLEMENTED();
}

SYM_EXPORT(shim_sigaction, sigaction);

int shim_libc_current_sigrtmin() {
  UNIMPLEMENTED();
}

SYM_EXPORT(shim_libc_current_sigrtmin, __libc_current_sigrtmin);
