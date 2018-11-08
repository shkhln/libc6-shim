#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include "../shim.h"

__asm__(".symver shim_signal,signal@GLIBC_2.0");
__asm__(".symver shim_signal,signal@GLIBC_2.2.5");
sig_t shim_signal(int sig, sig_t func) {
  return 0;
}

int shim___libc_current_sigrtmin_impl() {
  UNIMPLEMENTED();
}

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

int shim_kill_impl(pid_t pid, int sig) {
  UNIMPLEMENTED();
}

int shim_killpg_impl(pid_t pgrp, int sig) {
  UNIMPLEMENTED();
}

void shim_psignal(int sig, const char* s) {
  UNIMPLEMENTED();
}

int shim_raise_impl(int sig) {
  UNIMPLEMENTED();
}

char* shim_strsignal_impl(int sig) {
  UNIMPLEMENTED()
}
