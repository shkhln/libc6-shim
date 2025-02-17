#include <signal.h>

#define LINUX_SIGBUS   7
#define LINUX_SIGUSR1 10
#define LINUX_SIGSEGV 11
#define LINUX_SIGUSR2 12
#define LINUX_SIGCHLD 17
#define LINUX_SIGCONT 18
#define LINUX_SIGSTOP 19
#define LINUX_SIGTSTP 20
#define LINUX_SIGURG  23
#define LINUX_SIGXCPU 24
#define LINUX_SIGIO   29
#define LINUX_SIGPWR  30
#define LINUX_SIGSYS  31

#define LINUX_SIG_DFL SIG_DFL
#define LINUX_SIG_IGN SIG_IGN

typedef struct { uint64_t whatever[16]; } linux_sigset_t;
_Static_assert(sizeof(sigset_t) <= sizeof(linux_sigset_t), "");

struct linux_sigaction {
  void* linux_sa_handler;
  linux_sigset_t sa_mask;
  int sa_flags;
};

typedef struct linux_sigaction linux_sigaction;

#define LINUX_SA_NOCLDSTOP 0x00000001
#define LINUX_SA_NOCLDWAIT 0x00000002
#define LINUX_SA_SIGINFO   0x00000004
#define LINUX_SA_ONSTACK   0x08000000
#define LINUX_SA_RESTART   0x10000000
#define LINUX_SA_NODEFER   0x40000000
#define LINUX_SA_RESETHAND 0x80000000

#define KNOWN_LINUX_SIGACTION_FLAGS ( \
  LINUX_SA_NOCLDSTOP |                \
  LINUX_SA_NOCLDWAIT |                \
  LINUX_SA_SIGINFO   |                \
  LINUX_SA_ONSTACK   |                \
  LINUX_SA_RESTART   |                \
  LINUX_SA_NODEFER   |                \
  LINUX_SA_RESETHAND                  \
)

int linux_to_freebsd_signo(int linux_signal);

#define LINUX_SIG_BLOCK   0
#define LINUX_SIG_UNBLOCK 1
#define LINUX_SIG_SETMASK 2

#define LINUX_SS_ONSTACK 1
#define LINUX_SS_DISABLE 2

#define KNOWN_LINUX_SIGSTACK_FLAGS (LINUX_SS_ONSTACK | LINUX_SS_DISABLE)

typedef void linux_stack_t;
