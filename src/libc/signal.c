#include <assert.h>
#include <errno.h>
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
  errno = EINVAL;
  return -1;
}

#define LINUX_SIGBUS   7
#define LINUX_SIGUSR1 10
#define LINUX_SIGUSR2 12
#define LINUX_SIGCHLD 17
#define LINUX_SIGCONT 18
#define LINUX_SIGSTOP 19
#define LINUX_SIGTSTP 20
#define LINUX_SIGURG  23
#define LINUX_SIGIO   29
#define LINUX_SIGSYS  31

int linux_to_freebsd_signo(int linux_signal) {

  if (linux_signal < 7 || linux_signal == 11
    || (linux_signal >  7 && linux_signal < 10)
    || (linux_signal > 12 && linux_signal < 17)
    || (linux_signal > 20 && linux_signal < 23)
    || (linux_signal > 23 && linux_signal < 29))
  {
    return linux_signal;
  }

  switch (linux_signal) {
    case LINUX_SIGBUS:  return SIGBUS;
    case LINUX_SIGUSR1: return SIGUSR1;
    case LINUX_SIGUSR2: return SIGUSR2;
    case LINUX_SIGCHLD: return SIGCHLD;
    case LINUX_SIGCONT: return SIGCONT;
    case LINUX_SIGSTOP: return SIGSTOP;
    case LINUX_SIGTSTP: return SIGTSTP;
    case LINUX_SIGURG:  return SIGURG;
    case LINUX_SIGIO:   return SIGIO;
    case LINUX_SIGSYS:  return SIGSYS;
    default:
      return -1;
  }
}

int shim_sigaddset_impl(sigset_t* set, int linux_signo) {
  return sigaddset(set, linux_to_freebsd_signo(linux_signo));
}

int shim_sigdelset_impl(sigset_t* set, int linux_signo) {
  return sigdelset(set, linux_to_freebsd_signo(linux_signo));
}

int shim_siginterrupt_impl(int sig, int flag) {
  UNIMPLEMENTED();
}

int shim_sigismember_impl(const sigset_t* set, int linux_signo) {
  return sigismember(set, linux_to_freebsd_signo(linux_signo));
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
