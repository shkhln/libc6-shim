#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "../shim.h"
#include "signal.h"
#include "time.h"

static int shim___libc_current_sigrtmin_impl() {
  return SIGRTMIN;
}

static int shim___libc_current_sigrtmax_impl() {
  return SIGRTMAX;
}

SHIM_WRAP(__libc_current_sigrtmin);
SHIM_WRAP(__libc_current_sigrtmax);

int linux_to_freebsd_signo(int linux_signal) {

  if ( (linux_signal >=  0 && linux_signal <  7)
    || (linux_signal >   7 && linux_signal < 10)
    ||  linux_signal == 11
    || (linux_signal >  12 && linux_signal < 17)
    || (linux_signal >  20 && linux_signal < 23)
    || (linux_signal >  23 && linux_signal < 29)
    || (linux_signal >= SIGRTMIN && linux_signal <= SIGRTMAX))
  {
    return linux_signal;
  }

  switch (linux_signal) {
    //~ case LINUX_SIGBUS:  return SIGBUS;
    //~ case LINUX_SIGUSR1: return SIGUSR1;
    //~ case LINUX_SIGUSR2: return SIGUSR2;
    //~ case LINUX_SIGCHLD: return SIGCHLD;
    //~ case LINUX_SIGCONT: return SIGCONT;
    //~ case LINUX_SIGSTOP: return SIGSTOP;
    //~ case LINUX_SIGTSTP: return SIGTSTP;
    //~ case LINUX_SIGURG:  return SIGURG;
    //~ case LINUX_SIGIO:   return SIGIO;
    //~ case LINUX_SIGSYS:  return SIGSYS;
    case LINUX_SIGPWR:  return SIGUSR1; // same number
    default:
      return -1;
  }
}

static int shim_kill_impl(pid_t pid, int linux_sig) {
  int sig = linux_to_freebsd_signo(linux_sig);
  assert(sig != -1);
  return kill(pid, sig);
}

static int shim_killpg_impl(pid_t pgrp, int sig) {
  UNIMPLEMENTED();
}

static void shim_psignal_impl(int sig, const char* s) {
  UNIMPLEMENTED();
}

static int shim_raise_impl(int linux_sig) {
  int sig = linux_to_freebsd_signo(linux_sig);
  assert(sig != -1);
  return raise(sig);
}

static sig_t shim_signal_impl(int linux_sig, sig_t func) {
  int sig = linux_to_freebsd_signo(linux_sig);
  assert(sig != -1);
  return signal(sig, func);
}

static char* shim_strsignal_impl(int sig) {
  UNIMPLEMENTED()
}

SHIM_WRAP(kill);
SHIM_WRAP(killpg);
SHIM_WRAP(psignal);
SHIM_WRAP(raise);
SHIM_WRAP(signal);
SHIM_WRAP(strsignal);

static int linux_to_freebsd_sigaction_flags(int linux_flags) {

  assert((linux_flags & ~KNOWN_LINUX_SIGACTION_FLAGS) == 0);

  int flags = 0;

  if (linux_flags & LINUX_SA_NOCLDSTOP) flags |= SA_NOCLDSTOP;
  if (linux_flags & LINUX_SA_NOCLDWAIT) flags |= SA_NOCLDWAIT;
  if (linux_flags & LINUX_SA_SIGINFO)   flags |= SA_SIGINFO;
  if (linux_flags & LINUX_SA_ONSTACK)   flags |= SA_ONSTACK;
  if (linux_flags & LINUX_SA_RESTART)   flags |= SA_RESTART;
  if (linux_flags & LINUX_SA_NODEFER)   flags |= SA_NODEFER;
  if (linux_flags & LINUX_SA_RESETHAND) flags |= SA_RESETHAND;

  return flags;
}

static int freebsd_to_linux_sigaction_flags(int flags) {

  assert((flags & ~(SA_NOCLDSTOP | SA_NOCLDWAIT | SA_SIGINFO | SA_ONSTACK | SA_RESTART | SA_NODEFER | SA_RESETHAND)) == 0);

  int linux_flags = 0;

  if (flags & SA_NOCLDSTOP) linux_flags |= LINUX_SA_NOCLDSTOP;
  if (flags & SA_NOCLDWAIT) linux_flags |= LINUX_SA_NOCLDWAIT;
  if (flags & SA_SIGINFO)   linux_flags |= LINUX_SA_SIGINFO;
  if (flags & SA_ONSTACK)   linux_flags |= LINUX_SA_ONSTACK;
  if (flags & SA_RESTART)   linux_flags |= LINUX_SA_RESTART;
  if (flags & SA_NODEFER)   linux_flags |= LINUX_SA_NODEFER;
  if (flags & SA_RESETHAND) linux_flags |= LINUX_SA_RESETHAND;

  return linux_flags;
}

// note that LINUX_SIG_DFL == SIG_DFL and LINUX_SIG_IGN == SIG_IGN
static void linux_to_native_sigaction(struct sigaction* dst, const linux_sigaction* src) {
  dst->sa_handler = src->linux_sa_handler;
  memcpy(&dst->sa_mask, &src->sa_mask, sizeof(sigset_t));
  dst->sa_flags = linux_to_freebsd_sigaction_flags(src->sa_flags);
}

static void native_to_linux_sigaction(linux_sigaction* dst, const struct sigaction* src) {
  dst->linux_sa_handler = src->sa_handler;
  memcpy(&dst->sa_mask, &src->sa_mask, sizeof(sigset_t));
  dst->sa_flags = freebsd_to_linux_sigaction_flags(src->sa_flags);
}

static int shim_sigaction_impl(int linux_sig, const linux_sigaction* linux_act, linux_sigaction* linux_oact) {

  // we are not interested in letting things like Unity to register its own segfault handlers
  if (linux_sig == LINUX_SIGABRT || linux_sig == LINUX_SIGSEGV || linux_sig == LINUX_SIGBUS || linux_sig == LINUX_SIGSYS) {
    if (linux_oact != NULL) {
      linux_oact->linux_sa_handler = LINUX_SIG_DFL;
      sigemptyset((sigset_t*)&linux_oact->sa_mask);
      linux_oact->sa_flags = 0;
    }
    return 0;
  }

  int sig = linux_to_freebsd_signo(linux_sig);
  if (sig == -1) {
    return -1;
  }

  if (linux_act != NULL) {
    struct sigaction act;
    linux_to_native_sigaction(&act, linux_act);

    if (linux_oact != NULL) {
      struct sigaction oact;
      int err = sigaction(sig, &act, &oact);
      if (err != -1) {
        native_to_linux_sigaction(linux_oact, &oact);
      }
      return err;
    } else {
      return sigaction(sig, &act, NULL);
    }
  } else {

    if (linux_oact != NULL) {
      struct sigaction oact;
      int err = sigaction(sig, NULL, &oact);
      if (err != -1) {
        native_to_linux_sigaction(linux_oact, &oact);
      }
      return err;
    } else {
      return -1;
    }
  }
}

static int shim_sigemptyset_impl(linux_sigset_t* set) {
  return sigemptyset((sigset_t*)set);
}

static int shim_sigfillset_impl(linux_sigset_t* set) {
  return sigfillset((sigset_t*)set);
}

static int shim_sigpending_impl(linux_sigset_t* set) {
  return sigpending((sigset_t*)set);
}

SHIM_WRAP(sigemptyset);
SHIM_WRAP(sigfillset);
SHIM_WRAP(sigpending);

static int shim_sigaddset_impl(linux_sigset_t* set, int linux_signo) {
  return sigaddset((sigset_t*)set, linux_to_freebsd_signo(linux_signo));
}

static int shim_sigdelset_impl(linux_sigset_t* set, int linux_signo) {
  return sigdelset((sigset_t*)set, linux_to_freebsd_signo(linux_signo));
}

static int shim_siginterrupt_impl(int sig, int flag) {
  UNIMPLEMENTED();
}

static int shim_sigprocmask_impl(int linux_how, const linux_sigset_t* restrict set, linux_sigset_t* restrict oset) {
  switch (linux_how) {
    case LINUX_SIG_BLOCK:   return sigprocmask(SIG_BLOCK,   (sigset_t*)set, (sigset_t*)oset);
    case LINUX_SIG_UNBLOCK: return sigprocmask(SIG_UNBLOCK, (sigset_t*)set, (sigset_t*)oset);
    case LINUX_SIG_SETMASK: return sigprocmask(SIG_SETMASK, (sigset_t*)set, (sigset_t*)oset);
    default:
      UNIMPLEMENTED_ARGS("%d", linux_how);
  }
}

static int shim_sigismember_impl(const linux_sigset_t* set, int linux_signo) {
  return sigismember((sigset_t*)set, linux_to_freebsd_signo(linux_signo));
}

static int shim_sigqueue_impl(pid_t pid, int signo, const union sigval value) {
  UNIMPLEMENTED();
}

static int shim_sigsuspend_impl(const linux_sigset_t* sigmask) {
  return sigsuspend((sigset_t*)sigmask);
}

//TODO: linux_siginfo_t?
static int shim_sigtimedwait_impl(const linux_sigset_t* restrict set, siginfo_t* restrict info, const linux_timespec* restrict timeout) {
  UNIMPLEMENTED();
}

static int shim_sigwait_impl(const linux_sigset_t* restrict set, int* restrict linux_sig) {
  UNIMPLEMENTED();
}

//TODO: linux_siginfo_t?
static int shim_sigwaitinfo_impl(const linux_sigset_t* restrict set, siginfo_t* restrict info) {
  UNIMPLEMENTED();
}

SHIM_WRAP(sigaction);
SHIM_WRAP(sigaddset);
SHIM_WRAP(sigdelset);
SHIM_WRAP(siginterrupt);
SHIM_WRAP(sigismember);
SHIM_WRAP(sigprocmask);
SHIM_WRAP(sigqueue);
SHIM_WRAP(sigsuspend);
SHIM_WRAP(sigtimedwait);
SHIM_WRAP(sigwait);
SHIM_WRAP(sigwaitinfo);

static int linux_to_freebsd_sigstack_flags(int linux_flags) {

  assert((linux_flags & ~KNOWN_LINUX_SIGSTACK_FLAGS) == 0);

  int flags = 0;

  if (linux_flags & LINUX_SS_ONSTACK) flags |= SS_ONSTACK;
  if (linux_flags & LINUX_SS_DISABLE) flags |= SS_DISABLE;

  return flags;
}

static int shim_sigaltstack_impl(const linux_stack_t* restrict linux_ss, linux_stack_t* restrict linux_oss) {

  assert(linux_ss  != NULL);
  assert(linux_oss == NULL);

  LOG("%s: ss_sp = %p, ss_flags = %#x, ss_size = %#zx", __func__, linux_ss->ss_sp, linux_ss->ss_flags, linux_ss->ss_size);

  stack_t ss = {
    .ss_sp    = linux_ss->ss_sp,
    .ss_flags = linux_to_freebsd_sigstack_flags(linux_ss->ss_flags),
    .ss_size  = linux_ss->ss_size
  };

  return sigaltstack(&ss, NULL);
}

SHIM_WRAP(sigaltstack);
