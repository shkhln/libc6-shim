#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include "../shim.h"

int shim___libc_current_sigrtmin_impl() {
  UNIMPLEMENTED();
}

SHIM_WRAP(__libc_current_sigrtmin);

int shim_kill_impl(pid_t pid, int sig) {
  if (sig == 0) {
    return kill(pid, sig);
  } else {
    UNIMPLEMENTED_ARGS("%d, %d", pid, sig);
  }
}

int shim_killpg_impl(pid_t pgrp, int sig) {
  UNIMPLEMENTED();
}

void shim_psignal_impl(int sig, const char* s) {
  UNIMPLEMENTED();
}

int shim_raise_impl(int sig) {
  UNIMPLEMENTED();
}

sig_t shim_signal_impl(int sig, sig_t func) {
  return NULL;
}

char* shim_strsignal_impl(int sig) {
  UNIMPLEMENTED()
}

SHIM_WRAP(kill);
SHIM_WRAP(killpg);
SHIM_WRAP(psignal);
SHIM_WRAP(raise);
SHIM_WRAP(signal);
SHIM_WRAP(strsignal);

int shim_sigaction_impl(int sig, const struct sigaction* act, const struct sigaction* oact) {
  UNIMPLEMENTED();
}

int shim_sigaddset_impl(sigset_t* set, int signo) {
  UNIMPLEMENTED();
}

int shim_sigdelset_impl(sigset_t* set, int signo) {
  UNIMPLEMENTED();
}

int shim_siginterrupt_impl(int sig, int flag) {
  UNIMPLEMENTED();
}

int shim_sigismember_impl(const sigset_t* set, int signo) {
  UNIMPLEMENTED();
}

int shim_sigqueue_impl(pid_t pid, int signo, const union sigval value) {
  UNIMPLEMENTED();
}

int shim_sigwait_impl(const sigset_t* restrict set, int* restrict sig) {
  UNIMPLEMENTED();
}

typedef void linux_sigaction;

SHIM_WRAP(sigaction);
SHIM_WRAP(sigaddset);
SHIM_WRAP(sigdelset);
SHIM_WRAP(siginterrupt);
SHIM_WRAP(sigismember);
SHIM_WRAP(sigqueue);
SHIM_WRAP(sigwait);
